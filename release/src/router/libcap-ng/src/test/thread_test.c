#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <cap-ng.h>
#include <pthread.h>

//#define DEBUG 1

pthread_t thread1, thread2;

void *thread1_main(void *arg)
{
	capng_fill(CAPNG_SELECT_BOTH);
#ifdef DEBUG
	printf("thread1 filled capabilities\n");
#endif
	sleep(2);
	if (capng_have_capabilities(CAPNG_SELECT_CAPS) < CAPNG_FULL) {
		printf("Capabilities missing when there should be some\n");
		exit(1);
	}
#ifdef DEBUG
		printf("SUCCESS: Full capabilities reported\n");
#endif
	return NULL;
}

void *thread2_main(void *arg)
{
	sleep(1);
#ifdef DEBUG
	printf("thread2 getting capabilities\n");
#endif
	capng_get_caps_process();
	if (capng_have_capabilities(CAPNG_SELECT_CAPS) != CAPNG_NONE) {
		printf("Detected capabilities when there should not be any\n");
		exit(1);
	}
	capng_clear(CAPNG_SELECT_BOTH);
#ifdef DEBUG
	printf("SUCCESS: No capabilities reported\n");
#endif
	return NULL;
}

int main(void)
{
	// This test must be run as root which naturally has all capabilities
	// set. So, we need to clear the capabilities so that we can see if
	// the test works.
	capng_clear(CAPNG_SELECT_CAPS);
	capng_apply(CAPNG_SELECT_CAPS);

	printf("Testing thread separation of capabilities\n");
	pthread_create(&thread1, NULL, thread1_main, NULL);
	pthread_create(&thread2, NULL, thread2_main, NULL);
	sleep(3);
	return 0;
}

