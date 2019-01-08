/*
 * client_manager.h
 *
 *  Created on: 27 Sep 2017
 *      Author: Andrea Graziani
 */

#ifndef SRC_NETWORK_CLIENT_CLIENT_MANAGER_H_
#define SRC_NETWORK_CLIENT_CLIENT_MANAGER_H_

#include "../DataNetwork.h"

void client_start(DataNetwork *DataNetwork_obj, char command, char *command_argument);
DataNetwork *client_initialization_for_transmission(char *host_ip);

#endif /* SRC_NETWORK_CLIENT_CLIENT_MANAGER_H_ */
