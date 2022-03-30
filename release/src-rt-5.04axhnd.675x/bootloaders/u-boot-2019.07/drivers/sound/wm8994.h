/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chadrasekar <rcsekar@samsung.com>
 */

#ifndef __WM8994_H__
#define __WM8994_H__

/* Sources for AIF1/2 SYSCLK - use with set_dai_sysclk() */
#define WM8994_SYSCLK_MCLK1	1
#define WM8994_SYSCLK_MCLK2	2
#define WM8994_SYSCLK_FLL1	3
#define WM8994_SYSCLK_FLL2	4

/*  Avilable audi interface ports in wm8994 codec */
enum en_audio_interface {
	 WM8994_AIF1,
	 WM8994_AIF2,
	 WM8994_AIF3
};

/* OPCLK is also configured with set_dai_sysclk, specify division*10 as rate. */
#define WM8994_SYSCLK_OPCLK	5

#define WM8994_FLL1	1
#define WM8994_FLL2	2

#define WM8994_FLL_SRC_MCLK1	1
#define WM8994_FLL_SRC_MCLK2	2
#define WM8994_FLL_SRC_LRCLK	3
#define WM8994_FLL_SRC_BCLK	4

/* maximum available digital interfac in the dac to configure */
#define WM8994_MAX_AIF			2

#define WM8994_MAX_INPUT_CLK_FREQ	13500000
#define WM8994_ID			0x8994

enum wm8994_vmid_mode {
	WM8994_VMID_NORMAL,
	WM8994_VMID_FORCE,
};

/* wm 8994 family devices */
enum wm8994_type {
	WM8994 = 0,
	WM8958 = 1,
	WM1811 = 2,
};

/*
 * intialise wm8994 sound codec device for the given configuration
 *
 * @param blob			FDT node for codec values
 * @param aif_id		enum value of codec interface port in which
 *				soc i2s is connected
 * @param sampling_rate		Sampling rate ranges between from 8khz to 96khz
 * @param mclk_freq		Master clock frequency.
 * @param bits_per_sample	bits per Sample can be 16 or 24
 * @param channels		Number of channnels, maximum 2
 *
 * @returns -1 for error  and 0  Success.
 */
int wm8994_init(const void *blob, enum en_audio_interface aif_id,
			int sampling_rate, int mclk_freq,
			int bits_per_sample, unsigned int channels);
#endif /*__WM8994_H__ */
