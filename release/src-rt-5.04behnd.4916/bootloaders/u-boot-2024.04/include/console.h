/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdbool.h>
#include <stdio_dev.h>
#include <linux/errno.h>

extern char console_buffer[];

/* common/console.c */
int console_init_f(void);	/* Before relocation; uses the serial  stuff */
int console_init_r(void);	/* After  relocation; uses the console stuff */
int console_start(int file, struct stdio_dev *sdev);	/* Start a console device */
void console_stop(int file, struct stdio_dev *sdev);	/* Stop a console device */
int console_assign(int file, const char *devname);	/* Assign the console */
int ctrlc(void);
int had_ctrlc(void);	/* have we had a Control-C since last clear? */
void clear_ctrlc(void);	/* clear the Control-C condition */
int disable_ctrlc(int);	/* 1 to disable, 0 to enable Control-C detect */
int confirm_yesno(void);        /*  1 if input is "y", "Y", "yes" or "YES" */

/**
 * console_search_dev() - search for stdio device with given flags and name
 * @flags: device flags as per input/output/system
 * @name: device name
 *
 * Iterates over registered STDIO devices and match them with given @flags
 * and @name.
 *
 * Return: pointer to the &struct stdio_dev if found, or NULL otherwise
 */
struct stdio_dev *console_search_dev(int flags, const char *name);

#ifdef CONFIG_CONSOLE_RECORD
/**
 * console_record_init() - set up the console recording buffers
 *
 * This should be called as soon as malloc() is available so that the maximum
 * amount of console output can be recorded.
 *
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int console_record_init(void);

/**
 * console_record_reset() - reset the console recording buffers
 *
 * Removes any data in the buffers
 */
void console_record_reset(void);

/**
 * console_record_reset_enable() - reset and enable the console buffers
 *
 * This should be called to enable the console buffer.
 *
 * Return: 0 (always)
 */
int console_record_reset_enable(void);

/**
 * console_record_readline() - Read a line from the console output
 *
 * This reads the next available line from the console output previously
 * recorded.
 *
 * @str: Place to put string
 * @maxlen: Maximum length of @str including nul terminator
 * Return: length of string returned, or -ENOSPC if the console buffer was
 *	overflowed by the output
 */
int console_record_readline(char *str, int maxlen);

/**
 * console_record_avail() - Get the number of available bytes in console output
 *
 * Return: available bytes (0 if empty)
 */
int console_record_avail(void);

/**
 * console_record_isempty() - Returns if console output is empty
 *
 * Return: true if empty
 */
bool console_record_isempty(void);

/**
 * console_in_puts() - Write a string to the console input buffer
 *
 * This writes the given string to the console_in buffer which will then be
 * returned if a function calls e.g. `getc()`
 *
 * @str: the string to write
 * Return:  the number of bytes added
 */
int console_in_puts(const char *str);
#else
static inline int console_record_init(void)
{
	/* Always succeed, since it is not enabled */

	return 0;
}

static inline void console_record_reset(void)
{
	/* Nothing to do here */
}

static inline int console_record_reset_enable(void)
{
	/* Cannot enable it as it is not supported */
	return -ENOSYS;
}

static inline int console_record_readline(char *str, int maxlen)
{
	/* Nothing to read */
	return 0;
}

static inline int console_record_avail(void)
{
	/* There is never anything available */
	return 0;
}

static inline int console_in_puts(const char *str)
{
	/* There is never anything written */
	return 0;
}

static inline bool console_record_isempty(void)
{
	/* Always empty */
	return true;
}

#endif /* !CONFIG_CONSOLE_RECORD */

/**
 * console_announce_r() - print a U-Boot console on non-serial consoles
 *
 * When U-Boot starts up with a display it generally does not announce itself
 * on the display. The banner is instead emitted on the UART before relocation.
 * This function prints a banner on devices which (we assume) did not receive
 * it before relocation.
 *
 * Return: 0 (meaning no errors)
 */
int console_announce_r(void);

/**
 * console_puts_select_stderr() - Output a string to selected console devices
 *
 * This writes to stderr only. It is useful for outputting errors
 *
 * @serial_only: true to output only to serial, false to output to everything
 *	else
 * @s: String to output
 */
void console_puts_select_stderr(bool serial_only, const char *s);

/**
 * console_clear() - Clear the console
 *
 * Uses an ANSI sequence to clear the display, failing back to clearing the
 * video display directly if !CONFIG_VIDEO_ANSI
 *
 * Return: 0 if OK, -ve on error
 */
int console_clear(void);

/*
 * CONSOLE multiplexing.
 */
#ifdef CONFIG_CONSOLE_MUX
#include <iomux.h>
#endif

#endif
