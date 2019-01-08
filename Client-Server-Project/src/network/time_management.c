/*
 * TimeManager.c
 *
 *  Created on: Oct 23, 2017
 *      Author: Andrea Graziani
 */

/* Including project header files */
/* ============================================================================ */
#include "time_management.h"

#include "DataNetwork.h"
#include "data_transmission.h"
#include "../project_property.h"

/* Including libraries */
/* ============================================================================ */
#include <syscall.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
 * This function is used to checking if values stored into specified 'timespec' struct
 * is within the permitted time limit.
 *
 * @param *input -> It represents a 'timepsec' object.
 */
void __timespec_checking_limit(struct timespec *input) {

	if (input->tv_sec + (input->tv_nsec * pow(10, -9)) > RTT_RETRANSMIT_MAX) {

		input->tv_sec = RTT_RETRANSMIT_MAX;
		input->tv_nsec = 0;
	}
	else if (input->tv_sec * pow(10, 9) + input->tv_nsec < RTT_RETRANSMIT_MIN) {
		input->tv_sec = 0;
		input->tv_nsec = RTT_RETRANSMIT_MIN;
	}
}

/*
 * This function is used to convert nanoseconds stored into a 'timespec' struct into seconds.
 *
 * @param *input -> It represents a 'timepsec' object.
 */
void __timespec_converting_from_ns_to_s(struct timespec *input) {

	while (input->tv_nsec >= pow(10, 9)) {
		input->tv_sec += 1;
		input->tv_nsec -= pow(10, 9);
	}
}

/*
 * This function is used to initialize a 'TimeInfo' struct.
 *
 * @param *input -> It represents a 'TimeInfo' object.
 * @param using_automatic_RTO -> It represents a 'boolean' object.
 * @param RTO_specified_by_user -> RTO specified by user (immutable if 'using_automatic_RTO' == False).
 */
void TimeInfo_init(TimeInfo *input, boolean using_automatic_RTO, unsigned long RTO_specified_by_user) {

	/* Setting initial values... */
	/* ============================================================================ */
	input->Estimated_RTT.tv_sec = 0;
	input->Estimated_RTT.tv_nsec = 0;

	input->Dev_RTT.tv_sec = 0;
	input->Dev_RTT.tv_nsec = 0;

	/* According to RCF 6298, initial RTO is 1 second.
	 * If user 'using_automatic_RTO == False', then set RTO specified by user...*/
	/* ============================================================================ */
	if (using_automatic_RTO) {
		input->RTO.tv_sec = 1;
		input->RTO.tv_nsec = 0;
	} else {
		input->RTO.tv_sec = 0;
		input->RTO.tv_nsec = RTO_specified_by_user;

		__timespec_checking_limit(&input->RTO);
		__timespec_converting_from_ns_to_s(&input->RTO);
	}
}

/*
 *
 */
void DatagramTimer_init(DatagramTimer *DatagramTimer_obj, void **Datagram_ptr, DataNetwork **DataNetwork_ptr, char *abort_trasmission_ptr,
		char datagram_type, void (*handler)(int, siginfo_t*, void*)) {

	struct sigevent sev;
	struct sigaction sa;

	/* Configuration 'DatagramTimer' */
	/* ============================================================================ */
	if (DataNetwork_ptr != NULL)
		DatagramTimer_obj->DataNetwork_ptr = *DataNetwork_ptr;
	if (Datagram_ptr != NULL)
		DatagramTimer_obj->Datagram_ptr = *Datagram_ptr;

	DatagramTimer_obj->abort_trasmission_ptr = abort_trasmission_ptr;
	DatagramTimer_obj->datagram_type = datagram_type;

	/* Establish handler for notification signal */
	/* ============================================================================ */
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGRTMAX);
	if (sigaction(SIGRTMAX, &sa, NULL) == -1)
		exit_failure("sigaction");

	/* Configuration structure to transport application-defined values with signals. */
	/* ============================================================================ */
	sev.sigev_notify = SIGEV_THREAD_ID;
	sev._sigev_un._tid = syscall(SYS_gettid);
	sev.sigev_signo = SIGRTMAX;
	sev.sigev_value.sival_ptr = DatagramTimer_obj;

	/* Create new per-process timer using CLOCK_ID. */
	/* ============================================================================ */
	if (timer_create(CLOCK_REALTIME, &sev, &DatagramTimer_obj->timer) == -1)
		exit_failure("timer_create");
}

/*
 *
 */
void DatagramTimer_start_timer(DatagramTimer *obj) {

	if (timer_settime(obj->timer, 0, &obj->timervals, NULL) == -1)
		exit_failure("timer_create");
}

/*
 * This function is used to manage a timeout.
 *
 * @param *input -> It represents a 'DatagramTimer' object.
 */
void DatagramTimer_timeout(DatagramTimer *input) {

	/* The current RTO is doubled: This is the exponential backoff. */
	/* ============================================================================ */
	input->timervals.it_value.tv_sec *= 2;
	input->timervals.it_value.tv_nsec *= 2;

	/* Checking */
	__timespec_converting_from_ns_to_s(&input->timervals.it_value);
	__timespec_checking_limit(&input->timervals.it_value);

	/* Copy 'it_interval' into 'it_value' */
	memcpy(&input->timervals.it_interval, &input->timervals.it_value, sizeof(struct timespec));

	/* If we have reached the maximum number of retransmissions tell to caller to give up. */
	if (++input->times_retrasmitted > MAX_RETRANSMIT)
		*input->abort_trasmission_ptr = TRANSMISSION_FAILED;

#ifdef TEST
	return;
#endif

	/* Set the time until the next expiration */
	if (timer_settime(input->timer, 0, &input->timervals, NULL) == -1)
		exit_failure("timer_settime (DatagramTimer_timeout) ");

}

/*
 * Updates RTT estimators and calculates new RTO.
 *
 * @param *input -> It represents a 'TimeInfo' object.
 * @param *sending_time -> It represents a 'timespec' object.
 * @param *receiving_time -> It represents a 'timespec' object.
 */
void TimeInfo_calc_time(TimeInfo *input, struct timespec *sending_time, struct timespec *receiving_time) {

	const float alpha = 0.125;
	const float beta = 0.25;

	struct timespec RTT;

	/* Calculation */
	/* ============================================================================ */

	/* Calculate 'Round Trip Time' */
	RTT.tv_sec = receiving_time->tv_sec - sending_time->tv_sec;
	RTT.tv_nsec = receiving_time->tv_nsec - sending_time->tv_nsec;

	/* Calculate weighted average of RTT values */
	input->Estimated_RTT.tv_sec = (1.0 - alpha) * input->Estimated_RTT.tv_sec + alpha * RTT.tv_sec;
	input->Estimated_RTT.tv_nsec = (1.0 - alpha) * input->Estimated_RTT.tv_nsec + alpha * RTT.tv_nsec;

	/* Calculate variance */
	input->Dev_RTT.tv_sec = (1.0 - beta) * input->Dev_RTT.tv_sec + beta * abs(RTT.tv_sec - input->Estimated_RTT.tv_sec);
	input->Dev_RTT.tv_nsec = (1.0 - beta) * input->Dev_RTT.tv_nsec + beta * abs(RTT.tv_nsec - input->Estimated_RTT.tv_nsec);

	/* Calculate RTO */
	input->RTO.tv_sec = input->Estimated_RTT.tv_sec + (4.0 * input->Dev_RTT.tv_sec);
	input->RTO.tv_nsec = input->Estimated_RTT.tv_nsec + (4.0 * input->Dev_RTT.tv_nsec);

	/* Converting nanosec to sec */
	/* ============================================================================ */
	__timespec_converting_from_ns_to_s(&input->RTO);
	__timespec_converting_from_ns_to_s(&input->Estimated_RTT);
	__timespec_converting_from_ns_to_s(&input->Dev_RTT);

	/* Checking if RTO is within the permitted limits... */
	/* ============================================================================ */
	__timespec_checking_limit(&input->RTO);
}
