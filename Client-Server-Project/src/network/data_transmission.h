/*
 * data transmission.h
 *
 *  Created on: 2 Oct 2017
 *      Author: Andrea Graziani
 */

#ifndef SRC_NETWORK_DATA_TRANSMISSION_H_
#define SRC_NETWORK_DATA_TRANSMISSION_H_

/* Including libraries */
/* ============================================================================ */
#include <limits.h>

/* Including project header files */
/* ============================================================================ */
#include "DataNetwork.h"
#include "../file/Settings.h"

/* Macro */
/* ============================================================================ */

#define NO_CHECK_FIRST_REPLAY 0
#define CHECK_REPLAY 1

#define SERVER_COMMAND_PORT 1993

#define CONTROL_DATAGRAM 'C'
#define STANDARD_DATAGRAM 'S'

#define PAYLOAD 1500
#define SUCCESSFULLY_RECEIVED 'O'
#define INVALID -1
#define REQ 'R'
#define REQ_ACK 'S'
#define FIN 'F'
#define FIN_ACK 'X'
#define CLS 'C'
#define ACK 'A'
#define NACK 'N'

/*
 * This struct represents a request datagram
 */
typedef struct request_client_packet {

	Settings client_setting;
	char request_type;
	char request_type_argument[NAME_MAX];

} RequestPacket;

/*
 * This struct represents a control datagram
 */
typedef struct control_datagram {
	char type;
	char n;
} ControlDatagram;

/*
 * This struct represents a standard datagram
 */
typedef struct datagram {
	char id;
	char data[PAYLOAD];
	char flag;
	size_t len;
} Datagram;

/* Prototypes */
/* ============================================================================ */
int receiving_data_selective_repeat(DataNetwork *DataNetwork_obj, Settings *Settings_obj, int fd, char *check_replay);
int sending_data_selective_repeat(DataNetwork *DataNetwork_obj, Settings *Settings_obj, int fd);

#endif /* SRC_NETWORK_DATA_TRANSMISSION_H_ */
