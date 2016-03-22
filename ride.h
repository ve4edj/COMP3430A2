/*
		Course:			COMP3430
		Assignment:		2 - Amusement Park Simulator
		Due date:		Tuesday, March 22 2016
		Instructor:		John Braico
		Programmed By:	Erik Johnson
		Student #:		7711697
*/

#ifndef _RIDE_H
#define _RIDE_H

#include "attendee.h"
#include <pthread.h>

typedef struct {
	int number;
	int timeout;
	int duration;
	int numRiders;
	int currRider;
	attendee_t ** riders;
	int exitX;
	int exitY;
	pthread_mutex_t rideMutex;
	pthread_cond_t riderAdded;
	int triggered;
	int running;
} ride_t;

void * rideThread(void *);

#endif