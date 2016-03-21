#include "log.h"
#include <pthread.h>

void * logOutput(void * in) {

	pthread_exit(NULL);
}