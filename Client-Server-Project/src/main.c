/*
 * main.c
 *
 * Created on: 26 Sep 2017
 * Author: Andrea Graziani - 0189326
 */

#include "user_interface/interfaces.h"
#include "project_property.h"

#include <stdio.h>

#ifdef TEST

#include "test/test_set_1.h"
#include "test/test_set_2.h"
#include "test/test_set_3.h"

#endif

int main() {

#ifdef TEST
	fprintf(stderr, "TEST_SET_1\n");
	TEST_SET_1();
	fprintf(stderr, "--> OK\n\n");

	fprintf(stderr, "TEST_SET_2\n");
	TEST_SET_2();
	fprintf(stderr, "--> OK\n\n");

	fprintf(stderr, "TEST_SET_3\n");
	TEST_SET_3();
	fprintf(stderr, "--> OK\n\n");
	return 0;
#endif

	main_interface();

	return 0;

}

