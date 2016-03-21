#ifndef _ATTENDEE_H
#define _ATTENDEE_H

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
} attendee_t;

void * attendeeThread(void *);

#endif