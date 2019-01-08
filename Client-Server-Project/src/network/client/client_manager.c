/*
 * Settings.c
 *
 *  Created on: Oct 30, 2017
 *      Author: Andrea Graziani
 */

/* Including project header files */
/* ============================================================================ */
#include "../../user_interface/interface_functions.h"
#include "../../project_property.h"
#include "../../file/directory_manager.h"
#include "../data_transmission.h"

/* Including libraries */
/* ============================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#ifdef TEST
#include <assert.h>
#endif

/*
 * This function is used to retrieve necessary data for trasmission.
 *
 * @param: *host_ip -> A string containing host IP address.
 * @return: A pointer to a 'DataNetwork' struct.
 */
DataNetwork *client_initialization_for_transmission(char *host_ip) {

	DataNetwork *DataNetwork_obj;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int socket_fd;
	char *PORT = convert_int_to_string(SERVER_COMMAND_PORT);

	/* Initialization */
	/* ============================================================================ */
	DataNetwork_obj = calloc(1, sizeof(DataNetwork));
	if (DataNetwork_obj == NULL)
		exit_failure("calloc");

	/* Clear structure */
	memset(&hints, 0, sizeof(struct addrinfo));

	/* Setting... */
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	/* Translate name of a service location and/or a service name to set of
	 * socket addresses. */
	/* ============================================================================ */
	if (getaddrinfo(host_ip, PORT, &hints, &result) != 0) {

#ifdef TEST
		fprintf(stderr, "Invalid input or host unreachable!");
		return NULL;
#endif
		print_formatted_message(stderr, ERR_MSG, "Invalid input or host unreachable!", 0);
		return NULL;
	}

	/* getaddrinfo() returns a list of address structures. Try each address
	 * until we successfully connect.
	 * If socket (or connect) fails, we close the socket and try the next address. */
	/* ============================================================================ */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (socket_fd == -1)
			continue;
		else {

			/* Preparing 'DataNetwork' object */
			/* ============================================================================ */
			DataNetwork_obj->socket_fd = socket_fd;
			memcpy(&DataNetwork_obj->address, rp->ai_addr, sizeof(struct sockaddr));
			DataNetwork_obj->address_len = sizeof(struct sockaddr);

#ifdef TEST
			fprintf(stderr, "socket fd: %d\n", DataNetwork_obj->socket_fd);
			print_internet_socket_address_info(&DataNetwork_obj->address);
#endif
			return DataNetwork_obj;
		}
	}

	/* Free memory*/
	free(PORT);

	return NULL;
}

/*
 * This function is used to perform 'LIST' command
 */
void __client_list_command(DataNetwork *DataNetwork_obj, Settings *Settings_obj) {

	char check_replay = NO_CHECK_FIRST_REPLAY;

	/* Temporary file */
	FILE *temp_file = tmpfile();
	/* Temporary file 'file descriptor'*/
	int temp_file_fd = fileno(temp_file);

	/* Receiving data; All received packed are will be written into specified fd */
	if (receiving_data_selective_repeat(DataNetwork_obj, Settings_obj, temp_file_fd, &check_replay) != 1) {

		/* Restoring read/write file offset */
		if (lseek(temp_file_fd, 0, SEEK_SET) == -1)
			exit_failure("lseek");

		/* Deserialize data */
		FilePropertyList *FilePropertyList_obj = FilePropertyList_deserializer(temp_file_fd);

		/* Print output */
		print_file_list(&FilePropertyList_obj);

		/* Free useless memory */
		free(FilePropertyList_obj);
	}

	/* Close file descriptor */
	if (fclose(temp_file) == -1)
		exit_failure("close");
}

void __client_get_command(DataNetwork *DataNetwork_obj, Settings *Settings_obj, char* file_name) {

	char check_replay = NO_CHECK_FIRST_REPLAY;

	/* Generating necessary directories */
	/* ****************************************************************************** */

	/* Client's directory where save requested file */
	char *dir_file = get_application_directory(APPLICATION_CLIENT_FILE_FORLDER);
	/* Generate full path of requested file */
	char *file_dir = concatenate_strings(3, dir_file, "/", file_name);

	/* Preparing a temporary file (used to store request result) */
	/* ****************************************************************************** */
	FILE *temp_file = tmpfile();
	int temp_file_fd = fileno(temp_file);

	/* Receiving request result by server */
	/* ****************************************************************************** */
	if (receiving_data_selective_repeat(DataNetwork_obj, Settings_obj, temp_file_fd, &check_replay) == TRANSMISSION_SUCCESSFUL) {

		int response;

		/* Restoring read/write file offset */
		if (lseek(temp_file_fd, 0, SEEK_SET) == -1)
			exit_failure("lseek");

		/* Get and print (if necessary) result */
		/* ------------------------------------------------------------------------- */
		if (read(temp_file_fd, &response, sizeof(int)) == -1)
			exit_failure("read");

		/* Check result */
		if (response != 0)
			print_formatted_message(stderr, ERR_MSG, "--> Server reported: %s", 1, strerror(response));
		else {

			/* Create requested file */
			/* ------------------------------------------------------------------------- */
			int fd = open(file_dir, O_WRONLY | O_CREAT | O_TRUNC,
			S_IRWXU | S_IRWXG | S_IRWXO);
			if (fd == -1 && errno != 0)
				exit_failure("open");

			int s = receiving_data_selective_repeat(DataNetwork_obj, Settings_obj, fd, NULL);

			/* Close file descriptor */
			if (close(fd) == -1)
				exit_failure("close");

			/* If trasmission failed delete file */
			if (s == TRANSMISSION_FAILED) {
				print_formatted_message(stdout, STD_MSG, "'GET' operation FAILED!", 0);
				if (remove(file_dir) == -1)
					exit_failure("remove");
			} else
				print_formatted_message(stdout, STD_MSG, "'GET' operation COMPLETE!", 0);
		}
	}

	/* Close temp file descriptor */
	if (fclose(temp_file) == -1)
		exit_failure("close");

	/* Free unuseful memory */
	free(dir_file);
	free(file_dir);

}

/*
 *
 */
void __client_put_command(DataNetwork *DataNetwork_obj, Settings *Settings_obj, char* pathname) {

	char check_replay = NO_CHECK_FIRST_REPLAY;

	/* Preparing a temporary file (used to store request result) */
	/* ****************************************************************************** */
	FILE *temp_file = tmpfile();
	int temp_file_fd = fileno(temp_file);

	/* Receiving request result by server */
	/* ****************************************************************************** */
	if (receiving_data_selective_repeat(DataNetwork_obj, Settings_obj, temp_file_fd, &check_replay) == TRANSMISSION_SUCCESSFUL) {

		int response;

		/* Restoring read/write file offset */
		if (lseek(temp_file_fd, 0, SEEK_SET) == -1)
			exit_failure("lseek");

		/* Get and print (if necessary) result */
		/* ------------------------------------------------------------------------- */
		if (read(temp_file_fd, &response, sizeof(int)) == -1)
			exit_failure("read");

		/* Check result */
		if (response != 0) {
			print_formatted_message(stderr, ERR_MSG, "--> Server reported: %s", 1, strerror(response));
			print_formatted_message(stdout, STD_MSG, "'PUT' operation FAILED.", 0);
		} else {

			/* Open file descriptor to send data */
			int fd = open(pathname, O_RDONLY);
			if (fd == -1 && errno != 0)
				exit_failure("open");

			print_formatted_message(stdout, STD_MSG, "Transmission...", 0);
			/* Sending data */
			if (sending_data_selective_repeat(DataNetwork_obj, Settings_obj, fd) == TRANSMISSION_FAILED)
				print_formatted_message(stdout, STD_MSG, "'PUT' operation FAILED.", 0);
			else
				print_formatted_message(stdout, STD_MSG, "'PUT' operation COMPLETE.", 0);

			/* Close file descriptor */
			if (close(fd) == -1)
				exit_failure("close");
		}
	}

	/* Close temp file descriptor */
	if (fclose(temp_file) == -1)
		exit_failure("close");
}

/*
 * This function is used to send a request to server.
 */
void __send_request_to_server(DataNetwork *DataNetwork_obj, RequestPacket *RequestPacket_obj) {

	/* Size of 'request_type_argument' */
	size_t sz_request_argument = strlen(RequestPacket_obj->request_type_argument) * sizeof(char);

#ifdef TEST
	fprintf(stderr, "sz_request_argument: %li\n", sz_request_argument);
#endif

	/* Preparing request allocating following buffer... */
	/* ============================================================================ */
	void *client_request = calloc(1, sizeof(Settings) + sizeof(char) + sz_request_argument);
	if (client_request == NULL)
		exit_failure("calloc");

	memcpy(client_request, &RequestPacket_obj->client_setting, sizeof(Settings));
	memcpy(client_request + sizeof(Settings), &RequestPacket_obj->request_type, sizeof(char));
	strcpy(client_request + sizeof(Settings) + sizeof(char), RequestPacket_obj->request_type_argument);

#ifdef TEST
	assert(memcmp(client_request, &RequestPacket_obj->client_setting, sizeof(Settings)) == 0);
	assert(memcmp(client_request + sizeof(Settings), &RequestPacket_obj->request_type, sizeof(char)) == 0);
	assert(memcmp(client_request + sizeof(Settings) + sizeof(char), RequestPacket_obj->request_type_argument, sz_request_argument) == 0);
#endif

	/* Send request to server */
	/* ============================================================================ */
	int bytes_sent = sendto(DataNetwork_obj->socket_fd, client_request, sizeof(Settings) + sizeof(char) + sz_request_argument, 0,
			(struct sockaddr*) &(DataNetwork_obj->address), DataNetwork_obj->address_len);
	if (bytes_sent == -1)
		exit_failure("sendto");

#ifdef TEST
	exit(0);
#endif

	/* Free memory */
	/* ============================================================================ */
	free(client_request);
}

/*
 * This function is used to start client transmission.
 *
 * @param: *DataNetwork_obj -> A pointer to a 'DataNetwork' struct.
 * @param: command -> A char that speciefies command chosen by user.
 * @param: *command_argument -> Command argument..
 */
void client_start(DataNetwork *DataNetwork_obj, char command, char *command_argument) {

	/* Backup 'DataNetwork' data... */
	/* ============================================================================ */
	DataNetwork *DataNetwork_backup = calloc(1, sizeof(DataNetwork));
	if (DataNetwork_backup == NULL)
		exit_failure("calloc");

	memcpy(DataNetwork_backup, DataNetwork_obj, sizeof(DataNetwork));

	/* Preparing 'RequestPacket' object...  */
	/* ============================================================================ */
	RequestPacket *RequestPacket_obj = calloc(1, sizeof(RequestPacket));
	if (RequestPacket_obj == NULL)
		exit_failure("calloc");

	RequestPacket_obj->request_type = command;

	/* Parsing 'command_argument'...  */
	/* ============================================================================ */
	if (command == 'p') {

		/* Store into a buffer received 'command_argument' */
		char path[PATH_MAX];
		strcpy(path, command_argument);

		/* Split received command argument into token */
		char *token = strtok(path, "/");

		while (token != NULL) {
			strcpy(RequestPacket_obj->request_type_argument, token);
			token = strtok(NULL, "/");
		}
	} else if (command == 'g')
		strcpy(RequestPacket_obj->request_type_argument, command_argument);

#ifdef TEST
	fprintf(stderr, "'RequestPacket_obj->request_type': %c\n", RequestPacket_obj->request_type);
	fprintf(stderr, "'RequestPacket_obj->request_type_argument': %s\n", RequestPacket_obj->request_type_argument);
#endif

	/* Retrieve configuration's data */
	get_application_settings(&RequestPacket_obj->client_setting);

	/* Sending request to server */
	/* ============================================================================ */
	__send_request_to_server(DataNetwork_obj, RequestPacket_obj);

	/* Client jobs... */
	/* ============================================================================ */
	switch (command) {
	case 'l':
		print_formatted_message(stdout, STD_MSG, "'LIST' operation in progress.", 0);
		__client_list_command(DataNetwork_obj, &RequestPacket_obj->client_setting);
		break;
	case 'p':
		print_formatted_message(stdout, STD_MSG, "'PUT' operation in progress.", 0);
		__client_put_command(DataNetwork_obj, &RequestPacket_obj->client_setting, command_argument);
		break;
	case 'g':
		print_formatted_message(stdout, STD_MSG, "'GET' operation in progress.", 0);
		__client_get_command(DataNetwork_obj, &RequestPacket_obj->client_setting, command_argument);
		break;
	default:
		break;
	}

	/* Restore 'DataNetwork' backup */
	/* ============================================================================ */
	memcpy(DataNetwork_obj, DataNetwork_backup, sizeof(DataNetwork));

	/* Free memory */
	/* ============================================================================ */
	free(RequestPacket_obj);
	free(DataNetwork_backup);

#ifdef TEST
	return;
#endif

	request_user_input_to_continue();
}
