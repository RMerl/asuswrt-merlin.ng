// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Andreas Bießmann <andreas@biessmann.org>
 *
 * This file consolidates all the different hang() functions implemented in
 * u-boot.
 */

#include <common.h>
#include <bootstage.h>

/**
 * hang - stop processing by staying in an endless loop
 *
 * The purpose of this function is to stop further execution of code cause
 * something went completely wrong.  To catch this and give some feedback to
 * the user one needs to catch the bootstage_error (see show_boot_progress())
 * in the board code.
 */
void hang(void)
{
#if !defined(CONFIG_SPL_BUILD) || \
		(CONFIG_IS_ENABLED(LIBCOMMON_SUPPORT) && \
		 CONFIG_IS_ENABLED(SERIAL_SUPPORT))
	puts("### ERROR ### Please RESET the board ###\n");
#endif
	bootstage_error(BOOTSTAGE_ID_NEED_RESET);
	for (;;)
		;
}
