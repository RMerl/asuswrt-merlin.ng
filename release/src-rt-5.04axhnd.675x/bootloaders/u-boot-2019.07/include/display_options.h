/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 *
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __DISPLAY_OPTIONS_H
#define __DISPLAY_OPTIONS_H

/**
 * print_size() - Print a size with a suffix
 *
 * Print sizes as "xxx KiB", "xxx.y KiB", "xxx MiB", "xxx.y MiB",
 * xxx GiB, xxx.y GiB, etc as needed; allow for optional trailing string
 * (like "\n")
 *
 * @size:	Size to print
 * @suffix	String to print after the size
 */
void print_size(uint64_t size, const char *suffix);

/**
 * print_freq() - Print a frequency with a suffix
 *
 * Print frequencies as "x.xx GHz", "xxx KHz", etc as needed; allow for
 * optional trailing string (like "\n")
 *
 * @freq:	Frequency to print in Hz
 * @suffix	String to print after the frequency
 */
void print_freq(uint64_t freq, const char *suffix);

/**
 * print_buffer() - Print data buffer in hex and ascii form
 *
 * Data reads are buffered so that each memory address is only read once.
 * This is useful when displaying the contents of volatile registers.
 *
 * @addr:	Starting address to display at start of line
 * @data:	pointer to data buffer
 * @width:	data value width.  May be 1, 2, or 4.
 * @count:	number of values to display
 * @linelen:	Number of values to print per line; specify 0 for default length
 */
int print_buffer(ulong addr, const void *data, uint width, uint count,
		 uint linelen);

/**
 * display_options() - display the version string / build tag
 *
 * This displays the U-Boot version string. If a build tag is available this
 * is displayed also.
 */
int display_options(void);

/* Suggested length of the buffer to pass to display_options_get_banner() */
#define DISPLAY_OPTIONS_BANNER_LENGTH	200

/**
 * display_options_get_banner() - Get the U-Boot banner as a string
 *
 * This returns the U-Boot banner string
 *
 * @newlines: true to include two newlines at the start
 * @buf: place to put string
 * @size: Size of buf (string is truncated to fit)
 * @return buf
 */
char *display_options_get_banner(bool newlines, char *buf, int size);

/* This function is used for testing only */
char *display_options_get_banner_priv(bool newlines, const char *build_tag,
				      char *buf, int size);

#endif
