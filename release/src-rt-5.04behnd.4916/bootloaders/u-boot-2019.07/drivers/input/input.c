// SPDX-License-Identifier: GPL-2.0+
/*
 * Translate key codes into ASCII
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2004 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <stdio_dev.h>
#include <input.h>
#ifdef CONFIG_DM_KEYBOARD
#include <keyboard.h>
#endif
#include <linux/input.h>

enum {
	/* These correspond to the lights on the keyboard */
	FLAG_SCROLL_LOCK	= 1 << 0,
	FLAG_NUM_LOCK		= 1 << 1,
	FLAG_CAPS_LOCK		= 1 << 2,

	/* Special flag ORed with key code to indicate release */
	KEY_RELEASE		= 1 << 15,
	KEY_MASK		= 0xfff,
};

/*
 * These takes map key codes to ASCII. 0xff means no key, or special key.
 * Three tables are provided - one for plain keys, one for when the shift
 * 'modifier' key is pressed and one for when the ctrl modifier key is
 * pressed.
 */
static const uchar kbd_plain_xlate[] = {
	0xff, 0x1b, '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '-',  '=', '\b', '\t',	/* 0x00 - 0x0f */
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '[',  ']', '\r', 0xff,  'a',  's',  /* 0x10 - 0x1f */
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	'\'',  '`', 0xff, '\\', 'z',  'x',  'c',  'v',	/* 0x20 - 0x2f */
	'b',  'n',  'm',  ',' ,  '.', '/', 0xff, 0xff, 0xff,
	' ', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x30 - 0x3f */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  '7',
	'8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',	/* 0x40 - 0x4f */
	'2',  '3',  '0',  '.', 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x50 - 0x5F */
	'\r', 0xff, '/',  '*',
};

static unsigned char kbd_shift_xlate[] = {
	0xff, 0x1b, '!', '@', '#', '$', '%', '^',
	'&', '*', '(', ')', '_', '+', '\b', '\t',	/* 0x00 - 0x0f */
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '{', '}', '\r', 0xff, 'A', 'S',	/* 0x10 - 0x1f */
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'"', '~', 0xff, '|', 'Z', 'X', 'C', 'V',	/* 0x20 - 0x2f */
	'B', 'N', 'M', '<', '>', '?', 0xff, 0xff, 0xff,
	' ', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x30 - 0x3f */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '7',
	'8', '9', '-', '4', '5', '6', '+', '1',	/* 0x40 - 0x4f */
	'2', '3', '0', '.', 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x50 - 0x5F */
	'\r', 0xff, '/',  '*',
};

static unsigned char kbd_ctrl_xlate[] = {
	0xff, 0x1b, '1', 0x00, '3', '4', '5', 0x1E,
	'7', '8', '9', '0', 0x1F, '=', '\b', '\t',	/* 0x00 - 0x0f */
	0x11, 0x17, 0x05, 0x12, 0x14, 0x19, 0x15, 0x09,
	0x0f, 0x10, 0x1b, 0x1d, '\n', 0xff, 0x01, 0x13,	/* 0x10 - 0x1f */
	0x04, 0x06, 0x08, 0x09, 0x0a, 0x0b, 0x0c, ';',
	'\'', '~', 0x00, 0x1c, 0x1a, 0x18, 0x03, 0x16,	/* 0x20 - 0x2f */
	0x02, 0x0e, 0x0d, '<', '>', '?', 0xff, 0xff,
	0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x30 - 0x3f */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '7',
	'8', '9', '-', '4', '5', '6', '+', '1',		/* 0x40 - 0x4f */
	'2', '3', '0', '.', 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x50 - 0x5F */
	'\r', 0xff, '/',  '*',
};

/*
 * German keymap. Special letters are mapped according to code page 437.
 */
static const uchar kbd_plain_xlate_german[] = {
	0xff, 0x1b,  '1',  '2',  '3',  '4',  '5',  '6', /* scan 00-07 */
	 '7',  '8',  '9',  '0', 0xe1, '\'', 0x08, '\t', /* scan 08-0F */
	 'q',  'w',  'e',  'r',  't',  'z',  'u',  'i', /* scan 10-17 */
	 'o',  'p', 0x81,  '+', '\r', 0xff,  'a',  's', /* scan 18-1F */
	 'd',  'f',  'g',  'h',  'j',  'k',  'l', 0x94, /* scan 20-27 */
	0x84,  '^', 0xff,  '#',  'y',  'x',  'c',  'v', /* scan 28-2F */
	 'b',  'n',  'm',  ',',  '.',  '-', 0xff,  '*', /* scan 30-37 */
	 ' ',  ' ', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 38-3F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  '7', /* scan 40-47 */
	 '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1', /* scan 48-4F */
	 '2',  '3',  '0',  ',', 0xff, 0xff,  '<', 0xff, /* scan 50-57 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 58-5F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 60-67 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 68-6F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 70-77 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 78-7F */
	'\r', 0xff,  '/',  '*',
};

static unsigned char kbd_shift_xlate_german[] = {
	   0xff, 0x1b,  '!',  '"', 0x15,  '$',  '%',  '&', /* scan 00-07 */
	 '/',  '(',  ')',  '=',  '?',  '`', 0x08, '\t', /* scan 08-0F */
	 'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I', /* scan 10-17 */
	 'O',  'P', 0x9a,  '*', '\r', 0xff,  'A',  'S', /* scan 18-1F */
	 'D',  'F',  'G',  'H',  'J',  'K',  'L', 0x99, /* scan 20-27 */
	0x8e, 0xf8, 0xff, '\'',  'Y',  'X',  'C',  'V', /* scan 28-2F */
	 'B',  'N',  'M',  ';',  ':',  '_', 0xff,  '*', /* scan 30-37 */
	 ' ',  ' ', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 38-3F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  '7', /* scan 40-47 */
	 '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1', /* scan 48-4F */
	 '2',  '3',  '0',  ',', 0xff, 0xff,  '>', 0xff, /* scan 50-57 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 58-5F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 60-67 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 68-6F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 70-77 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 78-7F */
	'\r', 0xff,  '/',  '*',
};

static unsigned char kbd_right_alt_xlate_german[] = {
	0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, /* scan 00-07 */
	 '{',  '[',  ']',  '}', '\\', 0xff, 0xff, 0xff, /* scan 08-0F */
	 '@', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 10-17 */
	0xff, 0xff, 0xff,  '~', 0xff, 0xff, 0xff, 0xff, /* scan 18-1F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 20-27 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 28-2F */
	0xff, 0xff, 0xe6, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 30-37 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 38-3F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 40-47 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* scan 48-4F */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  '|', 0xff, /* scan 50-57 */
};

enum kbd_mask {
	KBD_ENGLISH	= 1 << 0,
	KBD_GERMAN	= 1 << 1,
};

static struct kbd_entry {
	int kbd_mask;		/* Which languages this is for */
	int left_keycode;	/* Left keycode to select this map */
	int right_keycode;	/* Right keycode to select this map */
	const uchar *xlate;	/* Ascii code for each keycode */
	int num_entries;	/* Number of entries in xlate */
} kbd_entry[] = {
	{ KBD_ENGLISH, -1, -1,
		kbd_plain_xlate, ARRAY_SIZE(kbd_plain_xlate) },
	{ KBD_GERMAN, -1, -1,
		kbd_plain_xlate_german, ARRAY_SIZE(kbd_plain_xlate_german) },
	{ KBD_ENGLISH, KEY_LEFTSHIFT, KEY_RIGHTSHIFT,
		kbd_shift_xlate, ARRAY_SIZE(kbd_shift_xlate) },
	{ KBD_GERMAN, KEY_LEFTSHIFT, KEY_RIGHTSHIFT,
		kbd_shift_xlate_german, ARRAY_SIZE(kbd_shift_xlate_german) },
	{ KBD_ENGLISH | KBD_GERMAN, KEY_LEFTCTRL, KEY_RIGHTCTRL,
		kbd_ctrl_xlate, ARRAY_SIZE(kbd_ctrl_xlate) },
	{ KBD_GERMAN, -1, KEY_RIGHTALT,
		kbd_right_alt_xlate_german,
		ARRAY_SIZE(kbd_right_alt_xlate_german) },
	{},
};

/*
 * Scan key code to ANSI 3.64 escape sequence table.  This table is
 * incomplete in that it does not include all possible extra keys.
 */
static struct {
	int kbd_scan_code;
	char *escape;
} kbd_to_ansi364[] = {
	{ KEY_UP, "\033[A"},
	{ KEY_DOWN, "\033[B"},
	{ KEY_RIGHT, "\033[C"},
	{ KEY_LEFT, "\033[D"},
};

/* Maximum number of output characters that an ANSI sequence expands to */
#define ANSI_CHAR_MAX	3

static int input_queue_ascii(struct input_config *config, int ch)
{
	if (config->fifo_in + 1 == INPUT_BUFFER_LEN) {
		if (!config->fifo_out)
			return -1; /* buffer full */
		else
			config->fifo_in = 0;
	} else {
		if (config->fifo_in + 1 == config->fifo_out)
			return -1; /* buffer full */
		config->fifo_in++;
	}
	debug(" {%02x} ", ch);
	config->fifo[config->fifo_in] = (uchar)ch;

	return 0;
}

int input_tstc(struct input_config *config)
{
	if (config->fifo_in == config->fifo_out && config->read_keys) {
		if (!(*config->read_keys)(config))
			return 0;
	}
	return config->fifo_in != config->fifo_out;
}

int input_getc(struct input_config *config)
{
	int err = 0;

	while (config->fifo_in == config->fifo_out) {
		if (config->read_keys)
			err = (*config->read_keys)(config);
		if (err)
			return -1;
	}

	if (++config->fifo_out == INPUT_BUFFER_LEN)
		config->fifo_out = 0;

	return config->fifo[config->fifo_out];
}

/**
 * Process a modifier/special key press or release and decide which key
 * translation array should be used as a result.
 *
 * TODO: Should keep track of modifier press/release
 *
 * @param config	Input state
 * @param key		Key code to process
 * @param release	0 if a press, 1 if a release
 * @return pointer to keycode->ascii translation table that should be used
 */
static struct input_key_xlate *process_modifier(struct input_config *config,
						int key, int release)
{
#ifdef CONFIG_DM_KEYBOARD
	struct udevice *dev = config->dev;
	struct keyboard_ops *ops = keyboard_get_ops(dev);
#endif
	struct input_key_xlate *table;
	int i;

	/* Start with the main table, and see what modifiers change it */
	assert(config->num_tables > 0);
	table = &config->table[0];
	for (i = 1; i < config->num_tables; i++) {
		struct input_key_xlate *tab = &config->table[i];

		if (key == tab->left_keycode || key == tab->right_keycode)
			table = tab;
	}

	/* Handle the lighted keys */
	if (!release) {
		int flip = -1;

		switch (key) {
		case KEY_SCROLLLOCK:
			flip = FLAG_SCROLL_LOCK;
			break;
		case KEY_NUMLOCK:
			flip = FLAG_NUM_LOCK;
			break;
		case KEY_CAPSLOCK:
			flip = FLAG_CAPS_LOCK;
			break;
		}

		if (flip != -1) {
			int leds = 0;

			config->flags ^= flip;
			if (config->flags & FLAG_NUM_LOCK)
				leds |= INPUT_LED_NUM;
			if (config->flags & FLAG_CAPS_LOCK)
				leds |= INPUT_LED_CAPS;
			if (config->flags & FLAG_SCROLL_LOCK)
				leds |= INPUT_LED_SCROLL;
			config->leds = leds;
			config->leds_changed = flip;

#ifdef CONFIG_DM_KEYBOARD
			if (ops->update_leds) {
				if (ops->update_leds(dev, config->leds))
					debug("Update keyboard's LED failed\n");
			}
#endif
		}
	}

	return table;
}

/**
 * Search an int array for a key value
 *
 * @param array	Array to search
 * @param count	Number of elements in array
 * @param key	Key value to find
 * @return element where value was first found, -1 if none
 */
static int array_search(int *array, int count, int key)
{
	int i;

	for (i = 0; i < count; i++) {
		if (array[i] == key)
			return i;
	}

	return -1;
}

/**
 * Sort an array so that those elements that exist in the ordering are
 * first in the array, and in the same order as the ordering. The algorithm
 * is O(count * ocount) and designed for small arrays.
 *
 * TODO: Move this to common / lib?
 *
 * @param dest		Array with elements to sort, also destination array
 * @param count		Number of elements to sort
 * @param order		Array containing ordering elements
 * @param ocount	Number of ordering elements
 * @return number of elements in dest that are in order (these will be at the
 *	start of dest).
 */
static int sort_array_by_ordering(int *dest, int count, int *order,
				   int ocount)
{
	int temp[count];
	int dest_count;
	int same;	/* number of elements which are the same */
	int i;

	/* setup output items, copy items to be sorted into our temp area */
	memcpy(temp, dest, count * sizeof(*dest));
	dest_count = 0;

	/* work through the ordering, move over the elements we agree on */
	for (i = 0; i < ocount; i++) {
		if (array_search(temp, count, order[i]) != -1)
			dest[dest_count++] = order[i];
	}
	same = dest_count;

	/* now move over the elements that are not in the ordering */
	for (i = 0; i < count; i++) {
		if (array_search(order, ocount, temp[i]) == -1)
			dest[dest_count++] = temp[i];
	}
	assert(dest_count == count);
	return same;
}

/**
 * Check a list of key codes against the previous key scan
 *
 * Given a list of new key codes, we check how many of these are the same
 * as last time.
 *
 * @param config	Input state
 * @param keycode	List of key codes to examine
 * @param num_keycodes	Number of key codes
 * @param same		Returns number of key codes which are the same
 */
static int input_check_keycodes(struct input_config *config,
			   int keycode[], int num_keycodes, int *same)
{
	/* Select the 'plain' xlate table to start with */
	if (!config->num_tables) {
		debug("%s: No xlate tables: cannot decode keys\n", __func__);
		return -1;
	}

	/* sort the keycodes into the same order as the previous ones */
	*same = sort_array_by_ordering(keycode, num_keycodes,
			config->prev_keycodes, config->num_prev_keycodes);

	memcpy(config->prev_keycodes, keycode, num_keycodes * sizeof(int));
	config->num_prev_keycodes = num_keycodes;

	return *same != num_keycodes;
}

/**
 * Checks and converts a special key code into ANSI 3.64 escape sequence.
 *
 * @param config	Input state
 * @param keycode	Key code to examine
 * @param output_ch	Buffer to place output characters into. It should
 *			be at least ANSI_CHAR_MAX bytes long, to allow for
 *			an ANSI sequence.
 * @param max_chars	Maximum number of characters to add to output_ch
 * @return number of characters output, if the key was converted, otherwise 0.
 *	This may be larger than max_chars, in which case the overflow
 *	characters are not output.
 */
static int input_keycode_to_ansi364(struct input_config *config,
		int keycode, char output_ch[], int max_chars)
{
	const char *escape;
	int ch_count;
	int i;

	for (i = ch_count = 0; i < ARRAY_SIZE(kbd_to_ansi364); i++) {
		if (keycode != kbd_to_ansi364[i].kbd_scan_code)
			continue;
		for (escape = kbd_to_ansi364[i].escape; *escape; escape++) {
			if (ch_count < max_chars)
				output_ch[ch_count] = *escape;
			ch_count++;
		}
		return ch_count;
	}

	return 0;
}

/**
 * Converts and queues a list of key codes in escaped ASCII string form
 * Convert a list of key codes into ASCII
 *
 * You must call input_check_keycodes() before this. It turns the keycode
 * list into a list of ASCII characters and sends them to the input layer.
 *
 * Characters which were seen last time do not generate fresh ASCII output.
 * The output (calls to queue_ascii) may be longer than num_keycodes, if the
 * keycode contains special keys that was encoded to longer escaped sequence.
 *
 * @param config	Input state
 * @param keycode	List of key codes to examine
 * @param num_keycodes	Number of key codes
 * @param output_ch	Buffer to place output characters into. It should
 *			be at last ANSI_CHAR_MAX * num_keycodes, to allow for
 *			ANSI sequences.
 * @param max_chars	Maximum number of characters to add to output_ch
 * @param same		Number of key codes which are the same
 * @return number of characters written into output_ch, or -1 if we would
 *	exceed max_chars chars.
 */
static int input_keycodes_to_ascii(struct input_config *config,
		int keycode[], int num_keycodes, char output_ch[],
		int max_chars, int same)
{
	struct input_key_xlate *table;
	int ch_count = 0;
	int i;

	table = &config->table[0];

	/* deal with modifiers first */
	for (i = 0; i < num_keycodes; i++) {
		int key = keycode[i] & KEY_MASK;

		if (key >= table->num_entries || table->xlate[key] == 0xff) {
			table = process_modifier(config, key,
					keycode[i] & KEY_RELEASE);
		}
	}

	/* Start conversion by looking for the first new keycode (by same). */
	for (i = same; i < num_keycodes; i++) {
		int key = keycode[i];
		int ch;

		/*
		 * For a normal key (with an ASCII value), add it; otherwise
		 * translate special key to escape sequence if possible.
		 */
		if (key < table->num_entries) {
			ch = table->xlate[key];
			if ((config->flags & FLAG_CAPS_LOCK) &&
			    ch >= 'a' && ch <= 'z')
				ch -= 'a' - 'A';
			/* ban digit numbers if 'Num Lock' is not on */
			if (!(config->flags & FLAG_NUM_LOCK)) {
				if (key >= KEY_KP7 && key <= KEY_KPDOT &&
				    key != KEY_KPMINUS && key != KEY_KPPLUS)
					ch = 0xff;
			}
			if (ch_count < max_chars && ch != 0xff)
				output_ch[ch_count++] = (uchar)ch;
		} else {
			ch_count += input_keycode_to_ansi364(config, key,
						output_ch, max_chars);
		}
	}

	if (ch_count > max_chars) {
		debug("%s: Output char buffer overflow size=%d, need=%d\n",
		      __func__, max_chars, ch_count);
		return -1;
	}

	/* ok, so return keys */
	return ch_count;
}

static int _input_send_keycodes(struct input_config *config, int keycode[],
				int num_keycodes, bool do_send)
{
	char ch[num_keycodes * ANSI_CHAR_MAX];
	int count, i, same = 0;
	int is_repeat = 0;
	unsigned delay_ms;

	config->modifiers = 0;
	if (!input_check_keycodes(config, keycode, num_keycodes, &same)) {
		/*
		 * Same as last time - is it time for another repeat?
		 * TODO(sjg@chromium.org) We drop repeats here and since
		 * the caller may not call in again for a while, our
		 * auto-repeat speed is not quite correct. We should
		 * insert another character if we later realise that we
		 * have missed a repeat slot.
		 */
		is_repeat = config->allow_repeats || (config->repeat_rate_ms &&
			(int)get_timer(config->next_repeat_ms) >= 0);
		if (!is_repeat)
			return 0;
	}

	count = input_keycodes_to_ascii(config, keycode, num_keycodes,
					ch, sizeof(ch), is_repeat ? 0 : same);
	if (do_send) {
		for (i = 0; i < count; i++)
			input_queue_ascii(config, ch[i]);
	}
	delay_ms = is_repeat ?
			config->repeat_rate_ms :
			config->repeat_delay_ms;

	config->next_repeat_ms = get_timer(0) + delay_ms;

	return count;
}

int input_send_keycodes(struct input_config *config, int keycode[],
			int num_keycodes)
{
	return _input_send_keycodes(config, keycode, num_keycodes, true);
}

int input_add_keycode(struct input_config *config, int new_keycode,
		      bool release)
{
	int keycode[INPUT_MAX_MODIFIERS + 1];
	int count, i;

	/* Add the old keycodes which are not removed by this new one */
	for (i = 0, count = 0; i < config->num_prev_keycodes; i++) {
		int code = config->prev_keycodes[i];

		if (new_keycode == code) {
			if (release)
				continue;
			new_keycode = -1;
		}
		keycode[count++] = code;
	}

	if (!release && new_keycode != -1)
		keycode[count++] = new_keycode;
	debug("\ncodes for %02x/%d: ", new_keycode, release);
	for (i = 0; i < count; i++)
		debug("%02x ", keycode[i]);
	debug("\n");

	/* Don't output any ASCII characters if this is a key release */
	return _input_send_keycodes(config, keycode, count, !release);
}

int input_add_table(struct input_config *config, int left_keycode,
		    int right_keycode, const uchar *xlate, int num_entries)
{
	struct input_key_xlate *table;

	if (config->num_tables == INPUT_MAX_MODIFIERS) {
		debug("%s: Too many modifier tables\n", __func__);
		return -1;
	}

	table = &config->table[config->num_tables++];
	table->left_keycode = left_keycode;
	table->right_keycode = right_keycode;
	table->xlate = xlate;
	table->num_entries = num_entries;

	return 0;
}

void input_set_delays(struct input_config *config, int repeat_delay_ms,
	       int repeat_rate_ms)
{
	config->repeat_delay_ms = repeat_delay_ms;
	config->repeat_rate_ms = repeat_rate_ms;
}

void input_allow_repeats(struct input_config *config, bool allow_repeats)
{
	config->allow_repeats = allow_repeats;
}

int input_leds_changed(struct input_config *config)
{
	if (config->leds_changed)
		return config->leds;

	return -1;
}

int input_add_tables(struct input_config *config, bool german)
{
	struct kbd_entry *entry;
	int mask;
	int ret;

	mask = german ? KBD_GERMAN : KBD_ENGLISH;
	for (entry = kbd_entry; entry->kbd_mask; entry++) {
		if (!(mask & entry->kbd_mask))
			continue;
		ret = input_add_table(config, entry->left_keycode,
				      entry->right_keycode, entry->xlate,
				      entry->num_entries);
		if (ret)
			return ret;
	}

	return 0;
}

int input_init(struct input_config *config, int leds)
{
	memset(config, '\0', sizeof(*config));
	config->leds = leds;

	return 0;
}

int input_stdio_register(struct stdio_dev *dev)
{
	int error;

	error = stdio_register(dev);
#if !defined(CONFIG_SPL_BUILD) || CONFIG_IS_ENABLED(ENV_SUPPORT)
	/* check if this is the standard input device */
	if (!error && strcmp(env_get("stdin"), dev->name) == 0) {
		/* reassign the console */
		if (OVERWRITE_CONSOLE ||
				console_assign(stdin, dev->name))
			return -1;
	}
#else
	error = error;
#endif

	return 0;
}
