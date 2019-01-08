/*
 * interface_functions.c
 *
 * Created on: 26 Sep 2017
 * Author: Andrea Graziani - 0189326
 */

/* Including project header files */
/* ============================================================================ */
#include "../project_property.h"
#include "../file/FilePropertyList_struct.h"
#include "interface_functions.h"

/* Including libraries */
/* ============================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* Macro */
/* ============================================================================ */
#define OFFSET_USER_INTERFACE 3

/*
 * This function is used to print margins.
 */
void __print_margin() {
	for (register int i = 0; i < OFFSET_USER_INTERFACE; i++)
		fprintf(stderr, " ");
}

/*
 * This function is used to print a menu.
 *
 * @param: help_menu_row -> Number of rows of menu.
 * @param: help_menu_col -> Number of columns of menu.
 * @param: help_menu_stirng -> A matrix contains data about menu.
 */
void __print_help_menu(int help_menu_row, int help_menu_col, char *help_menu_stirng[help_menu_row][help_menu_col]) {

	size_t max_len = 0;

	/* Print a title */
	__print_margin();
	fprintf(stderr, "- %s -\n\n", "Select an option:");

	/* Calc column len */
	for (register unsigned int i = 0; i < help_menu_row; ++i) {
		size_t len = strlen(help_menu_stirng[i][0]);
		if (len > max_len)
			max_len = len;
	}

	/* Print all avaible commands specified in 'help_menu_stirng' */
	for (register unsigned int i = 0; i < help_menu_row; ++i) {

		int diff_len = max_len - strlen(help_menu_stirng[i][0]);
		__print_margin();
		fprintf(stderr, help_menu_stirng[i][0]);

		/* Add some spaces */
		for (register unsigned int i = 0; i < diff_len + 3; i++)
			fprintf(stderr, " ");

		for (register unsigned int j = 1; j < help_menu_col; ++j)
			printf("%5s ", help_menu_stirng[i][j]);
		printf("\n");
	}
}

/*
 * This function is used to print a specified string right aligned.
 *
 * @param: *string -> A string.
 */
void __print_string_right_aligned(char *string) {

	/* This struct contains data about window size */
	struct winsize winsize_struct;
	/* Get information about current window and store them into 'winsize' structure */
	ioctl(fileno(stderr), TIOCGWINSZ, &winsize_struct);

	for (register int i = 0; i < winsize_struct.ws_col - strlen(string) - OFFSET_USER_INTERFACE; i++)
		fprintf(stderr, " ");

	fprintf(stderr, "%s\n", string);
}

/*
 * This function is used to clean user interface.
 */
void __clean_user_interface() {
	system("clear");
}

/*
 * This function is used to print cursor.
 */
void print_user_input_cursor() {

	fprintf(stderr, "\n");

	/* Add margin */
	__print_margin();

	fprintf(stderr, "--> ");
}

/*
 * This function is used to print an horizzontal line. This function automatically add offset.
 *
 * @param: *character -> A pointer to a char; it will be used to produce a horizontal line.
 */
void print_horizontal_line(char *character) {

	/* This struct contains data about window size */
	struct winsize winsize_struct;
	/* Get information about current window and store them into 'winsize' structure */
	ioctl(fileno(stderr), TIOCGWINSZ, &winsize_struct);

	/* Add margin */
	__print_margin();

	/* Print line */
	for (register int i = 0; i < winsize_struct.ws_col - OFFSET_USER_INTERFACE * 2; i++)
		fprintf(stderr, character);

	fprintf(stderr, "\n");
}

/*
 * This function is used to print a formatted message.
 *
 * @param: *stream -> A pointer to a FILE struct.
 * @param: *msg_type -> It specifies message type.
 * @param: *template -> Template of message (like printf).
 * @param: *n_args -> Number of arguments.
 * @param: ... -> Data used to produce message (like printf).
 */
void print_formatted_message(FILE *stream, char *msg_type, char *template, int n_args, ...) {

	va_list ap;

	/* Initialize 'va_list' */
	va_start(ap, n_args);

	/* Print interface margins */
	__print_margin();

	/* Print message type */
	if (msg_type != NULL)
		fprintf(stream, "%s: ", msg_type);
	else
		fprintf(stream, "           ");

	/* Print message */
	vfprintf(stream, template, ap);

	/* Print a new line */
	fprintf(stream, "\n");

	/* Free memory */
	va_end(ap);
}

/*
 * This function is used to print a message to user, inviting him to continue pressing 'Enter'
 */
void request_user_input_to_continue() {

	print_formatted_message(stderr, STD_MSG, "Press 'Enter' to continue...", 0);
	char x;

	if (read(STDIN_FILENO, &x, sizeof(char)) == -1)
		exit_failure("read (request_user_input_to_continue)");

}

/**
 * This function is used to print main user interface including menu.
 *
 * @param: *title -> A string.
 * @param: menu_row -> Number of rows of menu.
 * @param: menu_col -> Number of columns of menu.
 * @param: menu_stirng -> A matrix contains data about menu.
 */
void print_interface(char *title, int menu_row, int menu_col, char *menu_stirng[menu_row][menu_col]) {

	/* Clean interface */
	__clean_user_interface();

	/* Print application title */
	print_horizontal_line("=");
	__print_margin();
	fprintf(stderr, "%s - ver. %s\n", "CLIENT SERVER APPLICATION", VERSION);
	print_horizontal_line("=");
	__print_string_right_aligned(UNIVERSITY);
	__print_string_right_aligned(AUTHORS);

	/* Print subsection */
	__print_margin();
	fprintf(stderr, "%s\n", title);

	/* Print menu */
	if (menu_stirng != NULL) {
		print_horizontal_line("-");
		__print_help_menu(menu_row, menu_col, menu_stirng);
	}

	print_horizontal_line("=");

	/* Print user input */
	fprintf(stderr, "\n");
}

/*
 * This function is used to print data taken from a 'sockaddr_in' atruct.
 *
 * @param: *address -> A pointer to a 'sockaddr_in' struct.
 */
void print_internet_socket_address_info(struct sockaddr_in *address) {

	char host_buffer[NI_MAXHOST];
	char host_buffer_numeric[NI_MAXHOST];
	char service_buffer[NI_MAXSERV];
	char service_buffer_numeric[NI_MAXSERV];

	/* converts a socket address to a corresponding host and service,
	 * in a protocol-independent manner. */
	/* ============================================================================ */

	/* Getting data in numeric form */
	/* ============================================================================ */
	if (getnameinfo((const struct sockaddr *) address, sizeof(*address), host_buffer_numeric, sizeof(host_buffer_numeric), service_buffer_numeric,
			sizeof(service_buffer_numeric), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
		strcpy(service_buffer, "Unknown");
		strcpy(service_buffer_numeric, "Unknown");
	}

	/* Getting data in 'non-numeric' form */
	/* ============================================================================ */
	if (getnameinfo((const struct sockaddr *) address, sizeof(*address), host_buffer, sizeof(host_buffer), service_buffer, sizeof(service_buffer),
	NI_NAMEREQD) != 0) {
		strcpy(host_buffer, "Unknown");
		strcpy(service_buffer, "Unknown");
	}

#ifdef TEST
	fprintf(stderr, "Host: %s / %s <-> Port: %s / %s \n", host_buffer, host_buffer_numeric, service_buffer, service_buffer_numeric);
	return;
#endif

	/* Printing data */
	/* ============================================================================ */
	print_formatted_message(stderr, NULL, "Host: %s / %s <-> Port: %s / %s \n", 4, host_buffer, host_buffer_numeric, service_buffer,
			service_buffer_numeric);
}

/*
 * This function is used to print data stored into a 'FilePropertyList' struct.
 *
 * @param: **file_property_list -> A 'FilePropertyList' struct.
 */
void print_file_list(FilePropertyList **file_property_list) {

	__clean_user_interface();

	/* This struct contains data about window size */
	struct winsize winsize_struct;
	/* Get information about current window and store them into 'winsize' structure */
	ioctl(fileno(stderr), TIOCGWINSZ, &winsize_struct);
	/* A pointer to 'FilePropertyList' */
	FilePropertyList *support_pointer = *file_property_list;
	/* Difference lenght header */
	size_t diff_len = 0;

	/* colums size */
	size_t max_len_col_name = strlen("File name");
	size_t max_len_col_size = strlen("Size (B)");
	size_t max_len_last_access = strlen("2012-12-31 12:59:59");
	size_t max_len_last_modification = strlen("2012-12-31 12:59:59");

	/* Calc column len according to data */
	while (support_pointer != NULL) {

		size_t len_col_name = snprintf(NULL, 0, "%s", support_pointer->data->file_name);

		if (len_col_name > max_len_col_name)
			max_len_col_name = len_col_name;

		size_t len_col_size = snprintf(NULL, 0, "%lu", support_pointer->data->file_stat.st_size);

		if (len_col_size > max_len_col_size)
			max_len_col_size = len_col_size;

		/* Update pointer */
		support_pointer = support_pointer->next;
	}

	/* Restore pointer */
	support_pointer = *file_property_list;

	/* Print Title */
	print_horizontal_line("=");
	__print_margin();
	fprintf(stderr, "%s\n", "Avaible files on server");
	print_horizontal_line("=");
	fprintf(stderr, "\n");

	/* Print header */
	__print_margin();

	/* File name column header */
	diff_len = max_len_col_name - strlen("File name");
	fprintf(stderr, "File name");

	for (register unsigned int i = 0; i < diff_len + 3; i++)
		fprintf(stderr, " ");
	fprintf(stderr, " | ");

	/* Size file column header */
	diff_len = max_len_col_size - strlen("Size (B)");
	fprintf(stderr, "Size (B)");

	for (register unsigned int i = 0; i < diff_len + 3; i++)
		fprintf(stderr, " ");
	fprintf(stderr, " | ");

	/* Last access column header */
	diff_len = max_len_last_access - strlen("Last access");
	fprintf(stderr, "Last access");

	for (register unsigned int i = 0; i < diff_len + 3; i++)
		fprintf(stderr, " ");
	fprintf(stderr, " | ");

	/* Size file column header */
	diff_len = max_len_last_modification - strlen("Last modification");
	fprintf(stderr, "Last modification");

	for (register unsigned int i = 0; i < diff_len + 3; i++)
		fprintf(stderr, " ");
	fprintf(stderr, " | ");

	fprintf(stderr, "\n");
	print_horizontal_line("-");

	/*
	 * Print data
	 */
	while (support_pointer != NULL) {

		/* Char written on stderr */
		int char_written;

		__print_margin();

		/*
		 * Print file name
		 */

		char_written = fprintf(stderr, "%s", support_pointer->data->file_name);
		/* Add some spaces and separator */
		for (register int i = 0; i < (max_len_col_name - char_written + 3); i++)
			fprintf(stderr, " ");
		fprintf(stderr, " | ");

		/*
		 * Print file size
		 */

		char_written = fprintf(stderr, "%lu", support_pointer->data->file_stat.st_size);
		/* Add some spaces and separator */
		for (register int i = 0; i < (max_len_col_size - char_written + 3); i++)
			fprintf(stderr, " ");
		fprintf(stderr, " | ");

		/*
		 * Print last access
		 */

		/* Get data */
		struct timespec last_access = support_pointer->data->file_stat.st_atim;
		/* Convert to string */
		char *last_access_string = convert_timespec_to_string(&last_access);

		char_written = fprintf(stderr, "%s", last_access_string);
		/* Add some spaces and separator */
		for (register int i = 0; i < (max_len_last_access - char_written + 3); i++)
			fprintf(stderr, " ");
		fprintf(stderr, " | ");

		/* Free unuseful memory */
		free(last_access_string);

		/*
		 * Print last modification
		 */

		/* Get data */
		struct timespec last_mod = support_pointer->data->file_stat.st_atim;
		/* Convert to string */
		char *last_mod_string = convert_timespec_to_string(&last_mod);

		char_written = fprintf(stderr, "%s", last_mod_string);
		/* Add some spaces and separator */
		for (register int i = 0; i < (max_len_last_modification - char_written + 3); i++)
			fprintf(stderr, " ");
		fprintf(stderr, " | ");

		/* Free unuseful memory */
		free(last_mod_string);

		/*
		 * END
		 */

		fprintf(stderr, "\n");

		support_pointer = support_pointer->next;
	}

	print_horizontal_line("-");
}

