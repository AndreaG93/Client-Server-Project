/*
 * interface_functions.h
 *
 * Created on: 26 Sep 2017
 * Author: Andrea Graziani - 0189326
 */

#ifndef SRC_USER_INTERFACE_INTERFACE_FUNCTIONS_H_
#define SRC_USER_INTERFACE_INTERFACE_FUNCTIONS_H_

/* Including project headers */
#include "../file/FilePropertyList_struct.h"

/* Including libraries */
#include <sys/ioctl.h>
#include <stdio.h>
#include <netinet/in.h>

/* Macro */
#define STD_MSG "[  MSG  ]"
#define ERR_MSG  "[ ERROR ]"
#define FAIL_MSG "[FAILURE]"
#define MSG_INVALID_INPUT "Invalid input!"

/* Prototypes */
void print_interface(char *title, int menu_row, int menu_col, char *menu_stirng[menu_row][menu_col]);
void print_formatted_message(FILE *stream, char *msg_type, char *template, int n_args, ...);
void print_file_list(FilePropertyList **file_property_list);
void print_horizontal_line(char *character);
void request_user_input_to_continue();
void print_user_input_cursor();
void print_internet_socket_address_info(struct sockaddr_in *address);

#endif /* SRC_USER_INTERFACE_INTERFACE_FUNCTIONS_H_ */
