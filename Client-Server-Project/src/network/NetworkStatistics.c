/*
 * NetworkStatistics.c
 *
 *  Created on: Dec 17, 2017
 *      Author: andrea
 */

#include "../user_interface/interface_functions.h"
#include "NetworkStatistics.h"
#include <stdlib.h>

void print_network_statistics(NetworkStatistics *NetworkStatistics_obj) {

	print_horizontal_line("-");
	print_formatted_message(stderr, STD_MSG, "Network statistics:", 0);
	print_formatted_message(stderr, NULL, "Total data packages: %lli", 1, NetworkStatistics_obj->total_packages);
	print_formatted_message(stderr, NULL, "Total packages sent: %lli", 1, NetworkStatistics_obj->total_packages_sent);
	print_formatted_message(stderr, NULL, "Timeouts occurred: %lli", 1, NetworkStatistics_obj->total_packages_lost);
	print_formatted_message(stderr, NULL, "Time (s): %li", 1,
			NetworkStatistics_obj->end_transmission_time.tv_sec - NetworkStatistics_obj->start_transmission_time.tv_sec);

	long x = NetworkStatistics_obj->end_transmission_time.tv_nsec;
	long y = NetworkStatistics_obj->start_transmission_time.tv_nsec;

	if (x > y) {
		print_formatted_message(stderr, NULL, "Time (ns): %li", 1, x - y);
	} else
		print_formatted_message(stderr, NULL, "Time (ns): %li", 1, y - x);
	print_horizontal_line("-");
}

