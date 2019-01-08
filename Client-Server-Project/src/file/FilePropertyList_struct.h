/*
 * FilePropertyList_struct.h
 *
 *  Created on: 27 Sep 2017
 *      Author: Andrea Graziani
 */

#ifndef SRC_FILE_FILEPROPERTYLIST_STRUCT_H_
#define SRC_FILE_FILEPROPERTYLIST_STRUCT_H_

/* Including libraries */
#include <sys/stat.h>


/* Types */
typedef struct file_property {

	struct stat file_stat;
	char *file_name;

} FileProperty;

typedef struct file_property_list {

	FileProperty *data;

	/* pointer to next node */
	struct file_property_list *next;

} FilePropertyList;


/* Prototypes */
void FilePropertyList_clear(FilePropertyList **head_node);
void FilePropertyList_insert(FilePropertyList **head_node, FileProperty *data);
void FilePropertyList_serializer(FilePropertyList** input, int fd);
FilePropertyList *FilePropertyList_deserializer(int fd);
FilePropertyList *get_file_information(const char *directory);
int check_regular_file(const char *path);

#endif /* SRC_FILE_FILEPROPERTYLIST_STRUCT_H_ */
