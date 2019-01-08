/*
 * test_set_1.h
 *
 *  Created on: Dec 16, 2017
 *      Author: andrea
 */

#ifndef TEST_TEST_SET_1_H_
#define TEST_TEST_SET_1_H_

#include "../project_property.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../network/time_management.h"

void __TEST_1_timeout_time_estimator() {

	TimeInfo *TimeInfo_obj;
	struct timespec *sending_time;
	struct timespec *receiving_time;

	/* Allocation memory */
	/* ============================================================================ */

	/* Allocates a block long enough to contain 'TimeInfo' struct */
	TimeInfo_obj = calloc(1, sizeof(TimeInfo));
	if (TimeInfo_obj == NULL)
		exit_failure("calloc");

	/* Allocates a block long enough to contain 'TimeInfo' struct */
	receiving_time = calloc(1, sizeof(struct timespec));
	if (TimeInfo_obj == NULL)
		exit_failure("calloc");

	/* Allocates a block long enough to contain 'TimeInfo' struct */
	sending_time = calloc(1, sizeof(struct timespec));
	if (TimeInfo_obj == NULL)
		exit_failure("calloc");

	/* Initialization */
	/* ============================================================================ */
	TimeInfo_init(TimeInfo_obj, TRUE, 0);

	/* Firts RTT: 1 ms */
	/* ============================================================================ */
	sending_time->tv_nsec = 0;
	receiving_time->tv_nsec = 1000000;

	/* Calculation...*/
	TimeInfo_calc_time(TimeInfo_obj, sending_time, receiving_time);

	assert(TimeInfo_obj->Estimated_RTT.tv_sec == 0);
	assert(TimeInfo_obj->Estimated_RTT.tv_nsec == 125000);

	assert(TimeInfo_obj->Dev_RTT.tv_sec == 0);
	assert(TimeInfo_obj->Dev_RTT.tv_nsec == 218750);

	assert(TimeInfo_obj->RTO.tv_sec == 0);
	assert(TimeInfo_obj->RTO.tv_nsec == 1000000);

	/* Second RTT: 2 ms */
	/* ============================================================================ */
	sending_time->tv_nsec = 0;
	receiving_time->tv_nsec = 2000000;

	/* Calculation...*/
	TimeInfo_calc_time(TimeInfo_obj, sending_time, receiving_time);

	assert(TimeInfo_obj->Estimated_RTT.tv_sec == 0);
	assert(TimeInfo_obj->Estimated_RTT.tv_nsec == 359375);

	assert(TimeInfo_obj->Dev_RTT.tv_sec == 0);
	assert(TimeInfo_obj->Dev_RTT.tv_nsec == 574218); /* To be precise: 574218,75 */

	assert(TimeInfo_obj->RTO.tv_sec == 0);
	assert(TimeInfo_obj->RTO.tv_nsec == 2656247); /* To be precise: 2656250 */

	/* Third RTT: 1000 ms */
	/* ============================================================================ */
	sending_time->tv_nsec = 0;
	receiving_time->tv_nsec = 1000000000;

	/* Calculation...*/
	TimeInfo_calc_time(TimeInfo_obj, sending_time, receiving_time);

	assert(TimeInfo_obj->Estimated_RTT.tv_sec == 0);
	assert(TimeInfo_obj->Estimated_RTT.tv_nsec == 125314453); /* To be precise: 125314453.1 */

	assert(TimeInfo_obj->Dev_RTT.tv_sec == 0);
	assert(TimeInfo_obj->Dev_RTT.tv_nsec == 219102055); /* To be precise: 219102050,3 */

	assert(TimeInfo_obj->RTO.tv_sec == 1);
	assert(TimeInfo_obj->RTO.tv_nsec == 1722673);
}

void __TEST_2_timeout_time_estimator() {

	TimeInfo *TimeInfo_obj;

	/* Allocation memory */
	/* ============================================================================ */

	/* Allocates a block long enough to contain 'TimeInfo' struct */
	TimeInfo_obj = calloc(1, sizeof(TimeInfo));
	if (TimeInfo_obj == NULL)
		exit_failure("calloc");

	/* Initialization */
	/* ============================================================================ */
	TimeInfo_init(TimeInfo_obj, FALSE, 9000000000);

	assert(TimeInfo_obj->RTO.tv_sec == 2);
	assert(TimeInfo_obj->RTO.tv_nsec == 0);
}

void __TEST_3_DatagramTimer_timeout() {

	DatagramTimer *DatagramTimer_obj;

	/* Allocation memory */
	/* ============================================================================ */

	/* Allocates a block long enough to contain 'DatagramTimer' struct */
	DatagramTimer_obj = calloc(1, sizeof(DatagramTimer));
	if (DatagramTimer_obj == NULL)
		exit_failure("calloc");

	/* Initialization */
	/* ============================================================================ */
	DatagramTimer_obj->timervals.it_value.tv_nsec = 500000000;
	DatagramTimer_obj->timervals.it_value.tv_sec = 1;
	DatagramTimer_obj->times_retrasmitted = 0;

	DatagramTimer_timeout(DatagramTimer_obj);

	assert(DatagramTimer_obj->timervals.it_value.tv_nsec == 0);
	assert(DatagramTimer_obj->timervals.it_value.tv_sec == 2);

	DatagramTimer_timeout(DatagramTimer_obj);

	assert(DatagramTimer_obj->timervals.it_value.tv_nsec == 0);
	assert(DatagramTimer_obj->timervals.it_value.tv_sec == 2);

	/* Retransmission times */
	assert(DatagramTimer_obj->times_retrasmitted == 2);
}

/*
 * This function is used to run a test set.
 */
void TEST_SET_1() {
	__TEST_1_timeout_time_estimator();
	__TEST_2_timeout_time_estimator();
	__TEST_3_DatagramTimer_timeout();
}

#endif /* TEST_TEST_SET_1_H_ */
