/*
 * data_transmission.c
 *
 *  Created on: 1 Oct 2017
 *      Author: Andrea Graziani
 */

/* Including libraries */
/* ============================================================================ */

#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>

/* Including project header files */
/* ============================================================================ */
#include "../project_property.h"
#include "../file/directory_manager.h"
#include "../user_interface/interface_functions.h"
#include "data_transmission.h"
#include "time_management.h"
#include "NetworkStatistics.h"

/* Macro */
/* ============================================================================ */
#define DATAGRAM_LOSS 0
#define DATAGRAM_RECEIVED 1

/* Global variables */
/* ============================================================================ */
unsigned char GLOBAL_LOSS_PROBABILITY = 0;
unsigned char GLOBAL_AUTOMATIC_TIMEOUT = 1;
unsigned long long int LOST_PACKAGES = 0;
unsigned long long int SENT_PACKAGES = 0;

/*
 * This function is used to simulate datagram loss.
 */
unsigned char __loss_datagram_simulation() {

	if (get_random_int(1, 100) <= GLOBAL_LOSS_PROBABILITY) {

#ifdef  VERBOSE
		fprintf(stderr, "DATAGRAM_LOSS\n");
#endif
		return DATAGRAM_LOSS;
	} else
		return DATAGRAM_RECEIVED;
}

/*
 * This function is used to receive a datagram.
 *
 */
int __receive_datagram(DataNetwork *DataNetwork_obj, void *buffer, size_t buffer_size, char *check_replay) {

	int bytes_received;

	/*
	 * Any process that knows the client's ephemeral port number could send datagrams to our client,
	 * and these would be intermixed with the normal server replies.
	 *
	 * We must ignore any received datagrams that are not from the server to whom we sent the datagram.
	 *
	 * This security mechanism is disabled when 'check_replay' is 0.
	 * ==================================================================================================  */

	struct sockaddr *preply_addr = malloc(DataNetwork_obj->address_len);
	if (preply_addr == NULL)
		exit_failure("malloc");

	do {
		/* Receiving data in 'check replay' mode */
		/* *********************************************************************************************** */
		bytes_received = recvfrom(DataNetwork_obj->socket_fd, buffer, buffer_size, 0, preply_addr, &DataNetwork_obj->address_len);

		/* Checking error */
		if (bytes_received == -1) {
			if (errno == EINTR) {
#ifdef  VERBOSE_DEBUG
				/* Time out message */
				print_formatted_message(stderr, ERR_MSG, "'recvfrom': Time out", 0);
#endif
				/* Free memory */
				free(preply_addr);

				return -1;
			} else
				exit_failure("recvfrom_wc");
		}

		if (check_replay == NULL || *check_replay == CHECK_REPLAY) {

			/* Check reply */
			/* *********************************************************************************************** */
			if (memcmp((struct sockaddr*) &(DataNetwork_obj->address), preply_addr, DataNetwork_obj->address_len) != 0) {
#ifdef  VERBOSE_DEBUG
				print_formatted_message(stderr, ERR_MSG, "--> Datagram Ignored...", 0);
#endif
				continue;
			}

		} else {

			memcpy(&DataNetwork_obj->address, preply_addr, sizeof(*preply_addr));

			/* Check replay... */
			*check_replay = CHECK_REPLAY;
		}
	} while (0);

	/* Free memory */
	free(preply_addr);

#ifdef  VERBOSE_DEBUG
	print_formatted_message(stderr, NULL, "Received datagram from:", 0);
	print_internet_socket_address_info(&DataNetwork_obj->address);
	print_formatted_message(stderr, NULL, "Bytes received: %d", 1, bytes_received);
#endif

	return bytes_received;
}

/*
 *
 */
int __send_datagram(DataNetwork *DataNetwork_obj, void *buffer, size_t buffer_size) {

	int bytes_sent;

	/* Sending data */
	/* ************************************************************ */
	bytes_sent = sendto(DataNetwork_obj->socket_fd, buffer, buffer_size, 0, (struct sockaddr*) &(DataNetwork_obj->address),
			DataNetwork_obj->address_len);
	if (bytes_sent == -1)
		exit_failure("sendto (send_datagram)");

#ifdef VERBOSE_DEBUG

	/* Print data */
	print_formatted_message(stderr, NULL, "Sent datagram to:", 0);
	print_internet_socket_address_info(&DataNetwork_obj->address);
	print_formatted_message(stderr, NULL, "Bytes sent: %d", 1, bytes_sent);

#endif

	return bytes_sent;
}

/*+
 *
 */
static void time_out_handler(int sig, siginfo_t *si, void *uc) {

	LOST_PACKAGES++;
	SENT_PACKAGES++;

	DatagramTimer *DatagramTimer_obj;
	DatagramTimer_obj = si->si_value.sival_ptr;

	/* Timeout */
	/* ============================================================================ */
	DatagramTimer_timeout(DatagramTimer_obj);

#ifdef VERBOSE
	/* # UNSAFE # */
	fprintf(stderr, "'timer_getoverrun': %d\n", timer_getoverrun(DatagramTimer_obj->timer));
	fprintf(stderr, "'times_retrasmitted': %d\n", DatagramTimer_obj->times_retrasmitted);
	fprintf(stderr, "'abort_trasmission_ptr': %d\n", *DatagramTimer_obj->abort_trasmission_ptr);
	fprintf(stderr, "Timeout --> sec: %li; nsec: %li\n", DatagramTimer_obj->timervals.it_value.tv_sec, DatagramTimer_obj->timervals.it_value.tv_nsec);
	fprintf(stderr, "Type: %c\n", DatagramTimer_obj->datagram_type);
#endif

	/* Send datagram, rearm timer and save 'sending' time */
	/* ============================================================================ */
	if (DatagramTimer_obj->datagram_type == STANDARD_DATAGRAM) {

		Datagram *x = (Datagram*) DatagramTimer_obj->Datagram_ptr;
		__send_datagram(DatagramTimer_obj->DataNetwork_ptr, x, x->len);

#ifdef VERBOSE
		/* # UNSAFE # */
		fprintf(stderr, "Send STANDARD message id: %d\n", x->id);
#endif

	} else {
		ControlDatagram *x = (ControlDatagram*) DatagramTimer_obj->Datagram_ptr;
		__send_datagram(DatagramTimer_obj->DataNetwork_ptr, x, sizeof(ControlDatagram));

#ifdef VERBOSE
		/* # UNSAFE # */
		fprintf(stderr, "Send CONTROL message type: %c\n", x->type);
#endif

	}

	if (timer_settime(DatagramTimer_obj->timer, 0, &DatagramTimer_obj->timervals, NULL) == -1)
		exit_failure("timer_create");

	/* If automatic timeout calculation is enabled, keep track of the sending time... */
	/* ---------------------------------------------------------------------------- */
	if (GLOBAL_AUTOMATIC_TIMEOUT)
		if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->sending_time) == -1)
			exit_failure("clock_gettime");
}

int sending_data_selective_repeat(DataNetwork *DataNetwork_obj, Settings *Settings_obj, int fd) {

	/* Restore variable... */
	LOST_PACKAGES = 0;
	SENT_PACKAGES = 0;

	Datagram *Datagram_obj;
	ControlDatagram *ControlDatagram_obj;
	DatagramTimer *DatagramTimer_obj;
	TimeInfo *TimeInfo_obj;
	DatagramTimer *DatagramTimer_control_obj;
	NetworkStatistics *NetworkStatistics_obj;

	/* Buffer used to store created 'Datagram' objects datagram until their ACK */
	void *Datagram_buffer;
	/* Buffer used to store 'DatagramTimer' objects */
	void *DatagramTimer_buffer;

	/* Following variable represents the lower extremity of sliding window */
	int l = 0;
	/* Following variable represents next ID packet to send */
	int n_id = 0;
	/* Following variable is used to flag as complete reading from input.*/
	char read_complete = 0;
	/* This variable is used to flag arrival of 'REQ ACK' datagram from client */
	char REQ_ACK_received = 0;
	/* This variable is used to flag sending of 'FIN' datagram to client */
	char FIN_sent = 0;
	/* This variable is used to ABORT transmission when client not respond */
	char abort_transmission = TRANSMISSION_SUCCESSFUL;

	/* Configuration setting */
	/* ============================================================================ */
	unsigned char SLIDING_WINDOWS = Settings_obj->sliding_window_size;

	GLOBAL_LOSS_PROBABILITY = Settings_obj->loss_probability;
	GLOBAL_AUTOMATIC_TIMEOUT = Settings_obj->auto_timeout;

	/* Allocation memory... */
	/* ============================================================================ */

	Datagram_buffer = malloc(SLIDING_WINDOWS * 2 * sizeof(Datagram));
	if (Datagram_buffer == NULL)
		exit_failure("calloc");

	DatagramTimer_buffer = calloc(1, SLIDING_WINDOWS * 2 * sizeof(DatagramTimer));
	if (DatagramTimer_buffer == NULL)
		exit_failure("calloc");

	DatagramTimer_control_obj = calloc(1, sizeof(DatagramTimer));
	if (DatagramTimer_control_obj == NULL)
		exit_failure("calloc");

	ControlDatagram_obj = calloc(1, sizeof(ControlDatagram));
	if (ControlDatagram_obj == NULL)
		exit_failure("calloc");

	TimeInfo_obj = calloc(1, sizeof(TimeInfo));
	if (TimeInfo_obj == NULL)
		exit_failure("calloc");

	NetworkStatistics_obj = calloc(1, sizeof(NetworkStatistics));
	if (NetworkStatistics_obj == NULL)
		exit_failure("calloc");

	/* Initialization memory and data structures... */
	/* ============================================================================ */

	/* The following instructions are used to invalidate 'Datagram' objects */
	memset(Datagram_buffer, -1, SLIDING_WINDOWS * 2 * sizeof(Datagram));

	/* Init 'TimeInfo_obj' */
	TimeInfo_init(TimeInfo_obj, Settings_obj->auto_timeout, Settings_obj->fixed_timeout);

	/* Init 'DatagramTimer' objects */
	for (int i = 0; i < SLIDING_WINDOWS * 2; i++) {

		/* Get poiners...*/
		DatagramTimer_obj = DatagramTimer_buffer + i * sizeof(DatagramTimer);
		Datagram_obj = Datagram_buffer + i * sizeof(Datagram);
		DatagramTimer_init(DatagramTimer_obj, (void **) &Datagram_obj, &DataNetwork_obj, &abort_transmission, STANDARD_DATAGRAM, time_out_handler);

		/* Confing RTO...*/
		memcpy(&DatagramTimer_obj->timervals.it_value, &TimeInfo_obj->RTO, sizeof(struct timespec));
		memcpy(&DatagramTimer_obj->timervals.it_interval, &TimeInfo_obj->RTO, sizeof(struct timespec));

	}

	/* Init 'DatagramTimer' for control 'Datagram' objects */
	DatagramTimer_init(DatagramTimer_control_obj, (void **) &ControlDatagram_obj, &DataNetwork_obj, &abort_transmission, CONTROL_DATAGRAM,
			time_out_handler);

	/* Confing RTO...*/
	memcpy(&DatagramTimer_control_obj->timervals.it_value, &TimeInfo_obj->RTO, sizeof(struct timespec));
	memcpy(&DatagramTimer_control_obj->timervals.it_interval, &TimeInfo_obj->RTO, sizeof(struct timespec));

	/* Get start transmission time */
	/* ============================================================================ */
	if (clock_gettime(CLOCK_REALTIME, &NetworkStatistics_obj->start_transmission_time) == -1)
		exit_failure("clock_gettime");

	/* ############################################################################ */
	/* START TRASMISSION */
	/* ############################################################################ */

	/* Data trasmission... */
	/* ============================================================================ */
	for (int i = 0; abort_transmission == TRANSMISSION_SUCCESSFUL; i++) {

#ifdef VERBOSE
		fprintf(stderr, "------------------------");
		fprintf(stderr, "Iteration n.%d------------------------\n", i);
		fprintf(stderr, "read complete: %d\n", read_complete);
		fprintf(stderr, "RTO: sec: %li, nsec %li\n", TimeInfo_obj->RTO.tv_sec, TimeInfo_obj->RTO.tv_nsec);
		fprintf(stderr, "Lower extremity of sliding window: %d\n", l);
		fprintf(stderr, "Next datagram ID: %d\n", n_id);
#endif

		/* ############################################################################ */
		/* ELABORATION DATA TO SEND */
		/* ############################################################################ */

		/* Sending REQ datagram to client... */
		/* =========================================================================================== */
		if (!REQ_ACK_received) {

			/* Send 'REQ' datagram */
			ControlDatagram_obj->type = REQ;

			__send_datagram(DataNetwork_obj, ControlDatagram_obj, sizeof(ControlDatagram));
			SENT_PACKAGES++;

			if (GLOBAL_AUTOMATIC_TIMEOUT)
				if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_control_obj->sending_time) == -1)
					exit_failure("clock_gettime");

			if (timer_settime(DatagramTimer_control_obj->timer, 0, &DatagramTimer_control_obj->timervals, NULL) == -1)
				exit_failure("timer_create");

#ifdef VERBOSE
			fprintf(stderr, "Sent 'REQ' datagram");
#endif

		}
		/* Creates a datagram and send them to client... */
		/* ============================================================================ */
		else if (((n_id >= l && n_id <= l + SLIDING_WINDOWS - 1) && n_id < SLIDING_WINDOWS * 2)
				|| ((n_id <= (l + SLIDING_WINDOWS - 1) % (SLIDING_WINDOWS * 2)) && ((l + SLIDING_WINDOWS - 1) > (SLIDING_WINDOWS * 2 - 1)))) {
#ifdef VERBOSE
			fprintf(stderr, "Processing datagram ID: %d\n", n_id);
#endif
			/* Creation packet to send */
			/* ---------------------------------------------------------------------------- */
			if (!read_complete) {
#ifdef VERBOSE
				fprintf(stderr, "reading\n");
#endif
				/* Update pointers */
				/* ---------------------------------------------------------------------------- */
				Datagram_obj = Datagram_buffer + n_id * sizeof(Datagram);
				DatagramTimer_obj = DatagramTimer_buffer + n_id * sizeof(DatagramTimer);

				/* Clear memory */
				memset(Datagram_obj->data, 0, PAYLOAD);

				/* Reading data from input file descriptor (if avaible) */
				/* ---------------------------------------------------------------------------- */
				int byte_read = read(fd, Datagram_obj->data, PAYLOAD);

				/* *** Checking 'read' errors... *** */

				/* CASE 1: 'read' fail */
				/* ---------------------------------------------------------------------------- */
				if (byte_read == -1)
					exit_failure("read");

				/* ---------------------------------------------------------------------------- */
				/* CASE 2: There are some data to sent but it was reach EOF for input file */
				else if (byte_read < PAYLOAD && byte_read > 0)
					read_complete = 1;

				/* CASE 3: No data has been read from input file descriptor */
				/* ---------------------------------------------------------------------------- */
				else if (byte_read == 0) {
					read_complete = 1;
					continue;
				}

				/* Add needed data */
				/* ---------------------------------------------------------------------------- */
				Datagram_obj->id = n_id;
				Datagram_obj->len = byte_read + sizeof(char);
				Datagram_obj->flag = 0;

				/* Confing RTO...*/
				memcpy(&DatagramTimer_obj->timervals.it_value, &TimeInfo_obj->RTO, sizeof(struct timespec));
				memcpy(&DatagramTimer_obj->timervals.it_interval, &TimeInfo_obj->RTO, sizeof(struct timespec));

				/* Send datagram, get a timestamp and start timer */
				/* ---------------------------------------------------------------------------- */

				/* Simulate datagram loss... */
				if (__loss_datagram_simulation() != DATAGRAM_LOSS) {

					__send_datagram(DataNetwork_obj, Datagram_obj, Datagram_obj->len);
#ifdef VERBOSE
					fprintf(stderr, "Sent datagram ID: %d\n", Datagram_obj->id);
#endif
				}

				/* If automatic timeout calculation is enabled, keep track of the sending time... */
				/* ---------------------------------------------------------------------------- */
				if (GLOBAL_AUTOMATIC_TIMEOUT)
					if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->sending_time) == -1)
						exit_failure("clock_gettime");

				if (timer_settime(DatagramTimer_obj->timer, 0, &DatagramTimer_obj->timervals, NULL) == -1)
					exit_failure("timer_create");

				/* Update Datagram ID counter */
				/* ---------------------------------------------------------------------------- */
				n_id = ((n_id + 1) == SLIDING_WINDOWS * 2) ? 0 : (n_id + 1);

				/* Update packets count */
				NetworkStatistics_obj->total_packages++;
				SENT_PACKAGES++;

#ifdef VERBOSE
				fprintf(stderr, "End reading\n");
#endif
			}
		}

#ifdef VERBOSE
		fprintf(stderr, "Receiving data...\n");
#endif

		/* ############################################################################################ */
		/* RECEIVE DATA FROM CLIENT */
		/* ############################################################################################ */
		if (__receive_datagram(DataNetwork_obj, ControlDatagram_obj, sizeof(ControlDatagram), NULL) == -1)
			continue;
#ifdef VERBOSE
		fprintf(stderr, "After receiving data...\n");
#endif
		/* ############################################################################ */
		/* ELABORATION RECEIVED DATA... */
		/* ############################################################################ */

		/* Received datagram type is: 'REQ ACK' */
		/* ============================================================================ */
		if (!REQ_ACK_received && ControlDatagram_obj->type == REQ_ACK) {

			/* Disarm timer, get timestamp and update RTO */
			/* ---------------------------------------------------------------------------- */
			DatagramTimer_control_obj->timervals.it_value.tv_sec = 0;
			DatagramTimer_control_obj->timervals.it_value.tv_nsec = 0;

			if (timer_settime(DatagramTimer_control_obj->timer, 0, &DatagramTimer_control_obj->timervals, NULL) == -1)
				exit_failure("timer_settime");

			/* If automatic timeout calculation is enabled, recalc RTO... */
			/* ---------------------------------------------------------------------------- */
			if (GLOBAL_AUTOMATIC_TIMEOUT) {
				if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_control_obj->receiving_time) == -1)
					exit_failure("clock_gettime");

				TimeInfo_calc_time(TimeInfo_obj, &DatagramTimer_control_obj->sending_time, &DatagramTimer_control_obj->receiving_time);
			}
			/* ... */
			DatagramTimer_control_obj->times_retrasmitted = 0;

			/* Flag */
			/* ---------------------------------------------------------------------------- */
			REQ_ACK_received = 1;

#ifdef VERBOSE
			fprintf(stderr, "Received datagram type is: 'REQ ACK'\n");
#endif
			continue;
		}
		/* Received datagram type is: 'ACK' */
		/* ============================================================================ */
		else if (REQ_ACK_received && !FIN_sent && ControlDatagram_obj->type == ACK) {

#ifdef VERBOSE
			fprintf(stderr, "Received datagram type is: 'ACK' -> ID: %d\n", ControlDatagram_obj->n);
#endif
			/* Disarm corresponding timer, get timestamp and update RTO */
			/* ---------------------------------------------------------------------------- */
			DatagramTimer_obj = DatagramTimer_buffer + ControlDatagram_obj->n * sizeof(DatagramTimer);

			DatagramTimer_obj->timervals.it_value.tv_sec = 0;
			DatagramTimer_obj->timervals.it_value.tv_nsec = 0;

			if (timer_settime(DatagramTimer_obj->timer, 0, &DatagramTimer_obj->timervals, NULL) == -1)
				exit_failure("timer_settime");

			/* If automatic timeout calculation is enabled, recalc RTO... */
			/* ---------------------------------------------------------------------------- */
			if (GLOBAL_AUTOMATIC_TIMEOUT) {
				if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->receiving_time) == -1)
					exit_failure("clock_gettime");

				TimeInfo_calc_time(TimeInfo_obj, &DatagramTimer_obj->sending_time, &DatagramTimer_obj->receiving_time);
			}

			DatagramTimer_obj->times_retrasmitted = 0;

			/* Get datagram corresponding to the received ACK and mark it as 'SUCCESSFULLY_RECEIVED';
			 * if possible, update lower extremity of sliding window and buffers */
			/* ---------------------------------------------------------------------------- */
			Datagram_obj = Datagram_buffer + ControlDatagram_obj->n * sizeof(Datagram);

			if (Datagram_obj->flag != INVALID)
				Datagram_obj->flag = SUCCESSFULLY_RECEIVED;

			while (Datagram_obj->id == l && Datagram_obj->flag == SUCCESSFULLY_RECEIVED) {

				Datagram_obj->flag = INVALID;
				l = ((l + 1) == SLIDING_WINDOWS * 2) ? 0 : (l + 1);
				Datagram_obj = Datagram_buffer + l * sizeof(Datagram);
			}

			/* Check if there are another datagram to send; otherwise send a FIN datagram */
			/* ---------------------------------------------------------------------------- */
			if (Datagram_obj->flag == INVALID && read_complete) {

				/* Confing RTO...*/
				memcpy(&DatagramTimer_control_obj->timervals.it_value, &TimeInfo_obj->RTO, sizeof(struct timespec));
				memcpy(&DatagramTimer_control_obj->timervals.it_interval, &TimeInfo_obj->RTO, sizeof(struct timespec));

				/* Send 'FIN' datagram to client */
				ControlDatagram_obj->type = FIN;
				__send_datagram(DataNetwork_obj, ControlDatagram_obj, sizeof(ControlDatagram));
				FIN_sent = 1;
				SENT_PACKAGES++;

				/* Start timer */
				if (timer_settime(DatagramTimer_control_obj->timer, 0, &DatagramTimer_control_obj->timervals, NULL) == -1)
					exit_failure("timer_create");

#ifdef VERBOSE
				fprintf(stderr, "Sent 'FIN' datagram\n");
#endif
			}
#ifdef VERBOSE
			fprintf(stderr, "UPDATE: Lower extremity of sliding window: %d\n", l);
#endif
		}
		/* Received datagram type is: 'FIN_ACK' */
		/* ============================================================================ */
		else if (FIN_sent && ControlDatagram_obj->type == FIN_ACK && REQ_ACK_received) {

			/* Disarm timer */
			/* ---------------------------------------------------------------------------- */
			DatagramTimer_control_obj->timervals.it_value.tv_sec = 0;
			DatagramTimer_control_obj->timervals.it_value.tv_nsec = 0;

			if (timer_settime(DatagramTimer_control_obj->timer, 0, &DatagramTimer_control_obj->timervals, NULL) == -1)
				exit_failure("timer_settime");

			/* Send a 'CLS' datagram */
			/* ---------------------------------------------------------------------------- */
			ControlDatagram_obj->type = CLS;
			__send_datagram(DataNetwork_obj, ControlDatagram_obj, sizeof(ControlDatagram));
			SENT_PACKAGES++;

#ifdef VERBOSE
			fprintf(stderr, "Received datagram type is: 'FIN ACK'\n");
			fprintf(stderr, "-> Sent a 'CLS' datagram! -> CLOSE TRASMISSION\n");
#endif
			break;
		}
	}

	/* Get end transmission time */
	/* ============================================================================ */
	if (clock_gettime(CLOCK_REALTIME, &NetworkStatistics_obj->end_transmission_time) == -1)
		exit_failure("clock_gettime");

	/* ############################################################################ */
	/* FREE MEMORY */
	/* ############################################################################ */

	/* Destroy all timers */
	/* ---------------------------------------------------------------------------- */
	for (int i = 0; i < SLIDING_WINDOWS * 2; i++) {
		/* Get poiners...*/
		DatagramTimer_obj = DatagramTimer_buffer + i * sizeof(DatagramTimer);
		if (timer_delete(DatagramTimer_obj->timer) == -1)
			exit_failure("timer_delete");
	}
	/* Destroy timer used for control datagram */
	if (timer_delete(DatagramTimer_control_obj->timer) == -1)
		exit_failure("timer_delete");

	/* Print statistics */
	NetworkStatistics_obj->total_packages_sent = SENT_PACKAGES;
	NetworkStatistics_obj->total_packages_lost = LOST_PACKAGES;
	print_network_statistics(NetworkStatistics_obj);

	/* Free memory */
	/* ---------------------------------------------------------------------------- */
	free(Datagram_buffer);
	free(DatagramTimer_buffer);
	free(ControlDatagram_obj);
	free(DatagramTimer_control_obj);
	free(TimeInfo_obj);
	free(NetworkStatistics_obj);

	return abort_transmission;
}

/*
 *
 */
static void time_out_handler_receiving(int sig, siginfo_t *si, void *uc) {

	DatagramTimer *DatagramTimer_obj;
	DatagramTimer_obj = si->si_value.sival_ptr;

	/* Timeout */
	/* ============================================================================ */
	DatagramTimer_timeout(DatagramTimer_obj);

#ifdef VERBOSE

	/* # UNSAFE # --> used ONLY for a simple debug */
	fprintf(stderr, "'timer_getoverrun': %d\n", timer_getoverrun(DatagramTimer_obj->timer));
	fprintf(stderr, "'times_retrasmitted': %d\n", DatagramTimer_obj->times_retrasmitted);
	fprintf(stderr, "'abort_trasmission_ptr': %d\n", *DatagramTimer_obj->abort_trasmission_ptr);
	fprintf(stderr, "Timeout --> sec: %li; nsec: %li\n", DatagramTimer_obj->timervals.it_value.tv_sec, DatagramTimer_obj->timervals.it_value.tv_nsec);
#endif

	if (timer_settime(DatagramTimer_obj->timer, 0, &DatagramTimer_obj->timervals, NULL) == -1)
		exit_failure("timer_create");
}

/*
 * int fd File descriptor of temporary file; it will be used to save all in-sequence packets.
 */
int receiving_data_selective_repeat(DataNetwork *DataNetwork_obj, Settings *Settings_obj, int fd, char *check_replay) {

	Datagram *Datagram_obj;
	Datagram *support_pointer;
	TimeInfo *TimeInfo_obj;
	DatagramTimer *DatagramTimer_obj;
	ControlDatagram *ControlDatagram_obj;
	void *Datagram_buffer;

	/* Following variable represents the lower extremity of sliding window */
	int l = 0;
	/* This variable is used to flag arrival of 'REQ ACK' datagram from server */
	char REQ_ACK_successfully_sended = 0;
	/* This variable is used to flag arrival of 'FIN' datagram from server */
	char FIN_received = 0;
	/* This variable is used to flag arrival of 'REQ' datagram from server */
	char REQ_received = 0;
	/* This variable is used to ABORT transmission when client not respond */
	char abort_transmission = 0;
	/* Byte received by client */
	size_t bytes_received;

	/* Configuration setting */
	/* ============================================================================ */
	unsigned char SLIDING_WINDOWS = Settings_obj->sliding_window_size;

	GLOBAL_LOSS_PROBABILITY = Settings_obj->loss_probability;
	GLOBAL_AUTOMATIC_TIMEOUT = Settings_obj->auto_timeout;

	/* Allocation memory... */
	/* ============================================================================ */

	Datagram_obj = calloc(1, sizeof(Datagram));
	if (Datagram_obj == NULL)
		exit_failure("calloc");

	Datagram_buffer = malloc(SLIDING_WINDOWS * 2 * sizeof(Datagram));
	if (Datagram_buffer == NULL)
		exit_failure("calloc");

	ControlDatagram_obj = calloc(1, sizeof(ControlDatagram));
	if (ControlDatagram_obj == NULL)
		exit_failure("calloc");

	TimeInfo_obj = calloc(1, sizeof(TimeInfo));
	if (TimeInfo_obj == NULL)
		exit_failure("calloc");

	DatagramTimer_obj = calloc(1, sizeof(DatagramTimer));
	if (DatagramTimer_obj == NULL)
		exit_failure("calloc");

	/* Initialization memory and data structures... */
	/* ============================================================================ */

	/* The following instructions is used to invalidate 'Datagram' objects */
	memset(Datagram_buffer, -1, SLIDING_WINDOWS * 2 * sizeof(Datagram));
	TimeInfo_init(TimeInfo_obj, Settings_obj->auto_timeout, Settings_obj->fixed_timeout);

	/* Initialization 'DatagramTimer_obj->sending_time' */
	if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->sending_time) == -1)
		exit_failure("clock_gettime");

	DatagramTimer_init(DatagramTimer_obj, NULL, NULL, &abort_transmission, CONTROL_DATAGRAM, time_out_handler_receiving);

	/* Confing RTO...*/
	memcpy(&DatagramTimer_obj->timervals.it_value, &TimeInfo_obj->RTO, sizeof(struct timespec));
	memcpy(&DatagramTimer_obj->timervals.it_interval, &TimeInfo_obj->RTO, sizeof(struct timespec));

	/* ############################################################################ */
	/* START TRASMISSION */
	/* ############################################################################ */

	/* Data trasmission... */
	/* ============================================================================ */
	for (int i = 0; abort_transmission == 0; i++) {

#ifdef VERBOSE
		fprintf(stderr, "------------------------");
		fprintf(stderr, "Iteration n.%d------------------------\n", i);
		fprintf(stderr, "# times to give up: %d\n", DatagramTimer_obj->times_retrasmitted);
		fprintf(stderr, "RTO: sec: %li, nsec %li\n", TimeInfo_obj->RTO.tv_sec, TimeInfo_obj->RTO.tv_nsec);
		fprintf(stderr, "Lower extremity of sliding window: %d\n", l);
		fprintf(stderr, "ABORT: %d\n", abort_transmission);
#endif

		if (timer_settime(DatagramTimer_obj->timer, 0, &DatagramTimer_obj->timervals, NULL) == -1)
			exit_failure("timer_settime");

		/* Receiving data */
		bytes_received = __receive_datagram(DataNetwork_obj, Datagram_obj, sizeof(Datagram), check_replay);

		if (bytes_received == -1)
			continue;

		/* Disarm timer, get timestamp and update RTO */
		/* ---------------------------------------------------------------------------- */
		DatagramTimer_obj->timervals.it_value.tv_sec = 0;
		DatagramTimer_obj->timervals.it_value.tv_nsec = 0;

		if (timer_settime(DatagramTimer_obj->timer, 0, &DatagramTimer_obj->timervals, NULL) == -1)
			exit_failure("timer_settime");

		if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->receiving_time) == -1)
			exit_failure("clock_gettime");

		TimeInfo_calc_time(TimeInfo_obj, &DatagramTimer_obj->sending_time, &DatagramTimer_obj->receiving_time);

		/* Confing RTO and arm timer */
		memcpy(&DatagramTimer_obj->timervals.it_value, &TimeInfo_obj->RTO, sizeof(struct timespec));
		memcpy(&DatagramTimer_obj->timervals.it_interval, &TimeInfo_obj->RTO, sizeof(struct timespec));

		/* ############################################################################ */
		/* RECEIVING DATA FROM SERVER */
		/* ############################################################################ */

		/* Determine datagram type */
		/* ============================================================================ */
		char x;
		memcpy(&x, Datagram_obj, sizeof(char));

		if (x == CLS || x == REQ || x == FIN)
			memcpy(ControlDatagram_obj, Datagram_obj, sizeof(ControlDatagram));
		else
			memcpy(&Datagram_obj->len, &bytes_received, sizeof(size_t));

		/* Received a datagram different by 'CLS' when 'FIN' is received */
		/* =========================================================================================== */
		if (REQ_received && FIN_received && ControlDatagram_obj->type != CLS) {
			break;
		}
		/* Received datagram type is: 'REQ' */
		/* ============================================================================ */
		else if (!REQ_ACK_successfully_sended && ControlDatagram_obj->type == REQ) {

#ifdef VERBOSE
			fprintf(stderr, "Received datagram type is: 'REQ'\n");
#endif

			/* Send a 'REQ ACK' datagram to server */
			/* ---------------------------------------------------------------------------- */
			ControlDatagram_obj->type = REQ_ACK;
			__send_datagram(DataNetwork_obj, ControlDatagram_obj, sizeof(ControlDatagram));
			REQ_received = 1;

			/* Update timer and restore # retransmission times */
			/* ---------------------------------------------------------------------------- */
			if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->sending_time) == -1)
				exit_failure("clock_gettime");
			DatagramTimer_obj->times_retrasmitted = 0;

#ifdef VERBOSE
			fprintf(stderr, "Sent a 'REQ ACK' datagram.\n");
#endif
		}
		/* Received datagram type is: 'FIN' */
		/* ============================================================================ */
		else if (REQ_received && REQ_ACK_successfully_sended && ControlDatagram_obj->type == FIN) {

			/* Send a 'FIN ACK' datagram to server */
			/* ---------------------------------------------------------------------------- */
			ControlDatagram_obj->type = FIN_ACK;
			__send_datagram(DataNetwork_obj, ControlDatagram_obj, sizeof(ControlDatagram));
			FIN_received = 1;

			/* Update timer and restore # retransmission times */
			/* ---------------------------------------------------------------------------- */
			if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->sending_time) == -1)
				exit_failure("clock_gettime");
			DatagramTimer_obj->times_retrasmitted = 5;

#ifdef VERBOSE
			fprintf(stderr, "Received datagram type is: 'FIN'\n");
			fprintf(stderr, "Sent a 'FIN ACK' datagram.\n");
#endif
		}
		/* Received datagram type is: 'CLS' */
		/* ============================================================================ */
		else if (REQ_received && REQ_ACK_successfully_sended && FIN_received && ControlDatagram_obj->type == CLS) {

#ifdef VERBOSE
			fprintf(stderr, "Received datagram type is: 'CLS'\n");
#endif
			/* Quitting */
			break;

		}
		/* Received standard datagram */
		/* ============================================================================ */
		else if (!FIN_received && REQ_received) {

#ifdef VERBOSE
			fprintf(stderr, "Arrived datagram ID: %d\n", Datagram_obj->id);
#endif
			/* Send ACK to server */
			/* ---------------------------------------------------------------------------- */
			ControlDatagram_obj->type = ACK;
			ControlDatagram_obj->n = Datagram_obj->id;

			if (__loss_datagram_simulation() != DATAGRAM_LOSS) {
				__send_datagram(DataNetwork_obj, ControlDatagram_obj, sizeof(ControlDatagram));
#ifdef VERBOSE
				fprintf(stderr, "Sent ACK ID: %d\n", ControlDatagram_obj->n);
#endif
			}

			/* An 'in-window' datagram was received */
			/* =========================================================================================== */
			if (((Datagram_obj->id >= l && Datagram_obj->id <= l + SLIDING_WINDOWS - 1) && Datagram_obj->id < SLIDING_WINDOWS * 2)
					|| ((Datagram_obj->id <= (l + SLIDING_WINDOWS - 1) % (SLIDING_WINDOWS * 2))
							&& ((l + SLIDING_WINDOWS - 1) > (SLIDING_WINDOWS * 2 - 1)))) {

				/* Update timer and restore # retransmission times */
				/* ---------------------------------------------------------------------------- */
				if (clock_gettime(CLOCK_REALTIME, &DatagramTimer_obj->sending_time) == -1)
					exit_failure("clock_gettime");
				DatagramTimer_obj->times_retrasmitted = 0;

				/* Update flag */
				/* ---------------------------------------------------------------------------- */
				REQ_ACK_successfully_sended = 1;
				Datagram_obj->flag = SUCCESSFULLY_RECEIVED;

				/* Write received datagram into buffer */
				/* ---------------------------------------------------------------------------- */
				int offset = Datagram_obj->id * sizeof(Datagram);
				support_pointer = Datagram_buffer + offset;
				memcpy(support_pointer, Datagram_obj, sizeof(Datagram));

				/* If avaible, get previously received datagrams from buffer */
				/* ---------------------------------------------------------------------------- */
				while (support_pointer->id == l && support_pointer->flag == SUCCESSFULLY_RECEIVED) {

					/* Write received data to disk */
					/* ---------------------------------------------------------------------------- */
					if (write(fd, support_pointer->data, (size_t) (support_pointer->len - sizeof(char))) == -1)
						exit_failure("write");
					support_pointer->flag = INVALID;

					/* Update lower extremity of sliding window */
					/* ---------------------------------------------------------------------------- */
					l = (((l + 1) == SLIDING_WINDOWS * 2) ? 0 : l + 1);

					/* Get next datagram from buffer */
					/* ---------------------------------------------------------------------------- */
					support_pointer = Datagram_buffer + (l * sizeof(Datagram));
				}
			}
		}
	}

	/* Destroy timer used for control datagram */
	if (timer_delete(DatagramTimer_obj->timer) == -1)
		exit_failure("timer_delete");

	/* Free memory */
	/* ---------------------------------------------------------------------------- */
	free(ControlDatagram_obj);
	free(Datagram_buffer);
	free(TimeInfo_obj);
	free(DatagramTimer_obj);
	free(Datagram_obj);

	/* Send resutl to caller */
	if (FIN_received == 1)
		return TRANSMISSION_SUCCESSFUL;
	else
		return TRANSMISSION_FAILED;
}

