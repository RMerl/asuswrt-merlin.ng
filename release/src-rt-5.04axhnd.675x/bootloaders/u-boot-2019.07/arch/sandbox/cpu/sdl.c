// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <errno.h>
#include <unistd.h>
#include <linux/input.h>
#include <SDL.h>
#include <asm/state.h>

/**
 * struct buf_info - a data buffer holding audio data
 *
 * @pos:	Current position playing in audio buffer
 * @size:	Size of data in audio buffer (0=empty)
 * @alloced:	Allocated size of audio buffer (max size it can hold)
 * @data:	Audio data
 */
struct buf_info {
	uint pos;
	uint size;
	uint alloced;
	uint8_t *data;
};

static struct sdl_info {
	SDL_Surface *screen;
	int width;
	int height;
	int depth;
	int pitch;
	uint frequency;
	uint sample_rate;
	bool audio_active;
	bool inited;
	int cur_buf;
	struct buf_info buf[2];
	bool running;
} sdl;

static void sandbox_sdl_poll_events(void)
{
	/*
	 * We don't want to include common.h in this file since it uses
	 * system headers. So add a declation here.
	 */
	extern void reset_cpu(unsigned long addr);
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			puts("LCD window closed - quitting\n");
			reset_cpu(1);
			break;
		}
	}
}

static int sandbox_sdl_ensure_init(void)
{
	if (!sdl.inited) {
		if (SDL_Init(0) < 0) {
			printf("Unable to initialize SDL: %s\n",
			       SDL_GetError());
			return -EIO;
		}

		atexit(SDL_Quit);

		sdl.inited = true;
	}
	return 0;
}

int sandbox_sdl_init_display(int width, int height, int log2_bpp)
{
	struct sandbox_state *state = state_get_current();
	int err;

	if (!width || !state->show_lcd)
		return 0;
	err = sandbox_sdl_ensure_init();
	if (err)
		return err;
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		printf("Unable to initialize SDL LCD: %s\n", SDL_GetError());
		return -EPERM;
	}
	SDL_WM_SetCaption("U-Boot", "U-Boot");

	sdl.width = width;
	sdl.height = height;
	sdl.depth = 1 << log2_bpp;
	sdl.pitch = sdl.width * sdl.depth / 8;
	sdl.screen = SDL_SetVideoMode(width, height, 0, 0);
	sandbox_sdl_poll_events();

	return 0;
}

int sandbox_sdl_sync(void *lcd_base)
{
	SDL_Surface *frame;

	frame = SDL_CreateRGBSurfaceFrom(lcd_base, sdl.width, sdl.height,
			sdl.depth, sdl.pitch,
			0x1f << 11, 0x3f << 5, 0x1f << 0, 0);
	SDL_BlitSurface(frame, NULL, sdl.screen, NULL);
	SDL_FreeSurface(frame);
	SDL_UpdateRect(sdl.screen, 0, 0, 0, 0);
	sandbox_sdl_poll_events();

	return 0;
}

#define NONE (-1)
#define NUM_SDL_CODES	(SDLK_UNDO + 1)

static int16_t sdl_to_keycode[NUM_SDL_CODES] = {
	/* 0 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, KEY_BACKSPACE, KEY_TAB,
	NONE, NONE, NONE, KEY_ENTER, NONE,
	NONE, NONE, NONE, NONE, KEY_POWER,	/* use PAUSE as POWER */

	/* 20 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, KEY_ESC, NONE, NONE,
	NONE, NONE, KEY_SPACE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 40 */
	NONE, NONE, NONE, NONE, KEY_COMMA,
	KEY_MINUS, KEY_DOT, KEY_SLASH, KEY_0, KEY_1,
	KEY_2, KEY_3, KEY_4, KEY_5, KEY_6,
	KEY_7, KEY_8, KEY_9, NONE, KEY_SEMICOLON,

	/* 60 */
	NONE, KEY_EQUAL, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 80 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, KEY_BACKSLASH, NONE, NONE,
	NONE, KEY_GRAVE, KEY_A, KEY_B, KEY_C,

	/* 100 */
	KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,
	KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
	KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
	KEY_S, KEY_T, KEY_U, KEY_V, KEY_W,

	/* 120 */
	KEY_X, KEY_Y, KEY_Z, NONE, NONE,
	NONE, NONE, KEY_DELETE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 140 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 160 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 180 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 200 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 220 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 240 */
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE,
	NONE, KEY_KP0, KEY_KP1, KEY_KP2, KEY_KP3,

	/* 260 */
	KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7, KEY_KP8,
	KEY_KP9, KEY_KPDOT, KEY_KPSLASH, KEY_KPASTERISK, KEY_KPMINUS,
	KEY_KPPLUS, KEY_KPENTER, KEY_KPEQUAL, KEY_UP, KEY_DOWN,
	KEY_RIGHT, KEY_LEFT, KEY_INSERT, KEY_HOME, KEY_END,

	/* 280 */
	KEY_PAGEUP, KEY_PAGEDOWN, KEY_F1, KEY_F2, KEY_F3,
	KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8,
	KEY_F9, KEY_F10, KEY_F11, KEY_F12, NONE,
	NONE, NONE, NONE, NONE, NONE,

	/* 300 */
	KEY_NUMLOCK, KEY_CAPSLOCK, KEY_SCROLLLOCK, KEY_RIGHTSHIFT,
		KEY_LEFTSHIFT,
	KEY_RIGHTCTRL, KEY_LEFTCTRL, KEY_RIGHTALT, KEY_LEFTALT, KEY_RIGHTMETA,
	KEY_LEFTMETA, NONE, KEY_FN, NONE, KEY_COMPOSE,
	NONE, KEY_PRINT, KEY_SYSRQ, KEY_PAUSE, NONE,

	/* 320 */
	NONE, NONE, NONE,
};

int sandbox_sdl_scan_keys(int key[], int max_keys)
{
	Uint8 *keystate;
	int i, count;

	sandbox_sdl_poll_events();
	keystate = SDL_GetKeyState(NULL);
	for (i = count = 0; i < NUM_SDL_CODES; i++) {
		if (count >= max_keys)
			break;
		else if (keystate[i])
			key[count++] = sdl_to_keycode[i];
	}

	return count;
}

int sandbox_sdl_key_pressed(int keycode)
{
	int key[8];	/* allow up to 8 keys to be pressed at once */
	int count;
	int i;

	count = sandbox_sdl_scan_keys(key, sizeof(key) / sizeof(key[0]));
	for (i = 0; i < count; i++) {
		if (key[i] == keycode)
			return 0;
	}

	return -ENOENT;
}

void sandbox_sdl_fill_audio(void *udata, Uint8 *stream, int len)
{
	struct buf_info *buf;
	int avail;
	int i;

	for (i = 0; i < 2; i++) {
		buf = &sdl.buf[sdl.cur_buf];
		avail = buf->size - buf->pos;
		if (avail <= 0) {
			sdl.cur_buf = 1 - sdl.cur_buf;
			continue;
		}
		if (avail > len)
			avail = len;

		SDL_MixAudio(stream, buf->data + buf->pos, avail,
			     SDL_MIX_MAXVOLUME);
		buf->pos += avail;
		len -= avail;

		/* Move to next buffer if we are at the end */
		if (buf->pos == buf->size)
			buf->size = 0;
		else
			break;
	}
}

int sandbox_sdl_sound_init(int rate, int channels)
{
	SDL_AudioSpec wanted;
	int i;

	if (sandbox_sdl_ensure_init())
		return -1;

	if (sdl.audio_active)
		return 0;

	/* Set the audio format */
	wanted.freq = rate;
	wanted.format = AUDIO_S16;
	wanted.channels = channels;
	wanted.samples = 1024;  /* Good low-latency value for callback */
	wanted.callback = sandbox_sdl_fill_audio;
	wanted.userdata = NULL;

	for (i = 0; i < 2; i++) {
		struct buf_info *buf = &sdl.buf[i];

		buf->alloced = sizeof(uint16_t) * wanted.freq * wanted.channels;
		buf->data = malloc(buf->alloced);
		if (!buf->data) {
			printf("%s: Out of memory\n", __func__);
			if (i == 1)
				free(sdl.buf[0].data);
			return -1;
		}
		buf->pos = 0;
		buf->size = 0;
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		printf("Unable to initialize SDL audio: %s\n", SDL_GetError());
		goto err;
	}

	/* Open the audio device, forcing the desired format */
	if (SDL_OpenAudio(&wanted, NULL) < 0) {
		printf("Couldn't open audio: %s\n", SDL_GetError());
		goto err;
	}
	sdl.audio_active = true;
	sdl.sample_rate = wanted.freq;
	sdl.cur_buf = 0;
	sdl.running = 0;

	return 0;

err:
	for (i = 0; i < 2; i++)
		free(sdl.buf[i].data);
	return -1;
}

int sandbox_sdl_sound_play(const void *data, uint size)
{
	struct buf_info *buf;

	if (!sdl.audio_active)
		return 0;

	buf = &sdl.buf[0];
	if (buf->size)
		buf = &sdl.buf[1];
	while (buf->size)
		usleep(1000);

	if (size > buf->alloced)
		return -E2BIG;

	memcpy(buf->data, data, size);
	buf->size = size;
	buf->pos = 0;
	if (!sdl.running) {
		SDL_PauseAudio(0);
		sdl.running = 1;
	}

	return 0;
}

int sandbox_sdl_sound_stop(void)
{
	if (sdl.running) {
		SDL_PauseAudio(1);
		sdl.running = 0;
	}

	return 0;
}
