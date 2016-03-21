#ifndef _ATTENDEE_H
#define _ATTENDEE_H

#include <pthread.h>

typedef enum attendeeState_t {
	AS_ENTER,
	AS_FINDRIDE,
	AS_ONRIDE,
	AS_EXIT
} attendeeState;

typedef struct {
	pthread_t threadID;
	char name;
	int delay;
	int speed;
	attendeeState state;
	int numRides;
	int currRide;
	int * rides;
	int xpos;
	int ypos;
} attendee_t;

void * attendeeThread(void *);

#endif