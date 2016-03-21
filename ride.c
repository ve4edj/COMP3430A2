#include "ride.h"
#include "log.h"
#include <stdio.h>

void * rideThread(void * in) {
	ride_t * self = (ride_t *)in;
	char buff[64];
	snprintf(buff, 64, "Ride %d: timeout %d, duration %d, %d riders", self->number, self->timeout, self->duration, self->numRiders);
	writeToLog(buff);
	// use a semaphore to wait for all the riders to be in
	pthread_exit(NULL);
}