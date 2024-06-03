/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 */


#ifndef __SOUND_ARCH_H__
#define __SOUND_ARCH_H__

/* I2S values */
#define I2S_PLL_CLK		192000000
#define I2S_SAMPLING_RATE	48000
#define I2S_BITS_PER_SAMPLE	16
#define I2S_CHANNELS		2
#define I2S_RFS			256
#define I2S_BFS			32

/* I2C values */
#define AUDIO_I2C_BUS		1
#define AUDIO_I2C_REG		0x1a

/* Audio Codec */
#define AUDIO_CODEC		"wm8994"

#define AUDIO_COMPAT		1
#endif
