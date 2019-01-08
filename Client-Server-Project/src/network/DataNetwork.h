/*
 * DataNetwork.h
 *
 *  Created on: Oct 16, 2017
 *      Author: Andrea Graziani
 */

#ifndef NETWORK_DATANETWORK_H_
#define NETWORK_DATANETWORK_H_

/* Including libraries */
/* ============================================================================ */
#include <netinet/in.h>

typedef struct data_network {

	/* Socket file descriptor */
	int socket_fd;
	/* Internet socket address */
	struct sockaddr_in address;
	/* Internet socket address length */
	socklen_t address_len;

} DataNetwork;

#endif /* NETWORK_DATANETWORK_H_ */
