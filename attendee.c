#include "attendee.h"
#include "ride.h"
#include "log.h"
#include "safeScreen.h"
#include <stdio.h>
#include <unistd.h>

#define LOCAL_LOG_BUFF_SIZE 128

extern ride_t * rides[10];

void * attendeeThread(void * in) {
	attendee_t * self = (attendee_t *)in;
	char buff[LOCAL_LOG_BUFF_SIZE];
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c: delay %d, speed %d, start position %d, %d rides assigned", self->name, self->delay, self->speed, self->xpos, self->numRides);
	writeToLog(buff);
	usleep(self->delay * 1000);
	while (' ' != safe_get_screen_char(FALSE, self->xpos, self->ypos)) { }
	safe_set_screen_char(FALSE, self->xpos, self->ypos, self->name);
	safe_update_screen();
	snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c started", self->name);
	writeToLog(buff);

	int new_col, new_row;
	do {
		self->state = AS_FINDRIDE;
		char target = '0' + self->rides[self->currRide];
		new_col = self->xpos;
		new_row = self->ypos;
		while (TRUE) {
			lockScreen();
			safe_find_target(TRUE, target, &new_col, &new_row);
			if (safe_move_to_target(TRUE, self->xpos, self->ypos, &new_col, &new_row)) {
				safe_set_screen_char(TRUE, self->xpos, self->ypos, ' ');
				unlockScreen();
				self->xpos = -1;
				self->ypos = -1;
				self->state = AS_ONRIDE;
				snprintf(buff, LOCAL_LOG_BUFF_SIZE, "Attendee %c entered ride %d", self->name, self->rides[self->currRide]);
				writeToLog(buff);
				pthread_exit(NULL);		// found the target, now we should go to the next one, make the next target the exit?
			} else {
				safe_set_screen_char(TRUE, self->xpos, self->ypos, ' ');
				safe_set_screen_char(TRUE, new_col, new_row, self->name);
				unlockScreen();
				self->xpos = new_col;
				self->ypos = new_row;
			}
			safe_update_screen();
			usleep(self->speed * 1000);
		}
	} while (self->currRide++ < self->numRides);

	pthread_exit(NULL);
}