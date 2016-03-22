/*
		Course:			COMP3430
		Assignment:		2 - Amusement Park Simulator
		Due date:		Tuesday, March 22 2016
		Instructor:		John Braico
		Programmed By:	Erik Johnson
		Student #:		7711697
*/

#include "attendee.h"
#include "ride.h"
#include "log.h"
#include "safeScreen.h"
#include <stdio.h>
#include <unistd.h>

#define LOCAL_LOG_BUFF_SIZE 128

extern ride_t * rides[10];

int moveTowardsTarget(attendee_t * self, char target) {
	int result = 0;
	int new_col = self->xpos;
	int new_row = self->ypos;
	lockScreen();
	safe_find_target(TRUE, target, &new_col, &new_row);
	if (safe_move_to_target(TRUE, self->xpos, self->ypos, &new_col, &new_row)) {
		safe_set_screen_char(TRUE, self->xpos, self->ypos, ' ');
		result = 1;
	} else {
		safe_set_screen_char(TRUE, self->xpos, self->ypos, ' ');
		safe_set_screen_char(TRUE, new_col, new_row, self->name);
	}
	unlockScreen();
	safe_update_screen();
	self->xpos = new_col;
	self->ypos = new_row;
	return result;
}

void * attendeeThread(void * in) {
	attendee_t * self = (attendee_t *)in;
	char buff[LOCAL_LOG_BUFF_SIZE];
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c - thread loaded: delay %d, speed %d, start position %d, %d rides assigned", self->name, self->delay, self->speed, self->xpos, self->numRides);
	writeToLog(buff);
	usleep(self->delay * 1000);

	while (self->state != AS_EXIT) {
		switch (self->state) {
		case AS_ENTER:
			while (' ' != safe_get_screen_char(FALSE, self->xpos, self->ypos)) { }
			safe_set_screen_char(FALSE, self->xpos, self->ypos, self->name);
			safe_update_screen();
			snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c entered the park", self->name);
			writeToLog(buff);
			self->state = AS_FINDRIDE;
			break;
		case AS_FINDRIDE:
			if (self->wantsToLeave) {
				self->state = AS_FINDEXIT;
				break;
			}
			if (moveTowardsTarget(self, '0' + self->rides[self->currRide]))
				self->state = AS_WAITFORRIDE;
			usleep(self->speed * 1000);
			break;
		case AS_WAITFORRIDE:
			pthread_mutex_lock(&rides[self->rides[self->currRide]]->rideMutex);
			rides[self->rides[self->currRide]]->riders[rides[self->rides[self->currRide]]->currRider++] = self;				// add self to the ride's riders list at the next available position
			pthread_cond_signal(&rides[self->rides[self->currRide]]->riderAdded);
			pthread_mutex_unlock(&rides[self->rides[self->currRide]]->rideMutex);
			snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c entered ride %d", self->name, self->rides[self->currRide]);
			writeToLog(buff);
			self->state = AS_ONRIDE;
			break;
		case AS_ONRIDE:
			pthread_mutex_lock(&self->attendeeMutex);
			pthread_cond_wait(&self->rideFinished, &self->attendeeMutex);
			pthread_mutex_unlock(&self->attendeeMutex);
			self->state = AS_RIDEFINISHED;
			break;
		case AS_RIDEFINISHED:
			self->xpos = rides[self->rides[self->currRide]]->exitX;
			self->ypos = rides[self->rides[self->currRide]]->exitY;
			int found = 0;
			while (!found) {
				found = 1;
				lockScreen();
				if (' ' == safe_get_screen_char(TRUE, self->xpos, self->ypos + 1)) { self->ypos++; }
				else if (' ' == safe_get_screen_char(TRUE, self->xpos + 1, self->ypos + 1)) { self->xpos++; self->ypos++; }
				else if (' ' == safe_get_screen_char(TRUE, self->xpos - 1, self->ypos + 1)) { self->xpos--; self->ypos++; }
				else if (' ' == safe_get_screen_char(TRUE, self->xpos + 1, self->ypos)) { self->xpos++; }
				else if (' ' == safe_get_screen_char(TRUE, self->xpos - 1, self->ypos)) { self->xpos--; }
				else if (' ' == safe_get_screen_char(TRUE, self->xpos, self->ypos - 1)) { self->ypos--; }
				else if (' ' == safe_get_screen_char(TRUE, self->xpos + 1, self->ypos - 1)) { self->xpos++; self->ypos--; }
				else if (' ' == safe_get_screen_char(TRUE, self->xpos - 1, self->ypos - 1)) { self->xpos--; self->ypos--; }
				else {
					found = 0;
					unlockScreen();
					usleep(10);
				}
			}
			safe_set_screen_char(TRUE, self->xpos, self->ypos, self->name);
			unlockScreen();
			snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c left ride %d", self->name, self->rides[self->currRide]);
			writeToLog(buff);
			if (++(self->currRide) < self->numRides)
				self->state = AS_FINDRIDE;
			else
				self->state = AS_FINDEXIT;
			break;
		case AS_FINDEXIT:
			if (moveTowardsTarget(self, '~'))
				self->state = AS_EXIT;
			usleep(self->speed * 1000);
			break;
		case AS_EXIT:
		default:
			break;
		}
	}
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c left the park", self->name);
	writeToLog(buff);
	pthread_exit(NULL);
}