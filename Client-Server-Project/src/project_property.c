/*
 * project_property.c
 *
 * Created on: 26 Sep 2017
 * Author: Andrea Graziani - 0189326
 */

/* Including project header files */
/* ============================================================================ */
#include "user_interface/interface_functions.h"
#include "project_property.h"

/* Including libraries */
/* ============================================================================ */
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <libio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
 * This function is used to print an error message and terminate application execution.
 *
 * @param '*function_name': A pointer to a string containing function name.
 */
void exit_failure(char *function_name) {

#ifdef TEST
	fprintf(stderr, "'%s': %s", function_name, strerror(errno));
	exit(EXIT_FAILURE);
#endif

	/* Print error message */
	print_formatted_message(stderr, FAIL_MSG, "'%s': %s", 2, function_name, strerror(errno));
	exit(EXIT_FAILURE);
}

/*
 * This function is used to produce a random 'int' number in [min, max].
 *
 * @param: 'min' -> Minimum of the interval.
 * @param: 'max' -> Maximum of the interval.
 * @return: An int.
 */
int get_random_int(int min, int max) {

	struct timespec my_timespec;

	/* Verifies inputs */
	if (!(min < max))
		exit_failure("get_random_int");

	/* The functions 'clock_gettime()' retrieve the time of the specified clock 'clk_id' and store
	 * them into 'timespec' structure */
	clock_gettime(CLOCK_MONOTONIC, &my_timespec);

	/* To produce a different pseudo-random series each time, we call 'srand' with different 'seed' */
	srand(my_timespec.tv_nsec);

	return (rand() % ((max + 1) - min)) + min;
}

/*
 * This function is used to convert data stored into a 'timespec' struct into a readable string.
 *
 * @param: *ts -> A pointer to a timespec struct.
 * @return: A string.
 */
char *convert_timespec_to_string(struct timespec *ts) {

	const size_t size_buffer = strlen("2012-12-31 12:59:59") + 1;
	char *buffer = calloc(1, size_buffer * sizeof(char) + sizeof(char));

	/* ISO C `broken-down time' structure. */
	struct tm t;

	/* Convert 'timespec' struct into 'tm' struct */
	if (localtime_r(&(ts->tv_sec), &t) == NULL) {
		free(buffer);
		return "";
	}
	/* Convert data to string */
	if (strftime(buffer, size_buffer, "%F %T", &t) == 0) {
		free(buffer);
		return "";
	}
	return buffer;
}

/*
 * This function is used to concatenate a set of strings.
 *
 * @param n_args -> The amount of strings to concatenate.
 * @param ... -> Strings.
 * @return: A string
 */
char *concatenate_strings(int n_args, ...) {

	size_t memory_size = 0;
	va_list ap;

	/* Initialize 'va_list' */
	va_start(ap, n_args);

	/* Calc buffer size */
	for (register int i = 1; i <= n_args; i++)
		memory_size += (strlen(va_arg(ap, char *)) + 1);

	/* Allocate memory */
	char *output = calloc(1, memory_size * sizeof(char));
	if (output == NULL)
		exit_failure("calloc");

	/* Restart 'va_list' */
	va_start(ap, n_args);

	for (register int i = 1; i <= n_args; i++)
		strcat(output, va_arg(ap, char *));

	/* Free memory */
	va_end(ap);

	return output;
}

/*
 * This function is used to read a string from specified file descriptor.
 *
 * @param *fd -> A file descriptor.
 * @return: A string.
 */
char *read_string_from_file_descriptor(int fd) {

	size_t byte_read = 0;
	/* Default number of elements stored into output string */
	const size_t COUNT_ELEMENTS = 5;
	/* Read 'char' from file descriptor */
	char c;
	/* Allocation memory step */
	char step = 1;
	/* Allocation memory */
	char *output = calloc(COUNT_ELEMENTS, sizeof(char));
	if (output == NULL)
		exit_failure("calloc");

	/* Reading data... */
	/* ============================================================================ */
	for (size_t i = 0;; i += sizeof(char)) {

#ifdef TEST
		fprintf(stderr, "Iteration: %li\n", i);
#endif

		/* Read a 'char' */
		if ((byte_read = read(fd, &c, sizeof(char))) == -1)
			exit_failure("read");

		/* Stop reading line */
		if (c == '\n' || c == EOF || c == '\0' || byte_read == 0) {
			output[i] = '\0';
			break;
		}

#ifdef TEST
		fprintf(stderr, "Read char: %c\n", c);
#endif

		/* Add read char */
		output[i] = c;

		/* If space is insufficent, then realloc buffer... */
		/* ============================================================================ */
		if (i == ((step * COUNT_ELEMENTS) - sizeof(char))) {

#ifdef TEST
			fprintf(stderr, "step: %d\n", step);
#endif

			char *new_memory = calloc((step + 1) * COUNT_ELEMENTS, sizeof(char));
			if (new_memory == NULL)
				exit_failure("calloc");

			memcpy(new_memory, output, COUNT_ELEMENTS * step);
			free(output);
			output = new_memory;
			step++;
		}
	}
	return output;
}

/*
 * This function is used to convert a numeric data into a string.
 *
 * @param *arg -> A numeric value.
 * @return: A string.
 */
char *convert_int_to_string(int arg) {

	int n = snprintf(NULL, 0, "%d", arg);
	char *buf = calloc(n + 1, sizeof(char));
	snprintf(buf, n + 1, "%d", arg);

	return buf;
}

/*
 * This function is used to convert a string into a numeric data type.
 *
 * @param *arg -> A string.
 * @return: A 'long' data type.
 */
long string_to_long(char *arg) {

	char *endptr;

	/* To distinguish success/failure after call */
	errno = 0;
	long value = strtol(arg, &endptr, 10);

	/* Check for various possible errors */
	if (errno != 0 || endptr == arg)
		return -1;
	else
		return value;
}
