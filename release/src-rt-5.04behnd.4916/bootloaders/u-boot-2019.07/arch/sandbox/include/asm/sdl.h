/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 */

#ifndef __SANDBOX_SDL_H
#define __SANDBOX_SDL_H

#include <errno.h>

#ifdef CONFIG_SANDBOX_SDL

/**
 * sandbox_sdl_init_display() - Set up SDL video ready for use
 *
 * @width:	Window width in pixels
 * @height	Window height in pixels
 * @log2_bpp:	Log to base 2 of the number of bits per pixel. So a 32bpp
 *		display will pass 5, since 2*5 = 32
 * @return 0 if OK, -ENODEV if no device, -EIO if SDL failed to initialize
 *		and -EPERM if the video failed to come up.
 */
int sandbox_sdl_init_display(int width, int height, int log2_bpp);

/**
 * sandbox_sdl_sync() - Sync current U-Boot LCD frame buffer to SDL
 *
 * This must be called periodically to update the screen for SDL so that the
 * user can see it.
 *
 * @lcd_base: Base of frame buffer
 * @return 0 if screen was updated, -ENODEV is there is no screen.
 */
int sandbox_sdl_sync(void *lcd_base);

/**
 * sandbox_sdl_scan_keys() - scan for pressed keys
 *
 * Works out which keys are pressed and returns a list
 *
 * @key:	Array to receive keycodes
 * @max_keys:	Size of array
 * @return number of keycodes found, 0 if none, -ENODEV if no keyboard
 */
int sandbox_sdl_scan_keys(int key[], int max_keys);

/**
 * sandbox_sdl_key_pressed() - check if a particular key is pressed
 *
 * @keycode:	Keycode to check (KEY_... - see include/linux/input.h
 * @return 0 if pressed, -ENOENT if not pressed. -ENODEV if keybord not
 * available,
 */
int sandbox_sdl_key_pressed(int keycode);

/**
 * sandbox_sdl_sound_play() - Play a sound
 *
 * @data:	Data to play (typically 16-bit)
 * @count:	Number of bytes in data
 */
int sandbox_sdl_sound_play(const void *data, uint count);

/**
 * sandbox_sdl_sound_stop() - stop playing a sound
 *
 * @return 0 if OK, -ENODEV if no sound is available
 */
int sandbox_sdl_sound_stop(void);

/**
 * sandbox_sdl_sound_init() - set up the sound system
 *
 * @rate:	Sample rate to use
 * @channels:	Number of channels to use (1=mono, 2=stereo)
 * @return 0 if OK, -ENODEV if no sound is available
 */
int sandbox_sdl_sound_init(int rate, int channels);

#else
static inline int sandbox_sdl_init_display(int width, int height,
					    int log2_bpp)
{
	return -ENODEV;
}

static inline int sandbox_sdl_sync(void *lcd_base)
{
	return -ENODEV;
}

static inline int sandbox_sdl_scan_keys(int key[], int max_keys)
{
	return -ENODEV;
}

static inline int sandbox_sdl_key_pressed(int keycode)
{
	return -ENODEV;
}

static inline int sandbox_sdl_sound_start(uint frequency)
{
	return -ENODEV;
}

static inline int sandbox_sdl_sound_play(const void *data, uint count)
{
	return -ENODEV;
}

static inline int sandbox_sdl_sound_stop(void)
{
	return -ENODEV;
}

static inline int sandbox_sdl_sound_init(int rate, int channels)
{
	return -ENODEV;
}

#endif

#endif
