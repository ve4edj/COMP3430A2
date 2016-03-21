#include "attendee.h"
#include "log.h"
#include <stdio.h>
#include <unistd.h>

#define LOCAL_LOG_BUFF_SIZE 128

void * attendeeThread(void * in) {
	attendee_t * self = (attendee_t *)in;
	char buff[LOCAL_LOG_BUFF_SIZE];
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c: delay %d, speed %d, start position %d, %d rides assigned", self->name, self->delay, self->speed, self->xpos, self->numRides);
	writeToLog(buff);
	usleep(self->delay * 1000);
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c started", self->name);
	writeToLog(buff);
	// do things
	pthread_exit(NULL);
}