/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Keyboard input helper functions (too small to be called a layer)
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef _INPUT_H
#define _INPUT_H

enum {
	INPUT_MAX_MODIFIERS	= 4,
	INPUT_BUFFER_LEN	= 16,
};

enum {
	/* Keyboard LEDs */
	INPUT_LED_SCROLL	= 1 << 0,
	INPUT_LED_NUM		= 1 << 1,
	INPUT_LED_CAPS		= 1 << 2,
};

/*
 * This table translates key codes to ASCII. Most of the entries are ASCII
 * codes, but entries after KEY_FIRST_MOD indicate that this key is a
 * modifier key, like shift, ctrl. KEY_FIRST_MOD + MOD_SHIFT is the shift
 * key, for example.
 */
struct input_key_xlate {
	/* keycode of the modifiers which select this table, -1 if none */
	int left_keycode;
	int right_keycode;
	const uchar *xlate;	/* keycode to ASCII table */
	int num_entries;	/* number of entries in this table */
};

struct input_config {
	struct udevice *dev;
	uchar fifo[INPUT_BUFFER_LEN];
	int fifo_in, fifo_out;

	/* Which modifiers are active (1 bit for each MOD_... value) */
	uchar modifiers;
	uchar flags;		/* active state keys (FLAGS_...) */
	uchar leds;		/* active LEDs (INPUT_LED_...) */
	uchar leds_changed;	/* LEDs that just changed */
	uchar num_tables;	/* number of modifier tables */
	int prev_keycodes[INPUT_BUFFER_LEN];	/* keys held last time */
	int num_prev_keycodes;	/* number of prev keys */
	struct input_key_xlate table[INPUT_MAX_MODIFIERS];

	/**
	 * Function the input helper calls to scan the keyboard
	 *
	 * @param config	Input state
	 * @return 0 if no keys read, otherwise number of keys read, or 1 if
	 *		unknown
	 */
	int (*read_keys)(struct input_config *config);
	bool allow_repeats;		/* Don't filter out repeats */
	unsigned int next_repeat_ms;	/* Next time we repeat a key */
	unsigned int repeat_delay_ms;	/* Time before autorepeat starts */
	unsigned int repeat_rate_ms;	/* Autorepeat rate in ms */
};

struct stdio_dev;

/**
 * Convert a list of key codes into ASCII and send them
 *
 * @param config	Input state
 * @param keycode	List of key codes to examine
 * @param num_keycodes	Number of key codes
 * @return number of ascii characters sent, or 0 if none, or -1 for an
 *	internal error
 */
int input_send_keycodes(struct input_config *config, int keycode[], int count);

/**
 * Add a new keycode to an existing list of keycodes
 *
 * This can be used to handle keyboards which do their own scanning. An
 * internal list of depressed keys is maintained by the input library. Then
 * this function is called to add a new key to the list (when a 'make code' is
 * received), or remove a key (when a 'break code' is received).
 *
 * This function looks after maintenance of the list of active keys, and calls
 * input_send_keycodes() with its updated list.
 *
 * @param config	Input state
 * @param new_keycode	New keycode to add/remove
 * @param release	true if this key was released, false if depressed
 * @return number of ascii characters sent, or 0 if none, or -1 for an
 *	internal error
 */
int input_add_keycode(struct input_config *config, int new_keycode,
		      bool release);

/**
 * Add a new key translation table to the input
 *
 * @param config	Input state
 * @param left_keycode	Key to hold to get into this table
 * @param right_keycode	Another key to hold to get into this table
 * @param xlate		Conversion table from key codes to ASCII
 * @param num_entries	Number of entries in xlate table
 */
int input_add_table(struct input_config *config, int left_keycode,
		    int right_keycode, const uchar *xlate, int num_entries);

/**
 * Test if keys are available to be read
 *
 * @param config	Input state
 * @return 0 if no keys available, 1 if keys are available
 */
int input_tstc(struct input_config *config);

/**
 * Read a key
 *
 * TODO: U-Boot wants 0 for no key, but Ctrl-@ is a valid key...
 *
 * @param config	Input state
 * @return key, or 0 if no key, or -1 if error
 */
int input_getc(struct input_config *config);

/**
 * Register a new device with stdio and switch to it if wanted
 *
 * @param dev	Pointer to device
 * @return 0 if ok, -1 on error
 */
int input_stdio_register(struct stdio_dev *dev);

/**
 * Set up the keyboard autorepeat delays
 *
 * @param repeat_delay_ms	Delay before key auto-repeat starts (in ms)
 * @param repeat_rate_ms	Delay between successive key repeats (in ms)
 */
void input_set_delays(struct input_config *config, int repeat_delay_ms,
	       int repeat_rate_ms);

/**
 * Tell the input layer whether to allow the caller to determine repeats
 *
 * Generally the input library handles processing of a list of scanned keys.
 * Repeated keys need to be generated based on a timer in this case, since all
 * that is provided is a list of keys current depressed.
 *
 * Keyboards which do their own scanning will resend codes when they want to
 * inject a repeating key. This function can be called at start-up to select
 * this behaviour.
 *
 * @param config	Input state
 * @param allow_repeats	true to repeat depressed keys every time
 *			input_send_keycodes() is called, false to do normal
 *			keyboard repeat processing with a timer.
 */
void input_allow_repeats(struct input_config *config, bool allow_repeats);

/**
 * Check if keyboard LEDs need to be updated
 *
 * This can be called after input_tstc() to see if keyboard LEDs need
 * updating.
 *
 * @param config	Input state
 * @return -1 if no LEDs need updating, other value if they do
 */
int input_leds_changed(struct input_config *config);

/**
 * Set up the key map tables
 *
 * This must be called after input_init() or keycode decoding will not work.
 *
 * @param config	Input state
 * @param german	true to use German keyboard layout, false for US
 * @return 0 if ok, -1 on error
 */
int input_add_tables(struct input_config *config, bool german);

/**
 * Set up the input handler with basic key maps.
 *
 * @param config	Input state
 * @param leds		Initial LED value (INPUT_LED_ mask), 0 suggested
 * @return 0 if ok, -1 on error
 */
int input_init(struct input_config *config, int leds);

#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#else
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif
