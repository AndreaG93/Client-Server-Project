/*
 * Settings.c
 *
 * Created on: Oct 30, 2017
 * Author: Andrea Graziani - 0189326
 */

/* Including project header files */
/* ============================================================================ */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* Including libraries */
/* ============================================================================ */
#include "directory_manager.h"
#include "Settings.h"
#include "../project_property.h"

/* Macro */
/* ============================================================================ */
#define DEFAULT_SLIDING_WINDOW_SIZE 20
#define DEFAULT_LOSS_PROBABILITY 0
#define DEFAULT_AUTO_TIMEOUT 1
#define DEFAULT_FIXED_TIMEOUT 0

/*
 * This function is used to save settings to disk.
 *
 * @param: *Settings_obj -> A pointer to a 'Settings' struct.
 */
void save_application_settings(Settings *Settings_obj) {

	int fd;

	/* Get directory where store metadata file */
	char *dir_temp = get_application_directory(APPLICATION_CLIENT_APP_DATA_FOLDER);
	/* Generate temporary file full path*/
	char *setting_file_path = concatenate_strings(2, dir_temp, "ClientApplication.conf");

	/* Open settings file (if exists)... */
	/* ============================================================================ */
	fd = open(setting_file_path, O_WRONLY | O_TRUNC);
	if (fd == -1)
		exit_failure("open");

	/* Write data on disk */
	if (write(fd, &Settings_obj->sliding_window_size, sizeof(unsigned char)) == -1)
		exit_failure("write");

	if (write(fd, &Settings_obj->loss_probability, sizeof(unsigned char)) == -1)
		exit_failure("write");

	if (write(fd, &Settings_obj->auto_timeout, sizeof(unsigned char)) == -1)
		exit_failure("write");

	if (write(fd, &Settings_obj->fixed_timeout, sizeof(unsigned long)) == -1)
		exit_failure("write");

	/* Free memory */
	free(dir_temp);
	free(setting_file_path);

	/* Closing file */
	if (close(fd) == -1)
		exit_failure("close");
}

/*
 * This function is used to retrieve settings from disk.
 *
 * @param: *Settings_obj -> A pointer to a 'Settings' struct where store data.
 */
void get_application_settings(Settings *Settings_obj) {

	int fd;

	/* Get directory where store metadata file */
	char *dir_temp = get_application_directory(APPLICATION_CLIENT_APP_DATA_FOLDER);
	/* Generate temporary file full path*/
	char *setting_file_path = concatenate_strings(2, dir_temp, "ClientApplication.conf");

	/* Open settings file (if exists)... */
	/* ============================================================================ */
	fd = open(setting_file_path, O_RDONLY);
	if (fd == -1 && errno != ENOENT)
		exit_failure("open");

	/* Otherwise create a new setting file */
	/* ============================================================================ */
	if (errno == ENOENT) {
		fd = open(setting_file_path, O_WRONLY | O_CREAT | O_TRUNC,
		S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd == -1 && errno != 0)
			exit_failure("open");

		/* Populate 'Settings' struct */
		Settings_obj->sliding_window_size = DEFAULT_SLIDING_WINDOW_SIZE;
		Settings_obj->loss_probability = DEFAULT_LOSS_PROBABILITY;
		Settings_obj->fixed_timeout = DEFAULT_FIXED_TIMEOUT;
		Settings_obj->auto_timeout = DEFAULT_AUTO_TIMEOUT;

		/* Write data on disk */
		if (write(fd, &Settings_obj->sliding_window_size, sizeof(unsigned char)) == -1)
			exit_failure("write");

		if (write(fd, &Settings_obj->loss_probability, sizeof(unsigned char)) == -1)
			exit_failure("write");

		if (write(fd, &Settings_obj->auto_timeout, sizeof(unsigned char)) == -1)
			exit_failure("write");

		if (write(fd, &Settings_obj->fixed_timeout, sizeof(unsigned long)) == -1)
			exit_failure("write");

	} else {

		/* Read data from disk */
		if (read(fd, &Settings_obj->sliding_window_size, sizeof(unsigned char)) == -1)
			exit_failure("read");

		if (read(fd, &Settings_obj->loss_probability, sizeof(unsigned char)) == -1)
			exit_failure("read");

		if (read(fd, &Settings_obj->auto_timeout, sizeof(unsigned char)) == -1)
			exit_failure("read");

		if (read(fd, &Settings_obj->fixed_timeout, sizeof(unsigned long)) == -1)
			exit_failure("read");
	}

#ifdef TEST
	fprintf(stderr, "loss_probability:    %d\n", Settings_obj->loss_probability);
	fprintf(stderr, "fixed_timeout:       %lu\n", Settings_obj->fixed_timeout);
	fprintf(stderr, "auto_timeout:        %d\n", Settings_obj->auto_timeout);
	fprintf(stderr, "sliding_window_size: %d\n", Settings_obj->sliding_window_size);
#endif

	/* Free memory */
	free(dir_temp);
	free(setting_file_path);

	/* Closing file */
	if (close(fd) == -1)
		exit_failure("close");
}

