/*
 * thread.c
 *
 * Created on: 10 Oct 2017
 * Author: Andrea Graziani - 0189326
 */

/* Including project header files */
/* ============================================================================ */
#include "../project_property.h"

/* Including libraries */
/* ============================================================================ */
#include <pthread.h>

/**
 * This function is used to create a new thread.
 *
 * @param: *(*thread_routine)(void *) -> Thread job.
 * @param: **thread_argument -> Argument to pass to a thread.
 *
 * @return: A 'pthread_t' data type contains thread id.
 */
pthread_t thread_initialization(void *(*thread_routine)(void *), void **thread_argument) {

	/* this variable represents thread_identifier */
	pthread_t thread_identifier;
	/* this variable represents thread attribute's struct */
	pthread_attr_t thread_attributes;

	/* this variable is used to check output */
	int output;

	/* set thread attributes to default */
	output = pthread_attr_init(&thread_attributes);
	if (output != 0)
		exit_failure("pthread_attr_init");

	/* create thread */
	output = pthread_create(&thread_identifier, &thread_attributes, thread_routine, thread_argument);
	if (output != 0)
		exit_failure("pthread_create");

	return thread_identifier;
}
