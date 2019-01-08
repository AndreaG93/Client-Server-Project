/*
 * thread.h
 *
 * Created on: 10 Oct 2017
 * Author: Andrea Graziani - 0189326
 */

#ifndef SRC_THREAD_THREAD_H_
#define SRC_THREAD_THREAD_H_

pthread_t thread_initialization(void *(*thread_routine)(void *), void **thread_argument);

#endif /* SRC_THREAD_THREAD_H_ */
