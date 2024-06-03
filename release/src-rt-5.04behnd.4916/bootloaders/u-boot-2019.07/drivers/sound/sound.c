// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
 */

#include <common.h>
#include <sound.h>

void sound_create_square_wave(uint sample_rate, unsigned short *data, int size,
			      uint freq, uint channels)
{
	const unsigned short amplitude = 16000; /* between 1 and 32767 */
	const int period = freq ? sample_rate / freq : 0;
	const int half = period / 2;

	assert(freq);

	/* Make sure we don't overflow our buffer */
	if (size % 2)
		size--;

	while (size) {
		int i, j;

		for (i = 0; size && i < half; i++) {
			size -= 2;
			for (j = 0; j < channels; j++)
				*data++ = amplitude;
		}
		for (i = 0; size && i < period - half; i++) {
			size -= 2;
			for (j = 0; j < channels; j++)
				*data++ = -amplitude;
		}
	}
}
