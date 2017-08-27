/*
 * SiliconBackplane Sharedcommon core hardware definitions.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: sbsharedc.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef	_SBSHAREDC_H
#define	_SBSHAREDC_H

#ifndef _LANGUAGE_ASSEMBLY

/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

typedef volatile struct {
	uint32	capabilities;		/* 0x0 */
	uint32	PAD[2];
	uint32	bist;			/* 0xc */
	uint32	bankstbyctl;		/* 0x10 */
	uint32	PAD[3];
	uint32	wlintstatus;		/* 0x20 */
	uint32	wlintmask;		/* 0x24 */
	uint32	btintstatus;		/* 0x28 */
	uint32	btintmask;		/* 0x2c */
	uint32	wlsem;			/* 0x30 */
	uint32	PAD[3];
	uint32	btsem;			/* 0x40 */
} sharedcregs_t;

#endif /* _LANGUAGE_ASSEMBLY */

/* capabilities */
#define SC_CAP_SMEMSIZE_MASK	0x0000000f
#define SC_CAP_NUMSEMS_MASK	0x000000f0
#define SC_CAP_NUMSEMS_SHIFT	4

/* wlan/bt intstatus and intmask */
#define SC_I_SEM(s)		(1 << (s))

/* wlan/bt semaphore */
#define SC_SEM_REQ(s)	(1 << ((s) << 1))	/* Request */
#define SC_SEM_GIVE(s)	(2 << ((s) << 1))	/* Release */
#define SC_SEM_NAV(s)	(3 << ((s) << 1))	/* Unavailable */

#endif	/* _SBSHAREDC_H */
