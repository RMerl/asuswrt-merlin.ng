/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Watchdog driver for the FTWDT010 Watch Dog Driver
 *
 * (c) Copyright 2004 Faraday Technology Corp. (www.faraday-tech.com)
 * Based on sa1100_wdt.c by Oleg Drokin <green@crimea.edu>
 * Based on SoftDog driver by Alan Cox <alan@redhat.com>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 *
 * 27/11/2004 Initial release, Faraday.
 * 12/01/2011 Port to u-boot, Macpaul Lin.
 */

#ifndef __FTWDT010_H
#define __FTWDT010_H

struct ftwdt010_wdt {
	unsigned int	wdcounter;	/* Counter Reg		- 0x00 */
	unsigned int	wdload;		/* Counter Auto Reload Reg - 0x04 */
	unsigned int	wdrestart;	/* Counter Restart Reg	- 0x08 */
	unsigned int	wdcr;		/* Control Reg		- 0x0c */
	unsigned int	wdstatus;	/* Status Reg		- 0x10 */
	unsigned int	wdclear;	/* Timer Clear		- 0x14 */
	unsigned int	wdintrlen;	/* Interrupt Length	- 0x18 */
};

/*
 * WDLOAD - Counter Auto Reload Register
 *   The Auto Reload Register is set to 0x03EF1480 (66Mhz) by default.
 *   Which means in a 66MHz system, the period of Watch Dog timer reset is
 *   one second.
 */
#define FTWDT010_WDLOAD(x)		((x) & 0xffffffff)

/*
 * WDRESTART - Watch Dog Timer Counter Restart Register
 *   If writing 0x5AB9 to WDRESTART register, Watch Dog timer will
 *   automatically reload WDLOAD to WDCOUNTER and restart counting.
 */
#define FTWDT010_WDRESTART_MAGIC	0x5AB9

/* WDCR - Watch Dog Timer Control Register */
#define FTWDT010_WDCR_ENABLE		(1 << 0)
#define FTWDT010_WDCR_RST		(1 << 1)
#define FTWDT010_WDCR_INTR		(1 << 2)
/* FTWDT010_WDCR_EXT bit: Watch Dog Timer External Signal Enable */
#define FTWDT010_WDCR_EXT		(1 << 3)
/* FTWDT010_WDCR_CLOCK bit: Clock Source: 0: PCLK, 1: EXTCLK.
 *  The clock source PCLK cannot be gated when system sleeps, even if
 *  WDCLOCK bit is turned on.
 *
 *  Faraday's Watch Dog timer can be driven by an external clock. The
 *  programmer just needs to write one to WdCR[WdClock] bit.
 *
 *  Note: There is a limitation between EXTCLK and PCLK:
 *  EXTCLK cycle time / PCLK cycle time > 2.
 *  If the system does not need an external clock,
 *  just keep WdCR[WdClock] bit in its default value.
 */
#define FTWDT010_WDCR_CLOCK		(1 << 4)

/*
 * WDSTATUS - Watch Dog Timer Status Register
 *   This bit is set when the counter reaches Zero
 */
#define FTWDT010_WDSTATUS(x)		((x) & 0x1)

/*
 * WDCLEAR - Watch Dog Timer Clear Register
 *   Writing one to this register will clear WDSTATUS.
 */
#define FTWDT010_WDCLEAR		(1 << 0)

/*
 * WDINTRLEN - Watch Dog Timer Interrupt Length
 *   This register controls the duration length of wd_rst, wd_intr and wd_ext.
 *   The default value is 0xFF.
 */
#define FTWDT010_WDINTRLEN(x)		((x) & 0xff)

/*
 * Variable timeout should be set in ms.
 * (CONFIG_SYS_CLK_FREQ/1000) equals 1 ms.
 * WDLOAD = timeout * TIMEOUT_FACTOR.
 */
#define FTWDT010_TIMEOUT_FACTOR		(CONFIG_SYS_CLK_FREQ / 1000) /* 1 ms */

void ftwdt010_wdt_reset(void);
void ftwdt010_wdt_disable(void);

#endif /* __FTWDT010_H */
