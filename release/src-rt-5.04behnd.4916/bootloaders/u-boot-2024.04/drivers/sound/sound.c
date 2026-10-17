// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
 */

#include <common.h>
#include <log.h>
#include <sound.h>

void sound_create_square_wave(uint sample_rate, unsigned short *data, int size,
			      uint freq, uint channels)
{
	const unsigned short amplitude = 16000; /* between 1 and 32767 */
	const int period = freq ? sample_rate / freq : 0;
	const int half = period / 2;

	if (!half) {
		memset(data, 0, size);
		return;
	}

	/* Make sure we don't overflow our buffer */
	if (size % 2)
		size--;

	while (size) {
		int i, j;

		for (i = 0; size && i < half; i++) {
			for (j = 0; size && j < channels; j++, size -= 2)
				*data++ = amplitude;
		}
		for (i = 0; size && i < period - half; i++) {
			for (j = 0; size && j < channels; j++, size -= 2)
				*data++ = -amplitude;
		}
	}
}
