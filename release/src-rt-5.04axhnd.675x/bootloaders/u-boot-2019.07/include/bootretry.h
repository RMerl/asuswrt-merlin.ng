/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __BOOTRETRY_H
#define __BOOTRETRY_H

#ifdef CONFIG_BOOT_RETRY_TIME
/**
 * bootretry_tstc_timeout() - ensure we get a keypress before timeout
 *
 * Check for a keypress repeatedly, resetting the watchdog each time. If a
 * keypress is not received within the command timeout, return an error.
 *
 * @return 0 if a key is received in time, -ETIMEDOUT if not
 */
int bootretry_tstc_timeout(void);

/**
 * bootretry_init_cmd_timeout() - set up command timeout
 *
 * Get the required command timeout from the environment.
 */
void bootretry_init_cmd_timeout(void);

/**
 * bootretry_reset_cmd_timeout() - reset command timeout
 *
 * Reset the command timeout so that the user has a fresh start. This is
 * typically used when input is received from the user.
 */
void bootretry_reset_cmd_timeout(void);

/** bootretry_dont_retry() - Indicate that we should not retry the boot */
void bootretry_dont_retry(void);
#else
static inline int bootretry_tstc_timeout(void)
{
	return 0;
}

static inline void bootretry_init_cmd_timeout(void)
{
}

static inline void bootretry_reset_cmd_timeout(void)
{
}

static inline void bootretry_dont_retry(void)
{
}

#endif

#endif
