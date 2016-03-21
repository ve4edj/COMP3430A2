#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "safeScreen.h"
#include "screen.h"
#include "attendee.h"
#include "ride.h"
#include "log.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_RIDES 10
#define MAX_ATTENDEES 52
ride_t * rides[MAX_RIDES];
pthread_t rideThreads[MAX_RIDES];
attendee_t * attendees[MAX_ATTENDEES];
pthread_t attendeeThreads[MAX_ATTENDEES];

pthread_t kbThread, screenThread, logThread;

void loadPark(char * parkFile) {
	initialize_screen();
	if (load_screen(parkFile) < 0) {
		perror("Unable to load map file");
		finish_screen();
		exit(EXIT_FAILURE);
	}
}

int attendeeNameToIdx(char atName) {
	if ('a' <= atName && 'z' >= atName)
		return atName - 'a';
	if ('A' <= atName && 'Z' >= atName)
		return (atName - 'A') + 26;
	return -1;
}

#define EVENT_BUFFER_SIZE 64
char eventBuffer[EVENT_BUFFER_SIZE];

typedef enum loadState_t {
	LS_RIDE,
	LS_ATTENDEE
} loadState;

typedef enum ridePart_t {
	RP_NUM,
	RP_TIMEOUT,
	RP_DURATION
} ridePart;

typedef enum attendeePart_t {
	AP_DELAY,
	AP_NAME,
	AP_XPOS,
	AP_SPEED,
	AP_RIDES
} attendeePart;

void loadEvents(char * eventFile) {
	FILE * events = fopen(eventFile, "r");
	if (NULL != events) {
		for (int i = 0; i < MAX_RIDES; rides[i++] = NULL);
		for (int i = 0; i < MAX_ATTENDEES; attendees[i++] = NULL);
		loadState ls = LS_RIDE;
		ridePart rp = RP_NUM;
		ride_t * rt = NULL;
		attendeePart ap = AP_DELAY;
		attendee_t * at = NULL;
		int startupDelay = 0;
		while (fgets(eventBuffer, EVENT_BUFFER_SIZE, events)) {
			if (eventBuffer[0] == '\n') {
				ls = LS_ATTENDEE;
				continue;
			}
			char * toke = strtok(eventBuffer, ",");
			while (NULL != toke) {
				switch (ls) {
				case LS_RIDE:
					switch (rp) {
					case RP_NUM:
						rt = malloc(sizeof(ride_t));
						if (NULL == rt) {
							perror("Error in malloc");
							exit(EXIT_FAILURE);
						}
						rt->number = atoi(toke);
						rp = RP_TIMEOUT;
						break;
					case RP_TIMEOUT:
						rt->timeout = atoi(toke);
						rp = RP_DURATION;
						break;
					case RP_DURATION:
						rt->duration = atoi(toke);
						rides[rt->number] = rt;
						rt = NULL;
						rp = RP_NUM;
						break;
					}
					break;
				case LS_ATTENDEE:
					switch (ap) {
					case AP_DELAY:
						at = malloc(sizeof(attendee_t));
						if (NULL == at) {
							perror("Error in malloc");
							exit(EXIT_FAILURE);
						}
						startupDelay += atoi(toke);
						at->delay = startupDelay;
						ap = AP_NAME;
						break;
					case AP_NAME:
						at->name = toke[0];
						ap = AP_XPOS;
						break;
					case AP_XPOS:
						at->xpos = atoi(toke);
						at->ypos = 0;
						ap = AP_SPEED;
						break;
					case AP_SPEED:
						at->speed = atoi(toke);
						ap = AP_RIDES;
						break;
					case AP_RIDES:
						at->currRide = 0;
						int numRides = strlen(toke);
						if (toke[numRides-1] == '\n')
							numRides--;
						at->numRides = numRides;
						at->rides = malloc(sizeof(int) * numRides);
						if (NULL == at->rides) {
							perror("Error in malloc");
							exit(EXIT_FAILURE);
						}
						for (int i = 0; i < numRides; i++) {
							at->rides[i] = toke[i] - '0';
						}
						at->state = AS_ENTER;
						attendees[attendeeNameToIdx(at->name)] = at;
						at = NULL;
						ap = AP_DELAY;
						break;
					}
					break;
				}
				toke = strtok(NULL, ",");
			}
		}
	} else {
		perror("Unable to load events file");
		exit(EXIT_FAILURE);
	}
}

void * keyboardInput(void * in) {
	writeToLog("Started keyboard event thread");
	char ch;
	while ((ch = getch()) != '`') {
		int idx = attendeeNameToIdx(ch);
		if (0 <= idx) {
			if (NULL == attendees[idx]) {
				// make a new attendee and start its thread
			} else {
				// make attendee[idx] want to leave
			}
		} else if ('0' <= ch && '9' >= ch) {
			// start the ride if it's stopped, stop the ride if it's started
		}
	}
	writeToLog("Exiting keyboard event thread");
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <mapfile> <eventfile> <outfile>\n", argv[0]);
		return EXIT_FAILURE;
	}

	pthread_create(&logThread, NULL, logOutput, (void *)argv[3]);

	loadEvents(argv[2]);
	writeToLog("Event file loaded");
	loadPark(argv[1]);
	writeToLog("Park file loaded");

	int rideLengths[MAX_RIDES] = {0};
	for (int r = 0; r < SCREEN_HEIGHT; r++) {
		for (int c = 0; c < SCREEN_WIDTH; c++) {
			char ch = get_screen_char(c, r);
			if (('0' <= ch) && ('9' >= ch)) {
				rideLengths[ch - '0']++;
			}
		}
	}
	for (int i = 0; i < MAX_RIDES; i++) {
		if (NULL != rides[i]) {
			rides[i]->numRiders = rideLengths[i];
			rides[i]->currRider = 0;
			rides[i]->riders = calloc(rideLengths[i], sizeof(attendee_t));
			pthread_create(&(rideThreads[i]), NULL, rideThread, (void *)rides[i]);
			rides[i]->threadID = rideThreads[i];
		}
	}
	for (int i = 0; i < MAX_ATTENDEES; i++) {
		if (NULL != attendees[i]) {
			pthread_create(&(attendeeThreads[i]), NULL, attendeeThread, (void *)attendees[i]);
			attendees[i]->threadID = attendeeThreads[i];
		}
	}

	pthread_create(&kbThread, NULL, keyboardInput, (void *)NULL);
	pthread_join(kbThread, NULL);





	typedef struct {
		int row, col;
	} Pos;

	char ch;
	Pos letters[52];
	Pos *current;
	int new_col, new_row;
	char targets[2*MAX_RIDES + 1] = "";
	const int CHARDIFF = '0' - '!';

	for (int i = 0; i < 52; i++) {
		do {
			letters[i].col = random() % SCREEN_WIDTH;
			letters[i].row = 0;
		} while (SPACE != safe_get_screen_char(FALSE, letters[i].col, letters[i].row));
		safe_set_screen_char(FALSE, letters[i].col, letters[i].row, ((i < 26) ? 'a' : 'A' - 26) + i);
	}
	safe_update_screen();

	while ((ch = getch()) != '`') {
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
			if (ch >= 'a' && ch <= 'z')
				current = &letters[ch-'a'];
			if (ch >= 'A' && ch <= 'Z')
				current = &letters[(ch-'A')+26];
			if (current->col >= 0) {
				char target = '0' + ch % MAX_RIDES;
				new_col = current->col;
				new_row = current->row;
				safe_find_target(TRUE, target, &new_col, &new_row);
				if (safe_move_to_target(TRUE, current->col, current->row, &new_col, &new_row)) {
					safe_set_screen_char(FALSE, current->col, current->row, ' ');
					current->col = -1;
					current->row = -1;
					int oldlen = strlen(targets);
					targets[oldlen] = target - CHARDIFF;
					targets[oldlen+1] = target;
					targets[oldlen+2] = '\0';
				} else {
					safe_set_screen_char(FALSE, current->col, current->row, ' ');
					safe_set_screen_char(FALSE, new_col, new_row, ch);
					current->col = new_col;
					current->row = new_row;
				}
			}
		}
		safe_blink_screen(targets);
		safe_update_screen();
	}

	stopLog();
	pthread_join(logThread, NULL);
	finish_screen();

	return EXIT_SUCCESS;
}