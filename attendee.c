#include "attendee.h"
#include "log.h"
#include <stdio.h>

void * attendeeThread(void * in) {
	attendee_t * self = (attendee_t *)in;
	char buff[128];
	snprintf(buff, 128, "Attendee %c: delay %d, speed %d, start position %d, %d rides assigned", self->name, self->delay, self->speed, self->xpos, self->numRides);
	writeToLog(buff);
	// do things
	pthread_exit(NULL);
}