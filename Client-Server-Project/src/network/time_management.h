/*
 * TimeManager.h
 *
 *  Created on: Oct 23, 2017
 *      Author: Andrea Graziani
 */

#ifndef NETWORK_TIME_MANAGEMENT_H_
#define NETWORK_TIME_MANAGEMENT_H_

/* Including libraries */
/* ============================================================================ */
#include <time.h>
#include <signal.h>

/* Including project header files */
/* ============================================================================ */
#include "DataNetwork.h"
#include "data_transmission.h"
#include "../project_property.h"

/* Macro */
/* ============================================================================ */
/* max retransmit timeout value, in seconds */
#define RTT_RETRANSMIT_MAX 2
/* min retransmit timeout value, in nanoseconds */
#define RTT_RETRANSMIT_MIN 1000000
/* max # times to retransmit */
#define MAX_RETRANSMIT 30

/* Types */
/* ============================================================================ */
typedef struct time_info {

	/* Current 'Recovery Time Objective' (RTO) to use */
	struct timespec RTO;

	/* Weighted average of RTT values */
	struct timespec Estimated_RTT;
	/* Variance */
	struct timespec Dev_RTT;

} TimeInfo;

typedef struct datagram_timer {

	void *Datagram_ptr;
	DataNetwork *DataNetwork_ptr;
	char *abort_trasmission_ptr;

	char datagram_type;
	timer_t timer;
	char times_retrasmitted;
	struct itimerspec timervals;
	struct timespec sending_time;
	struct timespec receiving_time;

} DatagramTimer;

/* Prototypes */
/* ============================================================================ */
void DatagramTimer_init(DatagramTimer *DatagramTimer_obj, void **Datagram_ptr, DataNetwork **DataNetwork_ptr, char *abort_trasmission_ptr,
		char datagram_type, void (*handler)(int, siginfo_t*, void*));
void TimeInfo_init(TimeInfo *input, boolean using_automatic_RTO, unsigned long RTO_specified_by_user);
void TimeInfo_calc_time(TimeInfo *input, struct timespec *sending_time, struct timespec *receiving_time);
void DatagramTimer_timeout(DatagramTimer *input);
void DatagramTimer_start_timer(DatagramTimer *obj);


#endif /* NETWORK_TIME_MANAGEMENT_H_ */
