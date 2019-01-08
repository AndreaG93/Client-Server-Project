/*
 * test_set_2.h
 *
 *  Created on: Dec 16, 2017
 *      Author: andrea
 */

#ifndef TEST_TEST_SET_2_H_
#define TEST_TEST_SET_2_H_

#include "../project_property.h"
#include "../network/time_management.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

void __TEST_1_concatenate_strings() {

	const char *str_n1 = "Shiny";
	const char *str_n2 = "Chariot";

	/* Concatenate strings */
	char *my_string = concatenate_strings(3, str_n1, " ", str_n2);

	assert(strcmp(my_string, "Shiny Chariot") == 0);
	assert(my_string[13] == '\0');

	/* Free unused memory */
	free(my_string);
}

void __TEST_2_convert_int_to_string() {

	const int x = 7;

	char *my_string = convert_int_to_string(x);

	assert(strcmp(my_string, "7") == 0);
	assert(my_string[1] == '\0');

	/* Free unused memory */
	free(my_string);
}

void __TEST_3_read_string_from_file_descriptor() {

	const char *str_n1 = "Diana Cavendish";

	/* Preparing a temporary file  */
	/* ============================================================================ */
	FILE *temp_file = tmpfile();
	int temp_file_fd = fileno(temp_file);
	if (temp_file_fd == -1)
		exit_failure("fileno");

	/* Write data */
	/* ============================================================================ */
	if (write(temp_file_fd, str_n1, strlen(str_n1) * sizeof(char)) == -1)
		exit_failure("write");

	if (lseek(temp_file_fd, 0, SEEK_SET) == -1)
		exit_failure("lseek");

	/* Read data */
	/* ============================================================================ */
	char *my_string = read_string_from_file_descriptor(temp_file_fd);
	assert(strcmp(my_string, str_n1) == 0);

	/* Close temp file descriptor */
	if (fclose(temp_file) == -1)
		exit_failure("close");

	/* Free unused memory */
	free(my_string);
}

/*
 * This function is used to run a test set.
 */
void TEST_SET_2() {
	__TEST_1_concatenate_strings();
	__TEST_2_convert_int_to_string();
	__TEST_3_read_string_from_file_descriptor();
}

#endif /* TEST_TEST_SET_2_H_ */
