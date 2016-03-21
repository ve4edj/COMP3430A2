#ifndef _RIDE_H
#define _RIDE_H

#include <pthread.h>
#include "attendee.h"

typedef struct {
	pthread_t threadID;
	int number;
	int timeout;
	int duration;
	int numRiders;
	int currRider;
	attendee_t ** riders;
} ride_t;

void * rideThread(void *);

#endif