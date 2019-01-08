/*
 * directory_manager.h
 *
 *  Created on: 1 Oct 2017
 *      Author: Andrea Graziani
 */

#ifndef SRC_FILE_DIRECTORY_MANAGER_H_
#define SRC_FILE_DIRECTORY_MANAGER_H_

/* Macro */
#define APPLICATION_CLIENT_APP_DATA_FOLDER "client_data"
#define APPLICATION_SERVER_APP_DATA_FOLDER "server_data"
#define APPLICATION_CLIENT_FILE_FORLDER "client"
#define APPLICATION_SERVER_FILE_FORLDER "server"

/* Prototypes */
char *get_application_directory(char *folder);

#endif /* SRC_FILE_DIRECTORY_MANAGER_H_ */
