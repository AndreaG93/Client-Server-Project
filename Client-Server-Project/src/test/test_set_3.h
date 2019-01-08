/*
 * test_set_3.h
 *
 *  Created on: Dec 17, 2017
 *      Author: andrea
 */

#ifndef TEST_TEST_SET_3_H_
#define TEST_TEST_SET_3_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "../network/client/client_manager.h"
#include "../network/DataNetwork.h"

void __TEST_1_client_initialization_for_transmission()
{
    /* Retrieve data about server... */
	DataNetwork *DataNetwork_obj = client_initialization_for_transmission("localhost");

	/* Send a 'GET' request... */
	client_start(DataNetwork_obj, 'g', "Shiny Chariot");
}

/*
 * This function is used to run a test set.
 */
void TEST_SET_3() {
	__TEST_1_client_initialization_for_transmission();
}

#endif /* TEST_TEST_SET_3_H_ */
