#include "attendee.h"

void * attendeeThread(void * in) {
	attendee_t * self = (attendee_t *)in;

	pthread_exit(NULL);
}