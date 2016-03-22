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
} ride_t;

void * rideThread(void *);

#endif