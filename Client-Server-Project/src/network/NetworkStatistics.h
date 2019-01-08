/*
 * NetworkStatistics.h
 *
 *  Created on: Dec 16, 2017
 *      Author: andrea
 */

#ifndef NETWORK_NETWORKSTATISTICS_H_
#define NETWORK_NETWORKSTATISTICS_H_

#include <time.h>


typedef struct network_statistics {

	unsigned long long int total_packages;
	unsigned long long int total_packages_sent;
	unsigned long long int total_packages_lost;

	struct timespec start_transmission_time;
	struct timespec end_transmission_time;

} NetworkStatistics;

void print_network_statistics(NetworkStatistics *NetworkStatistics_obj);

#endif /* NETWORK_NETWORKSTATISTICS_H_ */
