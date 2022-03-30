/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Keyboard matrix helper functions
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#ifndef _KEY_MATRIX_H
#define _KEY_MATRIX_H

#include <common.h>

/* Information about a matrix keyboard */
struct key_matrix {
	/* Dimensions of the keyboard matrix, in rows and columns */
	int num_rows;
	int num_cols;
	int key_count;	/* number of keys in the matrix (= rows * cols) */

	/*
	 * Information about keycode mappings. The plain_keycode array must
	 * exist but fn may be NULL in which case it is not decoded.
	 */
	const u8 *plain_keycode;        /* key code for each row / column */
	const u8 *fn_keycode;           /* ...when Fn held down */
	int fn_pos;                     /* position of Fn key in key (or -1) */
	int ghost_filter;		/* non-zero to enable ghost filter */
};

/* Information about a particular key (row, column pair) in the matrix */
struct key_matrix_key {
	uint8_t row;	/* row number (0 = first) */
	uint8_t col;	/* column number (0 = first) */
	uint8_t valid;	/* 1 if valid, 0 to ignore this */
};

/**
 * Decode a set of pressed keys into key codes
 *
 * Given a list of keys that are pressed, this converts this list into
 * a list of key codes. Each of the keys has a valid flag, which can be
 * used to mark a particular key as invalid (so that it is ignored).
 *
 * The plain keymap is used, unless the Fn key is detected along the way,
 * at which point we switch to the Fn key map.
 *
 * If key ghosting is detected, we simply ignore the keys and return 0.
 *
 * @param config        Keyboard matrix config
 * @param keys		List of keys to process (each is row, col)
 * @param num_keys	Number of keys to process
 * @param keycode	Returns a list of key codes, decoded from input
 * @param max_keycodes	Size of key codes array (suggest 8)
 *
 */
int key_matrix_decode(struct key_matrix *config, struct key_matrix_key *keys,
		      int num_keys, int keycode[], int max_keycodes);

/**
 * Read the keyboard configuration out of the fdt.
 *
 * Decode properties of named "linux,<type>keymap" where <type> is either
 * empty, or "fn-". Then set up the plain key map (and the FN keymap if
 * present).
 *
 * @param config        Keyboard matrix config
 * @param blob          FDT blob
 * @param node          Node containing compatible data
 * @return 0 if ok, -1 on error
 */
int key_matrix_decode_fdt(struct udevice *dev, struct key_matrix *config);

/**
 * Set up a new key matrix.
 *
 * @param config	Keyboard matrix config
 * @param rows		Number of rows in key matrix
 * @param cols		Number of columns in key matrix
 * @param ghost_filter	Non-zero to enable ghost filtering
 * @return 0 if ok, -1 on error
 */
int key_matrix_init(struct key_matrix *config, int rows, int cols,
		    int ghost_filter);

#endif
