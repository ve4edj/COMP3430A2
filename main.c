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

void die(char * msg) {
	perror(msg);
	finish_screen();
	exit(EXIT_FAILURE);
}

void loadPark(char * parkFile) {
	initialize_screen();
	if (load_screen(parkFile) < 0)
		die("Unable to load map file");
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
						if (NULL == rt)
							die("Error in malloc");
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
						if (NULL == at)
							die("Error in malloc");
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
						if (NULL == at->rides)
							die("Error in malloc");
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
		die("Unable to load events file");
	}
}

void * keyboardInput(void * in) {
	writeToLog("Started keyboard event thread");
	char ch;
	noecho();
	while ((ch = getch()) != '`') {
		int idx = attendeeNameToIdx(ch);
		if (0 <= idx) {
			if (NULL == attendees[idx]) {
				attendee_t * at = malloc(sizeof(attendee_t));
				if (NULL == at)
					die("Fail in malloc");
				at->delay = 0;
				at->speed = 500;
				at->name = ch;
				at->xpos = 0;
				at->ypos = 0;
				at->state = AS_ENTER;
				at->wantsToLeave = 0;
				at->numRides = 10;
				at->currRide = 0;
				at->rides = malloc(sizeof(int) * at->numRides);
				if (NULL == at->rides)
					die("Fail in malloc");
				for (int i = 0; i < at->numRides; i++) {
					at->rides[i] = (at->name + i) % MAX_RIDES;
				}
				attendees[idx] = at;
				pthread_create(&(attendeeThreads[idx]), NULL, attendeeThread, (void *)attendees[idx]);
			} else {
				attendees[idx]->wantsToLeave = 1;
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

	loadPark(argv[1]);
	writeToLog("Park file loaded");
	loadEvents(argv[2]);
	writeToLog("Event file loaded");

	int rideLengths[MAX_RIDES] = {0};
	int rideExitsX[MAX_RIDES] = {0};
	int rideExitsY[MAX_RIDES] = {0};
	for (int r = 0; r < SCREEN_HEIGHT; r++) {
		for (int c = 0; c < SCREEN_WIDTH; c++) {
			char ch = get_screen_char(c, r);
			if (('0' <= ch) && ('9' >= ch)) {
				rideLengths[ch - '0']++;
			}
			if (('!' <= ch) && ('*' >= ch)) {
				rideExitsX[ch - '!'] = c;
				rideExitsY[ch - '!'] = r;
			}
		}
	}
	for (int i = 0; i < MAX_RIDES; i++) {
		if (NULL != rides[i]) {
			rides[i]->numRiders = rideLengths[i];
			rides[i]->currRider = 0;
			rides[i]->riders = calloc(rideLengths[i], sizeof(attendee_t));
			rides[i]->exitX = rideExitsX[i];
			rides[i]->exitY = rideExitsY[i];
			pthread_create(&(rideThreads[i]), NULL, rideThread, (void *)rides[i]);
		} else {
			// if there is a nonzero length in rideLengths
			// create a default ride with timeout = 5 and duration = 20
		}
	}
	for (int i = 0; i < MAX_ATTENDEES; i++) {
		if (NULL != attendees[i]) {
			pthread_create(&(attendeeThreads[i]), NULL, attendeeThread, (void *)attendees[i]);
		}
	}

	pthread_create(&kbThread, NULL, keyboardInput, (void *)NULL);
	pthread_join(kbThread, NULL);
	stopLog();
	pthread_join(logThread, NULL);
	finish_screen();

	return EXIT_SUCCESS;
}