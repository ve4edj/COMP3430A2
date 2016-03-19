#include <pthread.h>

typedef struct {
	int number;
	int timeout;
	int duration;
	int maxRiders;
	attendee_t ** riders;
} ride_t;

void * rideThread(void *);