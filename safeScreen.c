#include "safeScreen.h"
#include "screen.h"
#include <pthread.h>

static pthread_mutex_t screenMutex = PTHREAD_MUTEX_INITIALIZER;

void safe_update_screen() {
	pthread_mutex_lock(&screenMutex);
	update_screen();
	pthread_mutex_unlock(&screenMutex);
}

void safe_set_screen_char(int col, int row, char ch) {
	pthread_mutex_lock(&screenMutex);
	set_screen_char(col, row, ch);
	pthread_mutex_unlock(&screenMutex);
}

char safe_get_screen_char(int col, int row) {
	pthread_mutex_lock(&screenMutex);
	char ch = get_screen_char(col, row);
	pthread_mutex_unlock(&screenMutex);
	return ch;
}

void safe_blink_screen(char * charset) {
	pthread_mutex_lock(&screenMutex);
	blink_screen(charset);
	pthread_mutex_unlock(&screenMutex);
}

int safe_find_target(char ch, int * col, int * row) {
	pthread_mutex_lock(&screenMutex);
	int target = find_target(ch, col, row);
	pthread_mutex_unlock(&screenMutex);
	return target;
}

int safe_move_to_target(int col, int row, int * to_col, int * to_row) {
	pthread_mutex_lock(&screenMutex);
	int target = move_to_target(col, row, to_col, to_row);
	pthread_mutex_unlock(&screenMutex);
	return target;
}