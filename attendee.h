/*
		Course:			COMP3430
		Assignment:		2 - Amusement Park Simulator
		Due date:		Tuesday, March 22 2016
		Instructor:		John Braico
		Programmed By:	Erik Johnson
		Student #:		7711697
*/

#ifndef _ATTENDEE_H
#define _ATTENDEE_H

#include <pthread.h>

typedef enum attendeeState_t {
	AS_ENTER,
	AS_FINDRIDE,
	AS_WAITFORRIDE,
	AS_ONRIDE,
	AS_RIDEFINISHED,
	AS_FINDEXIT,
	AS_EXIT
} attendeeState;

typedef struct {
	char name;
	int delay;
	int speed;
	attendeeState state;
	int numRides;
	int currRide;
	int * rides;
	int xpos;
	int ypos;
	int wantsToLeave;
	pthread_mutex_t attendeeMutex;
	pthread_cond_t rideFinished;
} attendee_t;

void * attendeeThread(void *);

#endif