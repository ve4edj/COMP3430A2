typedef struct {
	char name;
	pthread_t threadID;
	int delay;
	int speed;
	int currRide;
	int numRides;
	int * rides;
	int xpos;
} attendee_t;

void * attendeeThread(void *);