/*
		Course:			COMP3430
		Assignment:		2 - Amusement Park Simulator
		Due date:		Tuesday, March 22 2016
		Instructor:		John Braico
		Programmed By:	Erik Johnson
		Student #:		7711697
*/

#include "safeScreen.h"
#include "screen.h"
#include <pthread.h>

static pthread_mutex_t screenMutex = PTHREAD_MUTEX_INITIALIZER;

void safe_update_screen() {
	pthread_mutex_lock(&screenMutex);
	update_screen();
	pthread_mutex_unlock(&screenMutex);
}

char safe_get_screen_char(int multipart, int col, int row) {
	if (!multipart)
		pthread_mutex_lock(&screenMutex);
	char ch = get_screen_char(col, row);
	if (!multipart)
		pthread_mutex_unlock(&screenMutex);
	return ch;
}

void safe_set_screen_char(int multipart, int col, int row, char ch) {
	if (!multipart)
		pthread_mutex_lock(&screenMutex);
	set_screen_char(col, row, ch);
	if (!multipart)
		pthread_mutex_unlock(&screenMutex);
}

void safe_blink_screen(char * charset) {
	pthread_mutex_lock(&screenMutex);
	blink_screen(charset);
	pthread_mutex_unlock(&screenMutex);
}

int safe_find_target(int multipart, char ch, int * col, int * row) {
	if (!multipart)
		pthread_mutex_lock(&screenMutex);
	int target = find_target(ch, col, row);
	if (!multipart)
		pthread_mutex_unlock(&screenMutex);
	return target;
}

int safe_move_to_target(int multipart, int col, int row, int * to_col, int * to_row) {
	if (!multipart)
		pthread_mutex_lock(&screenMutex);
	int target = move_to_target(col, row, to_col, to_row);
	if (!multipart)
		pthread_mutex_unlock(&screenMutex);
	return target;
}

void lockScreen() {
	pthread_mutex_lock(&screenMutex);
}

void unlockScreen() {
	pthread_mutex_unlock(&screenMutex);
}