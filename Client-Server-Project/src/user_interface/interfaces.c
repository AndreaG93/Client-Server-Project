/*
 * interfaces.c
 *
 * Created on: 26 Sep 2017
 * Author: Andrea Graziani - 0189326
 */

/* Including project headers */
#include "interface_functions.h"
#include "../network/client/client_manager.h"
#include "../project_property.h"
#include "../file/directory_manager.h"
#include "../network/client/client_manager.h"
#include "../network/server/server_manager.h"
#include "../network/DataNetwork.h"
#include "../file/Settings.h"

/* Including libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*
 * This function is used to print 'server' user interface.
 */
void __server_interface() {

	/* Print interface */
	print_interface("Server", 0, 0, NULL);

	/* Start server job */
	start_server();
}

/*
 * This function is used to print 'client' user interface.
 *
 * @param: *DataNetwork_obj -> A pointer to a 'DataNetwork' contains some useful info.
 */
void __client_interface(DataNetwork *DataNetwork_obj) {

	/* User's choice */
	char *user_choice;

	char *msg = NULL;
	char *msg_type = NULL;

	/* menu */
	char *menu[4][2];

	menu[0][0] = "list";
	menu[0][1] = "-> List information about file avaible on server.";

	menu[1][0] = "put [FILE_PATH]";
	menu[1][1] = "-> Upload on server a file specified by directory.";

	menu[2][0] = "get [FILE_NAME]";
	menu[2][1] = "-> Download from server a specified file.";

	menu[3][0] = "back";
	menu[3][1] = "-> To close this session and back to main menu.";

	/* loop while user does not choose 3 */
	while (1) {

		/* display the menu and instructions*/
		print_interface("Client", 4, 2, menu);

		/* Print connection details */
		print_formatted_message(stderr, STD_MSG, "Connected to:", 0);
		print_internet_socket_address_info((struct sockaddr_in *) &DataNetwork_obj->address);
		print_horizontal_line("=");

		/* Print messages if needed */
		if (msg != NULL)
			print_formatted_message(stdout, msg_type, "%s", 1, msg);

		print_user_input_cursor();

		/* Get user choice */
		user_choice = read_string_from_file_descriptor(STDIN_FILENO);

		/* Split received command into token */
		char *token = strtok(user_choice, " ");

		/* Empty command */
		if (strcmp(user_choice, "") == 0) {
			msg = "Insert a command please...";
			msg_type = STD_MSG;
		}
		/* 'LIST' command */
		else if (strcmp(token, "list") == 0 && (strtok(NULL, " ") == NULL)) {

			/* Reload interface */
			print_interface("Client", 0, 0, NULL);
			client_start(DataNetwork_obj, 'l', "");

			/* Restore message */
			msg = NULL;
			msg_type = NULL;
		}
		/* 'PUT' command */
		else if (strcmp(token, "put") == 0) {

			/* get path token */
			token = strtok(NULL, " ");

			/* Check path */
			if (token != NULL) {
				if (access(token, R_OK) == 0) {

					if (check_regular_file(token)) {

						/* Reload interface */
						print_interface("Client", 0, 0, NULL);
						client_start(DataNetwork_obj, 'p', token);

						/* Restore message */
						msg = NULL;
						msg_type = NULL;
					} else {
						msg = "Usage: put [FILE_PATH]; You have specified a directory";
						msg_type = ERR_MSG;
					}

				} else {
					msg = strerror(errno);
					msg_type = ERR_MSG;
				}

			} else {
				msg = "Usage: put [FILE_PATH]";
				msg_type = ERR_MSG;
			}
		}
		/* 'GET' command */
		else if (strcmp(token, "get") == 0) {

			/* get path token */
			token = strtok(NULL, " ");

			/* Check path */
			if (token != NULL) {

				print_interface("Client", 0, 0, NULL);
				client_start(DataNetwork_obj, 'g', token);

				/* Restore message */
				msg = NULL;
				msg_type = NULL;

			} else {
				msg = "Usage: get [FILE_NAME]";
				msg_type = ERR_MSG;
			}
		}
		/* 'BACK' command */
		else if (strcmp(token, "back") == 0) {

			/* Free memory */
			free(user_choice);
			break;

		}
		/* Invalid command */
		else {
			msg = MSG_INVALID_INPUT;
			msg_type = ERR_MSG;
		}

		/* Free memory */
		free(user_choice);
	}
}

/*
 * This function is used to print option interface.
 */
void __option_interface() {

	/* Initialize 'Setting' object */
	Settings *Settings_obj;

	/* Allocation memory */
	Settings_obj = calloc(1, sizeof(Settings));
	if (Settings_obj == NULL)
		exit_failure("calloc");

	/* Retrieve setting's data */
	get_application_settings(Settings_obj);

	unsigned long support_var = 0;

	/* User's choice */
	char *user_choice;

	char *msg = NULL;
	char *msg_type = NULL;

	/* menu */
	char *menu[5][2];

	menu[0][0] = "l / loss-probability [value]";
	menu[0][1] = "-> Changes loss probability to [value].";

	menu[1][0] = "w / windows-size [size]";
	menu[1][1] = "-> Changes size of 'sliding window' to [size]. 0 < [size] < 255";

	menu[2][0] = "at / auto-timeout [value]";
	menu[2][1] = "-> Enables [value = 1] or disables [value = 0] automatic calculation of the timeout value.";

	menu[3][0] = "ft / fixed-timout [value]";
	menu[3][1] = "-> Specifies fixed timeout value to [value] (in nanoseconds!)";

	menu[4][0] = "back";
	menu[4][1] = "-> To close this page and back to main menu.";

	/* loop while user does not choose 3 */
	while (1) {

		/* display the menu and instructions*/
		print_interface("Options", 5, 2, menu);

		/* Print current value */
		print_formatted_message(stderr, NULL, "-> Current loss probability value: %d", 1, Settings_obj->loss_probability);
		print_formatted_message(stderr, NULL, "-> Current sliding window size: %d", 1, Settings_obj->sliding_window_size);
		print_formatted_message(stderr, NULL, "-> Automatic timeout: %s", 1, (Settings_obj->auto_timeout == 1) ? "Enabled" : "Disabled");

		if (!Settings_obj->auto_timeout) {
			print_formatted_message(stderr, NULL, "-> Timeout value (nanoseconds): %lu", 1, Settings_obj->fixed_timeout);
		}

		fprintf(stderr, "\n");
		print_horizontal_line("=");
		fprintf(stderr, "\n");

		/* Print messages if needed */
		if (msg != NULL)
			print_formatted_message(stdout, msg_type, "%s", 1, msg);
		else
			fprintf(stderr, "\n");

		/* Print cursor */
		print_user_input_cursor();

		/* get user's choice */
		user_choice = read_string_from_file_descriptor(STDIN_FILENO);

		/* Split received command into token */
		char *token = strtok(user_choice, " ");

		/* Empty command */
		/* ============================================================================ */
		if (strcmp(user_choice, "") == 0) {
			msg = "Insert a command please...";
			msg_type = STD_MSG;
		}
		/* 'auto-timeout' command */
		/* ============================================================================ */
		else if (strcmp(token, "auto-timeout") == 0 || strcmp(token, "at") == 0) {

			/* get path token */
			token = strtok(NULL, " ");

			/* Check value */
			if (token != NULL && (support_var = string_to_long(token)) != -1 && (support_var == 1 || support_var == 0)) {

				Settings_obj->auto_timeout = support_var;

				/* Restore message */
				msg = NULL;
				msg_type = NULL;

			} else {
				msg = "Usage: at / auto-timeout [value]";
				msg_type = ERR_MSG;
			}
		}
		/* 'auto-timeout' command */
		/* ============================================================================ */
		else if (strcmp(token, "fixed-timout") == 0 || strcmp(token, "ft") == 0) {

			/* get path token */
			token = strtok(NULL, " ");

			/* Check value */
			if (token != NULL && (support_var = string_to_long(token)) != -1 && support_var > 0) {

				Settings_obj->fixed_timeout = support_var;

				/* Restore message */
				msg = NULL;
				msg_type = NULL;

			} else {
				msg = "Usage: ft / fixed-timout [value]";
				msg_type = ERR_MSG;
			}
		}
		/* 'windows-size' command */
		/* ============================================================================ */
		else if (strcmp(token, "windows-size") == 0 || strcmp(token, "w") == 0) {

			/* get path token */
			token = strtok(NULL, " ");

			/* Check value */
			if (token != NULL && (support_var = string_to_long(token)) != -1 && support_var > 0 && support_var < 256) {

				Settings_obj->sliding_window_size = support_var;

				/* Restore message */
				msg = NULL;
				msg_type = NULL;

			} else {
				msg = "Usage: w / windows-size [size]; ATTENTION!: 0 < [size] < 255!";
				msg_type = ERR_MSG;
			}
		}
		/* 'loss-probability' command */
		/* ============================================================================ */
		else if (strcmp(token, "loss-probability") == 0 || strcmp(token, "l") == 0) {

			/* get path token */
			token = strtok(NULL, " ");

			/* Check value */
			if (token != NULL && (support_var = string_to_long(token)) != -1 && support_var >= 0 && support_var <= 100) {

				Settings_obj->loss_probability = support_var;

				/* Restore message */
				msg = NULL;
				msg_type = NULL;

			} else {
				msg = "Usage: l / loss-probability [value] -> [value] must be > 0 and <= 100!";
				msg_type = ERR_MSG;
			}
		}
		/* 'BACK' command */
		/* ============================================================================ */
		else if (strcmp(token, "back") == 0)
			break;
		/* Invalid command */
		/* ============================================================================ */
		else {
			msg = MSG_INVALID_INPUT;
			msg_type = ERR_MSG;
		}
	}

	/* Save Setting on disk */
	save_application_settings(Settings_obj);

	/* free memory */
	free(Settings_obj);
	free(user_choice);
}

/*
 * This function is used to print main user interface.
 */
void main_interface() {

	/* User's choice */
	char *user_choice;

	char *msg = NULL;
	char *msg_type = NULL;

	/* menu */
	char *menu[4][2];

	menu[0][0] = "server";
	menu[0][1] = "-> Start a new SERVER session.";

	menu[1][0] = "client [HOST]";
	menu[1][1] = "-> Start a new CLIENT session.";

	menu[2][0] = "options";
	menu[2][1] = "-> Open configuration window.";

	menu[3][0] = "exit";
	menu[3][1] = "-> close this application.";

	/* loop while user does not choose 3 */
	while (1) {

		/* display the menu and instructions*/
		print_interface("Home", 4, 2, menu);

		/* Print messages if needed */
		if (msg != NULL)
			print_formatted_message(stdout, msg_type, "%s", 1, msg);
		else
			fprintf(stderr, "\n");

		/* Print cursor */
		print_user_input_cursor();

		/* get user's choice */
		user_choice = read_string_from_file_descriptor(STDIN_FILENO);

		/* Split received command into token */
		char *token = strtok(user_choice, " ");

		/* Empty command */
		/* ============================================================================ */
		if (strcmp(user_choice, "") == 0) {
			msg = "Insert a command please...";
			msg_type = STD_MSG;
		}
		/* Exit */
		/* ============================================================================ */
		else if (strcmp(user_choice, "exit") == 0) {
			print_formatted_message(stdout, STD_MSG, "Closing application...", 0);
			free(user_choice);
			exit(EXIT_SUCCESS);
		}
		/* Server */
		/* ============================================================================ */
		else if (strcmp(token, "server") == 0 && (strtok(NULL, " ") == NULL)) {

			/* Start server */
			__server_interface();

			/* Restore message */
			msg = NULL;
			msg_type = NULL;
		}
		/* Options */
		/* ============================================================================ */
		else if (strcmp(token, "options") == 0 && (strtok(NULL, " ") == NULL)) {

			/* Start server */
			__option_interface();

			/* Restore message */
			msg = NULL;
			msg_type = NULL;
		}
		/* Client */
		/* ============================================================================ */
		else if (strcmp(token, "client") == 0) {

			/* get path token */
			token = strtok(NULL, " ");

			/* Check path */
			if (token != NULL) {

				DataNetwork *x = client_initialization_for_transmission(token);

				if (x != NULL) {

					fprintf(stderr, "dsadas\n");
					__client_interface(x);

					/* Close socket and free memory */
					/* ---------------------------------------------------------------------------- */
					if (close(x->socket_fd) == -1)
						exit_failure("close");
					free(x);

				} else {
					msg = "Could not connect";
					msg_type = ERR_MSG;
				}

			} else {
				msg = "Usage: client [HOST]";
				msg_type = ERR_MSG;
			}
		}
		/* Invalid command */
		/* ============================================================================ */
		else {
			msg = MSG_INVALID_INPUT;
			msg_type = ERR_MSG;
		}

		/* Free memory */
		free(user_choice);

	}
}
