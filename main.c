#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "screen.h"
#include "attendee.h"
#include "ride.h"

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
		int rideIdx = -1;
		attendeePart ap = AP_DELAY;
		int attendeeIdx = -1;
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
						rides[++rideIdx] = malloc(sizeof(ride_t));
						if (NULL == rides[rideIdx]) {
							perror("Error in malloc");
							exit(EXIT_FAILURE);
						}
						rides[rideIdx]->number = atoi(toke);
						rp = RP_TIMEOUT;
						break;
					case RP_TIMEOUT:
						rides[rideIdx]->timeout = atoi(toke);
						rp = RP_DURATION;
						break;
					case RP_DURATION:
						rides[rideIdx]->duration = atoi(toke);
						rp = RP_NUM;
						break;
					}
					break;
				case LS_ATTENDEE:
					switch (ap) {
					case AP_DELAY:
						attendees[++attendeeIdx] = malloc(sizeof(attendee_t));
						if (NULL == attendees[attendeeIdx]) {
							perror("Error in malloc");
							exit(EXIT_FAILURE);
						}
						attendees[attendeeIdx]->delay = atoi(toke);
						ap = AP_NAME;
						break;
					case AP_NAME:
						attendees[attendeeIdx]->name = toke[0];
						ap = AP_XPOS;
						break;
					case AP_XPOS:
						attendees[attendeeIdx]->xpos = atoi(toke);
						ap = AP_SPEED;
						break;
					case AP_SPEED:
						attendees[attendeeIdx]->speed = atoi(toke);
						ap = AP_RIDES;
						break;
					case AP_RIDES:
						attendees[attendeeIdx]->currRide = 0;
						int numRides = strlen(toke);
						if (toke[numRides-1] == '\n')
							numRides--;
						attendees[attendeeIdx]->numRides = numRides;
						attendees[attendeeIdx]->rides = malloc(sizeof(int) * numRides);
						if (NULL == attendees[attendeeIdx]->rides) {
							perror("Error in malloc");
							exit(EXIT_FAILURE);
						}
						for (int i = 0; i < numRides; i++) {
							attendees[attendeeIdx]->rides[i] = toke[i] - '0';
						}
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

void * keyboardInput(void *) {

	pthread_exit(NULL);
}

pthread_mutex_t screenMutex = PTHREAD_MUTEX_INITIALIZER;
void * screenOutput(void *) {

	pthread_exit(NULL);
}

void * logOutput(void *) {

	pthread_exit(NULL);
}

typedef struct {
	int row, col;
} Pos;

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <mapfile> <eventfile> <outfile>\n", argv[0]);
		return EXIT_FAILURE;
	}
	loadPark(argv[1]);
	loadEvents(argv[2]);

	pthread_create(&screenThread, NULL, screenOutput, (void *)NULL);
	pthread_create(&logThread, NULL, logOutput, (void *)NULL);

	for (int i = 0; i < MAX_RIDES; i++) {
		if (NULL != rides[i]) {
			pthread_create(&(rideThreads[i]), NULL, rideThread, (void *)rides[i]);
		}
	}

	for (int i = 0; i < MAX_ATTENDEES; i++) {
		if (NULL != attendees[i]) {
			pthread_create(&(attendeeThreads[i]), NULL, attendeeThread, (void *)attendees[i]);
		}
	}

	pthread_create(&kbThread, NULL, keyboardInput, (void *)NULL);

/*
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
		} while (SPACE != get_screen_char(letters[i].col, letters[i].row));
		set_screen_char(letters[i].col, letters[i].row, ((i < 26) ? 'a' : 'A' - 26) + i);
	}
	update_screen();

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
				find_target(target, &new_col, &new_row);
				if (move_to_target(current->col, current->row, &new_col, &new_row)) {
					set_screen_char(current->col, current->row, ' ');
					current->col = -1;
					current->row = -1;
					int oldlen = strlen(targets);
					targets[oldlen] = target - CHARDIFF;
					targets[oldlen+1] = target;
					targets[oldlen+2] = '\0';
				} else {
					set_screen_char(current->col, current->row, ' ');
					set_screen_char(new_col, new_row, ch);
					current->col = new_col;
					current->row = new_row;
				}
			}
		}
		blink_screen(targets);
		update_screen();
	}
*/
	finish_screen();

	return EXIT_SUCCESS;
}