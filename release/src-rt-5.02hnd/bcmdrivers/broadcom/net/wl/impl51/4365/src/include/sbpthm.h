/*
 * Initialization and support routines for self-booting compressed image.
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
 * $Id: rtecdc.c 405571 2013-06-03 20:03:49Z $
 */
#ifndef _pcie_phtm_h_
#define _pcie_phtm_h_
typedef volatile struct {
	dma64regs_t	dmaxmt;		/* dma tx */
	uint32 PAD[2];
	dma64regs_t	dmarcv;		/* dma rx */
	uint32 PAD[2];
} dma64_t;

#define DMA_MAX	5
#define EP_MAX	9
/* Host interface registers */
typedef volatile struct {
	/* Device control */
	uint32 devcontrol;			/* DevControl, 0x000, rev 2 */
	uint32 devstatus;			/* DevStatus,  0x004, rev 2 */
	uint32 PAD[1];
	uint32 biststatus;			/* BISTStatus, 0x00C, rev 2 */

	/* USB control */
	uint32 usbsetting;			/* USBSetting, 0x010, rev 2 */
	uint32 usbframe;			/* USBFrame,   0x014, rev 2 */
	uint32 lpmcontrol;			/* LpmRegister, 0x18, rev 9 */
	uint32 PAD[1];

	/* 2nd level DMA int status/mask, IntStatus0-4, IntMask0-4 */
	struct {
		uint32 status;
		uint32 mask;
	} dmaint[DMA_MAX];			/* 0x020 - 0x44, rev 2 */

	/* Top level interrupt status and mask */
	uint32 usbintstatus;			/* IntStatus, 0x048, rev 2 */
	uint32 usbintmask;			/* IntMask,   0x04C, rev 2 */

	/* Endpoint status */
	uint32 epstatus;			/* CtrlOutStatus, 0x050, rev 2 */
	uint32 txfifowtermark;			/* bytes threshold before commit tx, POR=0x100 */

	/* Dedicated 2nd level DMA int status/mask for Setup Data, rev 3 */
	uint32 sdintstatus;			/* IntStatus5, 0x58, rev 3 */
	uint32 sdintmask;			/* IntMask5, 0x5C, rev 3 */
	uint32 PAD[40];

	/* Lazy interrupt control, IntRcv0-4Lazy, 0x100-0x110, rev 2 */
	uint32 intrcvlazy[DMA_MAX];
	/* Setup token Lazy interrupt control, rev 3 */
	uint32 sdintrcvlazy;			/* IntRcvLazy5 (Setup Data), 0x114, rev 3 */
	uint32 PAD[50];

	uint32 clkctlstatus;			/* ClockCtlStatus, 0x1E0, rev 3 */
	uint32 PAD[7];

	/* DMA engine regs, 0x200-0x29C, rev 2 */
	dma32regp_t dmaregs[DMA_MAX];
	dma32diag_t dmafifo;			/* fifo diag regs, 0x2A0-0x2AC */
	uint32 PAD[1];

	/* Endpoint byte counters, EPByteCount0-8, 0x2B4-0x2D4, rev 2 */
	uint32 epbytes[EP_MAX];
	uint32 PAD[2];

	uint32 hsicphyctrl1;		/* HSICPhyCtrl1 0x2e0, rev 10 */
	uint32 hsicphyctrl2;		/* HSICPhyCtrl2 0x2e4, rev 10 */
	uint32 hsicphystat1;		/* HSICPhyStat1 0x2e8, rev 9 */
	uint32 PAD[9];
	uint32 utmi_ctl;		/* utmi PHY contorol 0x310 */
	uint32 PAD[3];
	uint32 mdio_ctl;		/* mdio_ctl, 0x320 */
	uint32 mdio_data;		/* mdio_data, 0x324 */
	uint32 phymiscctl;		/* PhyMiscCtl, 0x328, rev 4 */
	uint32 PAD[5];

	/* Dedicated Setup Data DMA engine, 0x340-0x35C, rev 3 */
	dma32regp_t sddmaregs;
	uint32 PAD[40];

	/* Core registers */
	uint32 commandaddr;			/* CommmandAddress, 0x400, rev 2 */
	/* EndPointConfig0-8, 0x404-0x424, rev 2 */
	uint32 epinfo[EP_MAX];
	uint32 PAD[54];

	/*
	 * dma64 registers, including Setup Data DMA engine, for corerev >= 7
	 * 0ffsets 0x500 to 0x674.
	 */
	dma64_t dma64regs[DMA_MAX + 1];
	uint32 PAD[544];


	/* Sonics SiliconBackplane registers */
	sbconfig_t sbconfig;
} usbdev_sb_regs_t;
typedef volatile struct {
	dma64regs_t	xmt;		/* dma tx */
	uint32 PAD[2];
	dma64regs_t	rcv;		/* dma rx */
	uint32 PAD[2];
} dma64p_t;

/* dma64 sdiod corerev >= 1 */
typedef volatile struct {
	dma64p_t dma64regs[2];
	dma64diag_t dmafifo;		/* DMA Diagnostic Regs, 0x280-0x28c */
	uint32 PAD[92];
} sdiodma64_t;

/* dma32 sdiod corerev == 0 */
typedef volatile struct {
	dma32regp_t dma32regs[2];	/* dma tx & rx, 0x200-0x23c */
	dma32diag_t dmafifo;		/* DMA Diagnostic Regs, 0x240-0x24c */
	uint32 PAD[108];
} sdiodma32_t;

/* dma32 regs for pcmcia core */
typedef volatile struct {
	dma32regp_t dmaregs;		/* DMA Regs, 0x200-0x21c, rev8 */
	dma32diag_t dmafifo;		/* DMA Diagnostic Regs, 0x220-0x22c */
	uint32 PAD[116];
} pcmdma32_t;


/* core registers */
typedef volatile struct {
	uint32 corecontrol;		/* CoreControl, 0x000, rev8 */
	uint32 corestatus;		/* CoreStatus, 0x004, rev8  */
	uint32 PAD[1];
	uint32 biststatus;		/* BistStatus, 0x00c, rev8  */

	/* PCMCIA access */
	uint16 pcmciamesportaladdr;	/* PcmciaMesPortalAddr, 0x010, rev8   */
	uint16 PAD[1];
	uint16 pcmciamesportalmask;	/* PcmciaMesPortalMask, 0x014, rev8   */
	uint16 PAD[1];
	uint16 pcmciawrframebc;		/* PcmciaWrFrameBC, 0x018, rev8   */
	uint16 PAD[1];
	uint16 pcmciaunderflowtimer;	/* PcmciaUnderflowTimer, 0x01c, rev8   */
	uint16 PAD[1];

	/* interrupt */
	uint32 intstatus;		/* IntStatus, 0x020, rev8   */
	uint32 hostintmask;		/* IntHostMask, 0x024, rev8   */
	uint32 intmask;			/* IntSbMask, 0x028, rev8   */
	uint32 sbintstatus;		/* SBIntStatus, 0x02c, rev8   */
	uint32 sbintmask;		/* SBIntMask, 0x030, rev8   */
	uint32 funcintmask;		/* SDIO Function Interrupt Mask, SDIO rev4 */
	uint32 PAD[2];
	uint32 tosbmailbox;		/* ToSBMailbox, 0x040, rev8   */
	uint32 tohostmailbox;		/* ToHostMailbox, 0x044, rev8   */
	uint32 tosbmailboxdata;		/* ToSbMailboxData, 0x048, rev8   */
	uint32 tohostmailboxdata;	/* ToHostMailboxData, 0x04c, rev8   */

	/* synchronized access to registers in SDIO clock domain */
	uint32 sdioaccess;		/* SdioAccess, 0x050, rev8   */
	uint32 PAD[3];

	/* PCMCIA frame control */
	uint8 pcmciaframectrl;		/* pcmciaFrameCtrl, 0x060, rev8   */
	uint8 PAD[3];
	uint8 pcmciawatermark;		/* pcmciaWaterMark, 0x064, rev8   */
	uint8 PAD[155];

	/* interrupt batching control */
	uint32 intrcvlazy;		/* IntRcvLazy, 0x100, rev8 */
	uint32 PAD[3];

	/* counters */
	uint32 cmd52rd;			/* Cmd52RdCount, 0x110, rev8, SDIO: cmd52 reads */
	uint32 cmd52wr;			/* Cmd52WrCount, 0x114, rev8, SDIO: cmd52 writes */
	uint32 cmd53rd;			/* Cmd53RdCount, 0x118, rev8, SDIO: cmd53 reads */
	uint32 cmd53wr;			/* Cmd53WrCount, 0x11c, rev8, SDIO: cmd53 writes */
	uint32 abort;			/* AbortCount, 0x120, rev8, SDIO: aborts */
	uint32 datacrcerror;		/* DataCrcErrorCount, 0x124, rev8, SDIO: frames w/bad CRC */
	uint32 rdoutofsync;		/* RdOutOfSyncCount, 0x128, rev8, SDIO/PCMCIA: Rd Frm OOS */
	uint32 wroutofsync;		/* RdOutOfSyncCount, 0x12c, rev8, SDIO/PCMCIA: Wr Frm OOS */
	uint32 writebusy;		/* WriteBusyCount, 0x130, rev8, SDIO: dev asserted "busy" */
	uint32 readwait;		/* ReadWaitCount, 0x134, rev8, SDIO: read: no data avail */
	uint32 readterm;		/* ReadTermCount, 0x138, rev8, SDIO: rd frm terminates */
	uint32 writeterm;		/* WriteTermCount, 0x13c, rev8, SDIO: wr frm terminates */
	uint32 PAD[40];
	uint32 clockctlstatus;		/* ClockCtlStatus, 0x1e0, rev8 */
	uint32 PAD[7];

	/* DMA engines */
	volatile union {
		pcmdma32_t pcm32;
		sdiodma32_t sdiod32;
		sdiodma64_t sdiod64;
	} dma;

	/* SDIO/PCMCIA CIS region */
	char cis[512];			/* 512 byte CIS, 0x400-0x5ff, rev6 */

	/* PCMCIA function control registers */
	char pcmciafcr[256];		/* PCMCIA FCR, 0x600-6ff, rev6 */
	uint16 PAD[55];

	/* PCMCIA backplane access */
	uint16 backplanecsr;		/* BackplaneCSR, 0x76E, rev6 */
	uint16 backplaneaddr0;		/* BackplaneAddr0, 0x770, rev6 */
	uint16 backplaneaddr1;		/* BackplaneAddr1, 0x772, rev6 */
	uint16 backplaneaddr2;		/* BackplaneAddr2, 0x774, rev6 */
	uint16 backplaneaddr3;		/* BackplaneAddr3, 0x776, rev6 */
	uint16 backplanedata0;		/* BackplaneData0, 0x778, rev6 */
	uint16 backplanedata1;		/* BackplaneData1, 0x77a, rev6 */
	uint16 backplanedata2;		/* BackplaneData2, 0x77c, rev6 */
	uint16 backplanedata3;		/* BackplaneData3, 0x77e, rev6 */
	uint16 PAD[31];

	/* sprom "size" & "blank" info */
	uint16 spromstatus;		/* SPROMStatus, 0x7BE, rev2 */
	uint32 PAD[464];

	/* Sonics SiliconBackplane registers */
	sbconfig_t sbconfig;		/* SbConfig Regs, 0xf00-0xfff, rev8 */
} sdpcmd_regs_t;

/* intrcvlazy */
#define	IRL_FC_SHIFT		24
#define	IRL_TO_MASK	0x00ffffff	/* timeout */
#define	IRL_FC_MASK	0xff000000	/* frame count */

/* DMA interrupt bits */
#define	I_PC			(1L << 10)	/* descriptor error */
#define	I_PD			(1L << 11)	/* data error */
#define	I_DE			(1L << 12)	/* Descriptor protocol Error */
#define	I_RU			(1L << 13)	/* Receive descriptor Underflow */
#define	I_RO			(1L << 14)	/* Receive fifo Overflow */
#define	I_XU			(1L << 15)	/* Transmit fifo Underflow */
#define	I_RI			(1L << 16)	/* Receive Interrupt */
#define	I_XI			(1L << 24)	/* Transmit Interrupt */
#define	I_ERRORS		(I_PC | I_PD | I_DE | I_RU | I_RO | I_XU)


#endif /* _pcie_phtm_h_  */
