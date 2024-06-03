// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 */

/*
 * cleanup_before_linux() - Prepare the CPU to jump to Linux
 *
 * This function is called just before we call Linux, it
 * prepares the processor for linux
 */
int cleanup_before_linux(void)
{
	return 0;
}
