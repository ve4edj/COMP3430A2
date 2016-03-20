#include "ride.h"
#include "screen.h"

void * rideThread(void * in) {
	ride_t * self = (ride_t *)in;
	// use a semaphone to wait for all the riders to be in
	pthread_exit(NULL);
}