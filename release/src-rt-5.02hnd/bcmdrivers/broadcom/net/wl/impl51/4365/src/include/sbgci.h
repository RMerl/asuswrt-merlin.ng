/*
 * SiliconBackplane GCI core hardware definitions
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
 * $Id: sbgci.h 502186 2014-09-12 02:52:10Z $
 */

#ifndef _SBGCI_H
#define _SBGCI_H

#if !defined(_LANGUAGE_ASSEMBLY) && !defined(__ASSEMBLY__)

/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

typedef volatile struct {
	uint32	gci_corecaps0;		/* 0x000 */
	uint32	gci_corecaps1;		/* 0x004 */
	uint32	gci_corecaps2;		/* 0x008 */
	uint32	gci_corectrl;		/* 0x00c */
	uint32	gci_corestat;		/* 0x010 */
	uint32	gci_intstat;		/* 0x014 */
	uint32	gci_intmask;		/* 0x018 */
	uint32	gci_wakemask;		/* 0x01c */
	uint32	gci_levelintstat;	/* 0x020 */
	uint32	gci_eventintstat;	/* 0x024 */
	uint32	gci_wakelevelintstat;	/* 0x028 */
	uint32	gci_wakeeventintstat;	/* 0x02c */
	uint32	semaphoreintstatus;	/* 0x030 */
	uint32	semaphoreintmask;	/* 0x034 */
	uint32	semaphorerequest;	/* 0x038 */
	uint32	semaphorereserve;	/* 0x03c */
	uint32	gci_indirect_addr;	/* 0x040 */
	uint32	gci_gpioctl;		/* 0x044 */
	uint32	gci_gpiostatus;		/* 0x048 */
	uint32	gci_gpiomask;		/* 0x04c */
	uint32	eventsummary;		/* 0x050 */
	uint32	gci_miscctl;		/* 0x054 */
	uint32	gci_gpiointmask;	/* 0x058 */
	uint32	gci_gpiowakemask;	/* 0x05c */
	uint32	gci_input[32];		/* 0x060 */
	uint32	gci_event[32];		/* 0x0e0 */
	uint32	gci_output[4];		/* 0x160 */
	uint32	gci_control_0;		/* 0x170 */
	uint32	gci_control_1;		/* 0x174 */
	uint32	gci_intpolreg;		/* 0x178 */
	uint32	gci_levelintmask;	/* 0x17c */
	uint32	gci_eventintmask;	/* 0x180 */
	uint32	wakelevelintmask;	/* 0x184 */
	uint32	wakeeventintmask;	/* 0x188 */
	uint32	hwmask;			/* 0x18c */
	uint32	PAD;
	uint32	gci_inbandeventintmask;	/* 0x194 */
	uint32	PAD;
	uint32	gci_inbandeventstatus;	/* 0x19c */
	uint32	gci_seciauxtx;		/* 0x1a0 */
	uint32	gci_seciauxrx;		/* 0x1a4 */
	uint32	gci_secitx_datatag;	/* 0x1a8 */
	uint32	gci_secirx_datatag;	/* 0x1ac */
	uint32	gci_secitx_datamask;	/* 0x1b0 */
	uint32	gci_seciusef0tx_reg;	/* 0x1b4 */
	uint32	gci_secif0tx_offset;	/* 0x1b8 */
	uint32	gci_secif0rx_offset;	/* 0x1bc */
	uint32	gci_secif1tx_offset;	/* 0x1c0 */
	uint32	gci_rxfifo_common_ctrl;	/* 0x1c4 */
	uint32	gci_rxfifoctrl;		/* 0x1c8 */
	uint32	PAD;
	uint32	gci_seciuartescval;	/* 0x1d0 */
	uint32	gic_seciuartautobaudctr;	/* 0x1d4 */
	uint32	gci_secififolevel;	/* 0x1d8 */
	uint32	gci_seciuartdata;	/* 0x1dc */
	uint32	gci_secibauddiv;	/* 0x1e0 */
	uint32	gci_secifcr;		/* 0x1e4 */
	uint32	gci_secilcr;		/* 0x1e8 */
	uint32	gci_secimcr;		/* 0x1ec */
	uint32	gci_secilsr;		/* 0x1f0 */
	uint32	gci_secimsr;		/* 0x1f4 */
	uint32	gci_baudadj;		/* 0x1f8 */
	uint32	gci_inbandintmask;	/* 0x1fc */
	uint32  gci_chipctrl;		/* 0x200 */
	uint32  gci_chipsts; 		/* 0x204 */
	uint32	gci_gpioout; 		/* 0x208 */
	uint32	gci_gpioout_read; 	/* 0x20C */
	uint32	gci_mpwaketx; 		/* 0x210 */
	uint32	gci_mpwakedetect; 	/* 0x214 */
	uint32	gci_seciin_ctrl; 	/* 0x218 */
	uint32	gci_seciout_ctrl; 	/* 0x21C */
	uint32	gci_seciin_auxfifo_en; 	/* 0x220 */
	uint32	gci_seciout_txen_txbr; 	/* 0x224 */
	uint32	gci_seciin_rxbrstatus; 	/* 0x228 */
	uint32	gci_seciin_rxerrstatus; /* 0x22C */
	uint32	gci_seciin_fcstatus; 	/* 0x230 */
	uint32	gci_seciout_txstatus; 	/* 0x234 */
	uint32	gci_seciout_txbrstatus; /* 0x238 */
	uint32	PAD[49];
	uint32	gci_chipid;		/* 0x300 */
	uint32	PAD[3];
	uint32	otpstatus;		/* 0x310 */
	uint32	otpcontrol;		/* 0x314 */
	uint32	otpprog;		/* 0x318 */
	uint32	otplayout;		/* 0x31c */
	uint32	otplayoutextension;	/* 0x320 */
	uint32	otpcontrol1;		/* 0x324 */
} gciregs_t;

#endif /* !_LANGUAGE_ASSEMBLY && !__ASSEMBLY__ */


#endif	/* _SBGCI_H */
