/*
 * MPC86xx Internal Memory Map
 *
 * Copyright 2004, 2011 Freescale Semiconductor
 * Jeff Brown (Jeffrey@freescale.com)
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 *
 */

#ifndef __IMMAP_86xx__
#define __IMMAP_86xx__

#include <fsl_immap.h>
#include <asm/types.h>
#include <asm/fsl_dma.h>
#include <asm/fsl_lbc.h>
#include <asm/fsl_i2c.h>

/* Local-Access Registers and MCM Registers(0x0000-0x2000) */
typedef struct ccsr_local_mcm {
	uint	ccsrbar;	/* 0x0 - Control Configuration Status Registers Base Address Register */
	char	res1[4];
	uint	altcbar;	/* 0x8 - Alternate Configuration Base Address Register */
	char	res2[4];
	uint	altcar;		/* 0x10 - Alternate Configuration Attribute Register */
	char	res3[12];
	uint	bptr;		/* 0x20 - Boot Page Translation Register */
	char	res4[3044];
	uint	lawbar0;	/* 0xc08 - Local Access Window 0 Base Address Register */
	char	res5[4];
	uint	lawar0;		/* 0xc10 - Local Access Window 0 Attributes Register */
	char	res6[20];
	uint	lawbar1;	/* 0xc28 - Local Access Window 1 Base Address Register */
	char	res7[4];
	uint	lawar1;		/* 0xc30 - Local Access Window 1 Attributes Register */
	char	res8[20];
	uint	lawbar2;	/* 0xc48 - Local Access Window 2 Base Address Register */
	char	res9[4];
	uint	lawar2;		/* 0xc50 - Local Access Window 2 Attributes Register */
	char	res10[20];
	uint	lawbar3;	/* 0xc68 - Local Access Window 3 Base Address Register */
	char	res11[4];
	uint	lawar3;		/* 0xc70 - Local Access Window 3 Attributes Register */
	char	res12[20];
	uint	lawbar4;	/* 0xc88 - Local Access Window 4 Base Address Register */
	char	res13[4];
	uint	lawar4;		/* 0xc90 - Local Access Window 4 Attributes Register */
	char	res14[20];
	uint	lawbar5;	/* 0xca8 - Local Access Window 5 Base Address Register */
	char	res15[4];
	uint	lawar5;		/* 0xcb0 - Local Access Window 5 Attributes Register */
	char	res16[20];
	uint	lawbar6;	/* 0xcc8 - Local Access Window 6 Base Address Register */
	char	res17[4];
	uint	lawar6;		/* 0xcd0 - Local Access Window 6 Attributes Register */
	char	res18[20];
	uint	lawbar7;	/* 0xce8 - Local Access Window 7 Base Address Register */
	char	res19[4];
	uint	lawar7;		/* 0xcf0 - Local Access Window 7 Attributes Register */
	char	res20[20];
	uint	lawbar8;	/* 0xd08 - Local Access Window 8 Base Address Register */
	char	res21[4];
	uint	lawar8;		/* 0xd10 - Local Access Window 8 Attributes Register */
	char	res22[20];
	uint	lawbar9;	/* 0xd28 - Local Access Window 9 Base Address Register */
	char	res23[4];
	uint	lawar9;		/* 0xd30 - Local Access Window 9 Attributes Register */
	char	res24[716];
	uint	abcr;		/* 0x1000 - MCM CCB Address Configuration Register */
	char	res25[4];
	uint    dbcr;           /* 0x1008 - MCM MPX data bus Configuration Register */
	char	res26[4];
	uint	pcr;		/* 0x1010 - MCM CCB Port Configuration Register */
	char	res27[44];
	uint	hpmr0;		/* 0x1040 - MCM HPM Threshold Count Register 0 */
	uint	hpmr1;		/* 0x1044 - MCM HPM Threshold Count Register 1 */
	uint	hpmr2;		/* 0x1048 - MCM HPM Threshold Count Register 2 */
	uint	hpmr3;		/* 0x104c - MCM HPM Threshold Count Register 3 */
	char	res28[16];
	uint	hpmr4;		/* 0x1060 - MCM HPM Threshold Count Register 4 */
	uint	hpmr5;		/* 0x1064 - MCM HPM Threshold Count Register 5 */
	uint	hpmccr;		/* 0x1068 - MCM HPM Cycle Count Register */
	char	res29[3476];
	uint	edr;		/* 0x1e00 - MCM Error Detect Register */
	char	res30[4];
	uint	eer;		/* 0x1e08 - MCM Error Enable Register */
	uint	eatr;		/* 0x1e0c - MCM Error Attributes Capture Register */
	uint	eladr;		/* 0x1e10 - MCM Error Low Address Capture Register */
	uint	ehadr;		/* 0x1e14 - MCM Error High Address Capture Register */
	char	res31[488];
} ccsr_local_mcm_t;

/* Daul I2C Registers(0x3000-0x4000) */
typedef struct ccsr_i2c {
	struct fsl_i2c_base	i2c[2];
	u8	res[4096 - 2 * sizeof(struct fsl_i2c_base)];
} ccsr_i2c_t;

/* DUART Registers(0x4000-0x5000) */
typedef struct ccsr_duart {
	char	res1[1280];
	u_char	urbr1_uthr1_udlb1;/* 0x4500 - URBR1, UTHR1, UDLB1 with the same address offset of 0x04500 */
	u_char	uier1_udmb1;	/* 0x4501 - UIER1, UDMB1 with the same address offset of 0x04501 */
	u_char	uiir1_ufcr1_uafr1;/* 0x4502 - UIIR1, UFCR1, UAFR1 with the same address offset of 0x04502 */
	u_char	ulcr1;		/* 0x4503 - UART1 Line Control Register */
	u_char	umcr1;		/* 0x4504 - UART1 Modem Control Register */
	u_char	ulsr1;		/* 0x4505 - UART1 Line Status Register */
	u_char	umsr1;		/* 0x4506 - UART1 Modem Status Register */
	u_char	uscr1;		/* 0x4507 - UART1 Scratch Register */
	char	res2[8];
	u_char	udsr1;		/* 0x4510 - UART1 DMA Status Register */
	char	res3[239];
	u_char	urbr2_uthr2_udlb2;/* 0x4600 - URBR2, UTHR2, UDLB2 with the same address offset of 0x04600 */
	u_char	uier2_udmb2;	/* 0x4601 - UIER2, UDMB2 with the same address offset of 0x04601 */
	u_char	uiir2_ufcr2_uafr2;/* 0x4602 - UIIR2, UFCR2, UAFR2 with the same address offset of 0x04602 */
	u_char	ulcr2;		/* 0x4603 - UART2 Line Control Register */
	u_char	umcr2;		/* 0x4604 - UART2 Modem Control Register */
	u_char	ulsr2;		/* 0x4605 - UART2 Line Status Register */
	u_char	umsr2;		/* 0x4606 - UART2 Modem Status Register */
	u_char	uscr2;		/* 0x4607 - UART2 Scratch Register */
	char	res4[8];
	u_char	udsr2;		/* 0x4610 - UART2 DMA Status Register */
	char	res5[2543];
} ccsr_duart_t;

/* PCI Express Registers(0x8000-0x9000) and (0x9000-0xA000) */
typedef struct ccsr_pex {
	uint	cfg_addr;	/* 0x8000 - PEX Configuration Address Register */
	uint	cfg_data;	/* 0x8004 - PEX Configuration Data Register */
	char	res1[4];
	uint	out_comp_to;	/* 0x800C - PEX Outbound Completion Timeout Register */
	char	res2[16];
	uint	pme_msg_det;	/* 0x8020 - PEX PME & message detect register */
	uint    pme_msg_int_en;	/* 0x8024 - PEX PME & message interrupt enable register */
	uint    pme_msg_dis;	/* 0x8028 - PEX PME & message disable register */
	uint    pm_command;	/* 0x802c - PEX PM Command register */
	char	res3[3016];
	uint    block_rev1;	/* 0x8bf8 - PEX Block Revision register 1 */
	uint    block_rev2;	/* 0x8bfc - PEX Block Revision register 2 */
	uint	potar0;	        /* 0x8c00 - PEX Outbound Transaction Address Register 0 */
	uint	potear0;	/* 0x8c04 - PEX Outbound Translation Extended Address Register 0 */
	char	res4[8];
	uint	powar0;	        /* 0x8c10 - PEX Outbound Window Attributes Register 0 */
	char	res5[12];
	uint	potar1;	        /* 0x8c20 - PEX Outbound Transaction Address Register 1 */
	uint	potear1;	/* 0x8c24 - PEX Outbound Translation Extended Address Register 1 */
	uint	powbar1;	/* 0x8c28 - PEX Outbound Window Base Address Register 1 */
	char	res6[4];
	uint	powar1;	        /* 0x8c30 - PEX Outbound Window Attributes Register 1 */
	char	res7[12];
	uint	potar2;	        /* 0x8c40 - PEX Outbound Transaction Address Register 2 */
	uint	potear2;	/* 0x8c44 - PEX Outbound Translation Extended Address Register 2 */
	uint	powbar2;	/* 0x8c48 - PEX Outbound Window Base Address Register 2 */
	char	res8[4];
	uint	powar2;	        /* 0x8c50 - PEX Outbound Window Attributes Register 2 */
	char	res9[12];
	uint	potar3;	        /* 0x8c60 - PEX Outbound Transaction Address Register 3 */
	uint	potear3;	/* 0x8c64 - PEX Outbound Translation Extended Address Register 3 */
	uint	powbar3;	/* 0x8c68 - PEX Outbound Window Base Address Register 3 */
	char	res10[4];
	uint	powar3;	        /* 0x8c70 - PEX Outbound Window Attributes Register 3 */
	char	res11[12];
	uint	potar4;	        /* 0x8c80 - PEX Outbound Transaction Address Register 4 */
	uint	potear4;	/* 0x8c84 - PEX Outbound Translation Extended Address Register 4 */
	uint	powbar4;	/* 0x8c88 - PEX Outbound Window Base Address Register 4 */
	char	res12[4];
	uint	powar4;	        /* 0x8c90 - PEX Outbound Window Attributes Register 4 */
	char	res13[12];
	char	res14[256];
	uint	pitar3;	        /* 0x8da0 - PEX Inbound Translation Address Register 3  */
	char	res15[4];
	uint	piwbar3;	/* 0x8da8 - PEX Inbound Window Base Address Register 3 */
	uint	piwbear3;	/* 0x8dac - PEX Inbound Window Base Extended Address Register 3 */
	uint	piwar3;	        /* 0x8db0 - PEX Inbound Window Attributes Register 3 */
	char	res16[12];
	uint	pitar2;	        /* 0x8dc0 - PEX Inbound Translation Address Register 2  */
	char	res17[4];
	uint	piwbar2;	/* 0x8dc8 - PEX Inbound Window Base Address Register 2 */
	uint	piwbear2;	/* 0x8dcc - PEX Inbound Window Base Extended Address Register 2 */
	uint	piwar2;	        /* 0x8dd0 - PEX Inbound Window Attributes Register 2 */
	char	res18[12];
	uint	pitar1;	        /* 0x8de0 - PEX Inbound Translation Address Register 1  */
	char	res19[4];
	uint	piwbar1;	/* 0x8de8 - PEX Inbound Window Base Address Register 1 */
	uint	piwbear1;
	uint	piwar1;	        /* 0x8df0 - PEX Inbound Window Attributes Register 1 */
	char	res20[12];
	uint	pedr;		/* 0x8e00 - PEX Error Detect Register */
	char    res21[4];
	uint	peer;		/* 0x8e08 - PEX Error Interrupt Enable Register */
	char    res22[4];
	uint	pecdr;		/* 0x8e10 - PEX Error Disable Register */
	char    res23[12];
	uint	peer_stat;	/* 0x8e20 - PEX Error Capture Status Register */
	char    res24[4];
	uint	perr_cap0;	/* 0x8e28 - PEX Error Capture Register 0 */
	uint	perr_cap1;	/* 0x8e2c - PEX Error Capture Register 1 */
	uint	perr_cap2;	/* 0x8e30 - PEX Error Capture Register 2 */
	uint	perr_cap3;	/* 0x8e34 - PEX Error Capture Register 3 */
	char	res25[452];
	char    res26[4];
} ccsr_pex_t;

/* Hyper Transport Register Block (0xA000-0xB000) */
typedef struct ccsr_ht {
	uint    hcfg_addr;      /* 0xa000 - HT Configuration Address register */
	uint    hcfg_data;      /* 0xa004 - HT Configuration Data register */
	char	res1[3064];
	uint    howtar0;        /* 0xac00 - HT Outbound Window 0 Translation register */
	char	res2[12];
	uint    howar0;         /* 0xac10 - HT Outbound Window 0 Attributes register */
	char	res3[12];
	uint    howtar1;        /* 0xac20 - HT Outbound Window 1 Translation register */
	char	res4[4];
	uint    howbar1;        /* 0xac28 - HT Outbound Window 1 Base Address register */
	char	res5[4];
	uint    howar1;         /* 0xac30 - HT Outbound Window 1 Attributes register */
	char	res6[12];
	uint    howtar2;        /* 0xac40 - HT Outbound Window 2 Translation register */
	char	res7[4];
	uint    howbar2;        /* 0xac48 - HT Outbound Window 2 Base Address register */
	char	res8[4];
	uint    howar2;         /* 0xac50 - HT Outbound Window 2 Attributes register */
	char	res9[12];
	uint    howtar3;        /* 0xac60 - HT Outbound Window 3 Translation register */
	char	res10[4];
	uint    howbar3;        /* 0xac68 - HT Outbound Window 3 Base Address register */
	char	res11[4];
	uint    howar3;         /* 0xac70 - HT Outbound Window 3 Attributes  register */
	char	res12[12];
	uint    howtar4;        /* 0xac80 - HT Outbound Window 4 Translation register */
	char	res13[4];
	uint    howbar4;        /* 0xac88 - HT Outbound Window 4 Base Address register */
	char	res14[4];
	uint    howar4;         /* 0xac90 - HT Outbound Window 4 Attributes register */
	char	res15[236];
	uint    hiwtar4;        /* 0xad80 - HT Inbound Window 4 Translation register */
	char	res16[4];
	uint    hiwbar4;        /* 0xad88 - HT Inbound Window 4 Base Address register */
	char	res17[4];
	uint    hiwar4;         /* 0xad90 - HT Inbound Window 4 Attributes register */
	char	res18[12];
	uint    hiwtar3;        /* 0xada0 - HT Inbound Window 3 Translation register */
	char	res19[4];
	uint    hiwbar3;        /* 0xada8 - HT Inbound Window 3 Base Address register */
	char	res20[4];
	uint    hiwar3;         /* 0xadb0 - HT Inbound Window 3 Attributes register */
	char	res21[12];
	uint    hiwtar2;        /* 0xadc0 - HT Inbound Window 2 Translation register */
	char	res22[4];
	uint    hiwbar2;        /* 0xadc8 - HT Inbound Window 2 Base Address register */
	char	res23[4];
	uint    hiwar2;         /* 0xadd0 - HT Inbound Window 2 Attributes register */
	char	res24[12];
	uint    hiwtar1;        /* 0xade0 - HT Inbound Window 1 Translation register */
	char	res25[4];
	uint    hiwbar1;        /* 0xade8 - HT Inbound Window 1 Base Address register */
	char	res26[4];
	uint    hiwar1;         /* 0xadf0 - HT Inbound Window 1 Attributes register */
	char	res27[12];
	uint    hedr;           /* 0xae00 - HT Error Detect register */
	char	res28[4];
	uint    heier;          /* 0xae08 - HT Error Interrupt Enable register */
	char	res29[4];
	uint    hecdr;          /* 0xae10 - HT Error Capture Disbale register */
	char	res30[12];
	uint    hecsr;          /* 0xae20 - HT Error Capture Status register */
	char	res31[4];
	uint    hec0;           /* 0xae28 - HT Error Capture 0 register */
	uint    hec1;           /* 0xae2c - HT Error Capture 1 register */
	uint    hec2;           /* 0xae30 - HT Error Capture 2 register */
	char    res32[460];
} ccsr_ht_t;

/* DMA Registers(0x2_1000-0x2_2000) */
typedef struct ccsr_dma {
	char	res1[256];
	struct fsl_dma dma[4];
	uint	dgsr;		/* 0x21300 - DMA General Status Register */
	char	res2[3324];
} ccsr_dma_t;

/* tsec1-4: 24000-28000 */
typedef struct ccsr_tsec {
	uint    id;		/* 0x24000 - Controller ID Register */
	char	res1[12];
	uint	ievent;		/* 0x24010 - Interrupt Event Register */
	uint	imask;		/* 0x24014 - Interrupt Mask Register */
	uint	edis;		/* 0x24018 - Error Disabled Register */
	char	res2[4];
	uint	ecntrl;		/* 0x24020 - Ethernet Control Register */
	char    res2_1[4];
	uint	ptv;		/* 0x24028 - Pause Time Value Register */
	uint	dmactrl;	/* 0x2402c - DMA Control Register */
	uint	tbipa;		/* 0x24030 - TBI PHY Address Register */
	char	res3[88];
	uint	fifo_tx_thr;	/* 0x2408c - FIFO transmit threshold register */
	char	res4[8];
	uint	fifo_tx_starve;	/* 0x24098 - FIFO transmit starve register */
	uint	fifo_tx_starve_shutoff;/* 0x2409c - FIFO transmit starve shutoff register */
	char    res4_1[4];
	uint    fifo_rx_pause;  /* 0x240a4 - FIFO receive pause threshold register */
	uint    fifo_rx_alarm;  /* 0x240a8 - FIFO receive alarm threshold register */
	char	res5[84];
	uint	tctrl;		/* 0x24100 - Transmit Control Register */
	uint	tstat;		/* 0x24104 - Transmit Status Register */
	uint    dfvlan;		/* 0x24108 - Default VLAN control word */
	char    res6[4];
	uint    txic;		/* 0x24110 - Transmit interrupt coalescing Register */
	uint    tqueue;         /* 0x24114 - Transmit Queue Control Register */
	char	res7[40];
	uint    tr03wt;         /* 0x24140 - TxBD Rings 0-3 round-robin weightings */
	uint    tw47wt;         /* 0x24144 - TxBD Rings 4-7 round-robin weightings */
	char    res8[52];
	uint	tbdbph;	        /* 0x2417c - Transmit Data Buffer Pointer High Register */
	char    res9[4];
	uint    tbptr0;         /* 0x24184 - Transmit Buffer Descriptor Pointer for Ring 0 */
	char    res10[4];
	uint    tbptr1;         /* 0x2418C - Transmit Buffer Descriptor Pointer for Ring 1 */
	char    res11[4];
	uint    tbptr2;         /* 0x24194 - Transmit Buffer Descriptor Pointer for Ring 2 */
	char    res12[4];
	uint    tbptr3;         /* 0x2419C - Transmit Buffer Descriptor Pointer for Ring 3 */
	char    res13[4];
	uint    tbptr4;         /* 0x241A4 - Transmit Buffer Descriptor Pointer for Ring 4 */
	char    res14[4];
	uint    tbptr5;         /* 0x241AC - Transmit Buffer Descriptor Pointer for Ring 5 */
	char    res15[4];
	uint    tbptr6;         /* 0x241B4 - Transmit Buffer Descriptor Pointer for Ring 6 */
	char    res16[4];
	uint    tbptr7;         /* 0x241BC - Transmit Buffer Descriptor Pointer for Ring 7 */
	char    res17[64];
	uint	tbaseh;		/* 0x24200 - Transmit Descriptor Base Address High Register */
	uint	tbase0;		/* 0x24204 - Transmit Descriptor Base Address Register of Ring 0 */
	char    res18[4];
	uint    tbase1;         /* 0x2420C - Transmit Descriptor base address of Ring 1 */
	char    res19[4];
	uint    tbase2;         /* 0x24214 - Transmit Descriptor base address of Ring 2 */
	char    res20[4];
	uint    tbase3;         /* 0x2421C - Transmit Descriptor base address of Ring 3 */
	char    res21[4];
	uint    tbase4;         /* 0x24224 - Transmit Descriptor base address of Ring 4 */
	char    res22[4];
	uint    tbase5;         /* 0x2422C - Transmit Descriptor base address of Ring 5 */
	char    res23[4];
	uint    tbase6;         /* 0x24234 - Transmit Descriptor base address of Ring 6 */
	char    res24[4];
	uint    tbase7;         /* 0x2423C - Transmit Descriptor base address of Ring 7 */
	char    res25[192];
	uint	rctrl;		/* 0x24300 - Receive Control Register */
	uint	rstat;		/* 0x24304 - Receive Status Register */
	char	res26[8];
	uint    rxic;           /* 0x24310 - Receive Interrupt Coalecing Register */
	uint    rqueue;         /* 0x24314 - Receive queue control register */
	char	res27[24];
	uint    rbifx;		/* 0x24330 - Receive bit field extract control Register */
	uint    rqfar;		/* 0x24334 - Receive queue filing table address Register */
	uint    rqfcr;		/* 0x24338 - Receive queue filing table control Register */
	uint    rqfpr;		/* 0x2433c - Receive queue filing table property Register */
	uint	mrblr;		/* 0x24340 - Maximum Receive Buffer Length Register */
	char	res28[56];
	uint    rbdbph;		/* 0x2437C - Receive Data Buffer Pointer High */
	char    res29[4];
	uint	rbptr0;		/* 0x24384 - Receive Buffer Descriptor Pointer for Ring 0 */
	char    res30[4];
	uint	rbptr1;		/* 0x2438C - Receive Buffer Descriptor Pointer for Ring 1 */
	char    res31[4];
	uint	rbptr2;		/* 0x24394 - Receive Buffer Descriptor Pointer for Ring 2 */
	char    res32[4];
	uint	rbptr3;		/* 0x2439C - Receive Buffer Descriptor Pointer for Ring 3 */
	char    res33[4];
	uint	rbptr4;		/* 0x243A4 - Receive Buffer Descriptor Pointer for Ring 4 */
	char    res34[4];
	uint	rbptr5;		/* 0x243AC - Receive Buffer Descriptor Pointer for Ring 5 */
	char    res35[4];
	uint	rbptr6;		/* 0x243B4 - Receive Buffer Descriptor Pointer for Ring 6 */
	char    res36[4];
	uint	rbptr7;		/* 0x243BC - Receive Buffer Descriptor Pointer for Ring 7 */
	char    res37[64];
	uint	rbaseh;		/* 0x24400 - Receive Descriptor Base Address High 0 */
	uint	rbase0;		/* 0x24404 - Receive Descriptor Base Address of Ring 0 */
	char    res38[4];
	uint	rbase1;		/* 0x2440C - Receive Descriptor Base Address of Ring 1 */
	char    res39[4];
	uint	rbase2;		/* 0x24414 - Receive Descriptor Base Address of Ring 2 */
	char    res40[4];
	uint	rbase3;		/* 0x2441C - Receive Descriptor Base Address of Ring 3 */
	char    res41[4];
	uint	rbase4;		/* 0x24424 - Receive Descriptor Base Address of Ring 4 */
	char    res42[4];
	uint	rbase5;		/* 0x2442C - Receive Descriptor Base Address of Ring 5 */
	char    res43[4];
	uint	rbase6;		/* 0x24434 - Receive Descriptor Base Address of Ring 6 */
	char    res44[4];
	uint	rbase7;		/* 0x2443C - Receive Descriptor Base Address of Ring 7 */
	char    res45[192];
	uint	maccfg1;	/* 0x24500 - MAC Configuration 1 Register */
	uint	maccfg2;	/* 0x24504 - MAC Configuration 2 Register */
	uint	ipgifg;		/* 0x24508 - Inter Packet Gap/Inter Frame Gap Register */
	uint	hafdup;		/* 0x2450c - Half Duplex Register */
	uint	maxfrm;		/* 0x24510 - Maximum Frame Length Register */
	char	res46[12];
	uint	miimcfg;	/* 0x24520 - MII Management Configuration Register */
	uint	miimcom;	/* 0x24524 - MII Management Command Register */
	uint	miimadd;	/* 0x24528 - MII Management Address Register */
	uint	miimcon;	/* 0x2452c - MII Management Control Register */
	uint	miimstat;	/* 0x24530 - MII Management Status Register */
	uint	miimind;	/* 0x24534 - MII Management Indicator Register */
	uint    ifctrl;		/* 0x24538 - Interface Contrl Register */
	uint	ifstat;		/* 0x2453c - Interface Status Register */
	uint	macstnaddr1;	/* 0x24540 - Station Address Part 1 Register */
	uint	macstnaddr2;	/* 0x24544 - Station Address Part 2 Register */
	uint    mac01addr1;     /* 0x24548 - MAC exact match address 1, part 1 */
	uint    mac01addr2;     /* 0x2454C - MAC exact match address 1, part 2 */
	uint    mac02addr1;     /* 0x24550 - MAC exact match address 2, part 1 */
	uint    mac02addr2;     /* 0x24554 - MAC exact match address 2, part 2 */
	uint    mac03addr1;     /* 0x24558 - MAC exact match address 3, part 1 */
	uint    mac03addr2;     /* 0x2455C - MAC exact match address 3, part 2 */
	uint    mac04addr1;     /* 0x24560 - MAC exact match address 4, part 1 */
	uint    mac04addr2;     /* 0x24564 - MAC exact match address 4, part 2 */
	uint    mac05addr1;     /* 0x24568 - MAC exact match address 5, part 1 */
	uint    mac05addr2;     /* 0x2456C - MAC exact match address 5, part 2 */
	uint    mac06addr1;     /* 0x24570 - MAC exact match address 6, part 1 */
	uint    mac06addr2;     /* 0x24574 - MAC exact match address 6, part 2 */
	uint    mac07addr1;     /* 0x24578 - MAC exact match address 7, part 1 */
	uint    mac07addr2;     /* 0x2457C - MAC exact match address 7, part 2 */
	uint    mac08addr1;     /* 0x24580 - MAC exact match address 8, part 1 */
	uint    mac08addr2;     /* 0x24584 - MAC exact match address 8, part 2 */
	uint    mac09addr1;     /* 0x24588 - MAC exact match address 9, part 1 */
	uint    mac09addr2;     /* 0x2458C - MAC exact match address 9, part 2 */
	uint    mac10addr1;     /* 0x24590 - MAC exact match address 10, part 1 */
	uint    mac10addr2;     /* 0x24594 - MAC exact match address 10, part 2 */
	uint    mac11addr1;     /* 0x24598 - MAC exact match address 11, part 1 */
	uint    mac11addr2;     /* 0x2459C - MAC exact match address 11, part 2 */
	uint    mac12addr1;     /* 0x245A0 - MAC exact match address 12, part 1 */
	uint    mac12addr2;     /* 0x245A4 - MAC exact match address 12, part 2 */
	uint    mac13addr1;     /* 0x245A8 - MAC exact match address 13, part 1 */
	uint    mac13addr2;     /* 0x245AC - MAC exact match address 13, part 2 */
	uint    mac14addr1;     /* 0x245B0 - MAC exact match address 14, part 1 */
	uint    mac14addr2;     /* 0x245B4 - MAC exact match address 14, part 2 */
	uint    mac15addr1;     /* 0x245B8 - MAC exact match address 15, part 1 */
	uint    mac15addr2;     /* 0x245BC - MAC exact match address 15, part 2 */
	char	res48[192];
	uint	tr64;		/* 0x24680 - Transmit and Receive 64-byte Frame Counter */
	uint	tr127;		/* 0x24684 - Transmit and Receive 65-127 byte Frame Counter */
	uint	tr255;		/* 0x24688 - Transmit and Receive 128-255 byte Frame Counter */
	uint	tr511;		/* 0x2468c - Transmit and Receive 256-511 byte Frame Counter */
	uint	tr1k;		/* 0x24690 - Transmit and Receive 512-1023 byte Frame Counter */
	uint	trmax;		/* 0x24694 - Transmit and Receive 1024-1518 byte Frame Counter */
	uint	trmgv;		/* 0x24698 - Transmit and Receive 1519-1522 byte Good VLAN Frame */
	uint	rbyt;		/* 0x2469c - Receive Byte Counter */
	uint	rpkt;		/* 0x246a0 - Receive Packet Counter */
	uint	rfcs;		/* 0x246a4 - Receive FCS Error Counter */
	uint	rmca;		/* 0x246a8 - Receive Multicast Packet Counter */
	uint	rbca;		/* 0x246ac - Receive Broadcast Packet Counter */
	uint	rxcf;		/* 0x246b0 - Receive Control Frame Packet Counter */
	uint	rxpf;		/* 0x246b4 - Receive Pause Frame Packet Counter */
	uint	rxuo;		/* 0x246b8 - Receive Unknown OP Code Counter */
	uint	raln;		/* 0x246bc - Receive Alignment Error Counter */
	uint	rflr;		/* 0x246c0 - Receive Frame Length Error Counter */
	uint	rcde;		/* 0x246c4 - Receive Code Error Counter */
	uint	rcse;		/* 0x246c8 - Receive Carrier Sense Error Counter */
	uint	rund;		/* 0x246cc - Receive Undersize Packet Counter */
	uint	rovr;		/* 0x246d0 - Receive Oversize Packet Counter */
	uint	rfrg;		/* 0x246d4 - Receive Fragments Counter */
	uint	rjbr;		/* 0x246d8 - Receive Jabber Counter */
	uint	rdrp;		/* 0x246dc - Receive Drop Counter */
	uint	tbyt;		/* 0x246e0 - Transmit Byte Counter Counter */
	uint	tpkt;		/* 0x246e4 - Transmit Packet Counter */
	uint	tmca;		/* 0x246e8 - Transmit Multicast Packet Counter */
	uint	tbca;		/* 0x246ec - Transmit Broadcast Packet Counter */
	uint	txpf;		/* 0x246f0 - Transmit Pause Control Frame Counter */
	uint	tdfr;		/* 0x246f4 - Transmit Deferral Packet Counter */
	uint	tedf;		/* 0x246f8 - Transmit Excessive Deferral Packet Counter */
	uint	tscl;		/* 0x246fc - Transmit Single Collision Packet Counter */
	uint	tmcl;		/* 0x24700 - Transmit Multiple Collision Packet Counter */
	uint	tlcl;		/* 0x24704 - Transmit Late Collision Packet Counter */
	uint	txcl;		/* 0x24708 - Transmit Excessive Collision Packet Counter */
	uint	tncl;		/* 0x2470c - Transmit Total Collision Counter */
	char	res49[4];
	uint	tdrp;		/* 0x24714 - Transmit Drop Frame Counter */
	uint	tjbr;		/* 0x24718 - Transmit Jabber Frame Counter */
	uint	tfcs;		/* 0x2471c - Transmit FCS Error Counter */
	uint	txcf;		/* 0x24720 - Transmit Control Frame Counter */
	uint	tovr;		/* 0x24724 - Transmit Oversize Frame Counter */
	uint	tund;		/* 0x24728 - Transmit Undersize Frame Counter */
	uint	tfrg;		/* 0x2472c - Transmit Fragments Frame Counter */
	uint	car1;		/* 0x24730 - Carry Register One */
	uint	car2;		/* 0x24734 - Carry Register Two */
	uint	cam1;		/* 0x24738 - Carry Mask Register One */
	uint	cam2;		/* 0x2473c - Carry Mask Register Two */
	uint    rrej;	        /* 0x24740 - Receive filer rejected packet counter */
	char	res50[188];
	uint	iaddr0;		/* 0x24800 - Indivdual address register 0 */
	uint	iaddr1;		/* 0x24804 - Indivdual address register 1 */
	uint	iaddr2;		/* 0x24808 - Indivdual address register 2 */
	uint	iaddr3;		/* 0x2480c - Indivdual address register 3 */
	uint	iaddr4;		/* 0x24810 - Indivdual address register 4 */
	uint	iaddr5;		/* 0x24814 - Indivdual address register 5 */
	uint	iaddr6;		/* 0x24818 - Indivdual address register 6 */
	uint	iaddr7;		/* 0x2481c - Indivdual address register 7 */
	char	res51[96];
	uint	gaddr0;		/* 0x24880 - Global address register 0 */
	uint	gaddr1;		/* 0x24884 - Global address register 1 */
	uint	gaddr2;		/* 0x24888 - Global address register 2 */
	uint	gaddr3;		/* 0x2488c - Global address register 3 */
	uint	gaddr4;		/* 0x24890 - Global address register 4 */
	uint	gaddr5;		/* 0x24894 - Global address register 5 */
	uint	gaddr6;		/* 0x24898 - Global address register 6 */
	uint	gaddr7;		/* 0x2489c - Global address register 7 */
	char	res52[352];
	uint    fifocfg;        /* 0x24A00 - FIFO interface configuration register */
	char    res53[500];
	uint    attr;           /* 0x24BF8 - DMA Attribute register */
	uint    attreli;        /* 0x24BFC - DMA Attribute extract length and index register */
	char    res54[1024];
} ccsr_tsec_t;

/* PIC Registers(0x4_0000-0x6_1000) */

typedef struct ccsr_pic {
	char	res1[64];
	uint	ipidr0;		/* 0x40040 - Interprocessor Interrupt Dispatch Register 0 */
	char	res2[12];
	uint	ipidr1;		/* 0x40050 - Interprocessor Interrupt Dispatch Register 1 */
	char	res3[12];
	uint	ipidr2;		/* 0x40060 - Interprocessor Interrupt Dispatch Register 2 */
	char	res4[12];
	uint	ipidr3;		/* 0x40070 - Interprocessor Interrupt Dispatch Register 3 */
	char	res5[12];
	uint	ctpr;		/* 0x40080 - Current Task Priority Register */
	char	res6[12];
	uint	whoami;		/* 0x40090 - Who Am I Register */
	char	res7[12];
	uint	iack;		/* 0x400a0 - Interrupt Acknowledge Register */
	char	res8[12];
	uint	eoi;		/* 0x400b0 - End Of Interrupt Register */
	char	res9[3916];
	uint	frr;		/* 0x41000 - Feature Reporting Register */
	char	res10[28];
	uint	gcr;		/* 0x41020 - Global Configuration Register */
#define MPC86xx_PICGCR_RST	0x80000000
#define MPC86xx_PICGCR_MODE	0x20000000
	char	res11[92];
	uint	vir;		/* 0x41080 - Vendor Identification Register */
	char	res12[12];
	uint	pir;		/* 0x41090 - Processor Initialization Register */
	char	res13[12];
	uint	ipivpr0;	/* 0x410a0 - IPI Vector/Priority Register 0 */
	char	res14[12];
	uint	ipivpr1;	/* 0x410b0 - IPI Vector/Priority Register 1 */
	char	res15[12];
	uint	ipivpr2;	/* 0x410c0 - IPI Vector/Priority Register 2 */
	char	res16[12];
	uint	ipivpr3;	/* 0x410d0 - IPI Vector/Priority Register 3 */
	char	res17[12];
	uint	svr;		/* 0x410e0 - Spurious Vector Register */
	char	res18[12];
	uint	tfrr;		/* 0x410f0 - Timer Frequency Reporting Register */
	char	res19[12];
	uint	gtccr0;		/* 0x41100 - Global Timer Current Count Register 0 */
	char	res20[12];
	uint	gtbcr0;		/* 0x41110 - Global Timer Base Count Register 0 */
	char	res21[12];
	uint	gtvpr0;		/* 0x41120 - Global Timer Vector/Priority Register 0 */
	char	res22[12];
	uint	gtdr0;		/* 0x41130 - Global Timer Destination Register 0 */
	char	res23[12];
	uint	gtccr1;		/* 0x41140 - Global Timer Current Count Register 1 */
	char	res24[12];
	uint	gtbcr1;		/* 0x41150 - Global Timer Base Count Register 1 */
	char	res25[12];
	uint	gtvpr1;		/* 0x41160 - Global Timer Vector/Priority Register 1 */
	char	res26[12];
	uint	gtdr1;		/* 0x41170 - Global Timer Destination Register 1 */
	char	res27[12];
	uint	gtccr2;		/* 0x41180 - Global Timer Current Count Register 2 */
	char	res28[12];
	uint	gtbcr2;		/* 0x41190 - Global Timer Base Count Register 2 */
	char	res29[12];
	uint	gtvpr2;		/* 0x411a0 - Global Timer Vector/Priority Register 2 */
	char	res30[12];
	uint	gtdr2;		/* 0x411b0 - Global Timer Destination Register 2 */
	char	res31[12];
	uint	gtccr3;		/* 0x411c0 - Global Timer Current Count Register 3 */
	char	res32[12];
	uint	gtbcr3;		/* 0x411d0 - Global Timer Base Count Register 3 */
	char	res33[12];
	uint	gtvpr3;		/* 0x411e0 - Global Timer Vector/Priority Register 3 */
	char	res34[12];
	uint	gtdr3;		/* 0x411f0 - Global Timer Destination Register 3 */
	char	res35[268];
	uint	tcr;		/* 0x41300 - Timer Control Register */
	char	res36[12];
	uint	irqsr0;		/* 0x41310 - IRQ_OUT Summary Register 0 */
	char	res37[12];
	uint	irqsr1;		/* 0x41320 - IRQ_OUT Summary Register 1 */
	char	res38[12];
	uint	cisr0;		/* 0x41330 - Critical Interrupt Summary Register 0 */
	char	res39[12];
	uint	cisr1;		/* 0x41340 - Critical Interrupt Summary Register 1 */
	char	res40[12];
	uint	pm0mr0;		/* 0x41350 - Performance monitor 0 mask register 0  */
	char	res41[12];
	uint	pm0mr1;		/* 0x41360 - Performance monitor 0 mask register 1  */
	char	res42[12];
	uint	pm1mr0;		/* 0x41370 - Performance monitor 1 mask register 0  */
	char	res43[12];
	uint	pm1mr1;		/* 0x41380 - Performance monitor 1 mask register 1  */
	char	res44[12];
	uint	pm2mr0;		/* 0x41390 - Performance monitor 2 mask register 0  */
	char	res45[12];
	uint	pm2mr1;		/* 0x413A0 - Performance monitor 2 mask register 1  */
	char	res46[12];
	uint	pm3mr0;		/* 0x413B0 - Performance monitor 3 mask register 0  */
	char	res47[12];
	uint	pm3mr1;		/* 0x413C0 - Performance monitor 3 mask register 1  */
	char	res48[60];
	uint	msgr0;		/* 0x41400 - Message Register 0 */
	char	res49[12];
	uint	msgr1;		/* 0x41410 - Message Register 1 */
	char	res50[12];
	uint	msgr2;		/* 0x41420 - Message Register 2 */
	char	res51[12];
	uint	msgr3;		/* 0x41430 - Message Register 3 */
	char	res52[204];
	uint	mer;		/* 0x41500 - Message Enable Register */
	char	res53[12];
	uint	msr;		/* 0x41510 - Message Status Register */
	char	res54[60140];
	uint	eivpr0;		/* 0x50000 - External Interrupt Vector/Priority Register 0 */
	char	res55[12];
	uint	eidr0;		/* 0x50010 - External Interrupt Destination Register 0 */
	char	res56[12];
	uint	eivpr1;		/* 0x50020 - External Interrupt Vector/Priority Register 1 */
	char	res57[12];
	uint	eidr1;		/* 0x50030 - External Interrupt Destination Register 1 */
	char	res58[12];
	uint	eivpr2;		/* 0x50040 - External Interrupt Vector/Priority Register 2 */
	char	res59[12];
	uint	eidr2;		/* 0x50050 - External Interrupt Destination Register 2 */
	char	res60[12];
	uint	eivpr3;		/* 0x50060 - External Interrupt Vector/Priority Register 3 */
	char	res61[12];
	uint	eidr3;		/* 0x50070 - External Interrupt Destination Register 3 */
	char	res62[12];
	uint	eivpr4;		/* 0x50080 - External Interrupt Vector/Priority Register 4 */
	char	res63[12];
	uint	eidr4;		/* 0x50090 - External Interrupt Destination Register 4 */
	char	res64[12];
	uint	eivpr5;		/* 0x500a0 - External Interrupt Vector/Priority Register 5 */
	char	res65[12];
	uint	eidr5;		/* 0x500b0 - External Interrupt Destination Register 5 */
	char	res66[12];
	uint	eivpr6;		/* 0x500c0 - External Interrupt Vector/Priority Register 6 */
	char	res67[12];
	uint	eidr6;		/* 0x500d0 - External Interrupt Destination Register 6 */
	char	res68[12];
	uint	eivpr7;		/* 0x500e0 - External Interrupt Vector/Priority Register 7 */
	char	res69[12];
	uint	eidr7;		/* 0x500f0 - External Interrupt Destination Register 7 */
	char	res70[12];
	uint	eivpr8;		/* 0x50100 - External Interrupt Vector/Priority Register 8 */
	char	res71[12];
	uint	eidr8;		/* 0x50110 - External Interrupt Destination Register 8 */
	char	res72[12];
	uint	eivpr9;		/* 0x50120 - External Interrupt Vector/Priority Register 9 */
	char	res73[12];
	uint	eidr9;		/* 0x50130 - External Interrupt Destination Register 9 */
	char	res74[12];
	uint	eivpr10;	/* 0x50140 - External Interrupt Vector/Priority Register 10 */
	char	res75[12];
	uint	eidr10;		/* 0x50150 - External Interrupt Destination Register 10 */
	char	res76[12];
	uint	eivpr11;	/* 0x50160 - External Interrupt Vector/Priority Register 11 */
	char	res77[12];
	uint	eidr11;		/* 0x50170 - External Interrupt Destination Register 11 */
	char	res78[140];
	uint	iivpr0;		/* 0x50200 - Internal Interrupt Vector/Priority Register 0 */
	char	res79[12];
	uint	iidr0;		/* 0x50210 - Internal Interrupt Destination Register 0 */
	char	res80[12];
	uint	iivpr1;		/* 0x50220 - Internal Interrupt Vector/Priority Register 1 */
	char	res81[12];
	uint	iidr1;		/* 0x50230 - Internal Interrupt Destination Register 1 */
	char	res82[12];
	uint	iivpr2;		/* 0x50240 - Internal Interrupt Vector/Priority Register 2 */
	char	res83[12];
	uint	iidr2;		/* 0x50250 - Internal Interrupt Destination Register 2 */
	char	res84[12];
	uint	iivpr3;		/* 0x50260 - Internal Interrupt Vector/Priority Register 3 */
	char	res85[12];
	uint	iidr3;		/* 0x50270 - Internal Interrupt Destination Register 3 */
	char	res86[12];
	uint	iivpr4;		/* 0x50280 - Internal Interrupt Vector/Priority Register 4 */
	char	res87[12];
	uint	iidr4;		/* 0x50290 - Internal Interrupt Destination Register 4 */
	char	res88[12];
	uint	iivpr5;		/* 0x502a0 - Internal Interrupt Vector/Priority Register 5 */
	char	res89[12];
	uint	iidr5;		/* 0x502b0 - Internal Interrupt Destination Register 5 */
	char	res90[12];
	uint	iivpr6;		/* 0x502c0 - Internal Interrupt Vector/Priority Register 6 */
	char	res91[12];
	uint	iidr6;		/* 0x502d0 - Internal Interrupt Destination Register 6 */
	char	res92[12];
	uint	iivpr7;		/* 0x502e0 - Internal Interrupt Vector/Priority Register 7 */
	char	res93[12];
	uint	iidr7;		/* 0x502f0 - Internal Interrupt Destination Register 7 */
	char	res94[12];
	uint	iivpr8;		/* 0x50300 - Internal Interrupt Vector/Priority Register 8 */
	char	res95[12];
	uint	iidr8;		/* 0x50310 - Internal Interrupt Destination Register 8 */
	char	res96[12];
	uint	iivpr9;		/* 0x50320 - Internal Interrupt Vector/Priority Register 9 */
	char	res97[12];
	uint	iidr9;		/* 0x50330 - Internal Interrupt Destination Register 9 */
	char	res98[12];
	uint	iivpr10;	/* 0x50340 - Internal Interrupt Vector/Priority Register 10 */
	char	res99[12];
	uint	iidr10;		/* 0x50350 - Internal Interrupt Destination Register 10 */
	char	res100[12];
	uint	iivpr11;	/* 0x50360 - Internal Interrupt Vector/Priority Register 11 */
	char	res101[12];
	uint	iidr11;		/* 0x50370 - Internal Interrupt Destination Register 11 */
	char	res102[12];
	uint	iivpr12;	/* 0x50380 - Internal Interrupt Vector/Priority Register 12 */
	char	res103[12];
	uint	iidr12;		/* 0x50390 - Internal Interrupt Destination Register 12 */
	char	res104[12];
	uint	iivpr13;	/* 0x503a0 - Internal Interrupt Vector/Priority Register 13 */
	char	res105[12];
	uint	iidr13;		/* 0x503b0 - Internal Interrupt Destination Register 13 */
	char	res106[12];
	uint	iivpr14;	/* 0x503c0 - Internal Interrupt Vector/Priority Register 14 */
	char	res107[12];
	uint	iidr14;		/* 0x503d0 - Internal Interrupt Destination Register 14 */
	char	res108[12];
	uint	iivpr15;	/* 0x503e0 - Internal Interrupt Vector/Priority Register 15 */
	char	res109[12];
	uint	iidr15;		/* 0x503f0 - Internal Interrupt Destination Register 15 */
	char	res110[12];
	uint	iivpr16;	/* 0x50400 - Internal Interrupt Vector/Priority Register 16 */
	char	res111[12];
	uint	iidr16;		/* 0x50410 - Internal Interrupt Destination Register 16 */
	char	res112[12];
	uint	iivpr17;	/* 0x50420 - Internal Interrupt Vector/Priority Register 17 */
	char	res113[12];
	uint	iidr17;		/* 0x50430 - Internal Interrupt Destination Register 17 */
	char	res114[12];
	uint	iivpr18;	/* 0x50440 - Internal Interrupt Vector/Priority Register 18 */
	char	res115[12];
	uint	iidr18;		/* 0x50450 - Internal Interrupt Destination Register 18 */
	char	res116[12];
	uint	iivpr19;	/* 0x50460 - Internal Interrupt Vector/Priority Register 19 */
	char	res117[12];
	uint	iidr19;		/* 0x50470 - Internal Interrupt Destination Register 19 */
	char	res118[12];
	uint	iivpr20;	/* 0x50480 - Internal Interrupt Vector/Priority Register 20 */
	char	res119[12];
	uint	iidr20;		/* 0x50490 - Internal Interrupt Destination Register 20 */
	char	res120[12];
	uint	iivpr21;	/* 0x504a0 - Internal Interrupt Vector/Priority Register 21 */
	char	res121[12];
	uint	iidr21;		/* 0x504b0 - Internal Interrupt Destination Register 21 */
	char	res122[12];
	uint	iivpr22;	/* 0x504c0 - Internal Interrupt Vector/Priority Register 22 */
	char	res123[12];
	uint	iidr22;		/* 0x504d0 - Internal Interrupt Destination Register 22 */
	char	res124[12];
	uint	iivpr23;	/* 0x504e0 - Internal Interrupt Vector/Priority Register 23 */
	char	res125[12];
	uint	iidr23;		/* 0x504f0 - Internal Interrupt Destination Register 23 */
	char	res126[12];
	uint	iivpr24;	/* 0x50500 - Internal Interrupt Vector/Priority Register 24 */
	char	res127[12];
	uint	iidr24;		/* 0x50510 - Internal Interrupt Destination Register 24 */
	char	res128[12];
	uint	iivpr25;	/* 0x50520 - Internal Interrupt Vector/Priority Register 25 */
	char	res129[12];
	uint	iidr25;		/* 0x50530 - Internal Interrupt Destination Register 25 */
	char	res130[12];
	uint	iivpr26;	/* 0x50540 - Internal Interrupt Vector/Priority Register 26 */
	char	res131[12];
	uint	iidr26;		/* 0x50550 - Internal Interrupt Destination Register 26 */
	char	res132[12];
	uint	iivpr27;	/* 0x50560 - Internal Interrupt Vector/Priority Register 27 */
	char	res133[12];
	uint	iidr27;		/* 0x50570 - Internal Interrupt Destination Register 27 */
	char	res134[12];
	uint	iivpr28;	/* 0x50580 - Internal Interrupt Vector/Priority Register 28 */
	char	res135[12];
	uint	iidr28;		/* 0x50590 - Internal Interrupt Destination Register 28 */
	char	res136[12];
	uint	iivpr29;	/* 0x505a0 - Internal Interrupt Vector/Priority Register 29 */
	char	res137[12];
	uint	iidr29;		/* 0x505b0 - Internal Interrupt Destination Register 29 */
	char	res138[12];
	uint	iivpr30;	/* 0x505c0 - Internal Interrupt Vector/Priority Register 30 */
	char	res139[12];
	uint	iidr30;		/* 0x505d0 - Internal Interrupt Destination Register 30 */
	char	res140[12];
	uint	iivpr31;	/* 0x505e0 - Internal Interrupt Vector/Priority Register 31 */
	char	res141[12];
	uint	iidr31;		/* 0x505f0 - Internal Interrupt Destination Register 31 */
	char	res142[4108];
	uint	mivpr0;		/* 0x51600 - Messaging Interrupt Vector/Priority Register 0 */
	char	res143[12];
	uint	midr0;		/* 0x51610 - Messaging Interrupt Destination Register 0 */
	char	res144[12];
	uint	mivpr1;		/* 0x51620 - Messaging Interrupt Vector/Priority Register 1 */
	char	res145[12];
	uint	midr1;		/* 0x51630 - Messaging Interrupt Destination Register 1 */
	char	res146[12];
	uint	mivpr2;		/* 0x51640 - Messaging Interrupt Vector/Priority Register 2 */
	char	res147[12];
	uint	midr2;		/* 0x51650 - Messaging Interrupt Destination Register 2 */
	char	res148[12];
	uint	mivpr3;		/* 0x51660 - Messaging Interrupt Vector/Priority Register 3 */
	char	res149[12];
	uint	midr3;		/* 0x51670 - Messaging Interrupt Destination Register 3 */
	char	res150[59852];
	uint	ipi0dr0;	/* 0x60040 - Processor 0 Interprocessor Interrupt Dispatch Register 0 */
	char	res151[12];
	uint	ipi0dr1;	/* 0x60050 - Processor 0 Interprocessor Interrupt Dispatch Register 1 */
	char	res152[12];
	uint	ipi0dr2;	/* 0x60060 - Processor 0 Interprocessor Interrupt Dispatch Register 2 */
	char	res153[12];
	uint	ipi0dr3;	/* 0x60070 - Processor 0 Interprocessor Interrupt Dispatch Register 3 */
	char	res154[12];
	uint	ctpr0;		/* 0x60080 - Current Task Priority Register for Processor 0 */
	char	res155[12];
	uint	whoami0;	/* 0x60090 - Who Am I Register for Processor 0 */
	char	res156[12];
	uint	iack0;		/* 0x600a0 - Interrupt Acknowledge Register for Processor 0 */
	char	res157[12];
	uint	eoi0;		/* 0x600b0 - End Of Interrupt Register for Processor 0 */
	char	res158[3916];
} ccsr_pic_t;

/* RapidIO Registers(0xc_0000-0xe_0000) */

typedef struct ccsr_rio {
	uint	didcar;		/* 0xc0000 - Device Identity Capability Register */
	uint	dicar;		/* 0xc0004 - Device Information Capability Register */
	uint	aidcar;		/* 0xc0008 - Assembly Identity Capability Register */
	uint	aicar;		/* 0xc000c - Assembly Information Capability Register */
	uint	pefcar;		/* 0xc0010 - Processing Element Features Capability Register */
	uint	spicar;		/* 0xc0014 - Switch Port Information Capability Register */
	uint	socar;		/* 0xc0018 - Source Operations Capability Register */
	uint	docar;		/* 0xc001c - Destination Operations Capability Register */
	char	res1[32];
	uint	msr;		/* 0xc0040 - Mailbox Command And Status Register */
	uint	pwdcsr;		/* 0xc0044 - Port-Write and Doorbell Command And Status Register */
	char	res2[4];
	uint	pellccsr;	/* 0xc004c - Processing Element Logic Layer Control Command and Status Register */
	char	res3[12];
	uint	lcsbacsr;	/* 0xc005c - Local Configuration Space Base Address Command and Status Register */
	uint	bdidcsr;	/* 0xc0060 - Base Device ID Command and Status Register */
	char	res4[4];
	uint	hbdidlcsr;	/* 0xc0068 - Host Base Device ID Lock Command and Status Register */
	uint	ctcsr;		/* 0xc006c - Component Tag Command and Status Register */
	char	res5[144];
	uint	pmbh0csr;	/* 0xc0100 - 8/16 LP-LVDS Port Maintenance Block Header 0 Command and Status Register */
	char	res6[28];
	uint	pltoccsr;	/* 0xc0120 - Port Link Time-out Control Command and Status Register */
	uint	prtoccsr;	/* 0xc0124 - Port Response Time-out Control Command and Status Register */
	char	res7[20];
	uint	pgccsr;		/* 0xc013c - Port General Command and Status Register */
	uint	plmreqcsr;	/* 0xc0140 - Port Link Maintenance Request Command and Status Register */
	uint	plmrespcsr;	/* 0xc0144 - Port Link Maintenance Response Command and Status Register */
	uint	plascsr;	/* 0xc0148 - Port Local Ackid Status Command and Status Register */
	char	res8[12];
	uint	pescsr;		/* 0xc0158 - Port Error and Status Command and Status Register */
	uint	pccsr;		/* 0xc015c - Port Control Command and Status Register */
	char	res9[1184];
	uint	erbh;		/* 0xc0600 - Error Reporting Block Header Register */
	char	res10[4];
	uint	ltledcsr;	/* 0xc0608 - Logical/Transport layer error detect status register */
	uint	ltleecsr;	/* 0xc060c - Logical/Transport layer error enable register */
	char	res11[4];
	uint	ltlaccsr;	/* 0xc0614 - Logical/Transport layer addresss capture register */
	uint	ltldidccsr;	/* 0xc0618 - Logical/Transport layer device ID capture register */
	uint	ltlcccsr;	/* 0xc061c - Logical/Transport layer control capture register */
	char	res12[32];
	uint	edcsr;	        /* 0xc0640 - Port 0 error detect status register */
	uint	erecsr;	        /* 0xc0644 - Port 0 error rate enable status register */
	uint	ecacsr;	        /* 0xc0648 - Port 0 error capture attributes register */
	uint	pcseccsr0;	/* 0xc064c - Port 0 packet/control symbol error capture register 0 */
	uint	peccsr1;	/* 0xc0650 - Port 0 error capture command and status register 1 */
	uint	peccsr2;	/* 0xc0654 - Port 0 error capture command and status register 2 */
	uint	peccsr3;	/* 0xc0658 - Port 0 error capture command and status register 3 */
	char	res13[12];
	uint	ercsr;	        /* 0xc0668 - Port 0 error rate command and status register */
	uint	ertcsr;	        /* 0xc066C - Port 0 error rate threshold status register*/
	char	res14[63892];
	uint	llcr;		/* 0xd0004 - Logical Layer Configuration Register */
	char	res15[12];
	uint	epwisr;		/* 0xd0010 - Error / Port-Write Interrupt Status Register */
	char	res16[12];
	uint	lretcr;		/* 0xd0020 - Logical Retry Error Threshold Configuration Register */
	char	res17[92];
	uint	pretcr;		/* 0xd0080 - Physical Retry Erorr Threshold Configuration Register */
	char	res18[124];
	uint	adidcsr;	/* 0xd0100 - Port 0 Alt. Device ID Command and Status Register */
	char	res19[28];
	uint	ptaacr;	        /* 0xd0120 - Port 0 Pass-Through/Accept-All Configuration Register */
	char	res20[12];
	uint	iecsr;	        /* 0xd0130 - Port 0 Implementation Error Status Register */
	char	res21[12];
	uint	pcr;		/* 0xd0140 - Port 0 Phsyical Configuration RegisterRegister */
	char	res22[20];
	uint	slcsr;	        /* 0xd0158 - Port 0 Serial Link Command and Status Register */
	char	res23[4];
	uint	sleir;	        /* 0xd0160 - Port 0 Serial Link Error Injection Register */
	char	res24[2716];
	uint	rowtar0;	/* 0xd0c00 - RapidIO Outbound Window Translation Address Register 0 */
	uint	rowtear0;	/* 0xd0c04 - RapidIO Outbound Window Translation Ext. Address Register 0 */
	char	res25[8];
	uint	rowar0;		/* 0xd0c10 - RapidIO Outbound Attributes Register 0 */
	char	res26[12];
	uint	rowtar1;	/* 0xd0c20 - RapidIO Outbound Window Translation Address Register 1 */
	uint	rowtear1;	/* 0xd0c24 - RapidIO Outbound Window Translation Ext. Address Register 1 */
	uint	rowbar1;	/* 0xd0c28 - RapidIO Outbound Window Base Address Register 1 */
	char	res27[4];
	uint	rowar1;		/* 0xd0c30 - RapidIO Outbound Attributes Register 1 */
	uint	rows1r1;	/* 0xd0c34 - RapidIO Outbound Window Segment 1 Register 1 */
	uint	rows2r1;	/* 0xd0c38 - RapidIO Outbound Window Segment 2 Register 1 */
	uint	rows3r1;	/* 0xd0c3c - RapidIO Outbound Window Segment 3 Register 1 */
	uint	rowtar2;	/* 0xd0c40 - RapidIO Outbound Window Translation Address Register 2 */
	uint	rowtear2;	/* 0xd0c44 - RapidIO Outbound Window Translation Ext. Address Register 2 */
	uint	rowbar2;	/* 0xd0c48 - RapidIO Outbound Window Base Address Register 2 */
	char	res28[4];
	uint	rowar2;		/* 0xd0c50 - RapidIO Outbound Attributes Register 2 */
	uint	rows1r2;	/* 0xd0c54 - RapidIO Outbound Window Segment 1 Register 2 */
	uint	rows2r2;	/* 0xd0c58 - RapidIO Outbound Window Segment 2 Register 2 */
	uint	rows3r2;	/* 0xd0c5c - RapidIO Outbound Window Segment 3 Register 2 */
	uint	rowtar3;	/* 0xd0c60 - RapidIO Outbound Window Translation Address Register 3 */
	uint	rowtear3;	/* 0xd0c64 - RapidIO Outbound Window Translation Ext. Address Register 3 */
	uint	rowbar3;	/* 0xd0c68 - RapidIO Outbound Window Base Address Register 3 */
	char	res29[4];
	uint	rowar3;		/* 0xd0c70 - RapidIO Outbound Attributes Register 3 */
	uint	rows1r3;	/* 0xd0c74 - RapidIO Outbound Window Segment 1 Register 3 */
	uint	rows2r3;	/* 0xd0c78 - RapidIO Outbound Window Segment 2 Register 3 */
	uint	rows3r3;	/* 0xd0c7c - RapidIO Outbound Window Segment 3 Register 3 */
	uint	rowtar4;	/* 0xd0c80 - RapidIO Outbound Window Translation Address Register 4 */
	uint	rowtear4;	/* 0xd0c84 - RapidIO Outbound Window Translation Ext. Address Register 4 */
	uint	rowbar4;	/* 0xd0c88 - RapidIO Outbound Window Base Address Register 4 */
	char	res30[4];
	uint	rowar4;		/* 0xd0c90 - RapidIO Outbound Attributes Register 4 */
	uint	rows1r4;	/* 0xd0c94 - RapidIO Outbound Window Segment 1 Register 4 */
	uint	rows2r4;	/* 0xd0c98 - RapidIO Outbound Window Segment 2 Register 4 */
	uint	rows3r4;	/* 0xd0c9c - RapidIO Outbound Window Segment 3 Register 4 */
	uint	rowtar5;	/* 0xd0ca0 - RapidIO Outbound Window Translation Address Register 5 */
	uint	rowtear5;	/* 0xd0ca4 - RapidIO Outbound Window Translation Ext. Address Register 5 */
	uint	rowbar5;	/* 0xd0ca8 - RapidIO Outbound Window Base Address Register 5 */
	char	res31[4];
	uint	rowar5;		/* 0xd0cb0 - RapidIO Outbound Attributes Register 5 */
	uint	rows1r5;	/* 0xd0cb4 - RapidIO Outbound Window Segment 1 Register 5 */
	uint	rows2r5;	/* 0xd0cb8 - RapidIO Outbound Window Segment 2 Register 5 */
	uint	rows3r5;	/* 0xd0cbc - RapidIO Outbound Window Segment 3 Register 5 */
	uint	rowtar6;	/* 0xd0cc0 - RapidIO Outbound Window Translation Address Register 6 */
	uint	rowtear6;	/* 0xd0cc4 - RapidIO Outbound Window Translation Ext. Address Register 6 */
	uint	rowbar6;	/* 0xd0cc8 - RapidIO Outbound Window Base Address Register 6 */
	char	res32[4];
	uint	rowar6;		/* 0xd0cd0 - RapidIO Outbound Attributes Register 6 */
	uint	rows1r6;	/* 0xd0cd4 - RapidIO Outbound Window Segment 1 Register 6 */
	uint	rows2r6;	/* 0xd0cd8 - RapidIO Outbound Window Segment 2 Register 6 */
	uint	rows3r6;	/* 0xd0cdc - RapidIO Outbound Window Segment 3 Register 6 */
	uint	rowtar7;	/* 0xd0ce0 - RapidIO Outbound Window Translation Address Register 7 */
	uint	rowtear7;	/* 0xd0ce4 - RapidIO Outbound Window Translation Ext. Address Register 7 */
	uint	rowbar7;	/* 0xd0ce8 - RapidIO Outbound Window Base Address Register 7 */
	char	res33[4];
	uint	rowar7;		/* 0xd0cf0 - RapidIO Outbound Attributes Register 7 */
	uint	rows1r7;	/* 0xd0cf4 - RapidIO Outbound Window Segment 1 Register 7 */
	uint	rows2r7;	/* 0xd0cf8 - RapidIO Outbound Window Segment 2 Register 7 */
	uint	rows3r7;	/* 0xd0cfc - RapidIO Outbound Window Segment 3 Register 7 */
	uint	rowtar8;	/* 0xd0d00 - RapidIO Outbound Window Translation Address Register 8 */
	uint	rowtear8;	/* 0xd0d04 - RapidIO Outbound Window Translation Ext. Address Register 8 */
	uint	rowbar8;	/* 0xd0d08 - RapidIO Outbound Window Base Address Register 8 */
	char	res34[4];
	uint	rowar8;		/* 0xd0d10 - RapidIO Outbound Attributes Register 8 */
	uint	rows1r8;	/* 0xd0d14 - RapidIO Outbound Window Segment 1 Register 8 */
	uint	rows2r8;	/* 0xd0d18 - RapidIO Outbound Window Segment 2 Register 8 */
	uint	rows3r8;	/* 0xd0d1c - RapidIO Outbound Window Segment 3 Register 8 */
	char	res35[64];
	uint	riwtar4;	/* 0xd0d60 - RapidIO Inbound Window Translation Address Register 4 */
	uint	riwbar4;	/* 0xd0d68 - RapidIO Inbound Window Base Address Register 4 */
	char	res36[4];
	uint	riwar4;		/* 0xd0d70 - RapidIO Inbound Attributes Register 4 */
	char	res37[12];
	uint	riwtar3;	/* 0xd0d80 - RapidIO Inbound Window Translation Address Register 3 */
	char	res38[4];
	uint	riwbar3;	/* 0xd0d88 - RapidIO Inbound Window Base Address Register 3 */
	char	res39[4];
	uint	riwar3;		/* 0xd0d90 - RapidIO Inbound Attributes Register 3 */
	char	res40[12];
	uint	riwtar2;	/* 0xd0da0 - RapidIO Inbound Window Translation Address Register 2 */
	char	res41[4];
	uint	riwbar2;	/* 0xd0da8 - RapidIO Inbound Window Base Address Register 2 */
	char	res42[4];
	uint	riwar2;		/* 0xd0db0 - RapidIO Inbound Attributes Register 2 */
	char	res43[12];
	uint	riwtar1;	/* 0xd0dc0 - RapidIO Inbound Window Translation Address Register 1 */
	char	res44[4];
	uint	riwbar1;	/* 0xd0dc8 - RapidIO Inbound Window Base Address Register 1 */
	char	res45[4];
	uint	riwar1;		/* 0xd0dd0 - RapidIO Inbound Attributes Register 1 */
	char	res46[12];
	uint	riwtar0;	/* 0xd0de0 - RapidIO Inbound Window Translation Address Register 0 */
	char	res47[12];
	uint	riwar0;		/* 0xd0df0 - RapidIO Inbound Attributes Register 0 */
	char	res48[12];
	uint	pnfedr;		/* 0xd0e00 - Port Notification/Fatal Error Detect Register */
	uint	pnfedir;	/* 0xd0e04 - Port Notification/Fatal Error Detect Register */
	uint	pnfeier;	/* 0xd0e08 - Port Notification/Fatal Error Interrupt Enable Register */
	uint	pecr;		/* 0xd0e0c - Port Error Control Register */
	uint	pepcsr0;	/* 0xd0e10 - Port Error Packet/Control Symbol Register 0 */
	uint	pepr1;		/* 0xd0e14 - Port Error Packet Register 1 */
	uint	pepr2;		/* 0xd0e18 - Port Error Packet Register 2 */
	char	res49[4];
	uint	predr;		/* 0xd0e20 - Port Recoverable Error Detect Register */
	char	res50[4];
	uint	pertr;		/* 0xd0e28 - Port Error Recovery Threshold Register */
	uint	prtr;		/* 0xd0e2c - Port Retry Threshold Register */
	char	res51[8656];
	uint	omr;		/* 0xd3000 - Outbound Mode Register */
	uint	osr;		/* 0xd3004 - Outbound Status Register */
	uint	eodqtpar;	/* 0xd3008 - Extended Outbound Descriptor Queue Tail Pointer Address Register */
	uint	odqtpar;	/* 0xd300c - Outbound Descriptor Queue Tail Pointer Address Register */
	uint	eosar;		/* 0xd3010 - Extended Outbound Unit Source Address Register */
	uint	osar;		/* 0xd3014 - Outbound Unit Source Address Register */
	uint	odpr;		/* 0xd3018 - Outbound Destination Port Register */
	uint	odatr;		/* 0xd301c - Outbound Destination Attributes Register */
	uint	odcr;		/* 0xd3020 - Outbound Doubleword Count Register */
	uint	eodqhpar;	/* 0xd3024 - Extended Outbound Descriptor Queue Head Pointer Address Register */
	uint	odqhpar;	/* 0xd3028 - Outbound Descriptor Queue Head Pointer Address Register */
	uint	oretr;	        /* 0xd302C - Outbound Retry Error Threshold Register */
	uint	omgr;	        /* 0xd3030 - Outbound Multicast Group Register */
	uint	omlr;	        /* 0xd3034 - Outbound Multicast List Register */
	char	res52[40];
	uint	imr;		/* 0xd3060 - Outbound Mode Register */
	uint	isr;		/* 0xd3064 - Inbound Status Register */
	uint	eidqtpar;	/* 0xd3068 - Extended Inbound Descriptor Queue Tail Pointer Address Register */
	uint	idqtpar;	/* 0xd306c - Inbound Descriptor Queue Tail Pointer Address Register */
	uint	eifqhpar;	/* 0xd3070 - Extended Inbound Frame Queue Head Pointer Address Register */
	uint	ifqhpar;	/* 0xd3074 - Inbound Frame Queue Head Pointer Address Register */
	uint	imirir;	        /* 0xd3078 - Inbound Maximum Interrutp Report Interval Register */
	char	res53[900];
	uint	oddmr;		/* 0xd3400 - Outbound Doorbell Mode Register */
	uint	oddsr;		/* 0xd3404 - Outbound Doorbell Status Register */
	char	res54[16];
	uint	oddpr;		/* 0xd3418 - Outbound Doorbell Destination Port Register */
	uint	oddatr;		/* 0xd341C - Outbound Doorbell Destination Attributes Register */
	char	res55[12];
	uint	oddretr;	/* 0xd342C - Outbound Doorbell Retry Threshold Configuration Register */
	char	res56[48];
	uint	idmr;		/* 0xd3460 - Inbound Doorbell Mode Register */
	uint	idsr;		/* 0xd3464 - Inbound Doorbell Status Register */
	uint	iedqtpar;	/* 0xd3468 - Extended Inbound Doorbell Queue Tail Pointer Address Register */
	uint	iqtpar;	        /* 0xd346c - Inbound Doorbell Queue Tail Pointer Address Register */
	uint	iedqhpar;	/* 0xd3470 - Extended Inbound Doorbell Queue Head Pointer Address Register */
	uint	idqhpar;	/* 0xd3474 - Inbound Doorbell Queue Head Pointer Address Register */
	uint	idmirir;	/* 0xd3478 - Inbound Doorbell Max Interrupt Report Interval Register */
	char	res57[100];
	uint	pwmr;		/* 0xd34e0 - Port-Write Mode Register */
	uint	pwsr;		/* 0xd34e4 - Port-Write Status Register */
	uint	epwqbar;	/* 0xd34e8 - Extended Port-Write Queue Base Address Register */
	uint	pwqbar;		/* 0xd34ec - Port-Write Queue Base Address Register */
	char	res58[51984];
} ccsr_rio_t;

/* Global Utilities Register Block(0xe_0000-0xf_ffff) */
typedef struct ccsr_gur {
	uint	porpllsr;	/* 0xe0000 - POR PLL ratio status register */
	uint	porbmsr;	/* 0xe0004 - POR boot mode status register */
	uint	porimpscr;	/* 0xe0008 - POR I/O impedance status and control register */
	uint	pordevsr;	/* 0xe000c - POR I/O device status regsiter */
	uint	pordbgmsr;	/* 0xe0010 - POR debug mode status register */
	char	res1[12];
	uint	gpporcr;	/* 0xe0020 - General-purpose POR configuration register */
	char	res2[12];
	uint	gpiocr;		/* 0xe0030 - GPIO control register */
	char	res3[12];
	uint	gpoutdr;	/* 0xe0040 - General-purpose output data register */
	char	res4[12];
	uint	gpindr;		/* 0xe0050 - General-purpose input data register */
	char	res5[12];
	uint	pmuxcr;		/* 0xe0060 - Alternate function signal multiplex control */
	char	res6[12];
	uint	devdisr;	/* 0xe0070 - Device disable control */
	char	res7[12];
	uint	powmgtcsr;	/* 0xe0080 - Power management status and control register */
	char	res8[12];
	uint	mcpsumr;	/* 0xe0090 - Machine check summary register */
	uint	rstrscr;	/* 0xe0094 - Reset request status and control register */
	char	res9[8];
	uint	pvr;		/* 0xe00a0 - Processor version register */
	uint	svr;		/* 0xe00a4 - System version register */
	char	res10a[8];
	uint	rstcr;		/* 0xe00b0 - Reset control register */
	char	res10b[1868];
	uint	clkdvdr;	/* 0xe0800 - Clock Divide register */
	char	res10c[796];
	uint	ddr1clkdr;	/* 0xe0b20 - DDRC1 Clock Disable register */
	char	res10d[4];
	uint	ddr2clkdr;	/* 0xe0b28 - DDRC2 Clock Disable register */
	char	res10e[724];
	uint	clkocr;		/* 0xe0e00 - Clock out select register */
	char	res11[12];
	uint	ddrdllcr;	/* 0xe0e10 - DDR DLL control register */
	char	res12[12];
	uint	lbcdllcr;	/* 0xe0e20 - LBC DLL control register */
	char	res13a[224];
	uint	srds1cr0;	/* 0xe0f04 - SerDes1 control register 0 */
	char	res13b[4];
	uint	srds1cr1;	/* 0xe0f08 - SerDes1 control register 1 */
	char	res14[24];
	uint	ddrioovcr;	/* 0xe0f24 - DDR IO Overdrive Control register */
	char	res15a[24];
	uint	srds2cr0;	/* 0xe0f40 - SerDes2 control register 0 */
	uint	srds2cr1;	/* 0xe0f44 - SerDes2 control register 1 */
	char	res16[184];
} ccsr_gur_t;

#define MPC8610_PORBMSR_HA      0x00070000
#define MPC8610_PORBMSR_HA_SHIFT	16
#define MPC8641_PORBMSR_HA      0x00060000
#define MPC8641_PORBMSR_HA_SHIFT	17
#define MPC8610_PORDEVSR_IO_SEL		0x00380000
#define MPC8610_PORDEVSR_IO_SEL_SHIFT		19
#define MPC8641_PORDEVSR_IO_SEL		0x000F0000
#define MPC8641_PORDEVSR_IO_SEL_SHIFT		16
#define MPC86xx_PORDEVSR_CORE1TE	0x00000080 /* ASMP (Core1 addr trans) */
#define MPC86xx_DEVDISR_PCIEX1	0x80000000
#define MPC86xx_DEVDISR_PCIEX2	0x40000000
#define MPC86xx_DEVDISR_PCI1	0x80000000
#define MPC86xx_DEVDISR_PCIE1	0x40000000
#define MPC86xx_DEVDISR_PCIE2	0x20000000
#define MPC86xx_DEVDISR_SRIO	0x00080000
#define MPC86xx_DEVDISR_RMSG	0x00040000
#define MPC86xx_DEVDISR_CPU0	0x00008000
#define MPC86xx_DEVDISR_CPU1	0x00004000
#define MPC86xx_RSTCR_HRST_REQ	0x00000002

/*
 * Watchdog register block(0xe_4000-0xe_4fff)
 */
typedef struct ccsr_wdt {
	uint	res0;
	uint	swcrr; /* System watchdog control register */
	uint	swcnr; /* System watchdog count register */
	char	res1[2];
	ushort	swsrr; /* System watchdog service register */
	char	res2[4080];
} ccsr_wdt_t;

typedef struct immap {
	ccsr_local_mcm_t	im_local_mcm;
	struct ccsr_ddr		im_ddr1;
	ccsr_i2c_t		im_i2c;
	ccsr_duart_t		im_duart;
	fsl_lbc_t		im_lbc;
	struct ccsr_ddr		im_ddr2;
	char                    res1[4096];
	ccsr_pex_t		im_pex1;
	ccsr_pex_t		im_pex2;
	ccsr_ht_t               im_ht;
	char                    res2[90112];
	ccsr_dma_t		im_dma;
	char                    res3[8192];
	ccsr_tsec_t		im_tsec1;
	ccsr_tsec_t		im_tsec2;
	ccsr_tsec_t             im_tsec3;
	ccsr_tsec_t             im_tsec4;
	char                    res4[98304];
	ccsr_pic_t		im_pic;
	char                    res5[389120];
	ccsr_rio_t		im_rio;
	ccsr_gur_t		im_gur;
	char			res6[12288];
	ccsr_wdt_t		im_wdt;
} immap_t;

extern immap_t  *immr;

#define CONFIG_SYS_MPC8xxx_DDR_OFFSET	0x2000
#define CONFIG_SYS_FSL_DDR_ADDR	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC8xxx_DDR_OFFSET)
#define CONFIG_SYS_MPC8xxx_DDR2_OFFSET	0x6000
#define CONFIG_SYS_FSL_DDR2_ADDR	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC8xxx_DDR2_OFFSET)
#define CONFIG_SYS_MPC86xx_DMA_OFFSET	0x21000
#define CONFIG_SYS_MPC86xx_DMA_ADDR	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC86xx_DMA_OFFSET)
#define CONFIG_SYS_MPC86xx_PIC_OFFSET	0x40000
#define CONFIG_SYS_MPC8xxx_PIC_ADDR	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC86xx_PIC_OFFSET)


#define CONFIG_SYS_MPC86xx_PCI1_OFFSET		0x8000
#ifdef CONFIG_ARCH_MPC8610
#define CONFIG_SYS_MPC86xx_PCIE1_OFFSET         0xa000
#else
#define CONFIG_SYS_MPC86xx_PCIE1_OFFSET         0x8000
#endif
#define CONFIG_SYS_MPC86xx_PCIE2_OFFSET         0x9000

#define CONFIG_SYS_PCI1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC86xx_PCI1_OFFSET)
#define CONFIG_SYS_PCI2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC86xx_PCI2_OFFSET)
#define CONFIG_SYS_PCIE1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC86xx_PCIE1_OFFSET)
#define CONFIG_SYS_PCIE2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC86xx_PCIE2_OFFSET)

#define CONFIG_SYS_TSEC1_OFFSET		0x24000
#define CONFIG_SYS_MDIO1_OFFSET		0x24000
#define CONFIG_SYS_LBC_ADDR		(&((immap_t *)CONFIG_SYS_IMMR)->im_lbc)

#define TSEC_BASE_ADDR		(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC1_OFFSET)
#define MDIO_BASE_ADDR		(CONFIG_SYS_IMMR + CONFIG_SYS_MDIO1_OFFSET)

#endif /*__IMMAP_86xx__*/
