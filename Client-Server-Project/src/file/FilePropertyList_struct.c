/*
 * FilePropertyList_struct.c
 *
 * Created on: 27 Sep 2017
 * Author: Andrea Graziani - 0189326
 */

/* Including project header files */
/* ============================================================================ */
#include "../project_property.h"
#include "FilePropertyList_struct.h"

/* Including libraries */
/* ============================================================================ */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>

/*
 * This function is used to clear a 'FilePropertyList' data structure.
 *
 * @param: **head_node -> Pointer to a 'FilePropertyList' data structure.
 */
void FilePropertyList_clear(FilePropertyList **head_node) {

	/* pointer to next node in list */
	FilePropertyList *next_node;

	while (*head_node != NULL) {
		next_node = (*head_node)->next;
		free(*head_node);
		*head_node = next_node;
	}
}

/*
 * This function is used to insert data into specified 'FilePropertyList' data structure.
 *
 * @param: **head_node -> Pointer to a 'FilePropertyList' data structure.
 * @param: *data -> Pointer to data to insert.
 */
void FilePropertyList_insert(FilePropertyList **head_node, FileProperty *data) {

	/* pointer to new node */
	FilePropertyList *new_node;
	/* pointer to new node */
	FilePropertyList *prev_node;
	/* pointer to current node in list */
	FilePropertyList *current_node;

	/* Allocate memory */
	new_node = calloc(1, sizeof(FilePropertyList));
	new_node->data = data;
	new_node->next = NULL;

	/* Find the correct location in the list */
	current_node = *head_node;
	prev_node = NULL;

	while (current_node != NULL
			&& (strcmp(current_node->data->file_name, data->file_name) < 0)) {
		prev_node = current_node;
		current_node = current_node->next;
	}

	/* Case 1: Insert node at the beginning of the list */
	if (prev_node == NULL) {
		new_node->next = *head_node;
		*head_node = new_node;
	}
	/* Case 2: Insert node between two node */
	else {
		prev_node->next = new_node;
		new_node->next = current_node;
	}
}

/*
 * This function is used to serialize data stored into specified 'FilePropertyList'.
 *
 * @param: **input -> Pointer to a 'FilePropertyList' data structure.
 * @param: fd -> A already opened file descriptor.
 */
void FilePropertyList_serializer(FilePropertyList** input, int fd) {

	/* How many items stored into 'FilePropertyList **input' */
	size_t items = 0;
	/* Alias for 'FilePropertyList **input' */
	FilePropertyList *support = *input;

	/* Size of fields */
	size_t sz_stat = sizeof(struct stat);

	/* Count counts how many items are into 'FilePropertyList **input'  */
	while (support != NULL) {

		/* Update pointer */
		support = support->next;
		/* Update counter */
		items++;
	}

#ifdef DEBUG_FILE_PROPERTY_SERIALIZER
	fprintf(stderr, "DEBUG_FILE_PROPERTY_SERIALIZER: 'items': %lu\n",
			items);
#endif

	/* Restore pointer */
	support = *input;

	/* Serialize on a file
	 *
	 * The first 4 byte of this memory is used to stored the number of items stored in input
	 * The following 4 byte are used to store the size of 'file_name' size.
	 * */

	/* Write information about number of items and size of 'file_name' */
	if (write(fd, &(items), sizeof(int)) == -1)
		exit_failure("write");

	/* Write data to memory */
	for (int i = 0; i < items; i++) {

		/* Null character */
		char x = '\0';

		if (write(fd, &(support->data->file_stat), sz_stat) == -1)
			exit_failure("write");
		if (write(fd, support->data->file_name,
				strlen(support->data->file_name)*sizeof(char)) == -1)
			exit_failure("write");
		if (write(fd, &x, sizeof(char)) == -1)
			exit_failure("write");
		support = support->next;
	}
}

/*
 * This function is used to deserialize a 'FilePropertyList' data structure.
 *
 * @param: fd -> A file descriptor.
 */
FilePropertyList *FilePropertyList_deserializer(int fd) {

	/* A 'FilePropertyList' structure */
	FilePropertyList *output = NULL;
	/* Items stored into 'void **input' */
	size_t items = 0;

	/* Read data about the number of items and the size of 'file_name_field' */
	if (read(fd, &items, sizeof(int)) == -1)
		exit_failure("read");

#ifdef DEBUG_FILE_PROPERTY_DESERIALIZER
	fprintf(stderr, "DEBUG_FILE_PROPERTY_DESERIALIZER: 'items': %lu\n",
			items);
#endif

	/* Read data from memory */
	for (int i = 0; i < items; i++) {

		/* Declaration data structure */
		FileProperty *FileProperty_obj;

		/* Allocation memory */
		FileProperty_obj = calloc(1, sizeof(FileProperty));
		if (FileProperty_obj == NULL)
			exit_failure("calloc");

		/* Read data */
		if (read(fd, &(FileProperty_obj->file_stat), sizeof(struct stat)) == -1)
			exit_failure("read");

		FileProperty_obj->file_name = read_string_from_file_descriptor(fd);

#ifdef DEBUG_FILE_PROPERTY_DESERIALIZER
		assert(FileProperty_obj->file_name != NULL);
		fprintf(stderr,
				"DEBUG_FILE_PROPERTY_DESERIALIZER: 'obj->file_name': %s\n",
				FileProperty_obj->file_name);
		fprintf(stderr,
				"DEBUG_FILE_PROPERTY_DESERIALIZER: 'obj->file_stat.st_size': %lu\n",
				FileProperty_obj->file_stat.st_size);
#endif

		/* Insert into list */
		FilePropertyList_insert(&output, FileProperty_obj);
	}
	return output;
}

/*
 * This is used to check if specified file is regular.
 *
 * @param: *path -> A string containing file directory.
 *
 */
int check_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

/*
 * This function is used to retrieve information about files stored into application's folder.
 */
FilePropertyList *get_file_information(const char *directory) {

	/* 'dirent' structure */
	struct dirent *entry;
	/* 'FilePropertyList' list */
	FilePropertyList *FileProperty_list = NULL;

	/*
	 * The 'opendir()' function opens the directory specified by dirpath and returns a
	 * pointer to a structure of type DIR.
	 */
	DIR *shared_directory = opendir(directory);
	if (shared_directory == NULL)
		exit_failure("opendir");

	/*
	 * Each call to readdir() reads the next directory from the directory stream referred to
	 * by 'shared_directory' and returns a pointer to a statically allocated structure of type 'dirent'.
	 */
	for (int i = 0; (entry = readdir(shared_directory)) != NULL; i++)
		if (entry->d_type == DT_REG) {

#ifdef DEBUG_GET_FILE_INFORMATION
			fprintf(stderr, "\nFound file #%d\n", i);
#endif

			/* Allocate memory for a new 'FileNode' */
			FileProperty *obj = calloc(1, (sizeof(FileProperty)));

			/* Allocate memory for a full path file */
			char *full_path_file = concatenate_strings(3, directory, "/",
					entry->d_name);

			/* Copy data */
			obj->file_name = entry->d_name;

			if (stat(full_path_file, &(obj->file_stat)) == -1)
				exit_failure("stat");

#ifdef DEBUG_GET_FILE_INFORMATION
			fprintf(stderr, "->  Name file: %s\n", obj->file_name);
			fprintf(stderr, "->  Size file: %lu\n", obj->file_stat.st_size);
#endif

			/* Insert into list */
			FilePropertyList_insert(&FileProperty_list, obj);

			/* free memory */
			free(full_path_file);
		}

	/* This function closes the directory stream dirstream. */
	if (closedir(shared_directory) == -1)
		exit_failure("closedir");

#ifdef DEBUG_GET_FILE_INFORMATION

	FilePropertyList *support = FileProperty_list;
	fprintf(stderr, "\nReading from data structure.\n");

	while (support != NULL) {
		fprintf(stderr, "->  Name file: %s\n", support->data->file_name);
		fprintf(stderr, "->  size: %lu\n", support->data->file_stat.st_size);

		support = support->next;
	}

	fprintf(stderr, "\nEnd Reading from data structure.\n");

#endif

	/* File name */
	return FileProperty_list;
}
