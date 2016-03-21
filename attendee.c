#include "attendee.h"
#include "ride.h"
#include "log.h"
#include "safeScreen.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

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
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c starting: delay %d, speed %d, start position %d, %d rides assigned", self->name, self->delay, self->speed, self->xpos, self->numRides);
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
			// wait until there is room on the ride
			self->state = AS_ONRIDE;
			snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c entered ride %d", self->name, self->rides[self->currRide]);
			writeToLog(buff);
			break;
		case AS_ONRIDE:
			// wait until the ride is over
			self->state = AS_RIDEFINISHED;
			break;
		case AS_RIDEFINISHED:
			// find the exit from the ride
			if (self->wantsToLeave)
				self->state = AS_FINDEXIT;
			else if (self->currRide++ < self->numRides)
				self->state = AS_FINDRIDE;
			else
				self->state = AS_FINDEXIT;
			break;
		case AS_FINDEXIT:
			// navigate towards the nearest exit
			self->state = AS_EXIT;
			break;
		case AS_EXIT:
		default:
			break;
		}
	}
	pthread_exit(NULL);
}