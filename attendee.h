#ifndef _ATTENDEE_H
#define _ATTENDEE_H

#include <pthread.h>

typedef struct {
	char name;
	pthread_t threadID;
	int delay;
	int speed;
	int currRide;
	int numRides;
	int * rides;
	int xpos;
} attendee_t;

void * attendeeThread(void *);

#endif