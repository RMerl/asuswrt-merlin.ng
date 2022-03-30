/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __AUDIO_CODEC_H__
#define __AUDIO_CODEC_H__

/*
 * An audio codec turns digital data into sound with various parameters to
 * control its operation.
 */

/* Operations for sound */
struct audio_codec_ops {
	/**
	 * set_params() - Set audio codec parameters
	 *
	 * @dev: Sound device
	 * @inteface: Interface number to use on codec
	 * @rate: Sampling rate in Hz
	 * @mclk_freq: Codec clock frequency in Hz
	 * @bits_per_sample: Must be 16 or 24
	 * @channels: Number of channels to use (1=mono, 2=stereo)
	 * @return 0 if OK, -ve on error
	 */
	int (*set_params)(struct udevice *dev, int interface, int rate,
			  int mclk_freq, int bits_per_sample, uint channels);
};

#define audio_codec_get_ops(dev) ((struct audio_codec_ops *)(dev)->driver->ops)

/**
 * audio_codec_set_params() - Set audio codec parameters
 *
 * @dev: Sound device
 * @inteface: Interface number to use on codec
 * @rate: Sampling rate in Hz
 * @mclk_freq: Codec clock frequency in Hz
 * @bits_per_sample: Must be 16 or 24
 * @channels: Number of channels to use (1=mono, 2=stereo)
 * @return 0 if OK, -ve on error
 */
int audio_codec_set_params(struct udevice *dev, int interface, int rate,
			   int mclk_freq, int bits_per_sample, uint channels);

#endif  /* __AUDIO_CODEC_H__ */
