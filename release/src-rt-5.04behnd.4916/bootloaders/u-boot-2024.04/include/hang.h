/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __HANG_H
#define __HANG_H

#ifndef __ASSEMBLY__
/**
 * hang() - Print a message and stop execution
 *
 * This shows a 'hang' message where possible and then goes into an infinite
 * loop. This is called by panic() if CONFIG_PANIC_HANG is enabled.
 *
 * This function does not return.
 */
void hang(void) __attribute__ ((noreturn));
#endif

#endif
