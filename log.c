#include "log.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 256

static pthread_mutex_t logFileMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t logInUse = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t emptyCond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t fullCond = PTHREAD_COND_INITIALIZER;
static int bufferIsEmpty;
static int bufferIsFull;
static char buffer[BUFFER_SIZE];
static int in;
static int out;
static int count;
static int stop;

void * logOutput(void * in) {
	stop = 0;
	bufferIsEmpty = 1;
	bufferIsFull = 0;
	char ch;
	FILE * f = fopen((char *)in, "a");
	if (NULL != f) {
		fprintf(f, "---------------- Log Opened ----------------\n");
		while (!stop) {
			pthread_mutex_lock(&logFileMutex);
			while (bufferIsEmpty && !stop)
				pthread_cond_wait(&emptyCond, &logFileMutex);
			while (!bufferIsEmpty) {
				ch = buffer[out++];
				out %= BUFFER_SIZE;
				if (--count == 0)
					bufferIsEmpty = 1;
				fputc(ch, f);
			}
			bufferIsFull = 0;
			pthread_cond_signal(&fullCond);
			pthread_mutex_unlock(&logFileMutex);
		}
		fprintf(f, "---------------- Log Closed ----------------\n");
		fflush(f);
		fclose(f);
	} else {
		fprintf(stderr, "Error opening log file %s\n", (char *)in);
	}
	pthread_exit(NULL);
}

void writeToLog(char * str) {
	pthread_mutex_lock(&logInUse);
	for (int i = 0; i <= strlen(str); i++) {
		pthread_mutex_lock(&logFileMutex);
		while (bufferIsFull)
			pthread_cond_wait(&fullCond, &logFileMutex);
		if (strlen(str) == i)
			buffer[in++] = '\n';
		else
			buffer[in++] = str[i];
		in %= BUFFER_SIZE;
		if (++count == (BUFFER_SIZE - 1))
			bufferIsFull = 1;
		bufferIsEmpty = 0;
		usleep(10);
		pthread_cond_signal(&emptyCond);
		pthread_mutex_unlock(&logFileMutex);
	}
	pthread_mutex_unlock(&logInUse);
}

void stopLog() {
	pthread_mutex_lock(&logFileMutex);
	stop = 1;
	pthread_cond_signal(&emptyCond);
	pthread_mutex_unlock(&logFileMutex);
}