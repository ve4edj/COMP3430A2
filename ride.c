#include "ride.h"
#include "log.h"
#include "safeScreen.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#define LOCAL_LOG_BUFF_SIZE 128

int hasTimeoutElapsed(ride_t * self, struct timeval * startTime, int timeout) {
	struct timeval now;
	gettimeofday(&now, NULL);
	if ((((now.tv_sec - startTime->tv_sec) * 1000) + ((now.tv_usec - startTime->tv_usec) / 1000)) >= timeout)
		return 1;
	return 0;
}

void * rideThread(void * in) {
	ride_t * self = (ride_t *)in;
	char buff[LOCAL_LOG_BUFF_SIZE];
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Ride %d - thread loaded: timeout %d, duration %d, %d riders", self->number, self->timeout, self->duration, self->numRiders);
	writeToLog(buff);
	while (1) {
		struct timeval timeout;
		pthread_mutex_lock(&self->rideMutex);
		self->running = 0;
		do {
			gettimeofday(&timeout, NULL);
			pthread_cond_wait(&self->riderAdded, &self->rideMutex);
		} while (!(self->triggered) && (self->currRider < self->numRiders) && !hasTimeoutElapsed(self, &timeout, self->timeout));
		self->triggered = 0;
		self->running = 1;
		snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Ride %d started", self->number);
		writeToLog(buff);
		gettimeofday(&timeout, NULL);
		char rideName[2] = {'\0'};
		rideName[0] = self->number + '0';
		while (!(self->triggered) && !hasTimeoutElapsed(self, &timeout, self->duration)) {
			safe_blink_screen(rideName);
			usleep(self->duration * 10);
		}
		snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Ride %d stopped", self->number);
		writeToLog(buff);
		for (int i = 0; i < self->currRider; i++) {
			attendee_t * at = self->riders[i];
			pthread_mutex_lock(&at->attendeeMutex);
			pthread_cond_signal(&at->rideFinished);
			pthread_mutex_unlock(&at->attendeeMutex);
			self->riders[i] = NULL;
			usleep(at->speed * 100);
		}
		self->currRider = 0;
		self->triggered = 0;
		pthread_mutex_unlock(&self->rideMutex);
	}
	pthread_exit(NULL);
}