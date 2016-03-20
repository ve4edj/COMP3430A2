#ifndef _RIDE_H
#define _RIDE_H

#include <pthread.h>
#include "attendee.h"

typedef struct {
	int number;
	int timeout;
	int duration;
	int maxRiders;
	attendee_t ** riders;
} ride_t;

void * rideThread(void *);

#endif