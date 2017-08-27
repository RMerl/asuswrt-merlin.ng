/*
 * BCM43XX M2M DMA core hardware definitions.
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
 * $Id:m2mdma _core.h 421139 2013-08-30 17:56:15Z kiranm $
 */
#ifndef	_M2MDMA_CORE_H
#define	_M2MDMA_CORE_H
#include <sbhnddma.h>
/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif

#define M2M_SHARED_VERSION       0x0001
#define M2M_SHARED_VERSION_MASK  0x00FF
#define M2M_SHARED_ASSERT_BUILT  0x0100
#define M2M_SHARED_ASSERT        0x0200
#define M2M_SHARED_TRAP          0x0400
#define M2M_SHARED_IN_BRPT       0x0800
#define M2M_SHARED_SET_BRPT      0x1000
#define M2M_SHARED_PENDING_BRPT  0x2000
/* dma regs to control the flow between host2dev and dev2host  */
typedef struct m2m_devdmaregs {
	dma64regs_t	tx;
	uint32 		PAD[2];
	dma64regs_t	rx;
	uint32 		PAD[2];
} m2m_devdmaregs_t;


typedef struct dmaintregs {
	uint32 intstatus;
	uint32 intmask;
} dmaintregs_t;


/* rx header */
typedef volatile struct {
	uint16 len;
	uint16 flags;
} m2md_rxh_t;

typedef struct {
	uint32	flags;
	uint32  trap_addr;
	uint32  assert_exp_addr;
	uint32  assert_file_addr;
	uint32  assert_line;
	uint32	console_addr;		/* Address of hnd_cons_t */
	uint32  msgtrace_addr;
	uint32  fwid;
} m2m_shared_t;

/*
 * Software counters (first part matches hardware counters)
 */

typedef volatile struct {
	uint32 rxdescuflo;	/* receive descriptor underflows */
	uint32 rxfifooflo;	/* receive fifo overflows */
	uint32 txfifouflo;	/* transmit fifo underflows */
	uint32 runt;		/* runt (too short) frames recv'd from bus */
	uint32 badlen;		/* frame's rxh len does not match its hw tag len */
	uint32 badcksum;	/* frame's hw tag chksum doesn't agree with len value */
	uint32 seqbreak;	/* break in sequence # space from one rx frame to the next */
	uint32 rxfcrc;		/* frame rx header indicates crc error */
	uint32 rxfwoos;		/* frame rx header indicates write out of sync */
	uint32 rxfwft;		/* frame rx header indicates write frame termination */
	uint32 rxfabort;	/* frame rx header indicates frame aborted */
	uint32 woosint;		/* write out of sync interrupt */
	uint32 roosint;		/* read out of sync interrupt */
	uint32 rftermint;	/* read frame terminate interrupt */
	uint32 wftermint;	/* write frame terminate interrupt */
} m2md_cnt_t;

/* SB side: M2M DMA core registers */
typedef struct m2mregs {
	uint32 control;		/* Core control 0x0 */
	uint32 capabilities;	/* Core capabilities 0x4 */
	uint32 intcontrol;	/* Interrupt control 0x8 */
	uint32 PAD[5];
	uint32 intstatus;	/* Interrupt Status  0x20 */
	uint32 PAD[3];
	dmaintregs_t intregs[8]; /* 0x30 - 0x6c */
	uint32 PAD[36];
	uint32 intrxlazy[8];	/* 0x100 - 0x11c */
	uint32 PAD[48];
	uint32 clockctlstatus;  /* 0x1e0 */
	uint32 workaround;	/* 0x1e4 */
	uint32 powercontrol;	/* 0x1e8 */
	uint32 PAD[5];
	m2m_devdmaregs_t dmaregs[8]; /* 0x200 - 0x3f4 */
} m2md_regs_t;


#endif	/* _M2MDMA_CORE_H */
