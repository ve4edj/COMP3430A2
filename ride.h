typedef struct {
	int number;
	int timeout;
	int duration;
} ride_t;

void * rideThread(void *);