#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#include <stdio.h>
#include <alsa/asoundlib.h>

static int any_test_failed;

#define TEST_CHECK(cond) do \
		if (!(cond)) { \
			fprintf(stderr, "%s:%d: test failed: %s\n", __FILE__, __LINE__, #cond); \
			any_test_failed = 1; \
		} \
	while (0)

#define ALSA_CHECK(fn) ({ \
		int err = fn; \
		if (err < 0) { \
			fprintf(stderr, "%s:%d: ALSA function call failed (%s): %s\n", \
				__FILE__, __LINE__, snd_strerror(err), #fn); \
			any_test_failed = 1; \
		} \
		err; \
	})

#define TEST_EXIT_CODE() any_test_failed

#endif
