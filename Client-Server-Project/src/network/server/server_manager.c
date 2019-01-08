/*
 * server_manager.c
 *
 *  Created on: 27 Sep 2017
 *      Author: Andrea Graziani
 */

/* Including project headers */
#include "../DataNetwork.h"

#include "../../user_interface/interface_functions.h"
#include "../../file/directory_manager.h"
#include "../../project_property.h"
#include "../../thread/thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <netdb.h>
#include "../data_transmission.h"

#define METADATA_LIST_FILE "/server_file.temp"

pthread_mutex_t lock;

typedef struct thread_argument {

	RequestPacket RequestPacket_obj;
	DataNetwork DataNetwork_obj;

} ThreadArgument;

/*
 * This function is used to
 */
void __metadata_preload() {

	/* Get directory where are stored file */
	char *dir_file = get_application_directory(APPLICATION_SERVER_FILE_FORLDER);
	/* Get directory where store metadata file */
	char *dir_temp = get_application_directory(APPLICATION_SERVER_APP_DATA_FOLDER);

	/* Generate temporary file full path*/
	char *metadata_file_path = concatenate_strings(2, dir_temp,
	METADATA_LIST_FILE);

	/* File descriptor used to open a file where write data */
	int fd = open(metadata_file_path, O_WRONLY | O_CREAT | O_TRUNC,
	S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd == -1 && errno != 0)
		exit_failure("open");

	/* Retrieve data about file stored on server */
	FilePropertyList *data = get_file_information(dir_file);

	/* Serialize data */
	FilePropertyList_serializer(&data, fd);

	/* Close file descriptor */
	if (close(fd) == -1)
		exit_failure("close");

	/* Free useless memory */
	free(data);
	free(dir_temp);
	free(dir_file);
	free(metadata_file_path);
}

void __server_put_command(DataNetwork *DataNetwork_obj, Settings *Settings_obj, char *file_name) {

	int fd;
	int request_result;

	/* Generating necessary directories */
	/* ****************************************************************************** */

	/* Server's directory where save requested file */
	char *dir_file = get_application_directory(APPLICATION_SERVER_FILE_FORLDER);
	/* Generate full path of requested file */
	char *file_dir = concatenate_strings(3, dir_file, "/", file_name);

	/* Preparing a temporary file (used to store request result) */
	/* ****************************************************************************** */
	FILE *temp_file = tmpfile();
	int temp_file_fd = fileno(temp_file);

	/* Create requested file to server; TRUNC every existing file */
	/* ****************************************************************************** */
	pthread_mutex_lock(&lock);

	char *path = file_dir;

	/* Generate path */
	/* ****************************************************************************** */
	for (int i = 1;; i++) {

		/* Check if file already exists on server */
		errno = 0;
		fd = open(path, O_WRONLY);

		/* If file doesn't exist create a new one */
		if (fd != 0 && errno == ENOENT) {

			print_formatted_message(stdout, STD_MSG, "File %s created.", 1, path);
			errno = 0;
			fd = open(path, O_WRONLY | O_CREAT,
			S_IRWXU | S_IRWXG | S_IRWXO);
			break;
		}
		/* Otherwise append a 'label' */
		else {

			/* Close existing file */
			if (close(fd) == -1)
				exit_failure("close");

			/* Creating a unique label */
			/* ****************************************************************************** */
			char *n = convert_int_to_string(i);
			path = concatenate_strings(3, file_dir, "_", n);

			/* Free memory */
			free(n);
		}
	}

	/* Update pointers... */
	if (strcmp(file_dir, path) != 0) {
		free(file_dir);
		file_dir = path;
	}

	pthread_mutex_unlock(&lock);

	/* Save value of errno */
	request_result = errno;

	/* Send to client request's result */
	/* ****************************************************************************** */
	if (write(temp_file_fd, &request_result, sizeof(int)) == -1)
		exit_failure("write");
	if (lseek(temp_file_fd, 0, SEEK_SET) == -1)
		exit_failure("lseek");

	sending_data_selective_repeat(DataNetwork_obj, Settings_obj, temp_file_fd);

	/* Get file from client */
	/* ****************************************************************************** */
	if (request_result == 0) {

		int s = receiving_data_selective_repeat(DataNetwork_obj, Settings_obj, fd, NULL);

		/* Close requested file */
		if (close(fd) == -1)
			exit_failure("close");

		/* If trasmission failed delete file */
		if (s == 1){
			print_formatted_message(stdout, STD_MSG, "File %s removed.", 1, file_dir);
			if (remove(file_dir) == -1)
				exit_failure("remove");
		}

	}

	/* Close temp file descriptor */
	if (fclose(temp_file) == -1)
		exit_failure("close");

	/* Free memory */
	free(dir_file);
	free(file_dir);

	/* Update metadata */
	pthread_mutex_lock(&lock);
	__metadata_preload();
	pthread_mutex_unlock(&lock);

}

/*
 *
 */
void __server_get_command(DataNetwork *DataNetwork_obj, Settings *Settings_obj, char *file_name) {

	int fd;
	int request_result;

	/* Generating necessary directories */
	/* ****************************************************************************** */

	/* Server's directory where save requested file */
	char *dir_file = get_application_directory(APPLICATION_SERVER_FILE_FORLDER);
	/* Generate full path of requested file */
	char *file_dir = concatenate_strings(3, dir_file, "/", file_name);

	/* Preparing a temporary file (used to store request result) */
	/* ****************************************************************************** */
	FILE *temp_file = tmpfile();
	int temp_file_fd = fileno(temp_file);
	if (temp_file_fd == -1)
		exit_failure("fileno");

	/* Open (if possible) requested file */
	/* ****************************************************************************** */
	errno = 0;
	fd = open(file_dir, O_RDONLY);

	/* Save value of errno */
	request_result = errno;

	/* Send to client request's result */
	/* ****************************************************************************** */
	if (write(temp_file_fd, &request_result, sizeof(int)) == -1)
		exit_failure("write");
	if (lseek(temp_file_fd, 0, SEEK_SET) == -1)
		exit_failure("lseek");

	sending_data_selective_repeat(DataNetwork_obj, Settings_obj, temp_file_fd);

	/* Send to client requested file */
	/* ****************************************************************************** */
	if (request_result == 0) {

		sending_data_selective_repeat(DataNetwork_obj, Settings_obj, fd);

		/* Close requested file */
		if (close(fd) == -1)
			exit_failure("close");
	}

	/* Close temp file descriptor */
	if (fclose(temp_file) == -1)
		exit_failure("close");

	/* Free memory */
	free(dir_file);
	free(file_dir);
}

/*
 *
 */
void __server_list_command(DataNetwork *DataNetwork_obj, Settings *Settings_obj) {

	/* Get directory where store metadata file */
	char *dir_temp = get_application_directory(APPLICATION_SERVER_APP_DATA_FOLDER);
	/* Generate temporary file full path*/
	char *temp_file_dir = concatenate_strings(2, dir_temp, METADATA_LIST_FILE);

	/* File descriptor used to open a temporary file where write data received from server */
	int fd = open(temp_file_dir, O_RDONLY);
	if (fd == -1 && errno != 0)
		exit_failure("open");

	/* Sending data */
	sending_data_selective_repeat(DataNetwork_obj, Settings_obj, fd);

	/* Close file descriptor */
	if (close(fd) == -1)
		exit_failure("close");

	/* Free useless memory */
	free(temp_file_dir);
	free(dir_temp);
}

/*
 * This function is used to initialize server.
 */
int __server_initialization(char main_process) {

	/* Socket file descriptor */
	int sockfd;
	/* Server internet socket address */
	struct sockaddr_in servaddr;

	/* Creates a socket and specifies a communication style: in this case a datagram-based protocol.  */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
		exit_failure("socket");

	/* Crear server internet socket address structure */
	memset(&servaddr, 0, sizeof(servaddr));

	/* Set address family that is used to designate the type of addresses that socket can communicate with:
	 * in this case, Internet Protocol v4 addresses */
	servaddr.sin_family = AF_INET;
	/* Server accepts request on any network interface */
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Specify port 'SERVER_PORT' converting them from host byte order to network byte order. */
	if (main_process) {
		servaddr.sin_port = htons(SERVER_COMMAND_PORT);

		/*
		 * allow reuse of local address if there is not an active listening
		 * socket bound to the address
		 */
		int reuse = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
			exit_failure("setsockopt");

	} else
		servaddr.sin_port = 0;

	/* Bind the socket to server address */
	if (bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1)
		exit_failure("bind");

	socklen_t addrLen = sizeof(servaddr);
	if (getsockname(sockfd, (struct sockaddr *) &servaddr, &addrLen) == -1)
		exit_failure("getsockname");

	print_formatted_message(stderr, STD_MSG, "Server informations: ", 0);
	print_internet_socket_address_info(&servaddr);
	return sockfd;
}

/* this function is run by the second thread */
void *thread_job(void *x_void_ptr) {

	/* Mark this thread as 'detached'; when it terminates, all its resources are released
	 * and we cannot wait for it to terminate.  */
	/* ****************************************************************************** */
	pthread_detach(pthread_self());

	/* Getting data... */
	ThreadArgument *ThreadArgument_obj = (ThreadArgument*) x_void_ptr;

	/* Create a new socket and bind ephemeral port */
	ThreadArgument_obj->DataNetwork_obj.socket_fd = __server_initialization(0);

	/* 'LIST' command */
	/* ****************************************************************************** */
	if (ThreadArgument_obj->RequestPacket_obj.request_type == 'l') {
		print_formatted_message(stdout, STD_MSG, "'LIST' command successfully received!", 0);
		__server_list_command(&ThreadArgument_obj->DataNetwork_obj, &ThreadArgument_obj->RequestPacket_obj.client_setting);

	}
	/* 'GET' command */
	/* ****************************************************************************** */
	else if (ThreadArgument_obj->RequestPacket_obj.request_type == 'g') {
		print_formatted_message(stdout, STD_MSG, "'GET' command successfully received!", 0);
		__server_get_command(&ThreadArgument_obj->DataNetwork_obj, &ThreadArgument_obj->RequestPacket_obj.client_setting,
				ThreadArgument_obj->RequestPacket_obj.request_type_argument);

	}
	/* 'PUT' command */
	/* ****************************************************************************** */
	else if (ThreadArgument_obj->RequestPacket_obj.request_type == 'p') {
		print_formatted_message(stdout, STD_MSG, "'PUT' command successfully received!", 0);
		__server_put_command(&ThreadArgument_obj->DataNetwork_obj, &ThreadArgument_obj->RequestPacket_obj.client_setting,
				ThreadArgument_obj->RequestPacket_obj.request_type_argument);
	} else
		print_formatted_message(stdout, STD_MSG, "Unknown command.", 0);

	/* Closing socket */
	if (close(ThreadArgument_obj->DataNetwork_obj.socket_fd) == -1)
		exit_failure("close");

	/* Free memory */
	free(ThreadArgument_obj);
	return NULL;
}

void start_server() {

	ThreadArgument *ThreadArgument_obj;
	int received_bytes;

	/* Listener Socket file descriptor: Initialization server */
	int listener_socket_fd = __server_initialization(1);

	/* Preload metadata about shared files stored in application server directory */
	__metadata_preload();

	/* Load lock */
	if (pthread_mutex_init(&lock, NULL) != 0)
		exit_failure("pthread_mutex_init");

	while (1) {

		/* Preparing necessary data...  */
		/* ************************************************************************* */
		ThreadArgument_obj = calloc(1, sizeof(ThreadArgument));
		if (ThreadArgument_obj == NULL)
			exit_failure("calloc");

		ThreadArgument_obj->DataNetwork_obj.address_len = sizeof(ThreadArgument_obj->DataNetwork_obj.address);

		/* Server is now ready: print a message! */
		/* ************************************************************************* */
		print_formatted_message(stdout, STD_MSG, "Server loading complete! Waiting for requests!", 0);

		/* Receiving request from client */
		/* ************************************************************************* */
		while (1) {

			errno = 0;
			received_bytes = recvfrom(listener_socket_fd, &ThreadArgument_obj->RequestPacket_obj, sizeof(RequestPacket), 0,
					(struct sockaddr *) &ThreadArgument_obj->DataNetwork_obj.address, &ThreadArgument_obj->DataNetwork_obj.address_len);

			if (received_bytes == -1)
				if (errno == EINTR)
					continue;
				else
					exit_failure("recvfrom");
			else
				break;
		}

		/* Print information about received request */
		/* ************************************************************************* */

		/* Get data about client */

		print_horizontal_line("*");
		print_formatted_message(stderr, STD_MSG, "Received request from: ", 0);
		print_internet_socket_address_info(&(ThreadArgument_obj->DataNetwork_obj.address));
		print_formatted_message(stdout, NULL, "Total byte received from client: %d", 1, received_bytes);
		print_formatted_message(stdout, NULL, "Received command: %c", 1, ThreadArgument_obj->RequestPacket_obj.request_type);
		print_formatted_message(stdout, NULL, "Received command argument: %s", 1, ThreadArgument_obj->RequestPacket_obj.request_type_argument);
		print_formatted_message(stdout, NULL, "Status Auto-Timout: %s", 1,
				(ThreadArgument_obj->RequestPacket_obj.client_setting.auto_timeout == 1) ? "Enabled" : "Disable");

		if (!ThreadArgument_obj->RequestPacket_obj.client_setting.auto_timeout)
			print_formatted_message(stdout, NULL, "Fixed Argument: %lu", 1, ThreadArgument_obj->RequestPacket_obj.client_setting.fixed_timeout);

		print_formatted_message(stdout, NULL, "Loss probability: %d", 1, ThreadArgument_obj->RequestPacket_obj.client_setting.loss_probability);
		print_formatted_message(stdout, NULL, "Sliding Window Size: %d", 1, ThreadArgument_obj->RequestPacket_obj.client_setting.sliding_window_size);

		print_horizontal_line("*");

		/* Launch thread */
		/* ************************************************************************* */
		thread_initialization(thread_job, (void **) ThreadArgument_obj);
	}
}

