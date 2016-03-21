#include "ride.h"
#include "log.h"
#include <stdio.h>
#include <pthread.h>

#define LOCAL_LOG_BUFF_SIZE 64

void * rideThread(void * in) {
	ride_t * self = (ride_t *)in;
	char buff[LOCAL_LOG_BUFF_SIZE];
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Ride %d: timeout %d, duration %d, %d riders", self->number, self->timeout, self->duration, self->numRiders);
	writeToLog(buff);
	// use a semaphore to wait for all the riders to be in
	pthread_exit(NULL);
}