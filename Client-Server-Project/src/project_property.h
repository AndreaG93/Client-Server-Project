/*
 * project_property.h
 *
 * Created on: 23 Sep 2017
 * Author: Andrea Graziani - 0189326
 */

#ifndef SRC_PROJECT_PROPERTY_H_
#define SRC_PROJECT_PROPERTY_H_

/* Coding style convention */
/* ============================================================================ */
/*
 * I followed some coding convention styles to realize this project, which can be summarized as follows:
 *
 * <-1-> All macros and constants are written using 'SCREAMING_SNAKE_CASE' convention:
 *       * Examples: MAX_BUFFER_SIZE, DEBUG;
 *
 * <-2-> Types are written using 'CamelCase' convention.
 * 	     * Examples: CircularBuffer;
 *
 * <-3-> Function names are written in 'snake_case' convention.
 *       * Examples: add_file(), print_error();
 *
 * <-4-> All support functions that shouldn't be called directly, are written with one or more underscores
 *       at the beginning.
 *       * Examples: _free_memory(), __get_lock_on_file();
 *
 * <-5-> Function names that operate on structs are written in '[Type]_snake_case'.
 *       * Examples: CircularBurrer_add_item();
 *
 * <-6-> All local variables are written in 'snake_case'.
 * 	     * Examples: packet_lenght;
 */

/* Including libraries */
/* ============================================================================ */
#include <stdlib.h>

/* Macro */
/* ============================================================================ */
#define VERSION "0.9c"
#define AUTHORS "Andrea Graziani - 0189326"
#define UNIVERSITY "Universita' degli Studi di Roma Tor Vergata - IIW 2017-2018"
#define TRUE 1
#define FALSE 0
#define TRANSMISSION_FAILED 1
#define TRANSMISSION_SUCCESSFUL 0

/* Uncomment following to run some tests... */
//#define TEST
/* Uncomment following to run a verbose session... */
//#define VERBOSE

/* Types */
/* ============================================================================ */
typedef unsigned char boolean;

/* Prototypes */
/* ============================================================================ */
void exit_failure(char *function_name);
int get_random_int(int min, int max);
char *concatenate_strings(int n_args, ...);
char *read_string_from_file_descriptor(int fd);
char *convert_timespec_to_string(struct timespec *ts);
char *convert_int_to_string(int int_value);
long string_to_long(char *arg);

#endif /* SRC_PROJECT_PROPERTY_H_ */
