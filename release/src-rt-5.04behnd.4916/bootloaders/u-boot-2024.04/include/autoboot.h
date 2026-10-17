/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ifndef __AUTOBOOT_H
#define __AUTOBOOT_H

#include <stdbool.h>

#ifdef CONFIG_SANDBOX

/**
 * autoboot_keyed() - check whether keyed autoboot should be used
 *
 * This is only implemented for sandbox since other platforms don't have a way
 * of controlling the feature at runtime.
 *
 * Return: true if enabled, false if not
 */
bool autoboot_keyed(void);

/**
 * autoboot_set_keyed() - set whether keyed autoboot should be used
 *
 * @autoboot_keyed: true to enable the feature, false to disable
 * Return: old value of the flag
 */
bool autoboot_set_keyed(bool autoboot_keyed);
#else
static inline bool autoboot_keyed(void)
{
	/* There is no runtime flag, so just use the CONFIG */
	return IS_ENABLED(CONFIG_AUTOBOOT_KEYED);
}

static inline bool autoboot_set_keyed(bool autoboot_keyed)
{
	/* There is no runtime flag to set */
	return false;
}

#endif

#ifdef CONFIG_AUTOBOOT
/**
 * bootdelay_process() - process the bootd delay
 *
 * Process the boot delay, boot limit, then get the value of either
 * bootcmd, failbootcmd or altbootcmd depending on the current state.
 * Return this command so it can be executed.
 *
 * Return: command to executed
 */
const char *bootdelay_process(void);

/**
 * autoboot_command() - run the autoboot command
 *
 * If enabled, run the autoboot command returned from bootdelay_process().
 * Also do the CONFIG_AUTOBOOT_MENUKEY processing if enabled.
 *
 * @cmd: Command to run
 */
void autoboot_command(const char *cmd);
#else
static inline const char *bootdelay_process(void)
{
	return NULL;
}

static inline void autoboot_command(const char *s)
{
}
#endif

#endif
