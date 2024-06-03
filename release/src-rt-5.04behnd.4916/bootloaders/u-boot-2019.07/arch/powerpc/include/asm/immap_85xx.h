/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MPC85xx Internal Memory Map
 *
 * Copyright 2007-2012 Freescale Semiconductor, Inc.
 *
 * Copyright(c) 2002,2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
 */

#ifndef __IMMAP_85xx__
#define __IMMAP_85xx__

#include <asm/types.h>
#include <asm/fsl_dma.h>
#include <asm/fsl_i2c.h>
#include <fsl_ifc.h>
#include <fsl_sec.h>
#include <fsl_sfp.h>
#include <asm/fsl_lbc.h>
#include <fsl_fman.h>
#include <fsl_immap.h>

typedef struct ccsr_local {
	u32	ccsrbarh;	/* CCSR Base Addr High */
	u32	ccsrbarl;	/* CCSR Base Addr Low */
	u32	ccsrar;		/* CCSR Attr */
#define CCSRAR_C	0x80000000	/* Commit */
	u8	res1[4];
	u32	altcbarh;	/* Alternate Configuration Base Addr High */
	u32	altcbarl;	/* Alternate Configuration Base Addr Low */
	u32	altcar;		/* Alternate Configuration Attr */
	u8	res2[4];
	u32	bstrh;		/* Boot space translation high */
	u32	bstrl;		/* Boot space translation Low */
	u32	bstrar;		/* Boot space translation attributes */
	u8	res3[0xbd4];
	struct {
		u32	lawbarh;	/* LAWn base addr high */
		u32	lawbarl;	/* LAWn base addr low */
		u32	lawar;		/* LAWn attributes */
		u8	res4[4];
	} law[32];
	u8	res35[0x204];
} ccsr_local_t;

/* Local-Access Registers & ECM Registers */
typedef struct ccsr_local_ecm {
	u32	ccsrbar;	/* CCSR Base Addr */
	u8	res1[4];
	u32	altcbar;	/* Alternate Configuration Base Addr */
	u8	res2[4];
	u32	altcar;		/* Alternate Configuration Attr */
	u8	res3[12];
	u32	bptr;		/* Boot Page Translation */
	u8	res4[3044];
	u32	lawbar0;	/* Local Access Window 0 Base Addr */
	u8	res5[4];
	u32	lawar0;		/* Local Access Window 0 Attrs */
	u8	res6[20];
	u32	lawbar1;	/* Local Access Window 1 Base Addr */
	u8	res7[4];
	u32	lawar1;		/* Local Access Window 1 Attrs */
	u8	res8[20];
	u32	lawbar2;	/* Local Access Window 2 Base Addr */
	u8	res9[4];
	u32	lawar2;		/* Local Access Window 2 Attrs */
	u8	res10[20];
	u32	lawbar3;	/* Local Access Window 3 Base Addr */
	u8	res11[4];
	u32	lawar3;		/* Local Access Window 3 Attrs */
	u8	res12[20];
	u32	lawbar4;	/* Local Access Window 4 Base Addr */
	u8	res13[4];
	u32	lawar4;		/* Local Access Window 4 Attrs */
	u8	res14[20];
	u32	lawbar5;	/* Local Access Window 5 Base Addr */
	u8	res15[4];
	u32	lawar5;		/* Local Access Window 5 Attrs */
	u8	res16[20];
	u32	lawbar6;	/* Local Access Window 6 Base Addr */
	u8	res17[4];
	u32	lawar6;		/* Local Access Window 6 Attrs */
	u8	res18[20];
	u32	lawbar7;	/* Local Access Window 7 Base Addr */
	u8	res19[4];
	u32	lawar7;		/* Local Access Window 7 Attrs */
	u8	res19_8a[20];
	u32	lawbar8;	/* Local Access Window 8 Base Addr */
	u8	res19_8b[4];
	u32	lawar8;		/* Local Access Window 8 Attrs */
	u8	res19_9a[20];
	u32	lawbar9;	/* Local Access Window 9 Base Addr */
	u8	res19_9b[4];
	u32	lawar9;		/* Local Access Window 9 Attrs */
	u8	res19_10a[20];
	u32	lawbar10;	/* Local Access Window 10 Base Addr */
	u8	res19_10b[4];
	u32	lawar10;	/* Local Access Window 10 Attrs */
	u8	res19_11a[20];
	u32	lawbar11;	/* Local Access Window 11 Base Addr */
	u8	res19_11b[4];
	u32	lawar11;	/* Local Access Window 11 Attrs */
	u8	res20[652];
	u32	eebacr;		/* ECM CCB Addr Configuration */
	u8	res21[12];
	u32	eebpcr;		/* ECM CCB Port Configuration */
	u8	res22[3564];
	u32	eedr;		/* ECM Error Detect */
	u8	res23[4];
	u32	eeer;		/* ECM Error Enable */
	u32	eeatr;		/* ECM Error Attrs Capture */
	u32	eeadr;		/* ECM Error Addr Capture */
	u8	res24[492];
} ccsr_local_ecm_t;

#define DDR_EOR_RD_BDW_OPT_DIS	0x80000000 /* Read BDW Opt. disable */
#define DDR_EOR_ADDR_HASH_EN	0x40000000 /* Address hash enabled */

/* I2C Registers */
typedef struct ccsr_i2c {
	struct fsl_i2c_base	i2c[1];
	u8	res[4096 - 1 * sizeof(struct fsl_i2c_base)];
} ccsr_i2c_t;

#if defined(CONFIG_ARCH_MPC8540) || \
	defined(CONFIG_ARCH_MPC8541) || \
	defined(CONFIG_ARCH_MPC8548) || \
	defined(CONFIG_ARCH_MPC8555)
/* DUART Registers */
typedef struct ccsr_duart {
	u8	res1[1280];
/* URBR1, UTHR1, UDLB1 with the same addr */
	u8	urbr1_uthr1_udlb1;
/* UIER1, UDMB1 with the same addr01 */
	u8	uier1_udmb1;
/* UIIR1, UFCR1, UAFR1 with the same addr */
	u8	uiir1_ufcr1_uafr1;
	u8	ulcr1;		/* UART1 Line Control */
	u8	umcr1;		/* UART1 Modem Control */
	u8	ulsr1;		/* UART1 Line Status */
	u8	umsr1;		/* UART1 Modem Status */
	u8	uscr1;		/* UART1 Scratch */
	u8	res2[8];
	u8	udsr1;		/* UART1 DMA Status */
	u8	res3[239];
/* URBR2, UTHR2, UDLB2 with the same addr */
	u8	urbr2_uthr2_udlb2;
/* UIER2, UDMB2 with the same addr */
	u8	uier2_udmb2;
/* UIIR2, UFCR2, UAFR2 with the same addr */
	u8	uiir2_ufcr2_uafr2;
	u8	ulcr2;		/* UART2 Line Control */
	u8	umcr2;		/* UART2 Modem Control */
	u8	ulsr2;		/* UART2 Line Status */
	u8	umsr2;		/* UART2 Modem Status */
	u8	uscr2;		/* UART2 Scratch */
	u8	res4[8];
	u8	udsr2;		/* UART2 DMA Status */
	u8	res5[2543];
} ccsr_duart_t;
#else /* MPC8560 uses UART on its CPM */
typedef struct ccsr_duart {
	u8 res[4096];
} ccsr_duart_t;
#endif

/* eSPI Registers */
typedef struct ccsr_espi {
	u32	mode;		/* eSPI mode */
	u32	event;		/* eSPI event */
	u32	mask;		/* eSPI mask */
	u32	com;		/* eSPI command */
	u32	tx;		/* eSPI transmit FIFO access */
	u32	rx;		/* eSPI receive FIFO access */
	u8	res1[8];	/* reserved */
	u32	csmode[4];	/* 0x2c: sSPI CS0/1/2/3 mode */
	u8	res2[4048];	/* fill up to 0x1000 */
} ccsr_espi_t;

/* PCI Registers */
typedef struct ccsr_pcix {
	u32	cfg_addr;	/* PCIX Configuration Addr */
	u32	cfg_data;	/* PCIX Configuration Data */
	u32	int_ack;	/* PCIX IRQ Acknowledge */
	u8	res000c[52];
	u32	liodn_base;	/* PCIX LIODN base register */
	u8	res0044[2996];
	u32	ipver1;		/* PCIX IP block revision register 1 */
	u32	ipver2;		/* PCIX IP block revision register 2 */
	u32	potar0;		/* PCIX Outbound Transaction Addr 0 */
	u32	potear0;	/* PCIX Outbound Translation Extended Addr 0 */
	u32	powbar0;	/* PCIX Outbound Window Base Addr 0 */
	u32	powbear0;	/* PCIX Outbound Window Base Extended Addr 0 */
	u32	powar0;		/* PCIX Outbound Window Attrs 0 */
	u8	res2[12];
	u32	potar1;		/* PCIX Outbound Transaction Addr 1 */
	u32	potear1;	/* PCIX Outbound Translation Extended Addr 1 */
	u32	powbar1;	/* PCIX Outbound Window Base Addr 1 */
	u32	powbear1;	/* PCIX Outbound Window Base Extended Addr 1 */
	u32	powar1;		/* PCIX Outbound Window Attrs 1 */
	u8	res3[12];
	u32	potar2;		/* PCIX Outbound Transaction Addr 2 */
	u32	potear2;	/* PCIX Outbound Translation Extended Addr 2 */
	u32	powbar2;	/* PCIX Outbound Window Base Addr 2 */
	u32	powbear2;	/* PCIX Outbound Window Base Extended Addr 2 */
	u32	powar2;		/* PCIX Outbound Window Attrs 2 */
	u8	res4[12];
	u32	potar3;		/* PCIX Outbound Transaction Addr 3 */
	u32	potear3;	/* PCIX Outbound Translation Extended Addr 3 */
	u32	powbar3;	/* PCIX Outbound Window Base Addr 3 */
	u32	powbear3;	/* PCIX Outbound Window Base Extended Addr 3 */
	u32	powar3;		/* PCIX Outbound Window Attrs 3 */
	u8	res5[12];
	u32	potar4;		/* PCIX Outbound Transaction Addr 4 */
	u32	potear4;	/* PCIX Outbound Translation Extended Addr 4 */
	u32	powbar4;	/* PCIX Outbound Window Base Addr 4 */
	u32	powbear4;	/* PCIX Outbound Window Base Extended Addr 4 */
	u32	powar4;		/* PCIX Outbound Window Attrs 4 */
	u8	res6[268];
	u32	pitar3;		/* PCIX Inbound Translation Addr 3 */
	u32	pitear3;	/* PCIX Inbound Translation Extended Addr 3 */
	u32	piwbar3;	/* PCIX Inbound Window Base Addr 3 */
	u32	piwbear3;	/* PCIX Inbound Window Base Extended Addr 3 */
	u32	piwar3;		/* PCIX Inbound Window Attrs 3 */
	u8	res7[12];
	u32	pitar2;		/* PCIX Inbound Translation Addr 2 */
	u32	pitear2;	/* PCIX Inbound Translation Extended Addr 2 */
	u32	piwbar2;	/* PCIX Inbound Window Base Addr 2 */
	u32	piwbear2;	/* PCIX Inbound Window Base Extended Addr 2 */
	u32	piwar2;		/* PCIX Inbound Window Attrs 2 */
	u8	res8[12];
	u32	pitar1;		/* PCIX Inbound Translation Addr 1 */
	u32	pitear1;	/* PCIX Inbound Translation Extended Addr 1 */
	u32	piwbar1;	/* PCIX Inbound Window Base Addr 1 */
	u8	res9[4];
	u32	piwar1;		/* PCIX Inbound Window Attrs 1 */
	u8	res10[12];
	u32	pedr;		/* PCIX Error Detect */
	u32	pecdr;		/* PCIX Error Capture Disable */
	u32	peer;		/* PCIX Error Enable */
	u32	peattrcr;	/* PCIX Error Attrs Capture */
	u32	peaddrcr;	/* PCIX Error Addr Capture */
	u32	peextaddrcr;	/* PCIX Error Extended Addr Capture */
	u32	pedlcr;		/* PCIX Error Data Low Capture */
	u32	pedhcr;		/* PCIX Error Error Data High Capture */
	u32	gas_timr;	/* PCIX Gasket Timer */
	u8	res11[476];
} ccsr_pcix_t;

#define PCIX_COMMAND	0x62
#define POWAR_EN	0x80000000
#define POWAR_IO_READ	0x00080000
#define POWAR_MEM_READ	0x00040000
#define POWAR_IO_WRITE	0x00008000
#define POWAR_MEM_WRITE	0x00004000
#define POWAR_MEM_512M	0x0000001c
#define POWAR_IO_1M	0x00000013

#define PIWAR_EN	0x80000000
#define PIWAR_PF	0x20000000
#define PIWAR_LOCAL	0x00f00000
#define PIWAR_READ_SNOOP	0x00050000
#define PIWAR_WRITE_SNOOP	0x00005000
#define PIWAR_MEM_2G		0x0000001e

#ifndef CONFIG_MPC85XX_GPIO
typedef struct ccsr_gpio {
	u32	gpdir;
	u32	gpodr;
	u32	gpdat;
	u32	gpier;
	u32	gpimr;
	u32	gpicr;
} ccsr_gpio_t;
#endif

/* L2 Cache Registers */
typedef struct ccsr_l2cache {
	u32	l2ctl;		/* L2 configuration 0 */
	u8	res1[12];
	u32	l2cewar0;	/* L2 cache external write addr 0 */
	u8	res2[4];
	u32	l2cewcr0;	/* L2 cache external write control 0 */
	u8	res3[4];
	u32	l2cewar1;	/* L2 cache external write addr 1 */
	u8	res4[4];
	u32	l2cewcr1;	/* L2 cache external write control 1 */
	u8	res5[4];
	u32	l2cewar2;	/* L2 cache external write addr 2 */
	u8	res6[4];
	u32	l2cewcr2;	/* L2 cache external write control 2 */
	u8	res7[4];
	u32	l2cewar3;	/* L2 cache external write addr 3 */
	u8	res8[4];
	u32	l2cewcr3;	/* L2 cache external write control 3 */
	u8	res9[180];
	u32	l2srbar0;	/* L2 memory-mapped SRAM base addr 0 */
	u8	res10[4];
	u32	l2srbar1;	/* L2 memory-mapped SRAM base addr 1 */
	u8	res11[3316];
	u32	l2errinjhi;	/* L2 error injection mask high */
	u32	l2errinjlo;	/* L2 error injection mask low */
	u32	l2errinjctl;	/* L2 error injection tag/ECC control */
	u8	res12[20];
	u32	l2captdatahi;	/* L2 error data high capture */
	u32	l2captdatalo;	/* L2 error data low capture */
	u32	l2captecc;	/* L2 error ECC capture */
	u8	res13[20];
	u32	l2errdet;	/* L2 error detect */
	u32	l2errdis;	/* L2 error disable */
	u32	l2errinten;	/* L2 error interrupt enable */
	u32	l2errattr;	/* L2 error attributes capture */
	u32	l2erraddr;	/* L2 error addr capture */
	u8	res14[4];
	u32	l2errctl;	/* L2 error control */
	u8	res15[420];
} ccsr_l2cache_t;

#define MPC85xx_L2CTL_L2E			0x80000000
#define MPC85xx_L2CTL_L2SRAM_ENTIRE		0x00010000
#define MPC85xx_L2ERRDIS_MBECC			0x00000008
#define MPC85xx_L2ERRDIS_SBECC			0x00000004

/* DMA Registers */
typedef struct ccsr_dma {
	u8	res1[256];
	struct fsl_dma dma[4];
	u32	dgsr;		/* DMA General Status */
	u8	res2[11516];
} ccsr_dma_t;

/* tsec */
typedef struct ccsr_tsec {
	u8	res1[16];
	u32	ievent;		/* IRQ Event */
	u32	imask;		/* IRQ Mask */
	u32	edis;		/* Error Disabled */
	u8	res2[4];
	u32	ecntrl;		/* Ethernet Control */
	u32	minflr;		/* Minimum Frame Len */
	u32	ptv;		/* Pause Time Value */
	u32	dmactrl;	/* DMA Control */
	u32	tbipa;		/* TBI PHY Addr */
	u8	res3[88];
	u32	fifo_tx_thr;		/* FIFO transmit threshold */
	u8	res4[8];
	u32	fifo_tx_starve;		/* FIFO transmit starve */
	u32	fifo_tx_starve_shutoff;	/* FIFO transmit starve shutoff */
	u8	res5[96];
	u32	tctrl;		/* TX Control */
	u32	tstat;		/* TX Status */
	u8	res6[4];
	u32	tbdlen;		/* TX Buffer Desc Data Len */
	u8	res7[16];
	u32	ctbptrh;	/* Current TX Buffer Desc Ptr High */
	u32	ctbptr;		/* Current TX Buffer Desc Ptr */
	u8	res8[88];
	u32	tbptrh;		/* TX Buffer Desc Ptr High */
	u32	tbptr;		/* TX Buffer Desc Ptr Low */
	u8	res9[120];
	u32	tbaseh;		/* TX Desc Base Addr High */
	u32	tbase;		/* TX Desc Base Addr */
	u8	res10[168];
	u32	ostbd;		/* Out-of-Sequence(OOS) TX Buffer Desc */
	u32	ostbdp;		/* OOS TX Data Buffer Ptr */
	u32	os32tbdp;	/* OOS 32 Bytes TX Data Buffer Ptr Low */
	u32	os32iptrh;	/* OOS 32 Bytes TX Insert Ptr High */
	u32	os32iptrl;	/* OOS 32 Bytes TX Insert Ptr Low */
	u32	os32tbdr;	/* OOS 32 Bytes TX Reserved */
	u32	os32iil;	/* OOS 32 Bytes TX Insert Idx/Len */
	u8	res11[52];
	u32	rctrl;		/* RX Control */
	u32	rstat;		/* RX Status */
	u8	res12[4];
	u32	rbdlen;		/* RxBD Data Len */
	u8	res13[16];
	u32	crbptrh;	/* Current RX Buffer Desc Ptr High */
	u32	crbptr;		/* Current RX Buffer Desc Ptr */
	u8	res14[24];
	u32	mrblr;		/* Maximum RX Buffer Len */
	u32	mrblr2r3;	/* Maximum RX Buffer Len R2R3 */
	u8	res15[56];
	u32	rbptrh;		/* RX Buffer Desc Ptr High 0 */
	u32	rbptr;		/* RX Buffer Desc Ptr */
	u32	rbptrh1;	/* RX Buffer Desc Ptr High 1 */
	u32	rbptrl1;	/* RX Buffer Desc Ptr Low 1 */
	u32	rbptrh2;	/* RX Buffer Desc Ptr High 2 */
	u32	rbptrl2;	/* RX Buffer Desc Ptr Low 2 */
	u32	rbptrh3;	/* RX Buffer Desc Ptr High 3 */
	u32	rbptrl3;	/* RX Buffer Desc Ptr Low 3 */
	u8	res16[96];
	u32	rbaseh;		/* RX Desc Base Addr High 0 */
	u32	rbase;		/* RX Desc Base Addr */
	u32	rbaseh1;	/* RX Desc Base Addr High 1 */
	u32	rbasel1;	/* RX Desc Base Addr Low 1 */
	u32	rbaseh2;	/* RX Desc Base Addr High 2 */
	u32	rbasel2;	/* RX Desc Base Addr Low 2 */
	u32	rbaseh3;	/* RX Desc Base Addr High 3 */
	u32	rbasel3;	/* RX Desc Base Addr Low 3 */
	u8	res17[224];
	u32	maccfg1;	/* MAC Configuration 1 */
	u32	maccfg2;	/* MAC Configuration 2 */
	u32	ipgifg;		/* Inter Packet Gap/Inter Frame Gap */
	u32	hafdup;		/* Half Duplex */
	u32	maxfrm;		/* Maximum Frame Len */
	u8	res18[12];
	u32	miimcfg;	/* MII Management Configuration */
	u32	miimcom;	/* MII Management Cmd */
	u32	miimadd;	/* MII Management Addr */
	u32	miimcon;	/* MII Management Control */
	u32	miimstat;	/* MII Management Status */
	u32	miimind;	/* MII Management Indicator */
	u8	res19[4];
	u32	ifstat;		/* Interface Status */
	u32	macstnaddr1;	/* Station Addr Part 1 */
	u32	macstnaddr2;	/* Station Addr Part 2 */
	u8	res20[312];
	u32	tr64;		/* TX & RX 64-byte Frame Counter */
	u32	tr127;		/* TX & RX 65-127 byte Frame Counter */
	u32	tr255;		/* TX & RX 128-255 byte Frame Counter */
	u32	tr511;		/* TX & RX 256-511 byte Frame Counter */
	u32	tr1k;		/* TX & RX 512-1023 byte Frame Counter */
	u32	trmax;		/* TX & RX 1024-1518 byte Frame Counter */
	u32	trmgv;		/* TX & RX 1519-1522 byte Good VLAN Frame */
	u32	rbyt;		/* RX Byte Counter */
	u32	rpkt;		/* RX Packet Counter */
	u32	rfcs;		/* RX FCS Error Counter */
	u32	rmca;		/* RX Multicast Packet Counter */
	u32	rbca;		/* RX Broadcast Packet Counter */
	u32	rxcf;		/* RX Control Frame Packet Counter */
	u32	rxpf;		/* RX Pause Frame Packet Counter */
	u32	rxuo;		/* RX Unknown OP Code Counter */
	u32	raln;		/* RX Alignment Error Counter */
	u32	rflr;		/* RX Frame Len Error Counter */
	u32	rcde;		/* RX Code Error Counter */
	u32	rcse;		/* RX Carrier Sense Error Counter */
	u32	rund;		/* RX Undersize Packet Counter */
	u32	rovr;		/* RX Oversize Packet Counter */
	u32	rfrg;		/* RX Fragments Counter */
	u32	rjbr;		/* RX Jabber Counter */
	u32	rdrp;		/* RX Drop Counter */
	u32	tbyt;		/* TX Byte Counter Counter */
	u32	tpkt;		/* TX Packet Counter */
	u32	tmca;		/* TX Multicast Packet Counter */
	u32	tbca;		/* TX Broadcast Packet Counter */
	u32	txpf;		/* TX Pause Control Frame Counter */
	u32	tdfr;		/* TX Deferral Packet Counter */
	u32	tedf;		/* TX Excessive Deferral Packet Counter */
	u32	tscl;		/* TX Single Collision Packet Counter */
	u32	tmcl;		/* TX Multiple Collision Packet Counter */
	u32	tlcl;		/* TX Late Collision Packet Counter */
	u32	txcl;		/* TX Excessive Collision Packet Counter */
	u32	tncl;		/* TX Total Collision Counter */
	u8	res21[4];
	u32	tdrp;		/* TX Drop Frame Counter */
	u32	tjbr;		/* TX Jabber Frame Counter */
	u32	tfcs;		/* TX FCS Error Counter */
	u32	txcf;		/* TX Control Frame Counter */
	u32	tovr;		/* TX Oversize Frame Counter */
	u32	tund;		/* TX Undersize Frame Counter */
	u32	tfrg;		/* TX Fragments Frame Counter */
	u32	car1;		/* Carry One */
	u32	car2;		/* Carry Two */
	u32	cam1;		/* Carry Mask One */
	u32	cam2;		/* Carry Mask Two */
	u8	res22[192];
	u32	iaddr0;		/* Indivdual addr 0 */
	u32	iaddr1;		/* Indivdual addr 1 */
	u32	iaddr2;		/* Indivdual addr 2 */
	u32	iaddr3;		/* Indivdual addr 3 */
	u32	iaddr4;		/* Indivdual addr 4 */
	u32	iaddr5;		/* Indivdual addr 5 */
	u32	iaddr6;		/* Indivdual addr 6 */
	u32	iaddr7;		/* Indivdual addr 7 */
	u8	res23[96];
	u32	gaddr0;		/* Global addr 0 */
	u32	gaddr1;		/* Global addr 1 */
	u32	gaddr2;		/* Global addr 2 */
	u32	gaddr3;		/* Global addr 3 */
	u32	gaddr4;		/* Global addr 4 */
	u32	gaddr5;		/* Global addr 5 */
	u32	gaddr6;		/* Global addr 6 */
	u32	gaddr7;		/* Global addr 7 */
	u8	res24[96];
	u32	pmd0;		/* Pattern Match Data */
	u8	res25[4];
	u32	pmask0;		/* Pattern Mask */
	u8	res26[4];
	u32	pcntrl0;	/* Pattern Match Control */
	u8	res27[4];
	u32	pattrb0;	/* Pattern Match Attrs */
	u32	pattrbeli0;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd1;		/* Pattern Match Data */
	u8	res28[4];
	u32	pmask1;		/* Pattern Mask */
	u8	res29[4];
	u32	pcntrl1;	/* Pattern Match Control */
	u8	res30[4];
	u32	pattrb1;	/* Pattern Match Attrs */
	u32	pattrbeli1;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd2;		/* Pattern Match Data */
	u8	res31[4];
	u32	pmask2;		/* Pattern Mask */
	u8	res32[4];
	u32	pcntrl2;	/* Pattern Match Control */
	u8	res33[4];
	u32	pattrb2;	/* Pattern Match Attrs */
	u32	pattrbeli2;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd3;		/* Pattern Match Data */
	u8	res34[4];
	u32	pmask3;		/* Pattern Mask */
	u8	res35[4];
	u32	pcntrl3;	/* Pattern Match Control */
	u8	res36[4];
	u32	pattrb3;	/* Pattern Match Attrs */
	u32	pattrbeli3;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd4;		/* Pattern Match Data */
	u8	res37[4];
	u32	pmask4;		/* Pattern Mask */
	u8	res38[4];
	u32	pcntrl4;	/* Pattern Match Control */
	u8	res39[4];
	u32	pattrb4;	/* Pattern Match Attrs */
	u32	pattrbeli4;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd5;		/* Pattern Match Data */
	u8	res40[4];
	u32	pmask5;		/* Pattern Mask */
	u8	res41[4];
	u32	pcntrl5;	/* Pattern Match Control */
	u8	res42[4];
	u32	pattrb5;	/* Pattern Match Attrs */
	u32	pattrbeli5;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd6;		/* Pattern Match Data */
	u8	res43[4];
	u32	pmask6;		/* Pattern Mask */
	u8	res44[4];
	u32	pcntrl6;	/* Pattern Match Control */
	u8	res45[4];
	u32	pattrb6;	/* Pattern Match Attrs */
	u32	pattrbeli6;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd7;		/* Pattern Match Data */
	u8	res46[4];
	u32	pmask7;		/* Pattern Mask */
	u8	res47[4];
	u32	pcntrl7;	/* Pattern Match Control */
	u8	res48[4];
	u32	pattrb7;	/* Pattern Match Attrs */
	u32	pattrbeli7;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd8;		/* Pattern Match Data */
	u8	res49[4];
	u32	pmask8;		/* Pattern Mask */
	u8	res50[4];
	u32	pcntrl8;	/* Pattern Match Control */
	u8	res51[4];
	u32	pattrb8;	/* Pattern Match Attrs */
	u32	pattrbeli8;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd9;		/* Pattern Match Data */
	u8	res52[4];
	u32	pmask9;		/* Pattern Mask */
	u8	res53[4];
	u32	pcntrl9;	/* Pattern Match Control */
	u8	res54[4];
	u32	pattrb9;	/* Pattern Match Attrs */
	u32	pattrbeli9;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd10;		/* Pattern Match Data */
	u8	res55[4];
	u32	pmask10;	/* Pattern Mask */
	u8	res56[4];
	u32	pcntrl10;	/* Pattern Match Control */
	u8	res57[4];
	u32	pattrb10;	/* Pattern Match Attrs */
	u32	pattrbeli10;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd11;		/* Pattern Match Data */
	u8	res58[4];
	u32	pmask11;	/* Pattern Mask */
	u8	res59[4];
	u32	pcntrl11;	/* Pattern Match Control */
	u8	res60[4];
	u32	pattrb11;	/* Pattern Match Attrs */
	u32	pattrbeli11;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd12;		/* Pattern Match Data */
	u8	res61[4];
	u32	pmask12;	/* Pattern Mask */
	u8	res62[4];
	u32	pcntrl12;	/* Pattern Match Control */
	u8	res63[4];
	u32	pattrb12;	/* Pattern Match Attrs */
	u32	pattrbeli12;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd13;		/* Pattern Match Data */
	u8	res64[4];
	u32	pmask13;	/* Pattern Mask */
	u8	res65[4];
	u32	pcntrl13;	/* Pattern Match Control */
	u8	res66[4];
	u32	pattrb13;	/* Pattern Match Attrs */
	u32	pattrbeli13;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd14;		/* Pattern Match Data */
	u8	res67[4];
	u32	pmask14;	/* Pattern Mask */
	u8	res68[4];
	u32	pcntrl14;	/* Pattern Match Control */
	u8	res69[4];
	u32	pattrb14;	/* Pattern Match Attrs */
	u32	pattrbeli14;	/* Pattern Match Attrs Extract Len & Idx */
	u32	pmd15;		/* Pattern Match Data */
	u8	res70[4];
	u32	pmask15;	/* Pattern Mask */
	u8	res71[4];
	u32	pcntrl15;	/* Pattern Match Control */
	u8	res72[4];
	u32	pattrb15;	/* Pattern Match Attrs */
	u32	pattrbeli15;	/* Pattern Match Attrs Extract Len & Idx */
	u8	res73[248];
	u32	attr;		/* Attrs */
	u32	attreli;	/* Attrs Extract Len & Idx */
	u8	res74[1024];
} ccsr_tsec_t;

/* PIC Registers */
typedef struct ccsr_pic {
	u8	res1[64];
	u32	ipidr0;		/* Interprocessor IRQ Dispatch 0 */
	u8	res2[12];
	u32	ipidr1;		/* Interprocessor IRQ Dispatch 1 */
	u8	res3[12];
	u32	ipidr2;		/* Interprocessor IRQ Dispatch 2 */
	u8	res4[12];
	u32	ipidr3;		/* Interprocessor IRQ Dispatch 3 */
	u8	res5[12];
	u32	ctpr;		/* Current Task Priority */
	u8	res6[12];
	u32	whoami;		/* Who Am I */
	u8	res7[12];
	u32	iack;		/* IRQ Acknowledge */
	u8	res8[12];
	u32	eoi;		/* End Of IRQ */
	u8	res9[3916];
	u32	frr;		/* Feature Reporting */
	u8	res10[28];
	u32	gcr;		/* Global Configuration */
#define MPC85xx_PICGCR_RST	0x80000000
#define MPC85xx_PICGCR_M	0x20000000
	u8	res11[92];
	u32	vir;		/* Vendor Identification */
	u8	res12[12];
	u32	pir;		/* Processor Initialization */
	u8	res13[12];
	u32	ipivpr0;	/* IPI Vector/Priority 0 */
	u8	res14[12];
	u32	ipivpr1;	/* IPI Vector/Priority 1 */
	u8	res15[12];
	u32	ipivpr2;	/* IPI Vector/Priority 2 */
	u8	res16[12];
	u32	ipivpr3;	/* IPI Vector/Priority 3 */
	u8	res17[12];
	u32	svr;		/* Spurious Vector */
	u8	res18[12];
	u32	tfrr;		/* Timer Frequency Reporting */
	u8	res19[12];
	u32	gtccr0;		/* Global Timer Current Count 0 */
	u8	res20[12];
	u32	gtbcr0;		/* Global Timer Base Count 0 */
	u8	res21[12];
	u32	gtvpr0;		/* Global Timer Vector/Priority 0 */
	u8	res22[12];
	u32	gtdr0;		/* Global Timer Destination 0 */
	u8	res23[12];
	u32	gtccr1;		/* Global Timer Current Count 1 */
	u8	res24[12];
	u32	gtbcr1;		/* Global Timer Base Count 1 */
	u8	res25[12];
	u32	gtvpr1;		/* Global Timer Vector/Priority 1 */
	u8	res26[12];
	u32	gtdr1;		/* Global Timer Destination 1 */
	u8	res27[12];
	u32	gtccr2;		/* Global Timer Current Count 2 */
	u8	res28[12];
	u32	gtbcr2;		/* Global Timer Base Count 2 */
	u8	res29[12];
	u32	gtvpr2;		/* Global Timer Vector/Priority 2 */
	u8	res30[12];
	u32	gtdr2;		/* Global Timer Destination 2 */
	u8	res31[12];
	u32	gtccr3;		/* Global Timer Current Count 3 */
	u8	res32[12];
	u32	gtbcr3;		/* Global Timer Base Count 3 */
	u8	res33[12];
	u32	gtvpr3;		/* Global Timer Vector/Priority 3 */
	u8	res34[12];
	u32	gtdr3;		/* Global Timer Destination 3 */
	u8	res35[268];
	u32	tcr;		/* Timer Control */
	u8	res36[12];
	u32	irqsr0;		/* IRQ_OUT Summary 0 */
	u8	res37[12];
	u32	irqsr1;		/* IRQ_OUT Summary 1 */
	u8	res38[12];
	u32	cisr0;		/* Critical IRQ Summary 0 */
	u8	res39[12];
	u32	cisr1;		/* Critical IRQ Summary 1 */
	u8	res40[188];
	u32	msgr0;		/* Message 0 */
	u8	res41[12];
	u32	msgr1;		/* Message 1 */
	u8	res42[12];
	u32	msgr2;		/* Message 2 */
	u8	res43[12];
	u32	msgr3;		/* Message 3 */
	u8	res44[204];
	u32	mer;		/* Message Enable */
	u8	res45[12];
	u32	msr;		/* Message Status */
	u8	res46[60140];
	u32	eivpr0;		/* External IRQ Vector/Priority 0 */
	u8	res47[12];
	u32	eidr0;		/* External IRQ Destination 0 */
	u8	res48[12];
	u32	eivpr1;		/* External IRQ Vector/Priority 1 */
	u8	res49[12];
	u32	eidr1;		/* External IRQ Destination 1 */
	u8	res50[12];
	u32	eivpr2;		/* External IRQ Vector/Priority 2 */
	u8	res51[12];
	u32	eidr2;		/* External IRQ Destination 2 */
	u8	res52[12];
	u32	eivpr3;		/* External IRQ Vector/Priority 3 */
	u8	res53[12];
	u32	eidr3;		/* External IRQ Destination 3 */
	u8	res54[12];
	u32	eivpr4;		/* External IRQ Vector/Priority 4 */
	u8	res55[12];
	u32	eidr4;		/* External IRQ Destination 4 */
	u8	res56[12];
	u32	eivpr5;		/* External IRQ Vector/Priority 5 */
	u8	res57[12];
	u32	eidr5;		/* External IRQ Destination 5 */
	u8	res58[12];
	u32	eivpr6;		/* External IRQ Vector/Priority 6 */
	u8	res59[12];
	u32	eidr6;		/* External IRQ Destination 6 */
	u8	res60[12];
	u32	eivpr7;		/* External IRQ Vector/Priority 7 */
	u8	res61[12];
	u32	eidr7;		/* External IRQ Destination 7 */
	u8	res62[12];
	u32	eivpr8;		/* External IRQ Vector/Priority 8 */
	u8	res63[12];
	u32	eidr8;		/* External IRQ Destination 8 */
	u8	res64[12];
	u32	eivpr9;		/* External IRQ Vector/Priority 9 */
	u8	res65[12];
	u32	eidr9;		/* External IRQ Destination 9 */
	u8	res66[12];
	u32	eivpr10;	/* External IRQ Vector/Priority 10 */
	u8	res67[12];
	u32	eidr10;		/* External IRQ Destination 10 */
	u8	res68[12];
	u32	eivpr11;	/* External IRQ Vector/Priority 11 */
	u8	res69[12];
	u32	eidr11;		/* External IRQ Destination 11 */
	u8	res70[140];
	u32	iivpr0;		/* Internal IRQ Vector/Priority 0 */
	u8	res71[12];
	u32	iidr0;		/* Internal IRQ Destination 0 */
	u8	res72[12];
	u32	iivpr1;		/* Internal IRQ Vector/Priority 1 */
	u8	res73[12];
	u32	iidr1;		/* Internal IRQ Destination 1 */
	u8	res74[12];
	u32	iivpr2;		/* Internal IRQ Vector/Priority 2 */
	u8	res75[12];
	u32	iidr2;		/* Internal IRQ Destination 2 */
	u8	res76[12];
	u32	iivpr3;		/* Internal IRQ Vector/Priority 3 */
	u8	res77[12];
	u32	iidr3;		/* Internal IRQ Destination 3 */
	u8	res78[12];
	u32	iivpr4;		/* Internal IRQ Vector/Priority 4 */
	u8	res79[12];
	u32	iidr4;		/* Internal IRQ Destination 4 */
	u8	res80[12];
	u32	iivpr5;		/* Internal IRQ Vector/Priority 5 */
	u8	res81[12];
	u32	iidr5;		/* Internal IRQ Destination 5 */
	u8	res82[12];
	u32	iivpr6;		/* Internal IRQ Vector/Priority 6 */
	u8	res83[12];
	u32	iidr6;		/* Internal IRQ Destination 6 */
	u8	res84[12];
	u32	iivpr7;		/* Internal IRQ Vector/Priority 7 */
	u8	res85[12];
	u32	iidr7;		/* Internal IRQ Destination 7 */
	u8	res86[12];
	u32	iivpr8;		/* Internal IRQ Vector/Priority 8 */
	u8	res87[12];
	u32	iidr8;		/* Internal IRQ Destination 8 */
	u8	res88[12];
	u32	iivpr9;		/* Internal IRQ Vector/Priority 9 */
	u8	res89[12];
	u32	iidr9;		/* Internal IRQ Destination 9 */
	u8	res90[12];
	u32	iivpr10;	/* Internal IRQ Vector/Priority 10 */
	u8	res91[12];
	u32	iidr10;		/* Internal IRQ Destination 10 */
	u8	res92[12];
	u32	iivpr11;	/* Internal IRQ Vector/Priority 11 */
	u8	res93[12];
	u32	iidr11;		/* Internal IRQ Destination 11 */
	u8	res94[12];
	u32	iivpr12;	/* Internal IRQ Vector/Priority 12 */
	u8	res95[12];
	u32	iidr12;		/* Internal IRQ Destination 12 */
	u8	res96[12];
	u32	iivpr13;	/* Internal IRQ Vector/Priority 13 */
	u8	res97[12];
	u32	iidr13;		/* Internal IRQ Destination 13 */
	u8	res98[12];
	u32	iivpr14;	/* Internal IRQ Vector/Priority 14 */
	u8	res99[12];
	u32	iidr14;		/* Internal IRQ Destination 14 */
	u8	res100[12];
	u32	iivpr15;	/* Internal IRQ Vector/Priority 15 */
	u8	res101[12];
	u32	iidr15;		/* Internal IRQ Destination 15 */
	u8	res102[12];
	u32	iivpr16;	/* Internal IRQ Vector/Priority 16 */
	u8	res103[12];
	u32	iidr16;		/* Internal IRQ Destination 16 */
	u8	res104[12];
	u32	iivpr17;	/* Internal IRQ Vector/Priority 17 */
	u8	res105[12];
	u32	iidr17;		/* Internal IRQ Destination 17 */
	u8	res106[12];
	u32	iivpr18;	/* Internal IRQ Vector/Priority 18 */
	u8	res107[12];
	u32	iidr18;		/* Internal IRQ Destination 18 */
	u8	res108[12];
	u32	iivpr19;	/* Internal IRQ Vector/Priority 19 */
	u8	res109[12];
	u32	iidr19;		/* Internal IRQ Destination 19 */
	u8	res110[12];
	u32	iivpr20;	/* Internal IRQ Vector/Priority 20 */
	u8	res111[12];
	u32	iidr20;		/* Internal IRQ Destination 20 */
	u8	res112[12];
	u32	iivpr21;	/* Internal IRQ Vector/Priority 21 */
	u8	res113[12];
	u32	iidr21;		/* Internal IRQ Destination 21 */
	u8	res114[12];
	u32	iivpr22;	/* Internal IRQ Vector/Priority 22 */
	u8	res115[12];
	u32	iidr22;		/* Internal IRQ Destination 22 */
	u8	res116[12];
	u32	iivpr23;	/* Internal IRQ Vector/Priority 23 */
	u8	res117[12];
	u32	iidr23;		/* Internal IRQ Destination 23 */
	u8	res118[12];
	u32	iivpr24;	/* Internal IRQ Vector/Priority 24 */
	u8	res119[12];
	u32	iidr24;		/* Internal IRQ Destination 24 */
	u8	res120[12];
	u32	iivpr25;	/* Internal IRQ Vector/Priority 25 */
	u8	res121[12];
	u32	iidr25;		/* Internal IRQ Destination 25 */
	u8	res122[12];
	u32	iivpr26;	/* Internal IRQ Vector/Priority 26 */
	u8	res123[12];
	u32	iidr26;		/* Internal IRQ Destination 26 */
	u8	res124[12];
	u32	iivpr27;	/* Internal IRQ Vector/Priority 27 */
	u8	res125[12];
	u32	iidr27;		/* Internal IRQ Destination 27 */
	u8	res126[12];
	u32	iivpr28;	/* Internal IRQ Vector/Priority 28 */
	u8	res127[12];
	u32	iidr28;		/* Internal IRQ Destination 28 */
	u8	res128[12];
	u32	iivpr29;	/* Internal IRQ Vector/Priority 29 */
	u8	res129[12];
	u32	iidr29;		/* Internal IRQ Destination 29 */
	u8	res130[12];
	u32	iivpr30;	/* Internal IRQ Vector/Priority 30 */
	u8	res131[12];
	u32	iidr30;		/* Internal IRQ Destination 30 */
	u8	res132[12];
	u32	iivpr31;	/* Internal IRQ Vector/Priority 31 */
	u8	res133[12];
	u32	iidr31;		/* Internal IRQ Destination 31 */
	u8	res134[4108];
	u32	mivpr0;		/* Messaging IRQ Vector/Priority 0 */
	u8	res135[12];
	u32	midr0;		/* Messaging IRQ Destination 0 */
	u8	res136[12];
	u32	mivpr1;		/* Messaging IRQ Vector/Priority 1 */
	u8	res137[12];
	u32	midr1;		/* Messaging IRQ Destination 1 */
	u8	res138[12];
	u32	mivpr2;		/* Messaging IRQ Vector/Priority 2 */
	u8	res139[12];
	u32	midr2;		/* Messaging IRQ Destination 2 */
	u8	res140[12];
	u32	mivpr3;		/* Messaging IRQ Vector/Priority 3 */
	u8	res141[12];
	u32	midr3;		/* Messaging IRQ Destination 3 */
	u8	res142[59852];
	u32	ipi0dr0;	/* Processor 0 Interprocessor IRQ Dispatch 0 */
	u8	res143[12];
	u32	ipi0dr1;	/* Processor 0 Interprocessor IRQ Dispatch 1 */
	u8	res144[12];
	u32	ipi0dr2;	/* Processor 0 Interprocessor IRQ Dispatch 2 */
	u8	res145[12];
	u32	ipi0dr3;	/* Processor 0 Interprocessor IRQ Dispatch 3 */
	u8	res146[12];
	u32	ctpr0;		/* Current Task Priority for Processor 0 */
	u8	res147[12];
	u32	whoami0;	/* Who Am I for Processor 0 */
	u8	res148[12];
	u32	iack0;		/* IRQ Acknowledge for Processor 0 */
	u8	res149[12];
	u32	eoi0;		/* End Of IRQ for Processor 0 */
	u8	res150[130892];
} ccsr_pic_t;

/* CPM Block */
#ifndef CONFIG_CPM2
typedef struct ccsr_cpm {
	u8 res[262144];
} ccsr_cpm_t;
#else
/*
 * DPARM
 * General SIU
 */
typedef struct ccsr_cpm_siu {
	u8	res1[80];
	u32	smaer;
	u32	smser;
	u32	smevr;
	u8	res2[4];
	u32	lmaer;
	u32	lmser;
	u32	lmevr;
	u8	res3[2964];
} ccsr_cpm_siu_t;

/* IRQ Controller */
typedef struct ccsr_cpm_intctl {
	u16	sicr;
	u8	res1[2];
	u32	sivec;
	u32	sipnrh;
	u32	sipnrl;
	u32	siprr;
	u32	scprrh;
	u32	scprrl;
	u32	simrh;
	u32	simrl;
	u32	siexr;
	u8	res2[88];
	u32	sccr;
	u8	res3[124];
} ccsr_cpm_intctl_t;

/* input/output port */
typedef struct ccsr_cpm_iop {
	u32	pdira;
	u32	ppara;
	u32	psora;
	u32	podra;
	u32	pdata;
	u8	res1[12];
	u32	pdirb;
	u32	pparb;
	u32	psorb;
	u32	podrb;
	u32	pdatb;
	u8	res2[12];
	u32	pdirc;
	u32	pparc;
	u32	psorc;
	u32	podrc;
	u32	pdatc;
	u8	res3[12];
	u32	pdird;
	u32	ppard;
	u32	psord;
	u32	podrd;
	u32	pdatd;
	u8	res4[12];
} ccsr_cpm_iop_t;

/* CPM timers */
typedef struct ccsr_cpm_timer {
	u8	tgcr1;
	u8	res1[3];
	u8	tgcr2;
	u8	res2[11];
	u16	tmr1;
	u16	tmr2;
	u16	trr1;
	u16	trr2;
	u16	tcr1;
	u16	tcr2;
	u16	tcn1;
	u16	tcn2;
	u16	tmr3;
	u16	tmr4;
	u16	trr3;
	u16	trr4;
	u16	tcr3;
	u16	tcr4;
	u16	tcn3;
	u16	tcn4;
	u16	ter1;
	u16	ter2;
	u16	ter3;
	u16	ter4;
	u8	res3[608];
} ccsr_cpm_timer_t;

/* SDMA */
typedef struct ccsr_cpm_sdma {
	u8	sdsr;
	u8	res1[3];
	u8	sdmr;
	u8	res2[739];
} ccsr_cpm_sdma_t;

/* FCC1 */
typedef struct ccsr_cpm_fcc1 {
	u32	gfmr;
	u32	fpsmr;
	u16	ftodr;
	u8	res1[2];
	u16	fdsr;
	u8	res2[2];
	u16	fcce;
	u8	res3[2];
	u16	fccm;
	u8	res4[2];
	u8	fccs;
	u8	res5[3];
	u8	ftirr_phy[4];
} ccsr_cpm_fcc1_t;

/* FCC2 */
typedef struct ccsr_cpm_fcc2 {
	u32	gfmr;
	u32	fpsmr;
	u16	ftodr;
	u8	res1[2];
	u16	fdsr;
	u8	res2[2];
	u16	fcce;
	u8	res3[2];
	u16	fccm;
	u8	res4[2];
	u8	fccs;
	u8	res5[3];
	u8	ftirr_phy[4];
} ccsr_cpm_fcc2_t;

/* FCC3 */
typedef struct ccsr_cpm_fcc3 {
	u32	gfmr;
	u32	fpsmr;
	u16	ftodr;
	u8	res1[2];
	u16	fdsr;
	u8	res2[2];
	u16	fcce;
	u8	res3[2];
	u16	fccm;
	u8	res4[2];
	u8	fccs;
	u8	res5[3];
	u8	res[36];
} ccsr_cpm_fcc3_t;

/* FCC1 extended */
typedef struct ccsr_cpm_fcc1_ext {
	u32	firper;
	u32	firer;
	u32	firsr_h;
	u32	firsr_l;
	u8	gfemr;
	u8	res[15];

} ccsr_cpm_fcc1_ext_t;

/* FCC2 extended */
typedef struct ccsr_cpm_fcc2_ext {
	u32	firper;
	u32	firer;
	u32	firsr_h;
	u32	firsr_l;
	u8	gfemr;
	u8	res[31];
} ccsr_cpm_fcc2_ext_t;

/* FCC3 extended */
typedef struct ccsr_cpm_fcc3_ext {
	u8	gfemr;
	u8	res[47];
} ccsr_cpm_fcc3_ext_t;

/* TC layers */
typedef struct ccsr_cpm_tmp1 {
	u8	res[496];
} ccsr_cpm_tmp1_t;

/* BRGs:5,6,7,8 */
typedef struct ccsr_cpm_brg2 {
	u32	brgc5;
	u32	brgc6;
	u32	brgc7;
	u32	brgc8;
	u8	res[608];
} ccsr_cpm_brg2_t;

/* I2C */
typedef struct ccsr_cpm_i2c {
	u8	i2mod;
	u8	res1[3];
	u8	i2add;
	u8	res2[3];
	u8	i2brg;
	u8	res3[3];
	u8	i2com;
	u8	res4[3];
	u8	i2cer;
	u8	res5[3];
	u8	i2cmr;
	u8	res6[331];
} ccsr_cpm_i2c_t;

/* CPM core */
typedef struct ccsr_cpm_cp {
	u32	cpcr;
	u32	rccr;
	u8	res1[14];
	u16	rter;
	u8	res2[2];
	u16	rtmr;
	u16	rtscr;
	u8	res3[2];
	u32	rtsr;
	u8	res4[12];
} ccsr_cpm_cp_t;

/* BRGs:1,2,3,4 */
typedef struct ccsr_cpm_brg1 {
	u32	brgc1;
	u32	brgc2;
	u32	brgc3;
	u32	brgc4;
} ccsr_cpm_brg1_t;

/* SCC1-SCC4 */
typedef struct ccsr_cpm_scc {
	u32	gsmrl;
	u32	gsmrh;
	u16	psmr;
	u8	res1[2];
	u16	todr;
	u16	dsr;
	u16	scce;
	u8	res2[2];
	u16	sccm;
	u8	res3;
	u8	sccs;
	u8	res4[8];
} ccsr_cpm_scc_t;

typedef struct ccsr_cpm_tmp2 {
	u8	res[32];
} ccsr_cpm_tmp2_t;

/* SPI */
typedef struct ccsr_cpm_spi {
	u16	spmode;
	u8	res1[4];
	u8	spie;
	u8	res2[3];
	u8	spim;
	u8	res3[2];
	u8	spcom;
	u8	res4[82];
} ccsr_cpm_spi_t;

/* CPM MUX */
typedef struct ccsr_cpm_mux {
	u8	cmxsi1cr;
	u8	res1;
	u8	cmxsi2cr;
	u8	res2;
	u32	cmxfcr;
	u32	cmxscr;
	u8	res3[2];
	u16	cmxuar;
	u8	res4[16];
} ccsr_cpm_mux_t;

/* SI,MCC,etc */
typedef struct ccsr_cpm_tmp3 {
	u8 res[58592];
} ccsr_cpm_tmp3_t;

typedef struct ccsr_cpm_iram {
	u32	iram[8192];
	u8	res[98304];
} ccsr_cpm_iram_t;

typedef struct ccsr_cpm {
	/* Some references are into the unique & known dpram spaces,
	 * others are from the generic base.
	 */
#define im_dprambase		im_dpram1
	u8			im_dpram1[16*1024];
	u8			res1[16*1024];
	u8			im_dpram2[16*1024];
	u8			res2[16*1024];
	ccsr_cpm_siu_t		im_cpm_siu; /* SIU Configuration */
	ccsr_cpm_intctl_t	im_cpm_intctl; /* IRQ Controller */
	ccsr_cpm_iop_t		im_cpm_iop; /* IO Port control/status */
	ccsr_cpm_timer_t	im_cpm_timer; /* CPM timers */
	ccsr_cpm_sdma_t		im_cpm_sdma; /* SDMA control/status */
	ccsr_cpm_fcc1_t		im_cpm_fcc1;
	ccsr_cpm_fcc2_t		im_cpm_fcc2;
	ccsr_cpm_fcc3_t		im_cpm_fcc3;
	ccsr_cpm_fcc1_ext_t	im_cpm_fcc1_ext;
	ccsr_cpm_fcc2_ext_t	im_cpm_fcc2_ext;
	ccsr_cpm_fcc3_ext_t	im_cpm_fcc3_ext;
	ccsr_cpm_tmp1_t		im_cpm_tmp1;
	ccsr_cpm_brg2_t		im_cpm_brg2;
	ccsr_cpm_i2c_t		im_cpm_i2c;
	ccsr_cpm_cp_t		im_cpm_cp;
	ccsr_cpm_brg1_t		im_cpm_brg1;
	ccsr_cpm_scc_t		im_cpm_scc[4];
	ccsr_cpm_tmp2_t		im_cpm_tmp2;
	ccsr_cpm_spi_t		im_cpm_spi;
	ccsr_cpm_mux_t		im_cpm_mux;
	ccsr_cpm_tmp3_t		im_cpm_tmp3;
	ccsr_cpm_iram_t		im_cpm_iram;
} ccsr_cpm_t;
#endif

#ifdef CONFIG_SYS_SRIO
/* Architectural regsiters */
struct rio_arch {
	u32	didcar;	/* Device Identity CAR */
	u32	dicar;	/* Device Information CAR */
	u32	aidcar;	/* Assembly Identity CAR */
	u32	aicar;	/* Assembly Information CAR */
	u32	pefcar;	/* Processing Element Features CAR */
	u8	res0[4];
	u32	socar;	/* Source Operations CAR */
	u32	docar;	/* Destination Operations CAR */
	u8	res1[32];
	u32	mcsr;	/* Mailbox CSR */
	u32	pwdcsr;	/* Port-Write and Doorbell CSR */
	u8	res2[4];
	u32	pellccsr;	/* Processing Element Logic Layer CCSR */
	u8	res3[12];
	u32	lcsbacsr;	/* Local Configuration Space BACSR */
	u32	bdidcsr;	/* Base Device ID CSR */
	u8	res4[4];
	u32	hbdidlcsr;	/* Host Base Device ID Lock CSR */
	u32	ctcsr;	/* Component Tag CSR */
};

/* Extended Features Space: 1x/4x LP-Serial Port registers */
struct rio_lp_serial_port {
	u32	plmreqcsr;	/* Port Link Maintenance Request CSR */
	u32	plmrespcsr;	/* Port Link Maintenance Response CS */
	u32	plascsr;	/* Port Local Ackid Status CSR */
	u8	res0[12];
	u32	pescsr;	/* Port Error and Status CSR */
	u32	pccsr;	/* Port Control CSR */
};

/* Extended Features Space: 1x/4x LP-Serial registers */
struct rio_lp_serial {
	u32	pmbh0csr;	/* Port Maintenance Block Header 0 CSR */
	u8	res0[28];
	u32	pltoccsr;	/* Port Link Time-out CCSR */
	u32	prtoccsr;	/* Port Response Time-out CCSR */
	u8	res1[20];
	u32	pgccsr;	/* Port General CSR */
	struct rio_lp_serial_port	port[CONFIG_SYS_FSL_SRIO_MAX_PORTS];
};

/* Logical error reporting registers */
struct rio_logical_err {
	u32	erbh;	/* Error Reporting Block Header Register */
	u8	res0[4];
	u32	ltledcsr;	/* Logical/Transport layer error DCSR */
	u32	ltleecsr;	/* Logical/Transport layer error ECSR */
	u8	res1[4];
	u32	ltlaccsr;	/* Logical/Transport layer ACCSR */
	u32	ltldidccsr;	/* Logical/Transport layer DID CCSR */
	u32	ltlcccsr;	/* Logical/Transport layer control CCSR */
};

/* Physical error reporting port registers */
struct rio_phys_err_port {
	u32	edcsr;	/* Port error detect CSR */
	u32	erecsr;	/* Port error rate enable CSR */
	u32	ecacsr;	/* Port error capture attributes CSR */
	u32	pcseccsr0;	/* Port packet/control symbol ECCSR 0 */
	u32	peccsr[3];	/* Port error capture CSR */
	u8	res0[12];
	u32	ercsr;	/* Port error rate CSR */
	u32	ertcsr;	/* Port error rate threshold CSR */
	u8	res1[16];
};

/* Physical error reporting registers */
struct rio_phys_err {
	struct rio_phys_err_port	port[CONFIG_SYS_FSL_SRIO_MAX_PORTS];
};

/* Implementation Space: General Port-Common */
struct rio_impl_common {
	u8	res0[4];
	u32	llcr;	/* Logical Layer Configuration Register */
	u8	res1[8];
	u32	epwisr;	/* Error / Port-Write Interrupt SR */
	u8	res2[12];
	u32	lretcr;	/* Logical Retry Error Threshold CR */
	u8	res3[92];
	u32	pretcr;	/* Physical Retry Erorr Threshold CR */
	u8	res4[124];
};

/* Implementation Space: Port Specific */
struct rio_impl_port_spec {
	u32	adidcsr;	/* Port Alt. Device ID CSR */
	u8	res0[28];
	u32	ptaacr;	/* Port Pass-Through/Accept-All CR */
	u32	lopttlcr;
	u8	res1[8];
	u32	iecsr;	/* Port Implementation Error CSR */
	u8	res2[12];
	u32	pcr;		/* Port Phsyical Configuration Register */
	u8	res3[20];
	u32	slcsr;	/* Port Serial Link CSR */
	u8	res4[4];
	u32	sleicr;	/* Port Serial Link Error Injection */
	u32	a0txcr;	/* Port Arbitration 0 Tx CR */
	u32	a1txcr;	/* Port Arbitration 1 Tx CR */
	u32	a2txcr;	/* Port Arbitration 2 Tx CR */
	u32	mreqtxbacr[3];	/* Port Request Tx Buffer ACR */
	u32	mrspfctxbacr;	/* Port Response/Flow Control Tx Buffer ACR */
};

/* Implementation Space: register */
struct rio_implement {
	struct rio_impl_common	com;
	struct rio_impl_port_spec	port[CONFIG_SYS_FSL_SRIO_MAX_PORTS];
};

/* Revision Control Register */
struct rio_rev_ctrl {
	u32	ipbrr[2];	/* IP Block Revision Register */
};

struct rio_atmu_row {
	u32	rowtar; /* RapidIO Outbound Window TAR */
	u32	rowtear; /* RapidIO Outbound Window TEAR */
	u32	rowbar;
	u8	res0[4];
	u32	rowar; /* RapidIO Outbound Attributes Register */
	u32	rowsr[3]; /* Port RapidIO outbound window segment register */
};

struct rio_atmu_riw {
	u32	riwtar; /* RapidIO Inbound Window Translation AR */
	u8	res0[4];
	u32	riwbar; /* RapidIO Inbound Window Base AR */
	u8	res1[4];
	u32	riwar; /* RapidIO Inbound Attributes Register */
	u8	res2[12];
};

/* ATMU window registers */
struct rio_atmu_win {
	struct rio_atmu_row	outbw[CONFIG_SYS_FSL_SRIO_OB_WIN_NUM];
	u8	res0[64];
	struct rio_atmu_riw	inbw[CONFIG_SYS_FSL_SRIO_IB_WIN_NUM];
};

struct rio_atmu {
	struct rio_atmu_win	port[CONFIG_SYS_FSL_SRIO_MAX_PORTS];
};

#ifdef CONFIG_SYS_FSL_RMU
struct rio_msg {
	u32	omr; /* Outbound Mode Register */
	u32	osr; /* Outbound Status Register */
	u32	eodqdpar; /* Extended Outbound DQ DPAR */
	u32	odqdpar; /* Outbound Descriptor Queue DPAR */
	u32	eosar; /* Extended Outbound Unit Source AR */
	u32	osar; /* Outbound Unit Source AR */
	u32	odpr; /* Outbound Destination Port Register */
	u32	odatr; /* Outbound Destination Attributes Register */
	u32	odcr; /* Outbound Doubleword Count Register */
	u32	eodqepar; /* Extended Outbound DQ EPAR */
	u32	odqepar; /* Outbound Descriptor Queue EPAR */
	u32	oretr; /* Outbound Retry Error Threshold Register */
	u32	omgr; /* Outbound Multicast Group Register */
	u32	omlr; /* Outbound Multicast List Register */
	u8	res0[40];
	u32	imr;	 /* Outbound Mode Register */
	u32	isr; /* Inbound Status Register */
	u32	eidqdpar; /* Extended Inbound Descriptor Queue DPAR */
	u32	idqdpar; /* Inbound Descriptor Queue DPAR */
	u32	eifqepar; /* Extended Inbound Frame Queue EPAR */
	u32	ifqepar; /* Inbound Frame Queue EPAR */
	u32	imirir; /* Inbound Maximum Interrutp RIR */
	u8	res1[4];
	u32 eihqepar; /* Extended inbound message header queue EPAR */
	u32 ihqepar; /* Inbound message header queue EPAR */
	u8	res2[120];
};

struct rio_dbell {
	u32	odmr; /* Outbound Doorbell Mode Register */
	u32	odsr; /* Outbound Doorbell Status Register */
	u8	res0[16];
	u32	oddpr; /* Outbound Doorbell Destination Port */
	u32	oddatr; /* Outbound Doorbell Destination AR */
	u8	res1[12];
	u32	oddretr; /* Outbound Doorbell Retry Threshold CR */
	u8	res2[48];
	u32	idmr; /* Inbound Doorbell Mode Register */
	u32	idsr;	 /* Inbound Doorbell Status Register */
	u32	iedqdpar; /* Extended Inbound Doorbell Queue DPAR */
	u32	iqdpar; /* Inbound Doorbell Queue DPAR */
	u32	iedqepar; /* Extended Inbound Doorbell Queue EPAR */
	u32	idqepar; /* Inbound Doorbell Queue EPAR */
	u32	idmirir; /* Inbound Doorbell Max Interrupt RIR */
};

struct rio_pw {
	u32	pwmr; /* Port-Write Mode Register */
	u32	pwsr; /* Port-Write Status Register */
	u32	epwqbar; /* Extended Port-Write Queue BAR */
	u32	pwqbar; /* Port-Write Queue Base Address Register */
};
#endif

#ifdef CONFIG_SYS_FSL_SRIO_LIODN
struct rio_liodn {
	u32	plbr;
	u8	res0[28];
	u32	plaor;
	u8	res1[12];
	u32	pludr;
	u32	plldr;
	u8	res2[456];
};
#endif

/* RapidIO Registers */
struct ccsr_rio {
	struct rio_arch	arch;
	u8	res0[144];
	struct rio_lp_serial	lp_serial;
	u8	res1[1152];
	struct rio_logical_err	logical_err;
	u8	res2[32];
	struct rio_phys_err	phys_err;
	u8	res3[63808];
	struct rio_implement	impl;
	u8	res4[2552];
	struct rio_rev_ctrl	rev;
	struct rio_atmu	atmu;
#ifdef CONFIG_SYS_FSL_RMU
	u8	res5[8192];
	struct rio_msg	msg[CONFIG_SYS_FSL_SRIO_MSG_UNIT_NUM];
	u8	res6[512];
	struct rio_dbell	dbell;
	u8	res7[100];
	struct rio_pw	pw;
#endif
#ifdef CONFIG_SYS_FSL_SRIO_LIODN
	u8	res5[8192];
	struct rio_liodn liodn[CONFIG_SYS_FSL_SRIO_MAX_PORTS];
#endif
};
#endif

/* Quick Engine Block Pin Muxing Registers */
typedef struct par_io {
	u32	cpodr;
	u32	cpdat;
	u32	cpdir1;
	u32	cpdir2;
	u32	cppar1;
	u32	cppar2;
	u8	res[8];
} par_io_t;

#ifdef CONFIG_SYS_FSL_CPC
/*
 * Define a single offset that is the start of all the CPC register
 * blocks - if there is more than one CPC, we expect these to be
 * contiguous 4k regions
 */

typedef struct cpc_corenet {
	u32 	cpccsr0;	/* Config/status reg */
	u32	res1;
	u32	cpccfg0;	/* Configuration register */
	u32	res2;
	u32	cpcewcr0;	/* External Write reg 0 */
	u32	cpcewabr0;	/* External write base reg 0 */
	u32	res3[2];
	u32	cpcewcr1;	/* External Write reg 1 */
	u32	cpcewabr1;	/* External write base reg 1 */
	u32	res4[54];
	u32	cpcsrcr1;	/* SRAM control reg 1 */
	u32	cpcsrcr0;	/* SRAM control reg 0 */
	u32	res5[62];
	struct {
		u32	id;	/* partition ID */
		u32	res;
		u32	alloc;	/* partition allocation */
		u32	way;	/* partition way */
	} partition_regs[16];
	u32	res6[704];
	u32	cpcerrinjhi;	/* Error injection high */
	u32	cpcerrinjlo;	/* Error injection lo */
	u32	cpcerrinjctl;	/* Error injection control */
	u32	res7[5];
	u32	cpccaptdatahi;	/* capture data high */
	u32	cpccaptdatalo;	/* capture data low */
	u32	cpcaptecc;	/* capture ECC */
	u32	res8[5];
	u32	cpcerrdet;	/* error detect */
	u32	cpcerrdis;	/* error disable */
	u32	cpcerrinten;	/* errir interrupt enable */
	u32	cpcerrattr;	/* error attribute */
	u32	cpcerreaddr;	/* error extended address */
	u32	cpcerraddr;	/* error address */
	u32	cpcerrctl;	/* error control */
	u32	res9[41];	/* pad out to 4k */
	u32	cpchdbcr0;	/* hardware debug control register 0 */
	u32	res10[63];	/* pad out to 4k */
} cpc_corenet_t;

#define CPC_CSR0_CE	0x80000000	/* Cache Enable */
#define CPC_CSR0_PE	0x40000000	/* Enable ECC */
#define CPC_CSR0_FI	0x00200000	/* Cache Flash Invalidate */
#define CPC_CSR0_WT	0x00080000	/* Write-through mode */
#define CPC_CSR0_FL	0x00000800	/* Hardware cache flush */
#define CPC_CSR0_LFC	0x00000400	/* Cache Lock Flash Clear */
#define CPC_CFG0_SZ_MASK	0x00003fff
#define CPC_CFG0_SZ_K(x)	((x & CPC_CFG0_SZ_MASK) << 6)
#define CPC_CFG0_NUM_WAYS(x)	(((x >> 14) & 0x1f) + 1)
#define CPC_CFG0_LINE_SZ(x)	((((x >> 23) & 0x3) + 1) * 32)
#define CPC_SRCR1_SRBARU_MASK	0x0000ffff
#define CPC_SRCR1_SRBARU(x)	(((unsigned long long)x >> 32) \
				 & CPC_SRCR1_SRBARU_MASK)
#define	CPC_SRCR0_SRBARL_MASK	0xffff8000
#define CPC_SRCR0_SRBARL(x)	(x & CPC_SRCR0_SRBARL_MASK)
#define CPC_SRCR0_INTLVEN	0x00000100
#define CPC_SRCR0_SRAMSZ_1_WAY	0x00000000
#define CPC_SRCR0_SRAMSZ_2_WAY	0x00000002
#define CPC_SRCR0_SRAMSZ_4_WAY	0x00000004
#define CPC_SRCR0_SRAMSZ_8_WAY	0x00000006
#define CPC_SRCR0_SRAMSZ_16_WAY	0x00000008
#define CPC_SRCR0_SRAMSZ_32_WAY	0x0000000a
#define CPC_SRCR0_SRAMEN	0x00000001
#define	CPC_ERRDIS_TMHITDIS  	0x00000080	/* multi-way hit disable */
#define CPC_HDBCR0_CDQ_SPEC_DIS	0x08000000
#define CPC_HDBCR0_TAG_ECC_SCRUB_DIS	0x01000000
#define CPC_HDBCR0_DATA_ECC_SCRUB_DIS	0x00400000
#define CPC_HDBCR0_SPLRU_LEVEL_EN	0x001e0000
#endif /* CONFIG_SYS_FSL_CPC */

/* Global Utilities Block */
#ifdef CONFIG_FSL_CORENET
typedef struct ccsr_gur {
	u32	porsr1;		/* POR status 1 */
	u32	porsr2;		/* POR status 2 */
#ifdef	CONFIG_SYS_FSL_SINGLE_SOURCE_CLK
#define	FSL_DCFG_PORSR1_SYSCLK_SHIFT	15
#define	FSL_DCFG_PORSR1_SYSCLK_MASK	0x1
#define	FSL_DCFG_PORSR1_SYSCLK_SINGLE_ENDED	0x1
#define	FSL_DCFG_PORSR1_SYSCLK_DIFF	0x0
#endif
	u8	res_008[0x20-0x8];
	u32	gpporcr1;	/* General-purpose POR configuration */
	u32	gpporcr2;	/* General-purpose POR configuration 2 */
	u32	dcfg_fusesr;	/* Fuse status register */
#define FSL_CORENET_DCFG_FUSESR_VID_SHIFT	25
#define FSL_CORENET_DCFG_FUSESR_VID_MASK	0x1F
#define FSL_CORENET_DCFG_FUSESR_ALTVID_SHIFT	20
#define FSL_CORENET_DCFG_FUSESR_ALTVID_MASK	0x1F
	u8	res_02c[0x70-0x2c];
	u32	devdisr;	/* Device disable control */
	u32	devdisr2;	/* Device disable control 2 */
	u32	devdisr3;	/* Device disable control 3 */
	u32	devdisr4;	/* Device disable control 4 */
#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
	u32	devdisr5;	/* Device disable control 5 */
#define FSL_CORENET_DEVDISR_PBL	0x80000000
#define FSL_CORENET_DEVDISR_PMAN	0x40000000
#define FSL_CORENET_DEVDISR_ESDHC	0x20000000
#define FSL_CORENET_DEVDISR_DMA1	0x00800000
#define FSL_CORENET_DEVDISR_DMA2	0x00400000
#define FSL_CORENET_DEVDISR_USB1	0x00080000
#define FSL_CORENET_DEVDISR_USB2	0x00040000
#define FSL_CORENET_DEVDISR_SATA1	0x00008000
#define FSL_CORENET_DEVDISR_SATA2	0x00004000
#define FSL_CORENET_DEVDISR_PME	0x00000800
#define FSL_CORENET_DEVDISR_SEC	0x00000200
#define FSL_CORENET_DEVDISR_RMU	0x00000080
#define FSL_CORENET_DEVDISR_DCE	0x00000040
#define FSL_CORENET_DEVDISR2_DTSEC1_1	0x80000000
#define FSL_CORENET_DEVDISR2_DTSEC1_2	0x40000000
#define FSL_CORENET_DEVDISR2_DTSEC1_3	0x20000000
#define FSL_CORENET_DEVDISR2_DTSEC1_4	0x10000000
#define FSL_CORENET_DEVDISR2_DTSEC1_5	0x08000000
#define FSL_CORENET_DEVDISR2_DTSEC1_6	0x04000000
#define FSL_CORENET_DEVDISR2_DTSEC1_9	0x00800000
#define FSL_CORENET_DEVDISR2_DTSEC1_10	0x00400000
#ifdef CONFIG_FSL_FM_10GEC_REGULAR_NOTATION
#define FSL_CORENET_DEVDISR2_10GEC1_1   0x80000000
#define FSL_CORENET_DEVDISR2_10GEC1_2   0x40000000
#else
#define FSL_CORENET_DEVDISR2_10GEC1_1	0x00800000
#define FSL_CORENET_DEVDISR2_10GEC1_2	0x00400000
#define FSL_CORENET_DEVDISR2_10GEC1_3	0x80000000
#define FSL_CORENET_DEVDISR2_10GEC1_4	0x40000000
#endif
#define FSL_CORENET_DEVDISR2_DTSEC2_1	0x00080000
#define FSL_CORENET_DEVDISR2_DTSEC2_2	0x00040000
#define FSL_CORENET_DEVDISR2_DTSEC2_3	0x00020000
#define FSL_CORENET_DEVDISR2_DTSEC2_4	0x00010000
#define FSL_CORENET_DEVDISR2_DTSEC2_5	0x00008000
#define FSL_CORENET_DEVDISR2_DTSEC2_6	0x00004000
#define FSL_CORENET_DEVDISR2_DTSEC2_9	0x00000800
#define FSL_CORENET_DEVDISR2_DTSEC2_10	0x00000400
#define FSL_CORENET_DEVDISR2_10GEC2_1	0x00000800
#define FSL_CORENET_DEVDISR2_10GEC2_2	0x00000400
#define FSL_CORENET_DEVDISR2_FM1	0x00000080
#define FSL_CORENET_DEVDISR2_FM2	0x00000040
#define FSL_CORENET_DEVDISR2_CPRI	0x00000008
#define FSL_CORENET_DEVDISR3_PCIE1	0x80000000
#define FSL_CORENET_DEVDISR3_PCIE2	0x40000000
#define FSL_CORENET_DEVDISR3_PCIE3	0x20000000
#define FSL_CORENET_DEVDISR3_PCIE4	0x10000000
#define FSL_CORENET_DEVDISR3_SRIO1	0x08000000
#define FSL_CORENET_DEVDISR3_SRIO2	0x04000000
#define FSL_CORENET_DEVDISR3_QMAN	0x00080000
#define FSL_CORENET_DEVDISR3_BMAN	0x00040000
#define FSL_CORENET_DEVDISR3_LA1	0x00008000
#define FSL_CORENET_DEVDISR3_MAPLE1	0x00000800
#define FSL_CORENET_DEVDISR3_MAPLE2	0x00000400
#define FSL_CORENET_DEVDISR3_MAPLE3	0x00000200
#define FSL_CORENET_DEVDISR4_I2C1	0x80000000
#define FSL_CORENET_DEVDISR4_I2C2	0x40000000
#define FSL_CORENET_DEVDISR4_DUART1	0x20000000
#define FSL_CORENET_DEVDISR4_DUART2	0x10000000
#define FSL_CORENET_DEVDISR4_ESPI	0x08000000
#define FSL_CORENET_DEVDISR5_DDR1	0x80000000
#define FSL_CORENET_DEVDISR5_DDR2	0x40000000
#define FSL_CORENET_DEVDISR5_DDR3	0x20000000
#define FSL_CORENET_DEVDISR5_CPC1	0x08000000
#define FSL_CORENET_DEVDISR5_CPC2	0x04000000
#define FSL_CORENET_DEVDISR5_CPC3	0x02000000
#define FSL_CORENET_DEVDISR5_IFC	0x00800000
#define FSL_CORENET_DEVDISR5_GPIO	0x00400000
#define FSL_CORENET_DEVDISR5_DBG	0x00200000
#define FSL_CORENET_DEVDISR5_NAL	0x00100000
#define FSL_CORENET_DEVDISR5_TIMERS	0x00020000
#define FSL_CORENET_NUM_DEVDISR		5
#else
#define FSL_CORENET_DEVDISR_PCIE1	0x80000000
#define FSL_CORENET_DEVDISR_PCIE2	0x40000000
#define FSL_CORENET_DEVDISR_PCIE3	0x20000000
#define FSL_CORENET_DEVDISR_PCIE4	0x10000000
#define FSL_CORENET_DEVDISR_RMU		0x08000000
#define FSL_CORENET_DEVDISR_SRIO1	0x04000000
#define FSL_CORENET_DEVDISR_SRIO2	0x02000000
#define FSL_CORENET_DEVDISR_DMA1	0x00400000
#define FSL_CORENET_DEVDISR_DMA2	0x00200000
#define FSL_CORENET_DEVDISR_DDR1	0x00100000
#define FSL_CORENET_DEVDISR_DDR2	0x00080000
#define FSL_CORENET_DEVDISR_DBG		0x00010000
#define FSL_CORENET_DEVDISR_NAL		0x00008000
#define FSL_CORENET_DEVDISR_SATA1	0x00004000
#define FSL_CORENET_DEVDISR_SATA2	0x00002000
#define FSL_CORENET_DEVDISR_ELBC	0x00001000
#define FSL_CORENET_DEVDISR_USB1	0x00000800
#define FSL_CORENET_DEVDISR_USB2	0x00000400
#define FSL_CORENET_DEVDISR_ESDHC	0x00000100
#define FSL_CORENET_DEVDISR_GPIO	0x00000080
#define FSL_CORENET_DEVDISR_ESPI	0x00000040
#define FSL_CORENET_DEVDISR_I2C1	0x00000020
#define FSL_CORENET_DEVDISR_I2C2	0x00000010
#define FSL_CORENET_DEVDISR_DUART1	0x00000002
#define FSL_CORENET_DEVDISR_DUART2	0x00000001
#define FSL_CORENET_DEVDISR2_PME	0x80000000
#define FSL_CORENET_DEVDISR2_SEC	0x40000000
#define FSL_CORENET_DEVDISR2_QMBM	0x08000000
#define FSL_CORENET_DEVDISR2_FM1	0x02000000
#define FSL_CORENET_DEVDISR2_10GEC1	0x01000000
#define FSL_CORENET_DEVDISR2_DTSEC1_1	0x00800000
#define FSL_CORENET_DEVDISR2_DTSEC1_2	0x00400000
#define FSL_CORENET_DEVDISR2_DTSEC1_3	0x00200000
#define FSL_CORENET_DEVDISR2_DTSEC1_4	0x00100000
#define FSL_CORENET_DEVDISR2_DTSEC1_5	0x00080000
#define FSL_CORENET_DEVDISR2_FM2	0x00020000
#define FSL_CORENET_DEVDISR2_10GEC2	0x00010000
#define FSL_CORENET_DEVDISR2_DTSEC2_1	0x00008000
#define FSL_CORENET_DEVDISR2_DTSEC2_2	0x00004000
#define FSL_CORENET_DEVDISR2_DTSEC2_3	0x00002000
#define FSL_CORENET_DEVDISR2_DTSEC2_4	0x00001000
#define FSL_CORENET_DEVDISR2_DTSEC2_5	0x00000800
#define FSL_CORENET_NUM_DEVDISR		2
	u32	powmgtcsr;	/* Power management status & control */
#endif
	u8	res8[12];
	u32	coredisru;	/* uppper portion for support of 64 cores */
	u32	coredisrl;	/* lower portion for support of 64 cores */
	u8	res9[8];
	u32	pvr;		/* Processor version */
	u32	svr;		/* System version */
	u8	res10[8];
	u32	rstcr;		/* Reset control */
	u32	rstrqpblsr;	/* Reset request preboot loader status */
	u8	res11[8];
	u32	rstrqmr1;	/* Reset request mask */
#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
#define FSL_CORENET_RSTRQMR1_SRDS_RST_MSK      0x00000800
#endif
	u8	res12[4];
	u32	rstrqsr1;	/* Reset request status */
	u8	res13[4];
	u8	res14[4];
	u32	rstrqwdtmrl;	/* Reset request WDT mask */
	u8	res15[4];
	u32	rstrqwdtsrl;	/* Reset request WDT status */
	u8	res16[4];
	u32	brrl;		/* Boot release */
	u8	res17[24];
	u32	rcwsr[16];	/* Reset control word status */
#define RCW_SB_EN_REG_INDEX	7
#define RCW_SB_EN_MASK		0x00200000

#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
#define FSL_CORENET_RCWSR0_MEM_PLL_RAT_SHIFT	16
/* use reserved bits 18~23 as scratch space to host DDR PLL ratio */
#define FSL_CORENET_RCWSR0_MEM_PLL_RAT_RESV_SHIFT	8
#define FSL_CORENET_RCWSR0_MEM_PLL_RAT_MASK	0x3f
#if defined(CONFIG_ARCH_T4240) || defined(CONFIG_ARCH_T4160)
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL		0xfc000000
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT	26
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL		0x00fe0000
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT	17
#define FSL_CORENET2_RCWSR4_SRDS3_PRTCL		0x0000f800
#define FSL_CORENET2_RCWSR4_SRDS3_PRTCL_SHIFT	11
#define FSL_CORENET2_RCWSR4_SRDS4_PRTCL		0x000000f8
#define FSL_CORENET2_RCWSR4_SRDS4_PRTCL_SHIFT	3
#define FSL_CORENET_RCWSR6_BOOT_LOC	0x0f800000
#elif defined(CONFIG_ARCH_B4860) || defined(CONFIG_ARCH_B4420)
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL	0xfe000000
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT	25
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL	0x00ff0000
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT	16
#define FSL_CORENET_RCWSR6_BOOT_LOC	0x0f800000
#elif defined(CONFIG_ARCH_T1040) || defined(CONFIG_ARCH_T1042)
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL	0xff000000
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT	24
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL	0x00fe0000
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT	17
#define FSL_CORENET_RCWSR13_EC1	0x30000000 /* bits 418..419 */
#define FSL_CORENET_RCWSR13_EC1_FM1_DTSEC4_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC1_FM1_GPIO	0x10000000
#define FSL_CORENET_RCWSR13_EC1_FM1_DTSEC4_MII	0x20000000
#define FSL_CORENET_RCWSR13_EC2	0x0c000000 /* bits 420..421 */
#define FSL_CORENET_RCWSR13_EC2_FM1_DTSEC5_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC2_FM1_GPIO	0x04000000
#define FSL_CORENET_RCWSR13_MAC2_GMII_SEL	0x00000080
#define FSL_CORENET_RCWSR13_MAC2_GMII_SEL_L2_SWITCH	0x00000000
#define FSL_CORENET_RCWSR13_MAC2_GMII_SEL_ENET_PORT	0x00000080
#define CONFIG_SYS_FSL_SCFG_PIXCLKCR_OFFSET	0x28
#define PXCKEN_MASK	0x80000000
#define PXCK_MASK	0x00FF0000
#define PXCK_BITS_START	16
#elif defined(CONFIG_ARCH_T1024) || defined(CONFIG_ARCH_T1023)
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL		0xff800000
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT	23
#define FSL_CORENET_RCWSR6_BOOT_LOC		0x0f800000
#define FSL_CORENET_RCWSR13_EC1			0x30000000 /* bits 418..419 */
#define FSL_CORENET_RCWSR13_EC1_RGMII		0x00000000
#define FSL_CORENET_RCWSR13_EC1_GPIO		0x10000000
#define FSL_CORENET_RCWSR13_EC2			0x0c000000
#define FSL_CORENET_RCWSR13_EC2_RGMII		0x08000000
#define CONFIG_SYS_FSL_SCFG_PIXCLKCR_OFFSET	0x28
#define CONFIG_SYS_FSL_SCFG_IODSECR1_OFFSET	0xd00
#define PXCKEN_MASK				0x80000000
#define PXCK_MASK				0x00FF0000
#define PXCK_BITS_START				16
#elif defined(CONFIG_ARCH_T2080) || defined(CONFIG_ARCH_T2081)
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL		0xff000000
#define FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT	24
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL		0x00ff0000
#define FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT	16
#define FSL_CORENET_RCWSR6_BOOT_LOC		0x0f800000
#endif
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S1_PLL1	0x00800000
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S1_PLL2	0x00400000
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S2_PLL1	0x00200000
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S2_PLL2	0x00100000
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S3_PLL1	0x00080000
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S3_PLL2	0x00040000
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S4_PLL1	0x00020000
#define FSL_CORENET2_RCWSR5_SRDS_PLL_PD_S4_PLL2	0x00010000
#define FSL_CORENET2_RCWSR5_DDR_REFCLK_SEL_SHIFT 4
#define FSL_CORENET2_RCWSR5_DDR_REFCLK_SEL_MASK	0x00000011
#define FSL_CORENET2_RCWSR5_DDR_REFCLK_SINGLE_CLK	1

#else /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */
#define FSL_CORENET_RCWSR0_MEM_PLL_RAT_SHIFT	17
#define FSL_CORENET_RCWSR0_MEM_PLL_RAT_MASK	0x1f
#define FSL_CORENET_RCWSR4_SRDS_PRTCL		0xfc000000
#define FSL_CORENET_RCWSR5_DDR_SYNC		0x00000080
#define FSL_CORENET_RCWSR5_DDR_SYNC_SHIFT		 7
#define FSL_CORENET_RCWSR5_SRDS_EN		0x00002000
#define FSL_CORENET_RCWSR5_SRDS2_EN		0x00001000
#define FSL_CORENET_RCWSR6_BOOT_LOC	0x0f800000
#define FSL_CORENET_RCWSRn_SRDS_LPD_B2		0x3c000000 /* bits 162..165 */
#define FSL_CORENET_RCWSRn_SRDS_LPD_B3		0x003c0000 /* bits 170..173 */
#endif /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

#define FSL_CORENET_RCWSR7_MCK_TO_PLAT_RAT	0x00400000
#define FSL_CORENET_RCWSR8_HOST_AGT_B1		0x00e00000
#define FSL_CORENET_RCWSR8_HOST_AGT_B2		0x00100000
#define FSL_CORENET_RCWSR11_EC1			0x00c00000 /* bits 360..361 */
#ifdef CONFIG_ARCH_P4080
#define FSL_CORENET_RCWSR11_EC1_FM1_DTSEC1		0x00000000
#define FSL_CORENET_RCWSR11_EC1_FM1_USB1		0x00800000
#define FSL_CORENET_RCWSR11_EC2			0x001c0000 /* bits 363..365 */
#define FSL_CORENET_RCWSR11_EC2_FM2_DTSEC1		0x00000000
#define FSL_CORENET_RCWSR11_EC2_FM1_DTSEC2		0x00080000
#define FSL_CORENET_RCWSR11_EC2_USB2			0x00100000
#endif
#if defined(CONFIG_ARCH_P2041) || \
	defined(CONFIG_ARCH_P3041) || defined(CONFIG_ARCH_P5020)
#define FSL_CORENET_RCWSR11_EC1_FM1_DTSEC4_RGMII	0x00000000
#define FSL_CORENET_RCWSR11_EC1_FM1_DTSEC4_MII		0x00800000
#define FSL_CORENET_RCWSR11_EC1_FM1_DTSEC4_NONE		0x00c00000
#define FSL_CORENET_RCWSR11_EC2			0x00180000 /* bits 363..364 */
#define FSL_CORENET_RCWSR11_EC2_FM1_DTSEC5_RGMII	0x00000000
#define FSL_CORENET_RCWSR11_EC2_FM1_DTSEC5_MII		0x00100000
#define FSL_CORENET_RCWSR11_EC2_FM1_DTSEC5_NONE		0x00180000
#endif
#if defined(CONFIG_ARCH_P5040)
#define FSL_CORENET_RCWSR11_EC1_FM1_DTSEC5_RGMII        0x00000000
#define FSL_CORENET_RCWSR11_EC1_FM1_DTSEC5_MII          0x00800000
#define FSL_CORENET_RCWSR11_EC1_FM1_DTSEC5_NONE         0x00c00000
#define FSL_CORENET_RCWSR11_EC2                 0x00180000 /* bits 363..364 */
#define FSL_CORENET_RCWSR11_EC2_FM2_DTSEC5_RGMII        0x00000000
#define FSL_CORENET_RCWSR11_EC2_FM2_DTSEC5_MII          0x00100000
#define FSL_CORENET_RCWSR11_EC2_FM2_DTSEC5_NONE         0x00180000
#endif
#if defined(CONFIG_ARCH_T4240) || defined(CONFIG_ARCH_T4160)
#define FSL_CORENET_RCWSR13_EC1			0x60000000 /* bits 417..418 */
#define FSL_CORENET_RCWSR13_EC1_FM2_DTSEC5_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC1_FM2_GPIO		0x40000000
#define FSL_CORENET_RCWSR13_EC2			0x18000000 /* bits 419..420 */
#define FSL_CORENET_RCWSR13_EC2_FM1_DTSEC5_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC2_FM1_DTSEC6_RGMII	0x08000000
#define FSL_CORENET_RCWSR13_EC2_FM1_GPIO		0x10000000
#endif
#if defined(CONFIG_ARCH_T2080) || defined(CONFIG_ARCH_T2081)
#define FSL_CORENET_RCWSR13_EC1			0x60000000 /* bits 417..418 */
#define FSL_CORENET_RCWSR13_EC1_DTSEC3_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC1_GPIO		0x40000000
#define FSL_CORENET_RCWSR13_EC2			0x18000000 /* bits 419..420 */
#define FSL_CORENET_RCWSR13_EC2_DTSEC4_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC2_DTSEC10_RGMII	0x08000000
#define FSL_CORENET_RCWSR13_EC2_GPIO		0x10000000
#endif
	u8	res18[192];
	u32	scratchrw[4];	/* Scratch Read/Write */
	u8	res19[240];
	u32	scratchw1r[4];	/* Scratch Read (Write once) */
	u8	res20[240];
	u32	scrtsr[8];	/* Core reset status */
	u8	res21[224];
	u32	pex1liodnr;	/* PCI Express 1 LIODN */
	u32	pex2liodnr;	/* PCI Express 2 LIODN */
	u32	pex3liodnr;	/* PCI Express 3 LIODN */
	u32	pex4liodnr;	/* PCI Express 4 LIODN */
	u32	rio1liodnr;	/* RIO 1 LIODN */
	u32	rio2liodnr;	/* RIO 2 LIODN */
	u32	rio3liodnr;	/* RIO 3 LIODN */
	u32	rio4liodnr;	/* RIO 4 LIODN */
	u32	usb1liodnr;	/* USB 1 LIODN */
	u32	usb2liodnr;	/* USB 2 LIODN */
	u32	usb3liodnr;	/* USB 3 LIODN */
	u32	usb4liodnr;	/* USB 4 LIODN */
	u32	sdmmc1liodnr;	/* SD/MMC 1 LIODN */
	u32	sdmmc2liodnr;	/* SD/MMC 2 LIODN */
	u32	sdmmc3liodnr;	/* SD/MMC 3 LIODN */
	u32	sdmmc4liodnr;	/* SD/MMC 4 LIODN */
	u32	rio1maintliodnr;/* RIO 1 Maintenance LIODN */
	u32	rio2maintliodnr;/* RIO 2 Maintenance LIODN */
	u32	rio3maintliodnr;/* RIO 3 Maintenance LIODN */
	u32	rio4maintliodnr;/* RIO 4 Maintenance LIODN */
	u32	sata1liodnr;	/* SATA 1 LIODN */
	u32	sata2liodnr;	/* SATA 2 LIODN */
	u32	sata3liodnr;	/* SATA 3 LIODN */
	u32	sata4liodnr;	/* SATA 4 LIODN */
	u8	res22[20];
	u32	tdmliodnr;	/* TDM LIODN */
	u32     qeliodnr;       /* QE LIODN */
	u8      res_57c[4];
	u32	dma1liodnr;	/* DMA 1 LIODN */
	u32	dma2liodnr;	/* DMA 2 LIODN */
	u32	dma3liodnr;	/* DMA 3 LIODN */
	u32	dma4liodnr;	/* DMA 4 LIODN */
	u8	res23[48];
	u8	res24[64];
	u32	pblsr;		/* Preboot loader status */
	u32	pamubypenr;	/* PAMU bypass enable */
	u32	dmacr1;		/* DMA control */
	u8	res25[4];
	u32	gensr1;		/* General status */
	u8	res26[12];
	u32	gencr1;		/* General control */
	u8	res27[12];
	u8	res28[4];
	u32	cgensrl;	/* Core general status */
	u8	res29[8];
	u8	res30[4];
	u32	cgencrl;	/* Core general control */
	u8	res31[184];
	u32	sriopstecr;	/* SRIO prescaler timer enable control */
	u32	dcsrcr;		/* DCSR Control register */
	u8	res31a[56];
	u32	tp_ityp[64];	/* Topology Initiator Type Register */
	struct {
		u32	upper;
		u32	lower;
	} tp_cluster[16];	/* Core Cluster n Topology Register */
	u8	res32[1344];
	u32	pmuxcr;		/* Pin multiplexing control */
	u8	res33[60];
	u32	iovselsr;	/* I/O voltage selection status */
	u8	res34[28];
	u32	ddrclkdr;	/* DDR clock disable */
	u8	res35;
	u32	elbcclkdr;	/* eLBC clock disable */
	u8	res36[20];
	u32	sdhcpcr;	/* eSDHC polarity configuration */
	u8	res37[380];
} ccsr_gur_t;

#define TP_ITYP_AV	0x00000001		/* Initiator available */
#define TP_ITYP_TYPE(x)	(((x) & 0x6) >> 1)	/* Initiator Type */
#define TP_ITYP_TYPE_OTHER	0x0
#define TP_ITYP_TYPE_PPC	0x1	/* PowerPC */
#define TP_ITYP_TYPE_SC		0x2	/* StarCore DSP */
#define TP_ITYP_TYPE_HA		0x3	/* HW Accelerator */
#define TP_ITYP_THDS(x)	(((x) & 0x18) >> 3)	/* # threads */
#define TP_ITYP_VER(x)	(((x) & 0xe0) >> 5)	/* Initiator Version */

#define TP_CLUSTER_EOC		0x80000000	/* end of clusters */
#define TP_CLUSTER_INIT_MASK	0x0000003f	/* initiator mask */
#define TP_INIT_PER_CLUSTER	4

#define FSL_CORENET_DCSR_SZ_MASK	0x00000003
#define FSL_CORENET_DCSR_SZ_4M		0x0
#define FSL_CORENET_DCSR_SZ_1G		0x3

/*
 * On p4080 we have an LIODN for msg unit (rmu) but not maintenance
 * everything after has RMan thus msg unit LIODN is used for maintenance
 */
#define rmuliodnr rio1maintliodnr

typedef struct ccsr_clk {
	struct {
		u32 clkcncsr;	/* core cluster n clock control status */
		u8  res_004[0x0c];
		u32 clkcgnhwacsr;/* clock generator n hardware accelerator */
		u8  res_014[0x0c];
	} clkcsr[12];
	u8	res_100[0x680]; /* 0x100 */
	struct {
		u32 pllcngsr;
		u8 res10[0x1c];
	} pllcgsr[12];
	u8	res21[0x280];
	u32	pllpgsr;	/* 0xc00 Platform PLL General Status */
	u8	res16[0x1c];
	u32	plldgsr;	/* 0xc20 DDR PLL General Status */
	u8	res17[0x3dc];
} ccsr_clk_t;

#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
typedef struct ccsr_rcpm {
	u8	res_00[12];
	u32	tph10sr0;	/* Thread PH10 Status Register */
	u8	res_10[12];
	u32	tph10setr0;	/* Thread PH10 Set Control Register */
	u8	res_20[12];
	u32	tph10clrr0;	/* Thread PH10 Clear Control Register */
	u8	res_30[12];
	u32	tph10psr0;	/* Thread PH10 Previous Status Register */
	u8	res_40[12];
	u32	twaitsr0;	/* Thread Wait Status Register */
	u8	res_50[96];
	u32	pcph15sr;	/* Physical Core PH15 Status Register */
	u32	pcph15setr;	/* Physical Core PH15 Set Control Register */
	u32	pcph15clrr;	/* Physical Core PH15 Clear Control Register */
	u32	pcph15psr;	/* Physical Core PH15 Prev Status Register */
	u8	res_c0[16];
	u32	pcph20sr;	/* Physical Core PH20 Status Register */
	u32	pcph20setr;	/* Physical Core PH20 Set Control Register */
	u32	pcph20clrr;	/* Physical Core PH20 Clear Control Register */
	u32	pcph20psr;	/* Physical Core PH20 Prev Status Register */
	u32	pcpw20sr;	/* Physical Core PW20 Status Register */
	u8	res_e0[12];
	u32	pcph30sr;	/* Physical Core PH30 Status Register */
	u32	pcph30setr;	/* Physical Core PH30 Set Control Register */
	u32	pcph30clrr;	/* Physical Core PH30 Clear Control Register */
	u32	pcph30psr;	/* Physical Core PH30 Prev Status Register */
	u8	res_100[32];
	u32	ippwrgatecr;	/* IP Power Gating Control Register */
	u8	res_124[12];
	u32	powmgtcsr;	/* Power Management Control & Status Reg */
	u8	res_134[12];
	u32	ippdexpcr[4];	/* IP Powerdown Exception Control Reg */
	u8	res_150[12];
	u32	tpmimr0;	/* Thread PM Interrupt Mask Reg */
	u8	res_160[12];
	u32	tpmcimr0;	/* Thread PM Crit Interrupt Mask Reg */
	u8	res_170[12];
	u32	tpmmcmr0;	/* Thread PM Machine Check Interrupt Mask Reg */
	u8	res_180[12];
	u32	tpmnmimr0;	/* Thread PM NMI Mask Reg */
	u8	res_190[12];
	u32	tmcpmaskcr0;	/* Thread Machine Check Mask Control Reg */
	u32	pctbenr;	/* Physical Core Time Base Enable Reg */
	u32	pctbclkselr;	/* Physical Core Time Base Clock Select */
	u32	tbclkdivr;	/* Time Base Clock Divider Register */
	u8	res_1ac[4];
	u32	ttbhltcr[4];	/* Thread Time Base Halt Control Register */
	u32	clpcl10sr;	/* Cluster PCL10 Status Register */
	u32	clpcl10setr;	/* Cluster PCL30 Set Control Register */
	u32	clpcl10clrr;	/* Cluster PCL30 Clear Control Register */
	u32	clpcl10psr;	/* Cluster PCL30 Prev Status Register */
	u32	cddslpsetr;	/* Core Domain Deep Sleep Set Register */
	u32	cddslpclrr;	/* Core Domain Deep Sleep Clear Register */
	u32	cdpwroksetr;	/* Core Domain Power OK Set Register */
	u32	cdpwrokclrr;	/* Core Domain Power OK Clear Register */
	u32	cdpwrensr;	/* Core Domain Power Enable Status Register */
	u32	cddslsr;	/* Core Domain Deep Sleep Status Register */
	u8	res_1e8[8];
	u32	dslpcntcr[8];	/* Deep Sleep Counter Cfg Register */
	u8	res_300[3568];
} ccsr_rcpm_t;

#define ctbenrl pctbenr

#else
typedef struct ccsr_rcpm {
	u8	res1[4];
	u32	cdozsrl;	/* Core Doze Status */
	u8	res2[4];
	u32	cdozcrl;	/* Core Doze Control */
	u8	res3[4];
	u32	cnapsrl;	/* Core Nap Status */
	u8	res4[4];
	u32	cnapcrl;	/* Core Nap Control */
	u8	res5[4];
	u32	cdozpsrl;	/* Core Doze Previous Status */
	u8	res6[4];
	u32	cdozpcrl;	/* Core Doze Previous Control */
	u8	res7[4];
	u32	cwaitsrl;	/* Core Wait Status */
	u8	res8[8];
	u32	powmgtcsr;	/* Power Mangement Control & Status */
	u8	res9[12];
	u32	ippdexpcr0;	/* IP Powerdown Exception Control 0 */
	u8	res10[12];
	u8	res11[4];
	u32	cpmimrl;	/* Core PM IRQ Masking */
	u8	res12[4];
	u32	cpmcimrl;	/* Core PM Critical IRQ Masking */
	u8	res13[4];
	u32	cpmmcimrl;	/* Core PM Machine Check IRQ Masking */
	u8	res14[4];
	u32	cpmnmimrl;	/* Core PM NMI Masking */
	u8	res15[4];
	u32	ctbenrl;	/* Core Time Base Enable */
	u8	res16[4];
	u32	ctbclkselrl;	/* Core Time Base Clock Select */
	u8	res17[4];
	u32	ctbhltcrl;	/* Core Time Base Halt Control */
	u8	res18[0xf68];
} ccsr_rcpm_t;
#endif /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

#else
typedef struct ccsr_gur {
	u32	porpllsr;	/* POR PLL ratio status */
#ifdef CONFIG_ARCH_MPC8536
#define MPC85xx_PORPLLSR_DDR_RATIO	0x3e000000
#define MPC85xx_PORPLLSR_DDR_RATIO_SHIFT	25
#elif defined(CONFIG_ARCH_C29X)
#define MPC85xx_PORPLLSR_DDR_RATIO	0x00003f00
#define MPC85xx_PORPLLSR_DDR_RATIO_SHIFT	(9 - ((gur->pordevsr2 \
					& MPC85xx_PORDEVSR2_DDR_SPD_0) \
					>> MPC85xx_PORDEVSR2_DDR_SPD_0_SHIFT))
#else
#if defined(CONFIG_ARCH_BSC9131) || defined(CONFIG_ARCH_BSC9132)
#define MPC85xx_PORPLLSR_DDR_RATIO	0x00003f00
#else
#define MPC85xx_PORPLLSR_DDR_RATIO	0x00003e00
#endif
#define MPC85xx_PORPLLSR_DDR_RATIO_SHIFT	9
#endif
#define MPC85xx_PORPLLSR_QE_RATIO	0x3e000000
#define MPC85xx_PORPLLSR_QE_RATIO_SHIFT		25
#define MPC85xx_PORPLLSR_PLAT_RATIO	0x0000003e
#define MPC85xx_PORPLLSR_PLAT_RATIO_SHIFT	1
	u32	porbmsr;	/* POR boot mode status */
#define MPC85xx_PORBMSR_HA		0x00070000
#define MPC85xx_PORBMSR_HA_SHIFT	16
#define MPC85xx_PORBMSR_ROMLOC_SHIFT	24
#define PORBMSR_ROMLOC_SPI	0x6
#define PORBMSR_ROMLOC_SDHC	0x7
#define PORBMSR_ROMLOC_NAND_2K	0x9
#define PORBMSR_ROMLOC_NOR	0xf
	u32	porimpscr;	/* POR I/O impedance status & control */
	u32	pordevsr;	/* POR I/O device status regsiter */
#if defined(CONFIG_ARCH_P1023)
#define MPC85xx_PORDEVSR_SGMII1_DIS	0x10000000
#define MPC85xx_PORDEVSR_SGMII2_DIS	0x08000000
#define MPC85xx_PORDEVSR_TSEC1_PRTC	0x02000000
#else
#define MPC85xx_PORDEVSR_SGMII1_DIS	0x20000000
#define MPC85xx_PORDEVSR_SGMII2_DIS	0x10000000
#endif
#define MPC85xx_PORDEVSR_SGMII3_DIS	0x08000000
#define MPC85xx_PORDEVSR_SGMII4_DIS	0x04000000
#define MPC85xx_PORDEVSR_SRDS2_IO_SEL	0x38000000
#define MPC85xx_PORDEVSR_PCI1		0x00800000
#if defined(CONFIG_ARCH_P1022)
#define MPC85xx_PORDEVSR_IO_SEL		0x007c0000
#define MPC85xx_PORDEVSR_IO_SEL_SHIFT	18
#elif defined(CONFIG_ARCH_P1023)
#define MPC85xx_PORDEVSR_IO_SEL		0x00600000
#define MPC85xx_PORDEVSR_IO_SEL_SHIFT	21
#else
#if defined(CONFIG_ARCH_P1010)
#define MPC85xx_PORDEVSR_IO_SEL		0x00600000
#define MPC85xx_PORDEVSR_IO_SEL_SHIFT	21
#elif defined(CONFIG_ARCH_BSC9132)
#define MPC85xx_PORDEVSR_IO_SEL		0x00FE0000
#define MPC85xx_PORDEVSR_IO_SEL_SHIFT	17
#elif defined(CONFIG_ARCH_C29X)
#define MPC85xx_PORDEVSR_IO_SEL		0x00e00000
#define MPC85xx_PORDEVSR_IO_SEL_SHIFT	21
#else
#define MPC85xx_PORDEVSR_IO_SEL		0x00780000
#define MPC85xx_PORDEVSR_IO_SEL_SHIFT	19
#endif /* if defined(CONFIG_ARCH_P1010) */
#endif
#define MPC85xx_PORDEVSR_PCI2_ARB	0x00040000
#define MPC85xx_PORDEVSR_PCI1_ARB	0x00020000
#define MPC85xx_PORDEVSR_PCI1_PCI32	0x00010000
#define MPC85xx_PORDEVSR_PCI1_SPD	0x00008000
#define MPC85xx_PORDEVSR_PCI2_SPD	0x00004000
#define MPC85xx_PORDEVSR_DRAM_RTYPE	0x00000060
#define MPC85xx_PORDEVSR_RIO_CTLS	0x00000008
#define MPC85xx_PORDEVSR_RIO_DEV_ID	0x00000007
	u32	pordbgmsr;	/* POR debug mode status */
	u32	pordevsr2;	/* POR I/O device status 2 */
#if defined(CONFIG_ARCH_C29X)
#define MPC85xx_PORDEVSR2_DDR_SPD_0	0x00000008
#define MPC85xx_PORDEVSR2_DDR_SPD_0_SHIFT	3
#endif
#define MPC85xx_PORDEVSR2_SBC_MASK	0x10000000
/* The 8544 RM says this is bit 26, but it's really bit 24 */
#define MPC85xx_PORDEVSR2_SEC_CFG	0x00000080
	u8	res1[8];
	u32	gpporcr;	/* General-purpose POR configuration */
	u8	res2[12];
#if defined(CONFIG_ARCH_MPC8536)
	u32	gencfgr;	/* General Configuration Register */
#define MPC85xx_GENCFGR_SDHC_WP_INV	0x20000000
#else
	u32	gpiocr;		/* GPIO control */
#endif
	u8	res3[12];
#if defined(CONFIG_ARCH_MPC8569)
	u32	plppar1;	/* Platform port pin assignment 1 */
	u32	plppar2;	/* Platform port pin assignment 2 */
	u32	plpdir1;	/* Platform port pin direction 1 */
	u32	plpdir2;	/* Platform port pin direction 2 */
#else
	u32	gpoutdr;	/* General-purpose output data */
	u8	res4[12];
#endif
	u32	gpindr;		/* General-purpose input data */
	u8	res5[12];
	u32	pmuxcr;		/* Alt. function signal multiplex control */
#if defined(CONFIG_ARCH_P1010)
#define MPC85xx_PMUXCR_TSEC1_0_1588		0x40000000
#define MPC85xx_PMUXCR_TSEC1_0_RES		0xC0000000
#define MPC85xx_PMUXCR_TSEC1_1_1588_TRIG	0x10000000
#define MPC85xx_PMUXCR_TSEC1_1_GPIO_12		0x20000000
#define MPC85xx_PMUXCR_TSEC1_1_RES		0x30000000
#define MPC85xx_PMUXCR_TSEC1_2_DMA		0x04000000
#define MPC85xx_PMUXCR_TSEC1_2_GPIO		0x08000000
#define MPC85xx_PMUXCR_TSEC1_2_RES		0x0C000000
#define MPC85xx_PMUXCR_TSEC1_3_RES		0x01000000
#define MPC85xx_PMUXCR_TSEC1_3_GPIO_15		0x02000000
#define MPC85xx_PMUXCR_IFC_ADDR16_SDHC		0x00400000
#define MPC85xx_PMUXCR_IFC_ADDR16_USB		0x00800000
#define MPC85xx_PMUXCR_IFC_ADDR16_IFC_CS2	0x00C00000
#define MPC85xx_PMUXCR_IFC_ADDR17_18_SDHC	0x00100000
#define MPC85xx_PMUXCR_IFC_ADDR17_18_USB	0x00200000
#define MPC85xx_PMUXCR_IFC_ADDR17_18_DMA	0x00300000
#define MPC85xx_PMUXCR_IFC_ADDR19_SDHC_DATA	0x00040000
#define MPC85xx_PMUXCR_IFC_ADDR19_USB		0x00080000
#define MPC85xx_PMUXCR_IFC_ADDR19_DMA		0x000C0000
#define MPC85xx_PMUXCR_IFC_ADDR20_21_SDHC_DATA	0x00010000
#define MPC85xx_PMUXCR_IFC_ADDR20_21_USB	0x00020000
#define MPC85xx_PMUXCR_IFC_ADDR20_21_RES	0x00030000
#define MPC85xx_PMUXCR_IFC_ADDR22_SDHC		0x00004000
#define MPC85xx_PMUXCR_IFC_ADDR22_USB		0x00008000
#define MPC85xx_PMUXCR_IFC_ADDR22_RES		0x0000C000
#define MPC85xx_PMUXCR_IFC_ADDR23_SDHC		0x00001000
#define MPC85xx_PMUXCR_IFC_ADDR23_USB		0x00002000
#define MPC85xx_PMUXCR_IFC_ADDR23_RES		0x00003000
#define MPC85xx_PMUXCR_IFC_ADDR24_SDHC		0x00000400
#define MPC85xx_PMUXCR_IFC_ADDR24_USB		0x00000800
#define MPC85xx_PMUXCR_IFC_ADDR24_RES		0x00000C00
#define MPC85xx_PMUXCR_IFC_PAR_PERR_RES		0x00000300
#define MPC85xx_PMUXCR_IFC_PAR_PERR_USB		0x00000200
#define MPC85xx_PMUXCR_LCLK_RES			0x00000040
#define MPC85xx_PMUXCR_LCLK_USB			0x00000080
#define MPC85xx_PMUXCR_LCLK_IFC_CS3		0x000000C0
#define MPC85xx_PMUXCR_SPI_RES			0x00000030
#define MPC85xx_PMUXCR_SPI_GPIO			0x00000020
#define MPC85xx_PMUXCR_CAN1_UART		0x00000004
#define MPC85xx_PMUXCR_CAN1_TDM			0x00000008
#define MPC85xx_PMUXCR_CAN1_RES			0x0000000C
#define MPC85xx_PMUXCR_CAN2_UART		0x00000001
#define MPC85xx_PMUXCR_CAN2_TDM			0x00000002
#define MPC85xx_PMUXCR_CAN2_RES			0x00000003
#endif
#if defined(CONFIG_ARCH_P1023)
#define MPC85xx_PMUXCR_TSEC1_1		0x10000000
#else
#define MPC85xx_PMUXCR_SD_DATA		0x80000000
#define MPC85xx_PMUXCR_SDHC_CD		0x40000000
#define MPC85xx_PMUXCR_SDHC_WP		0x20000000
#define MPC85xx_PMUXCR_ELBC_OFF_USB2_ON	0x01000000
#define MPC85xx_PMUXCR_TDM_ENA		0x00800000
#define MPC85xx_PMUXCR_QE0		0x00008000
#define MPC85xx_PMUXCR_QE1		0x00004000
#define MPC85xx_PMUXCR_QE2		0x00002000
#define MPC85xx_PMUXCR_QE3		0x00001000
#define MPC85xx_PMUXCR_QE4		0x00000800
#define MPC85xx_PMUXCR_QE5		0x00000400
#define MPC85xx_PMUXCR_QE6		0x00000200
#define MPC85xx_PMUXCR_QE7		0x00000100
#define MPC85xx_PMUXCR_QE8		0x00000080
#define MPC85xx_PMUXCR_QE9		0x00000040
#define MPC85xx_PMUXCR_QE10		0x00000020
#define MPC85xx_PMUXCR_QE11		0x00000010
#define MPC85xx_PMUXCR_QE12		0x00000008
#endif
#if defined(CONFIG_ARCH_P1022)
#define MPC85xx_PMUXCR_TDM_MASK		0x0001cc00
#define MPC85xx_PMUXCR_TDM		0x00014800
#define MPC85xx_PMUXCR_SPI_MASK		0x00600000
#define MPC85xx_PMUXCR_SPI		0x00000000
#endif
#if defined(CONFIG_ARCH_BSC9131)
#define MPC85xx_PMUXCR_TSEC2_DMA_GPIO_IRQ	0x40000000
#define MPC85xx_PMUXCR_TSEC2_USB		0xC0000000
#define MPC85xx_PMUXCR_TSEC2_1588_PPS		0x10000000
#define MPC85xx_PMUXCR_TSEC2_1588_RSVD		0x30000000
#define MPC85xx_PMUXCR_IFC_AD_GPIO		0x04000000
#define MPC85xx_PMUXCR_IFC_AD_GPIO_MASK		0x0C000000
#define MPC85xx_PMUXCR_IFC_AD15_GPIO		0x01000000
#define MPC85xx_PMUXCR_IFC_AD15_TIMER2		0x02000000
#define MPC85xx_PMUXCR_IFC_AD16_GPO8		0x00400000
#define MPC85xx_PMUXCR_IFC_AD16_MSRCID0		0x00800000
#define MPC85xx_PMUXCR_IFC_AD17_GPO		0x00100000
#define MPC85xx_PMUXCR_IFC_AD17_GPO_MASK	0x00300000
#define MPC85xx_PMUXCR_IFC_AD17_MSRCID_DSP	0x00200000
#define MPC85xx_PMUXCR_IFC_CS2_GPO65		0x00040000
#define MPC85xx_PMUXCR_IFC_CS2_DSP_TDI		0x00080000
#define MPC85xx_PMUXCR_SDHC_USIM		0x00010000
#define MPC85xx_PMUXCR_SDHC_TDM_RFS_RCK		0x00020000
#define MPC85xx_PMUXCR_SDHC_GPIO77		0x00030000
#define MPC85xx_PMUXCR_SDHC_RESV		0x00004000
#define MPC85xx_PMUXCR_SDHC_TDM_TXD_RXD		0x00008000
#define MPC85xx_PMUXCR_SDHC_GPIO_TIMER4		0x0000C000
#define MPC85xx_PMUXCR_USB_CLK_UART_SIN		0x00001000
#define MPC85xx_PMUXCR_USB_CLK_GPIO69		0x00002000
#define MPC85xx_PMUXCR_USB_CLK_TIMER3		0x00003000
#define MPC85xx_PMUXCR_USB_UART_GPIO0		0x00000400
#define MPC85xx_PMUXCR_USB_RSVD			0x00000C00
#define MPC85xx_PMUXCR_USB_GPIO62_TRIG_IN	0x00000800
#define MPC85xx_PMUXCR_USB_D1_2_IIC2_SDA_SCL	0x00000100
#define MPC85xx_PMUXCR_USB_D1_2_GPIO71_72	0x00000200
#define MPC85xx_PMUXCR_USB_D1_2_RSVD		0x00000300
#define MPC85xx_PMUXCR_USB_DIR_GPIO2		0x00000040
#define MPC85xx_PMUXCR_USB_DIR_TIMER1		0x00000080
#define MPC85xx_PMUXCR_USB_DIR_MCP_B		0x000000C0
#define MPC85xx_PMUXCR_SPI1_UART3		0x00000010
#define MPC85xx_PMUXCR_SPI1_SIM			0x00000020
#define MPC85xx_PMUXCR_SPI1_CKSTP_IN_GPO74	0x00000030
#define MPC85xx_PMUXCR_SPI1_CS2_CKSTP_OUT_B	0x00000004
#define MPC85xx_PMUXCR_SPI1_CS2_dbg_adi1_rxen	0x00000008
#define MPC85xx_PMUXCR_SPI1_CS2_GPO75		0x0000000C
#define MPC85xx_PMUXCR_SPI1_CS3_ANT_TCXO_PWM	0x00000001
#define MPC85xx_PMUXCR_SPI1_CS3_dbg_adi2_rxen	0x00000002
#define MPC85xx_PMUXCR_SPI1_CS3_GPO76		0x00000003
#endif
#ifdef CONFIG_ARCH_BSC9132
#define MPC85xx_PMUXCR0_SIM_SEL_MASK	0x0003b000
#define MPC85xx_PMUXCR0_SIM_SEL		0x00014000
#endif
#if defined(CONFIG_ARCH_C29X)
#define MPC85xx_PMUXCR_SPI_MASK			0x00000300
#define MPC85xx_PMUXCR_SPI			0x00000000
#define MPC85xx_PMUXCR_SPI_GPIO			0x00000100
#endif
	u32	pmuxcr2;	/* Alt. function signal multiplex control 2 */
#if defined(CONFIG_ARCH_P1010)
#define MPC85xx_PMUXCR2_UART_GPIO		0x40000000
#define MPC85xx_PMUXCR2_UART_TDM		0x80000000
#define MPC85xx_PMUXCR2_UART_RES		0xC0000000
#define MPC85xx_PMUXCR2_IRQ2_TRIG_IN		0x10000000
#define MPC85xx_PMUXCR2_IRQ2_RES		0x30000000
#define MPC85xx_PMUXCR2_IRQ3_SRESET		0x04000000
#define MPC85xx_PMUXCR2_IRQ3_RES		0x0C000000
#define MPC85xx_PMUXCR2_GPIO01_DRVVBUS		0x01000000
#define MPC85xx_PMUXCR2_GPIO01_RES		0x03000000
#define MPC85xx_PMUXCR2_GPIO23_CKSTP		0x00400000
#define MPC85xx_PMUXCR2_GPIO23_RES		0x00800000
#define MPC85xx_PMUXCR2_GPIO23_USB		0x00C00000
#define MPC85xx_PMUXCR2_GPIO4_MCP		0x00100000
#define MPC85xx_PMUXCR2_GPIO4_RES		0x00200000
#define MPC85xx_PMUXCR2_GPIO4_CLK_OUT		0x00300000
#define MPC85xx_PMUXCR2_GPIO5_UDE		0x00040000
#define MPC85xx_PMUXCR2_GPIO5_RES		0x00080000
#define MPC85xx_PMUXCR2_READY_ASLEEP		0x00020000
#define MPC85xx_PMUXCR2_DDR_ECC_MUX		0x00010000
#define MPC85xx_PMUXCR2_DEBUG_PORT_EXPOSE	0x00008000
#define MPC85xx_PMUXCR2_POST_EXPOSE		0x00004000
#define MPC85xx_PMUXCR2_DEBUG_MUX_SEL_USBPHY	0x00002000
#define MPC85xx_PMUXCR2_PLL_LKDT_EXPOSE		0x00001000
#endif
#if defined(CONFIG_ARCH_P1022)
#define MPC85xx_PMUXCR2_ETSECUSB_MASK	0x001f8000
#define MPC85xx_PMUXCR2_USB		0x00150000
#endif
#if defined(CONFIG_ARCH_BSC9131) || defined(CONFIG_ARCH_BSC9132)
#if defined(CONFIG_ARCH_BSC9131)
#define MPC85xx_PMUXCR2_UART_CTS_B0_SIM_PD		0X40000000
#define MPC85xx_PMUXCR2_UART_CTS_B0_DSP_TMS		0X80000000
#define MPC85xx_PMUXCR2_UART_CTS_B0_GPIO42		0xC0000000
#define MPC85xx_PMUXCR2_UART_RTS_B0_PWM2		0x10000000
#define MPC85xx_PMUXCR2_UART_RTS_B0_DSP_TCK		0x20000000
#define MPC85xx_PMUXCR2_UART_RTS_B0_GPIO43		0x30000000
#define MPC85xx_PMUXCR2_UART_CTS_B1_SIM_PD		0x04000000
#define MPC85xx_PMUXCR2_UART_CTS_B1_SRESET_B		0x08000000
#define MPC85xx_PMUXCR2_UART_CTS_B1_GPIO44		0x0C000000
#define MPC85xx_PMUXCR2_UART_RTS_B1_PPS_LED		0x01000000
#define MPC85xx_PMUXCR2_UART_RTS_B1_RSVD		0x02000000
#define MPC85xx_PMUXCR2_UART_RTS_B1_GPIO45		0x03000000
#define MPC85xx_PMUXCR2_TRIG_OUT_ASLEEP			0x00400000
#define MPC85xx_PMUXCR2_TRIG_OUT_DSP_TRST_B		0x00800000
#define MPC85xx_PMUXCR2_ANT1_TIMER5			0x00100000
#define MPC85xx_PMUXCR2_ANT1_TSEC_1588			0x00200000
#define MPC85xx_PMUXCR2_ANT1_GPIO95_19			0x00300000
#define MPC85xx_PMUXCR2_ANT1_TX_RX_FRAME_MAX3_LOCK	0x00040000
#define MPC85xx_PMUXCR2_ANT1_TX_RX_FRAME_RSVD		0x00080000
#define MPC85xx_PMUXCR2_ANT1_TX_RX_FRAME_GPIO80_20	0x000C0000
#define MPC85xx_PMUXCR2_ANT1_DIO0_3_SPI3_CS0		0x00010000
#define MPC85xx_PMUXCR2_ANT1_DIO0_3_ANT2_DO_3		0x00020000
#define MPC85xx_PMUXCR2_ANT1_DIO0_3_GPIO81_84		0x00030000
#define MPC85xx_PMUXCR2_ANT1_DIO4_7_SPI4		0x00004000
#define MPC85xx_PMUXCR2_ANT1_DIO4_7_ANT2_DO4_7		0x00008000
#define MPC85xx_PMUXCR2_ANT1_DIO4_7_GPIO85_88		0x0000C000
#define MPC85xx_PMUXCR2_ANT1_DIO8_9_MAX2_1_LOCK		0x00001000
#define MPC85xx_PMUXCR2_ANT1_DIO8_9_ANT2_DO8_9		0x00002000
#define MPC85xx_PMUXCR2_ANT1_DIO8_9_GPIO21_22		0x00003000
#define MPC85xx_PMUXCR2_ANT1_DIO10_11_TIMER6_7		0x00000400
#define MPC85xx_PMUXCR2_ANT1_DIO10_11_ANT2_DO10_11	0x00000800
#define MPC85xx_PMUXCR2_ANT1_DIO10_11_GPIO23_24		0x00000C00
#define MPC85xx_PMUXCR2_ANT2_RSVD			0x00000100
#define MPC85xx_PMUXCR2_ANT2_GPO90_91_DMA		0x00000300
#define MPC85xx_PMUXCR2_ANT2_ENABLE_DIO0_10_USB		0x00000040
#define MPC85xx_PMUXCR2_ANT2_ENABLE_DIO0_10_GPIO	0x000000C0
#define MPC85xx_PMUXCR2_ANT2_DIO11_RSVD			0x00000010
#define MPC85xx_PMUXCR2_ANT2_DIO11_TIMER8		0x00000020
#define MPC85xx_PMUXCR2_ANT2_DIO11_GPIO61		0x00000030
#define MPC85xx_PMUXCR2_ANT3_AGC_GPO53			0x00000004
#define MPC85xx_PMUXCR2_ANT3_DO_TDM			0x00000001
#define MPC85xx_PMUXCR2_ANT3_DO_GPIO46_49		0x00000002
#endif
	u32	pmuxcr3;
#if defined(CONFIG_ARCH_BSC9131)
#define MPC85xx_PMUXCR3_ANT3_DO4_5_TDM			0x40000000
#define MPC85xx_PMUXCR3_ANT3_DO4_5_GPIO_50_51		0x80000000
#define MPC85xx_PMUXCR3_ANT3_DO6_7_TRIG_IN_SRESET_B	0x10000000
#define MPC85xx_PMUXCR3_ANT3_DO6_7_GPIO_52_53		0x20000000
#define MPC85xx_PMUXCR3_ANT3_DO8_MCP_B			0x04000000
#define MPC85xx_PMUXCR3_ANT3_DO8_GPIO54			0x08000000
#define MPC85xx_PMUXCR3_ANT3_DO9_10_CKSTP_IN_OUT	0x01000000
#define MPC85xx_PMUXCR3_ANT3_DO9_10_GPIO55_56		0x02000000
#define MPC85xx_PMUXCR3_ANT3_DO11_IRQ_OUT		0x00400000
#define MPC85xx_PMUXCR3_ANT3_DO11_GPIO57		0x00800000
#define MPC85xx_PMUXCR3_SPI2_CS2_GPO93			0x00100000
#define MPC85xx_PMUXCR3_SPI2_CS3_GPO94			0x00040000
#define MPC85xx_PMUXCR3_ANT2_AGC_RSVD			0x00010000
#define MPC85xx_PMUXCR3_ANT2_GPO89			0x00030000
#endif
#ifdef CONFIG_ARCH_BSC9132
#define MPC85xx_PMUXCR3_USB_SEL_MASK	0x0000ff00
#define MPC85xx_PMUXCR3_UART2_SEL	0x00005000
#define MPC85xx_PMUXCR3_UART3_SEL_MASK	0xc0000000
#define MPC85xx_PMUXCR3_UART3_SEL	0x40000000
#endif
	u32 pmuxcr4;
#else
	u8	res6[8];
#endif
	u32	devdisr;	/* Device disable control */
#define MPC85xx_DEVDISR_PCI1		0x80000000
#define MPC85xx_DEVDISR_PCI2		0x40000000
#define MPC85xx_DEVDISR_PCIE		0x20000000
#define MPC85xx_DEVDISR_LBC		0x08000000
#define MPC85xx_DEVDISR_PCIE2		0x04000000
#define MPC85xx_DEVDISR_PCIE3		0x02000000
#define MPC85xx_DEVDISR_SEC		0x01000000
#define MPC85xx_DEVDISR_SRIO		0x00080000
#define MPC85xx_DEVDISR_RMSG		0x00040000
#define MPC85xx_DEVDISR_DDR		0x00010000
#define MPC85xx_DEVDISR_CPU		0x00008000
#define MPC85xx_DEVDISR_CPU0		MPC85xx_DEVDISR_CPU
#define MPC85xx_DEVDISR_TB		0x00004000
#define MPC85xx_DEVDISR_TB0		MPC85xx_DEVDISR_TB
#define MPC85xx_DEVDISR_CPU1		0x00002000
#define MPC85xx_DEVDISR_TB1		0x00001000
#define MPC85xx_DEVDISR_DMA		0x00000400
#define MPC85xx_DEVDISR_TSEC1		0x00000080
#define MPC85xx_DEVDISR_TSEC2		0x00000040
#define MPC85xx_DEVDISR_TSEC3		0x00000020
#define MPC85xx_DEVDISR_TSEC4		0x00000010
#define MPC85xx_DEVDISR_I2C		0x00000004
#define MPC85xx_DEVDISR_DUART		0x00000002
	u8	res7[12];
	u32	powmgtcsr;	/* Power management status & control */
	u8	res8[12];
	u32	mcpsumr;	/* Machine check summary */
	u8	res9[12];
	u32	pvr;		/* Processor version */
	u32	svr;		/* System version */
	u8	res10[8];
	u32	rstcr;		/* Reset control */
#if defined(CONFIG_ARCH_MPC8568) || defined(CONFIG_ARCH_MPC8569)
	u8	res11a[76];
	par_io_t qe_par_io[7];
	u8	res11b[1600];
#elif defined(CONFIG_ARCH_P1021) || defined(CONFIG_ARCH_P1025)
	u8      res11a[12];
	u32     iovselsr;
	u8      res11b[60];
	par_io_t qe_par_io[3];
	u8      res11c[1496];
#else
	u8	res11a[1868];
#endif
	u32	clkdvdr;	/* Clock Divide register */
	u8	res12[1532];
	u32	clkocr;		/* Clock out select */
	u8	res13[12];
	u32	ddrdllcr;	/* DDR DLL control */
	u8	res14[12];
	u32	lbcdllcr;	/* LBC DLL control */
#if defined(CONFIG_ARCH_BSC9131)
	u8	res15[12];
	u32	halt_req_mask;
#define HALTED_TO_HALT_REQ_MASK_0	0x80000000
	u8	res18[232];
#else
	u8	res15[248];
#endif
	u32	lbiuiplldcr0;	/* LBIU PLL Debug Reg 0 */
	u32	lbiuiplldcr1;	/* LBIU PLL Debug Reg 1 */
	u32	ddrioovcr;	/* DDR IO Override Control */
	u32	tsec12ioovcr;	/* eTSEC 1/2 IO override control */
	u32	tsec34ioovcr;	/* eTSEC 3/4 IO override control */
	u8      res16[52];
	u32	sdhcdcr;	/* SDHC debug control register */
	u8      res17[61592];
} ccsr_gur_t;
#endif

#define SDHCDCR_CD_INV		0x80000000 /* invert SDHC card detect */

#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
#define MAX_SERDES 4
#if defined(CONFIG_ARCH_T1024) || defined(CONFIG_ARCH_T1023)
#define SRDS_MAX_LANES 4
#else
#define SRDS_MAX_LANES 8
#endif
#define SRDS_MAX_BANK 2
typedef struct serdes_corenet {
	struct {
		u32	rstctl;	/* Reset Control Register */
#define SRDS_RSTCTL_RST		0x80000000
#define SRDS_RSTCTL_RSTDONE	0x40000000
#define SRDS_RSTCTL_RSTERR	0x20000000
#define SRDS_RSTCTL_SWRST	0x10000000
#define SRDS_RSTCTL_SDEN	0x00000020
#define SRDS_RSTCTL_SDRST_B	0x00000040
#define SRDS_RSTCTL_PLLRST_B	0x00000080
#define SRDS_RSTCTL_RSTERR_SHIFT  29
		u32	pllcr0; /* PLL Control Register 0 */
#define SRDS_PLLCR0_POFF		0x80000000
#define SRDS_PLLCR0_RFCK_SEL_MASK	0x70000000
#define SRDS_PLLCR0_RFCK_SEL_100	0x00000000
#define SRDS_PLLCR0_RFCK_SEL_125	0x10000000
#define SRDS_PLLCR0_RFCK_SEL_156_25	0x20000000
#define SRDS_PLLCR0_RFCK_SEL_150	0x30000000
#define SRDS_PLLCR0_RFCK_SEL_161_13	0x40000000
#define SRDS_PLLCR0_RFCK_SEL_122_88	0x50000000
#define SRDS_PLLCR0_PLL_LCK		0x00800000
#define SRDS_PLLCR0_DCBIAS_OUT_EN      0x02000000
#define SRDS_PLLCR0_FRATE_SEL_MASK	0x000f0000
#define SRDS_PLLCR0_FRATE_SEL_5		0x00000000
#define SRDS_PLLCR0_FRATE_SEL_4_9152	0x00030000
#define SRDS_PLLCR0_FRATE_SEL_3_75	0x00050000
#define SRDS_PLLCR0_FRATE_SEL_5_15	0x00060000
#define SRDS_PLLCR0_FRATE_SEL_4		0x00070000
#define SRDS_PLLCR0_FRATE_SEL_3_125	0x00090000
#define SRDS_PLLCR0_FRATE_SEL_3_0	0x000a0000
#define SRDS_PLLCR0_FRATE_SEL_3_072	0x000c0000
#define SRDS_PLLCR0_DCBIAS_OVRD		0x000000F0
#define SRDS_PLLCR0_DCBIAS_OVRD_SHIFT	4
		u32	pllcr1; /* PLL Control Register 1 */
#define SRDS_PLLCR1_BCAP_EN		0x20000000
#define SRDS_PLLCR1_BCAP_OVD		0x10000000
#define SRDS_PLLCR1_PLL_FCAP		0x001F8000
#define SRDS_PLLCR1_PLL_FCAP_SHIFT	15
#define SRDS_PLLCR1_PLL_BWSEL		0x08000000
#define SRDS_PLLCR1_BYP_CAL		0x02000000
		u32	pllsr2;	/* At 0x00c, PLL Status Register 2 */
#define SRDS_PLLSR2_BCAP_EN		0x00800000
#define SRDS_PLLSR2_BCAP_EN_SHIFT	23
#define SRDS_PLLSR2_FCAP		0x003F0000
#define SRDS_PLLSR2_FCAP_SHIFT		16
#define SRDS_PLLSR2_DCBIAS		0x000F0000
#define SRDS_PLLSR2_DCBIAS_SHIFT	16
		u32	pllcr3;
		u32	pllcr4;
		u8	res_18[0x20-0x18];
	} bank[2];
	u8	res_40[0x90-0x40];
	u32	srdstcalcr;	/* 0x90 TX Calibration Control */
	u8	res_94[0xa0-0x94];
	u32	srdsrcalcr;	/* 0xa0 RX Calibration Control */
	u8	res_a4[0xb0-0xa4];
	u32	srdsgr0;	/* 0xb0 General Register 0 */
	u8	res_b4[0xe0-0xb4];
	u32	srdspccr0;	/* 0xe0 Protocol Converter Config 0 */
	u32	srdspccr1;	/* 0xe4 Protocol Converter Config 1 */
	u32	srdspccr2;	/* 0xe8 Protocol Converter Config 2 */
	u32	srdspccr3;	/* 0xec Protocol Converter Config 3 */
	u32	srdspccr4;	/* 0xf0 Protocol Converter Config 4 */
	u8	res_f4[0x100-0xf4];
	struct {
		u32	lnpssr;	/* 0x100, 0x120, ..., 0x1e0 */
		u8	res_104[0x120-0x104];
	} srdslnpssr[8];
	u8	res_200[0x800-0x200];
	struct {
		u32	gcr0;	/* 0x800 General Control Register 0 */
		u32	gcr1;	/* 0x804 General Control Register 1 */
		u32	gcr2;	/* 0x808 General Control Register 2 */
		u32	res_80c;
		u32	recr0;	/* 0x810 Receive Equalization Control */
		u32	res_814;
		u32	tecr0;	/* 0x818 Transmit Equalization Control */
		u32	res_81c;
		u32	ttlcr0;	/* 0x820 Transition Tracking Loop Ctrl 0 */
		u8	res_824[0x840-0x824];
	} lane[8];	/* Lane A, B, C, D, E, F, G, H */
	u8	res_a00[0x1000-0xa00];	/* from 0xa00 to 0xfff */
} serdes_corenet_t;

#else /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

#define SRDS_MAX_LANES		18
#define SRDS_MAX_BANK		3
typedef struct serdes_corenet {
	struct {
		u32	rstctl;	/* Reset Control Register */
#define SRDS_RSTCTL_RST		0x80000000
#define SRDS_RSTCTL_RSTDONE	0x40000000
#define SRDS_RSTCTL_RSTERR	0x20000000
#define SRDS_RSTCTL_SDPD	0x00000020
		u32	pllcr0; /* PLL Control Register 0 */
#define SRDS_PLLCR0_RFCK_SEL_MASK	0x70000000
#define SRDS_PLLCR0_PVCOCNT_EN		0x02000000
#define SRDS_PLLCR0_RFCK_SEL_100	0x00000000
#define SRDS_PLLCR0_RFCK_SEL_125	0x10000000
#define SRDS_PLLCR0_RFCK_SEL_156_25	0x20000000
#define SRDS_PLLCR0_RFCK_SEL_150	0x30000000
#define SRDS_PLLCR0_RFCK_SEL_161_13	0x40000000
#define SRDS_PLLCR0_FRATE_SEL_MASK	0x00030000
#define SRDS_PLLCR0_FRATE_SEL_5		0x00000000
#define SRDS_PLLCR0_FRATE_SEL_6_25	0x00010000
		u32	pllcr1; /* PLL Control Register 1 */
#define SRDS_PLLCR1_PLL_BWSEL	0x08000000
		u32	res[5];
	} bank[3];
	u32	res1[12];
	u32	srdstcalcr;	/* TX Calibration Control */
	u32	res2[3];
	u32	srdsrcalcr;	/* RX Calibration Control */
	u32	res3[3];
	u32	srdsgr0;	/* General Register 0 */
	u32	res4[11];
	u32	srdspccr0;	/* Protocol Converter Config 0 */
	u32	srdspccr1;	/* Protocol Converter Config 1 */
	u32	srdspccr2;	/* Protocol Converter Config 2 */
#define SRDS_PCCR2_RST_XGMII1		0x00800000
#define SRDS_PCCR2_RST_XGMII2		0x00400000
	u32	res5[197];
	struct serdes_lane {
		u32	gcr0;	/* General Control Register 0 */
#define SRDS_GCR0_RRST			0x00400000
#define SRDS_GCR0_1STLANE		0x00010000
#define SRDS_GCR0_UOTHL			0x00100000
		u32	gcr1;	/* General Control Register 1 */
#define SRDS_GCR1_REIDL_CTL_MASK	0x001f0000
#define SRDS_GCR1_REIDL_CTL_PCIE	0x00100000
#define SRDS_GCR1_REIDL_CTL_SRIO	0x00000000
#define SRDS_GCR1_REIDL_CTL_SGMII	0x00040000
#define SRDS_GCR1_OPAD_CTL		0x04000000
		u32	res1[4];
		u32	tecr0;	/* TX Equalization Control Reg 0 */
#define SRDS_TECR0_TEQ_TYPE_MASK	0x30000000
#define SRDS_TECR0_TEQ_TYPE_2LVL	0x10000000
		u32	res3;
		u32	ttlcr0;	/* Transition Tracking Loop Ctrl 0 */
#define SRDS_TTLCR0_FLT_SEL_MASK	0x3f000000
#define SRDS_TTLCR0_FLT_SEL_KFR_26	0x10000000
#define SRDS_TTLCR0_FLT_SEL_KPH_28	0x08000000
#define SRDS_TTLCR0_FLT_SEL_750PPM	0x03000000
#define SRDS_TTLCR0_PM_DIS		0x00004000
#define SRDS_TTLCR0_FREQOVD_EN		0x00000001
		u32	res4[7];
	} lane[24];
	u32 res6[384];
} serdes_corenet_t;
#endif /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

enum {
	FSL_SRDS_B1_LANE_A = 0,
	FSL_SRDS_B1_LANE_B = 1,
	FSL_SRDS_B1_LANE_C = 2,
	FSL_SRDS_B1_LANE_D = 3,
	FSL_SRDS_B1_LANE_E = 4,
	FSL_SRDS_B1_LANE_F = 5,
	FSL_SRDS_B1_LANE_G = 6,
	FSL_SRDS_B1_LANE_H = 7,
	FSL_SRDS_B1_LANE_I = 8,
	FSL_SRDS_B1_LANE_J = 9,
	FSL_SRDS_B2_LANE_A = 16,
	FSL_SRDS_B2_LANE_B = 17,
	FSL_SRDS_B2_LANE_C = 18,
	FSL_SRDS_B2_LANE_D = 19,
	FSL_SRDS_B3_LANE_A = 20,
	FSL_SRDS_B3_LANE_B = 21,
	FSL_SRDS_B3_LANE_C = 22,
	FSL_SRDS_B3_LANE_D = 23,
};

typedef struct ccsr_pme {
	u8	res0[0x804];
	u32	liodnbr;	/* LIODN Base Register */
	u8	res1[0x1f8];
	u32	srcidr;		/* Source ID Register */
	u8	res2[8];
	u32	liodnr;		/* LIODN Register */
	u8	res3[0x1e8];
	u32	pm_ip_rev_1;	/* PME IP Block Revision Reg 1*/
	u32	pm_ip_rev_2;	/* PME IP Block Revision Reg 1*/
	u8	res4[0x400];
} ccsr_pme_t;

struct ccsr_pamu {
	u32 ppbah;
	u32 ppbal;
	u32 pplah;
	u32 pplal;
	u32 spbah;
	u32 spbal;
	u32 splah;
	u32 splal;
	u32 obah;
	u32 obal;
	u32 olah;
	u32 olal;
};

#ifdef CONFIG_SYS_FSL_RAID_ENGINE
struct ccsr_raide {
	u8	res0[0x543];
	u32	liodnbr;			/* LIODN Base Register */
	u8	res1[0xab8];
	struct {
		struct {
			u32	cfg0;		/* cfg register 0 */
			u32	cfg1;		/* cfg register 1 */
			u8	res1[0x3f8];
		} ring[2];
		u8	res[0x800];
	} jq[2];
};
#endif

#ifdef CONFIG_SYS_DPAA_RMAN
struct ccsr_rman {
	u8	res0[0xf64];
	u32	mmliodnbr;	/* Message Manager LIODN Base Register */
	u32	mmitar;		/* RMAN Inbound Translation Address Register */
	u32	mmitdr;		/* RMAN Inbound Translation Data Register */
	u8	res4[0x1f090];
};
#endif

#ifdef CONFIG_SYS_PMAN
struct ccsr_pman {
	u8	res_00[0x40];
	u32	poes1;		/* PMAN Operation Error Status Register 1 */
	u32	poes2;		/* PMAN Operation Error Status Register 2 */
	u32	poeah;		/* PMAN Operation Error Address High */
	u32	poeal;		/* PMAN Operation Error Address Low */
	u8	res_50[0x50];
	u32	pr1;		/* PMAN Revision Register 1 */
	u32	pr2;		/* PMAN Revision Register 2 */
	u8	res_a8[0x8];
	u32	pcap;		/* PMAN Capabilities Register */
	u8	res_b4[0xc];
	u32	pc1;		/* PMAN Control Register 1 */
	u32	pc2;		/* PMAN Control Register 2 */
	u32	pc3;		/* PMAN Control Register 3 */
	u32	pc4;		/* PMAN Control Register 4 */
	u32	pc5;		/* PMAN Control Register 5 */
	u32	pc6;		/* PMAN Control Register 6 */
	u8	res_d8[0x8];
	u32	ppa1;		/* PMAN Prefetch Attributes Register 1 */
	u32	ppa2;		/* PMAN Prefetch Attributes Register 2 */
	u8	res_e8[0x8];
	u32	pics;		/* PMAN Interrupt Control and Status */
	u8	res_f4[0xf0c];
};
#endif

#ifdef CONFIG_FSL_CORENET
#define CONFIG_SYS_FSL_CORENET_CCM_OFFSET	0x0000
#ifdef CONFIG_SYS_PMAN
#define CONFIG_SYS_FSL_CORENET_PMAN1_OFFSET	0x4000
#define CONFIG_SYS_FSL_CORENET_PMAN2_OFFSET	0x5000
#define CONFIG_SYS_FSL_CORENET_PMAN3_OFFSET	0x6000
#endif
#define CONFIG_SYS_MPC8xxx_DDR_OFFSET		0x8000
#define CONFIG_SYS_MPC8xxx_DDR2_OFFSET		0x9000
#define CONFIG_SYS_MPC8xxx_DDR3_OFFSET		0xA000
#define CONFIG_SYS_FSL_CORENET_CLK_OFFSET	0xE1000
#define CONFIG_SYS_FSL_CORENET_RCPM_OFFSET	0xE2000
#ifdef CONFIG_SYS_FSL_SFP_VER_3_0
/* In SFPv3, OSPR register is now at offset 0x200.
 *  * So directly mapping sfp register map to this address */
#define CONFIG_SYS_OSPR_OFFSET                  0x200
#define CONFIG_SYS_SFP_OFFSET            (0xE8000 + CONFIG_SYS_OSPR_OFFSET)
#else
#define CONFIG_SYS_SFP_OFFSET                   0xE8000
#endif
#define CONFIG_SYS_FSL_CORENET_SERDES_OFFSET	0xEA000
#define CONFIG_SYS_FSL_CORENET_SERDES2_OFFSET	0xEB000
#define CONFIG_SYS_FSL_CORENET_SERDES3_OFFSET	0xEC000
#define CONFIG_SYS_FSL_CORENET_SERDES4_OFFSET	0xED000
#define CONFIG_SYS_FSL_CPC_OFFSET		0x10000
#define CONFIG_SYS_FSL_SCFG_OFFSET		0xFC000
#define CONFIG_SYS_FSL_PAMU_OFFSET		0x20000
#define CONFIG_SYS_MPC85xx_DMA1_OFFSET		0x100000
#define CONFIG_SYS_MPC85xx_DMA2_OFFSET		0x101000
#define CONFIG_SYS_MPC85xx_DMA3_OFFSET		0x102000
#define CONFIG_SYS_MPC85xx_DMA_OFFSET		CONFIG_SYS_MPC85xx_DMA1_OFFSET
#define CONFIG_SYS_MPC85xx_ESPI_OFFSET		0x110000
#define CONFIG_SYS_MPC85xx_ESDHC_OFFSET		0x114000
#define CONFIG_SYS_MPC85xx_LBC_OFFSET		0x124000
#define CONFIG_SYS_MPC85xx_IFC_OFFSET		0x124000
#define CONFIG_SYS_MPC85xx_GPIO_OFFSET		0x130000
#define CONFIG_SYS_MPC85xx_TDM_OFFSET		0x185000
#define CONFIG_SYS_MPC85xx_QE_OFFSET		0x140000
#define CONFIG_SYS_FSL_CORENET_RMAN_OFFSET	0x1e0000
#if defined(CONFIG_SYS_FSL_QORIQ_CHASSIS2) && !defined(CONFIG_ARCH_B4860) && \
	!defined(CONFIG_ARCH_B4420)
#define CONFIG_SYS_MPC85xx_PCIE1_OFFSET		0x240000
#define CONFIG_SYS_MPC85xx_PCIE2_OFFSET		0x250000
#define CONFIG_SYS_MPC85xx_PCIE3_OFFSET		0x260000
#define CONFIG_SYS_MPC85xx_PCIE4_OFFSET		0x270000
#else
#define CONFIG_SYS_MPC85xx_PCIE1_OFFSET		0x200000
#define CONFIG_SYS_MPC85xx_PCIE2_OFFSET		0x201000
#define CONFIG_SYS_MPC85xx_PCIE3_OFFSET		0x202000
#define CONFIG_SYS_MPC85xx_PCIE4_OFFSET		0x203000
#endif
#define CONFIG_SYS_MPC85xx_USB1_OFFSET		0x210000
#define CONFIG_SYS_MPC85xx_USB2_OFFSET		0x211000
#define CONFIG_SYS_MPC85xx_USB1_PHY_OFFSET 0x214000
#define CONFIG_SYS_MPC85xx_USB2_PHY_OFFSET 0x214100
#define CONFIG_SYS_MPC85xx_SATA1_OFFSET		0x220000
#define CONFIG_SYS_MPC85xx_SATA2_OFFSET		0x221000
#define CONFIG_SYS_FSL_SEC_OFFSET		0x300000
#define CONFIG_SYS_FSL_JR0_OFFSET		0x301000
#define CONFIG_SYS_SEC_MON_OFFSET		0x314000
#define CONFIG_SYS_FSL_CORENET_PME_OFFSET	0x316000
#define CONFIG_SYS_FSL_QMAN_OFFSET		0x318000
#define CONFIG_SYS_FSL_BMAN_OFFSET		0x31a000
#define CONFIG_SYS_FSL_RAID_ENGINE_OFFSET	0x320000
#define CONFIG_SYS_FSL_FM1_OFFSET		0x400000
#define CONFIG_SYS_FSL_FM1_RX0_1G_OFFSET	0x488000
#define CONFIG_SYS_FSL_FM1_RX1_1G_OFFSET	0x489000
#define CONFIG_SYS_FSL_FM1_RX2_1G_OFFSET	0x48a000
#define CONFIG_SYS_FSL_FM1_RX3_1G_OFFSET	0x48b000
#define CONFIG_SYS_FSL_FM1_RX4_1G_OFFSET	0x48c000
#define CONFIG_SYS_FSL_FM1_RX5_1G_OFFSET	0x48d000
#define CONFIG_SYS_FSL_FM1_RX0_10G_OFFSET	0x490000
#define CONFIG_SYS_FSL_FM1_RX1_10G_OFFSET	0x491000
#define CONFIG_SYS_FSL_FM1_DTSEC1_OFFSET	0x4e0000
#define CONFIG_SYS_FSL_FM2_OFFSET		0x500000
#define CONFIG_SYS_FSL_FM2_RX0_1G_OFFSET	0x588000
#define CONFIG_SYS_FSL_FM2_RX1_1G_OFFSET	0x589000
#define CONFIG_SYS_FSL_FM2_RX2_1G_OFFSET	0x58a000
#define CONFIG_SYS_FSL_FM2_RX3_1G_OFFSET	0x58b000
#define CONFIG_SYS_FSL_FM2_RX4_1G_OFFSET	0x58c000
#define CONFIG_SYS_FSL_FM2_RX5_1G_OFFSET	0x58d000
#define CONFIG_SYS_FSL_FM2_RX0_10G_OFFSET	0x590000
#define CONFIG_SYS_FSL_FM2_RX1_10G_OFFSET	0x591000
#define CONFIG_SYS_FSL_CLUSTER_1_L2_OFFSET	0xC20000
#else
#define CONFIG_SYS_MPC85xx_ECM_OFFSET		0x0000
#define CONFIG_SYS_MPC8xxx_DDR_OFFSET		0x2000
#define CONFIG_SYS_MPC85xx_LBC_OFFSET		0x5000
#define CONFIG_SYS_MPC8xxx_DDR2_OFFSET		0x6000
#define CONFIG_SYS_MPC85xx_ESPI_OFFSET		0x7000
#define CONFIG_SYS_MPC85xx_PCI1_OFFSET		0x8000
#define CONFIG_SYS_MPC85xx_PCIX_OFFSET		0x8000
#define CONFIG_SYS_MPC85xx_PCI2_OFFSET		0x9000
#define CONFIG_SYS_MPC85xx_PCIX2_OFFSET		0x9000
#define CONFIG_SYS_MPC85xx_PCIE1_OFFSET         0xa000
#define CONFIG_SYS_MPC85xx_PCIE2_OFFSET         0x9000
#if defined(CONFIG_ARCH_MPC8572) || defined(CONFIG_ARCH_P2020)
#define CONFIG_SYS_MPC85xx_PCIE3_OFFSET         0x8000
#else
#define CONFIG_SYS_MPC85xx_PCIE3_OFFSET         0xb000
#endif
#define CONFIG_SYS_MPC85xx_GPIO_OFFSET		0xF000
#define CONFIG_SYS_MPC85xx_SATA1_OFFSET		0x18000
#define CONFIG_SYS_MPC85xx_SATA2_OFFSET		0x19000
#define CONFIG_SYS_MPC85xx_IFC_OFFSET		0x1e000
#define CONFIG_SYS_MPC85xx_L2_OFFSET		0x20000
#define CONFIG_SYS_MPC85xx_DMA_OFFSET		0x21000
#define CONFIG_SYS_MPC85xx_USB1_OFFSET		0x22000
#define CONFIG_SYS_MPC85xx_USB2_OFFSET		0x23000
#define CONFIG_SYS_MPC85xx_USB1_PHY_OFFSET	0xE5000
#define CONFIG_SYS_MPC85xx_USB2_PHY_OFFSET	0xE5100
#ifdef CONFIG_TSECV2
#define CONFIG_SYS_TSEC1_OFFSET			0xB0000
#elif defined(CONFIG_TSECV2_1)
#define CONFIG_SYS_TSEC1_OFFSET			0x10000
#else
#define CONFIG_SYS_TSEC1_OFFSET			0x24000
#endif
#define CONFIG_SYS_MDIO1_OFFSET			0x24000
#define CONFIG_SYS_MPC85xx_ESDHC_OFFSET		0x2e000
#if defined(CONFIG_ARCH_C29X)
#define CONFIG_SYS_FSL_SEC_OFFSET		0x80000
#define CONFIG_SYS_FSL_JR0_OFFSET               0x81000
#else
#define CONFIG_SYS_FSL_SEC_OFFSET		0x30000
#define CONFIG_SYS_FSL_JR0_OFFSET               0x31000
#endif
#define CONFIG_SYS_MPC85xx_SERDES2_OFFSET	0xE3100
#define CONFIG_SYS_MPC85xx_SERDES1_OFFSET	0xE3000
#define CONFIG_SYS_SEC_MON_OFFSET		0xE6000
#define CONFIG_SYS_SFP_OFFSET			0xE7000
#define CONFIG_SYS_MPC85xx_CPM_OFFSET		0x80000
#define CONFIG_SYS_FSL_QMAN_OFFSET		0x88000
#define CONFIG_SYS_FSL_BMAN_OFFSET		0x8a000
#define CONFIG_SYS_FSL_FM1_OFFSET		0x100000
#define CONFIG_SYS_FSL_FM1_RX0_1G_OFFSET	0x188000
#define CONFIG_SYS_FSL_FM1_RX1_1G_OFFSET	0x189000
#define CONFIG_SYS_FSL_FM1_DTSEC1_OFFSET	0x1e0000
#endif

#define CONFIG_SYS_MPC85xx_PIC_OFFSET		0x40000
#define CONFIG_SYS_MPC85xx_GUTS_OFFSET		0xE0000
#define CONFIG_SYS_FSL_SRIO_OFFSET		0xC0000

#if defined(CONFIG_ARCH_BSC9132)
#define CONFIG_SYS_FSL_DSP_CCSR_DDR_OFFSET	0x10000
#define CONFIG_SYS_FSL_DSP_CCSR_DDR_ADDR \
	(CONFIG_SYS_FSL_DSP_CCSRBAR + CONFIG_SYS_FSL_DSP_CCSR_DDR_OFFSET)
#endif

#define CONFIG_SYS_FSL_CPC_ADDR	\
	(CONFIG_SYS_CCSRBAR + CONFIG_SYS_FSL_CPC_OFFSET)
#define CONFIG_SYS_FSL_SCFG_ADDR	\
	(CONFIG_SYS_CCSRBAR + CONFIG_SYS_FSL_SCFG_OFFSET)
#define CONFIG_SYS_FSL_SCFG_PIXCLK_ADDR	\
	(CONFIG_SYS_FSL_SCFG_ADDR + CONFIG_SYS_FSL_SCFG_PIXCLKCR_OFFSET)
#define CONFIG_SYS_FSL_SCFG_IODSECR1_ADDR \
	(CONFIG_SYS_FSL_SCFG_ADDR + CONFIG_SYS_FSL_SCFG_IODSECR1_OFFSET)
#define CONFIG_SYS_FSL_QMAN_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_QMAN_OFFSET)
#define CONFIG_SYS_FSL_BMAN_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_BMAN_OFFSET)
#define CONFIG_SYS_FSL_CORENET_PME_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_PME_OFFSET)
#define CONFIG_SYS_FSL_RAID_ENGINE_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_RAID_ENGINE_OFFSET)
#define CONFIG_SYS_FSL_CORENET_RMAN_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_RMAN_OFFSET)
#define CONFIG_SYS_MPC85xx_GUTS_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_GUTS_OFFSET)
#define CONFIG_SYS_FSL_CORENET_CCM_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_CCM_OFFSET)
#define CONFIG_SYS_FSL_CORENET_CLK_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_CLK_OFFSET)
#define CONFIG_SYS_FSL_CORENET_RCPM_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_RCPM_OFFSET)
#define CONFIG_SYS_MPC85xx_ECM_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_ECM_OFFSET)
#define CONFIG_SYS_FSL_DDR_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC8xxx_DDR_OFFSET)
#define CONFIG_SYS_FSL_DDR2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC8xxx_DDR2_OFFSET)
#define CONFIG_SYS_FSL_DDR3_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC8xxx_DDR3_OFFSET)
#define CONFIG_SYS_LBC_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_LBC_OFFSET)
#define CONFIG_SYS_IFC_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_IFC_OFFSET)
#define CONFIG_SYS_MPC85xx_ESPI_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_ESPI_OFFSET)
#define CONFIG_SYS_MPC85xx_PCIX_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCIX_OFFSET)
#define CONFIG_SYS_MPC85xx_PCIX2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCIX2_OFFSET)
#define CONFIG_SYS_MPC85xx_GPIO_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_GPIO_OFFSET)
#define CONFIG_SYS_MPC85xx_SATA1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_SATA1_OFFSET)
#define CONFIG_SYS_MPC85xx_SATA2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_SATA2_OFFSET)
#define CONFIG_SYS_MPC85xx_L2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_L2_OFFSET)
#define CONFIG_SYS_MPC85xx_DMA_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_DMA_OFFSET)
#define CONFIG_SYS_MPC85xx_ESDHC_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_ESDHC_OFFSET)
#define CONFIG_SYS_MPC8xxx_PIC_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PIC_OFFSET)
#define CONFIG_SYS_MPC85xx_CPM_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_CPM_OFFSET)
#define CONFIG_SYS_MPC85xx_SERDES1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_SERDES1_OFFSET)
#define CONFIG_SYS_MPC85xx_SERDES2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_SERDES2_OFFSET)
#define CONFIG_SYS_FSL_CORENET_SERDES_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_SERDES_OFFSET)
#define CONFIG_SYS_FSL_CORENET_SERDES2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_SERDES2_OFFSET)
#define CONFIG_SYS_FSL_CORENET_SERDES3_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_SERDES3_OFFSET)
#define CONFIG_SYS_FSL_CORENET_SERDES4_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CORENET_SERDES4_OFFSET)
#define CONFIG_SYS_MPC85xx_USB1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_USB1_OFFSET)
#define CONFIG_SYS_MPC85xx_USB2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_USB2_OFFSET)
#define CONFIG_SYS_MPC85xx_USB1_PHY_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_USB1_PHY_OFFSET)
#define CONFIG_SYS_MPC85xx_USB2_PHY_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_USB2_PHY_OFFSET)
#define CONFIG_SYS_FSL_SEC_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_SEC_OFFSET)
#define CONFIG_SYS_FSL_JR0_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_JR0_OFFSET)
#define CONFIG_SYS_FSL_FM1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_FM1_OFFSET)
#define CONFIG_SYS_FSL_FM1_DTSEC1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_FM1_DTSEC1_OFFSET)
#define CONFIG_SYS_FSL_FM2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_FM2_OFFSET)
#define CONFIG_SYS_FSL_SRIO_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_SRIO_OFFSET)
#define CONFIG_SYS_PAMU_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_PAMU_OFFSET)

#define CONFIG_SYS_PCI1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCI1_OFFSET)
#define CONFIG_SYS_PCI2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCI2_OFFSET)
#define CONFIG_SYS_PCIE1_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCIE1_OFFSET)
#define CONFIG_SYS_PCIE2_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCIE2_OFFSET)
#define CONFIG_SYS_PCIE3_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCIE3_OFFSET)
#define CONFIG_SYS_PCIE4_ADDR \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_PCIE4_OFFSET)

#define CONFIG_SYS_SFP_ADDR  \
	(CONFIG_SYS_IMMR + CONFIG_SYS_SFP_OFFSET)

#define CONFIG_SYS_SEC_MON_ADDR  \
	(CONFIG_SYS_IMMR + CONFIG_SYS_SEC_MON_OFFSET)

#define TSEC_BASE_ADDR		(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC1_OFFSET)
#define MDIO_BASE_ADDR		(CONFIG_SYS_IMMR + CONFIG_SYS_MDIO1_OFFSET)

#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
struct ccsr_cluster_l2 {
	u32 l2csr0;	/* 0x000 L2 cache control and status register 0 */
	u32 l2csr1;	/* 0x004 L2 cache control and status register 1 */
	u32 l2cfg0;	/* 0x008 L2 cache configuration register 0 */
	u8  res_0c[500];/* 0x00c - 0x1ff */
	u32 l2pir0;	/* 0x200 L2 cache partitioning ID register 0 */
	u8  res_204[4];
	u32 l2par0;	/* 0x208 L2 cache partitioning allocation register 0 */
	u32 l2pwr0;	/* 0x20c L2 cache partitioning way register 0 */
	u32 l2pir1;	/* 0x210 L2 cache partitioning ID register 1 */
	u8  res_214[4];
	u32 l2par1;	/* 0x218 L2 cache partitioning allocation register 1 */
	u32 l2pwr1;	/* 0x21c L2 cache partitioning way register 1 */
	u32 u2pir2;	/* 0x220 L2 cache partitioning ID register 2 */
	u8  res_224[4];
	u32 l2par2;	/* 0x228 L2 cache partitioning allocation register 2 */
	u32 l2pwr2;	/* 0x22c L2 cache partitioning way register 2 */
	u32 l2pir3;	/* 0x230 L2 cache partitioning ID register 3 */
	u8  res_234[4];
	u32 l2par3;	/* 0x238 L2 cache partitining allocation register 3 */
	u32 l2pwr3;	/* 0x23c L2 cache partitining way register 3 */
	u32 l2pir4;	/* 0x240 L2 cache partitioning ID register 3 */
	u8  res244[4];
	u32 l2par4;	/* 0x248 L2 cache partitioning allocation register 3 */
	u32 l2pwr4;	/* 0x24c L2 cache partitioning way register 3 */
	u32 l2pir5;	/* 0x250 L2 cache partitioning ID register 3 */
	u8  res_254[4];
	u32 l2par5;	/* 0x258 L2 cache partitioning allocation register 3 */
	u32 l2pwr5;	/* 0x25c L2 cache partitioning way register 3 */
	u32 l2pir6;	/* 0x260 L2 cache partitioning ID register 3 */
	u8  res_264[4];
	u32 l2par6;	/* 0x268 L2 cache partitioning allocation register 3 */
	u32 l2pwr6;	/* 0x26c L2 cache partitioning way register 3 */
	u32 l2pir7;	/* 0x270 L2 cache partitioning ID register 3 */
	u8  res274[4];
	u32 l2par7;	/* 0x278 L2 cache partitioning allocation register 3 */
	u32 l2pwr7;	/* 0x27c L2 cache partitioning way register 3 */
	u8  res_280[0xb80]; /* 0x280 - 0xdff */
	u32 l2errinjhi;	/* 0xe00 L2 cache error injection mask high */
	u32 l2errinjlo;	/* 0xe04 L2 cache error injection mask low */
	u32 l2errinjctl;/* 0xe08 L2 cache error injection control */
	u8  res_e0c[20];	/* 0xe0c - 0x01f */
	u32 l2captdatahi; /* 0xe20 L2 cache error capture data high */
	u32 l2captdatalo; /* 0xe24 L2 cache error capture data low */
	u32 l2captecc;	/* 0xe28 L2 cache error capture ECC syndrome */
	u8  res_e2c[20];	/* 0xe2c - 0xe3f */
	u32 l2errdet;	/* 0xe40 L2 cache error detect */
	u32 l2errdis;	/* 0xe44 L2 cache error disable */
	u32 l2errinten;	/* 0xe48 L2 cache error interrupt enable */
	u32 l2errattr;	/* 0xe4c L2 cache error attribute */
	u32 l2erreaddr;	/* 0xe50 L2 cache error extended address */
	u32 l2erraddr;	/* 0xe54 L2 cache error address */
	u32 l2errctl;	/* 0xe58 L2 cache error control */
};
#define CONFIG_SYS_FSL_CLUSTER_1_L2 \
	(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_CLUSTER_1_L2_OFFSET)
#endif /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

#define	CONFIG_SYS_DCSR_DCFG_OFFSET	0X20000
struct dcsr_dcfg_regs {
	u8  res_0[0x520];
	u32 ecccr1;
#define	DCSR_DCFG_ECC_DISABLE_USB1	0x00008000
#define	DCSR_DCFG_ECC_DISABLE_USB2	0x00004000
	u8  res_524[0x1000 - 0x524]; /* 0x524 - 0x1000 */
};

#define CONFIG_SYS_MPC85xx_SCFG \
	(CONFIG_SYS_IMMR + CONFIG_SYS_MPC85xx_SCFG_OFFSET)
#define CONFIG_SYS_MPC85xx_SCFG_OFFSET	0xfc000
/* The supplement configuration unit register */
struct ccsr_scfg {
	u32 dpslpcr;	/* 0x000 Deep Sleep Control register */
	u32 usb1dpslpcsr;/* 0x004 USB1 Deep Sleep Control Status register */
	u32 usb2dpslpcsr;/* 0x008 USB2 Deep Sleep Control Status register */
	u32 fmclkdpslpcr;/* 0x00c FM Clock Deep Sleep Control register */
	u32 res1[4];
	u32 esgmiiselcr;/* 0x020 Ethernet Switch SGMII Select Control reg */
	u32 res2;
	u32 pixclkcr;	/* 0x028 Pixel Clock Control register */
	u32 res3[245];
	u32 qeioclkcr;	/* 0x400 QUICC Engine IO Clock Control register */
	u32 emiiocr;	/* 0x404 EMI MDIO Control Register */
	u32 sdhciovselcr;/* 0x408 SDHC IO VSEL Control register */
	u32 qmifrstcr;	/* 0x40c QMAN Interface Reset Control register */
	u32 res4[60];
	u32 sparecr[8];	/* 0x500 Spare Control register(0-7) */
};
#endif /*__IMMAP_85xx__*/
