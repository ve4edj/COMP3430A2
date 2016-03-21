#ifndef _RIDE_H
#define _RIDE_H

#include "attendee.h"

typedef struct {
	int number;
	int timeout;
	int duration;
	int numRiders;
	int currRider;
	attendee_t ** riders;
} ride_t;

void * rideThread(void *);

#endif