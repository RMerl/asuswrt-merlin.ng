/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * QUICC Engine (QE) Internal Memory Map.
 * The Internal Memory Map for devices with QE on them. This
 * is the superset of all QE devices (8360, etc.).
 *
 * Copyright (c) 2006-2009, 2011 Freescale Semiconductor, Inc.
 * Author: Shlomi Gridih <gridish@freescale.com>
 */

#ifndef __IMMAP_QE_H__
#define __IMMAP_QE_H__

#ifdef CONFIG_MPC83xx
#if defined(CONFIG_ARCH_MPC8360)
#define QE_MURAM_SIZE		0xc000UL
#define MAX_QE_RISC		2
#define QE_NUM_OF_SNUM		28
#elif defined(CONFIG_ARCH_MPC832X) || defined(CONFIG_ARCH_MPC8309)
#define QE_MURAM_SIZE		0x4000UL
#define MAX_QE_RISC		1
#define QE_NUM_OF_SNUM		28
#endif
#endif

#ifdef CONFIG_ARCH_LS1021A
#define QE_MURAM_SIZE          0x6000UL
#define MAX_QE_RISC            1
#define QE_NUM_OF_SNUM         28
#endif

#ifdef CONFIG_PPC
#define QE_IMMR_OFFSET		0x00140000
#else
#define QE_IMMR_OFFSET		0x01400000
#endif

/* QE I-RAM */
typedef struct qe_iram {
	u32 iadd;		/* I-RAM Address Register */
	u32 idata;		/* I-RAM Data Register    */
	u8 res0[0x4];
	u32 iready;
	u8 res1[0x70];
} __attribute__ ((packed)) qe_iram_t;

/* QE Interrupt Controller */
typedef struct qe_ic {
	u32 qicr;
	u32 qivec;
	u32 qripnr;
	u32 qipnr;
	u32 qipxcc;
	u32 qipycc;
	u32 qipwcc;
	u32 qipzcc;
	u32 qimr;
	u32 qrimr;
	u32 qicnr;
	u8 res0[0x4];
	u32 qiprta;
	u32 qiprtb;
	u8 res1[0x4];
	u32 qricr;
	u8 res2[0x20];
	u32 qhivec;
	u8 res3[0x1C];
} __attribute__ ((packed)) qe_ic_t;

/* Communications Processor */
typedef struct cp_qe {
	u32 cecr;		/* QE command register */
	u32 ceccr;		/* QE controller configuration register */
	u32 cecdr;		/* QE command data register */
	u8 res0[0xA];
	u16 ceter;		/* QE timer event register */
	u8 res1[0x2];
	u16 cetmr;		/* QE timers mask register */
	u32 cetscr;		/* QE time-stamp timer control register */
	u32 cetsr1;		/* QE time-stamp register 1 */
	u32 cetsr2;		/* QE time-stamp register 2 */
	u8 res2[0x8];
	u32 cevter;		/* QE virtual tasks event register */
	u32 cevtmr;		/* QE virtual tasks mask register */
	u16 cercr;		/* QE RAM control register */
	u8 res3[0x2];
	u8 res4[0x24];
	u16 ceexe1;		/* QE external request 1 event register */
	u8 res5[0x2];
	u16 ceexm1;		/* QE external request 1 mask register */
	u8 res6[0x2];
	u16 ceexe2;		/* QE external request 2 event register */
	u8 res7[0x2];
	u16 ceexm2;		/* QE external request 2 mask register */
	u8 res8[0x2];
	u16 ceexe3;		/* QE external request 3 event register */
	u8 res9[0x2];
	u16 ceexm3;		/* QE external request 3 mask register */
	u8 res10[0x2];
	u16 ceexe4;		/* QE external request 4 event register */
	u8 res11[0x2];
	u16 ceexm4;		/* QE external request 4 mask register */
	u8 res12[0x2];
	u8 res13[0x280];
} __attribute__ ((packed)) cp_qe_t;

/* QE Multiplexer */
typedef struct qe_mux {
	u32 cmxgcr;		/* CMX general clock route register    */
	u32 cmxsi1cr_l;		/* CMX SI1 clock route low register    */
	u32 cmxsi1cr_h;		/* CMX SI1 clock route high register   */
	u32 cmxsi1syr;		/* CMX SI1 SYNC route register         */
	u32 cmxucr1;		/* CMX UCC1, UCC3 clock route register */
	u32 cmxucr2;		/* CMX UCC5, UCC7 clock route register */
	u32 cmxucr3;		/* CMX UCC2, UCC4 clock route register */
	u32 cmxucr4;		/* CMX UCC6, UCC8 clock route register */
	u32 cmxupcr;		/* CMX UPC clock route register        */
	u8 res0[0x1C];
} __attribute__ ((packed)) qe_mux_t;

/* QE Timers */
typedef struct qe_timers {
	u8 gtcfr1;		/* Timer 1 2 global configuration register */
	u8 res0[0x3];
	u8 gtcfr2;		/* Timer 3 4 global configuration register */
	u8 res1[0xB];
	u16 gtmdr1;		/* Timer 1 mode register */
	u16 gtmdr2;		/* Timer 2 mode register */
	u16 gtrfr1;		/* Timer 1 reference register */
	u16 gtrfr2;		/* Timer 2 reference register */
	u16 gtcpr1;		/* Timer 1 capture register */
	u16 gtcpr2;		/* Timer 2 capture register */
	u16 gtcnr1;		/* Timer 1 counter */
	u16 gtcnr2;		/* Timer 2 counter */
	u16 gtmdr3;		/* Timer 3 mode register */
	u16 gtmdr4;		/* Timer 4 mode register */
	u16 gtrfr3;		/* Timer 3 reference register */
	u16 gtrfr4;		/* Timer 4 reference register */
	u16 gtcpr3;		/* Timer 3 capture register */
	u16 gtcpr4;		/* Timer 4 capture register */
	u16 gtcnr3;		/* Timer 3 counter */
	u16 gtcnr4;		/* Timer 4 counter */
	u16 gtevr1;		/* Timer 1 event register */
	u16 gtevr2;		/* Timer 2 event register */
	u16 gtevr3;		/* Timer 3 event register */
	u16 gtevr4;		/* Timer 4 event register */
	u16 gtps;		/* Timer 1 prescale register */
	u8 res2[0x46];
} __attribute__ ((packed)) qe_timers_t;

/* BRG */
typedef struct qe_brg {
	u32 brgc1;		/* BRG1 configuration register  */
	u32 brgc2;		/* BRG2 configuration register  */
	u32 brgc3;		/* BRG3 configuration register  */
	u32 brgc4;		/* BRG4 configuration register  */
	u32 brgc5;		/* BRG5 configuration register  */
	u32 brgc6;		/* BRG6 configuration register  */
	u32 brgc7;		/* BRG7 configuration register  */
	u32 brgc8;		/* BRG8 configuration register  */
	u32 brgc9;		/* BRG9 configuration register  */
	u32 brgc10;		/* BRG10 configuration register */
	u32 brgc11;		/* BRG11 configuration register */
	u32 brgc12;		/* BRG12 configuration register */
	u32 brgc13;		/* BRG13 configuration register */
	u32 brgc14;		/* BRG14 configuration register */
	u32 brgc15;		/* BRG15 configuration register */
	u32 brgc16;		/* BRG16 configuration register */
	u8 res0[0x40];
} __attribute__ ((packed)) qe_brg_t;

/* SPI */
typedef struct spi {
	u8 res0[0x20];
	u32 spmode;		/* SPI mode register */
	u8 res1[0x2];
	u8 spie;		/* SPI event register */
	u8 res2[0x1];
	u8 res3[0x2];
	u8 spim;		/* SPI mask register */
	u8 res4[0x1];
	u8 res5[0x1];
	u8 spcom;		/* SPI command register  */
	u8 res6[0x2];
	u32 spitd;		/* SPI transmit data register (cpu mode) */
	u32 spird;		/* SPI receive data register (cpu mode) */
	u8 res7[0x8];
} __attribute__ ((packed)) spi_t;

/* SI */
typedef struct si1 {
	u16 siamr1;		/* SI1 TDMA mode register */
	u16 sibmr1;		/* SI1 TDMB mode register */
	u16 sicmr1;		/* SI1 TDMC mode register */
	u16 sidmr1;		/* SI1 TDMD mode register */
	u8 siglmr1_h;		/* SI1 global mode register high */
	u8 res0[0x1];
	u8 sicmdr1_h;		/* SI1 command register high */
	u8 res2[0x1];
	u8 sistr1_h;		/* SI1 status register high */
	u8 res3[0x1];
	u16 sirsr1_h;		/* SI1 RAM shadow address register high */
	u8 sitarc1;		/* SI1 RAM counter Tx TDMA */
	u8 sitbrc1;		/* SI1 RAM counter Tx TDMB */
	u8 sitcrc1;		/* SI1 RAM counter Tx TDMC */
	u8 sitdrc1;		/* SI1 RAM counter Tx TDMD */
	u8 sirarc1;		/* SI1 RAM counter Rx TDMA */
	u8 sirbrc1;		/* SI1 RAM counter Rx TDMB */
	u8 sircrc1;		/* SI1 RAM counter Rx TDMC */
	u8 sirdrc1;		/* SI1 RAM counter Rx TDMD */
	u8 res4[0x8];
	u16 siemr1;		/* SI1 TDME mode register 16 bits */
	u16 sifmr1;		/* SI1 TDMF mode register 16 bits */
	u16 sigmr1;		/* SI1 TDMG mode register 16 bits */
	u16 sihmr1;		/* SI1 TDMH mode register 16 bits */
	u8 siglmg1_l;		/* SI1 global mode register low 8 bits */
	u8 res5[0x1];
	u8 sicmdr1_l;		/* SI1 command register low 8 bits */
	u8 res6[0x1];
	u8 sistr1_l;		/* SI1 status register low 8 bits */
	u8 res7[0x1];
	u16 sirsr1_l;		/* SI1 RAM shadow address register low 16 bits */
	u8 siterc1;		/* SI1 RAM counter Tx TDME 8 bits */
	u8 sitfrc1;		/* SI1 RAM counter Tx TDMF 8 bits */
	u8 sitgrc1;		/* SI1 RAM counter Tx TDMG 8 bits */
	u8 sithrc1;		/* SI1 RAM counter Tx TDMH 8 bits */
	u8 sirerc1;		/* SI1 RAM counter Rx TDME 8 bits */
	u8 sirfrc1;		/* SI1 RAM counter Rx TDMF 8 bits */
	u8 sirgrc1;		/* SI1 RAM counter Rx TDMG 8 bits */
	u8 sirhrc1;		/* SI1 RAM counter Rx TDMH 8 bits */
	u8 res8[0x8];
	u32 siml1;		/* SI1 multiframe limit register */
	u8 siedm1;		/* SI1 extended diagnostic mode register */
	u8 res9[0xBB];
} __attribute__ ((packed)) si1_t;

/* SI Routing Tables */
typedef struct sir {
	u8 tx[0x400];
	u8 rx[0x400];
	u8 res0[0x800];
} __attribute__ ((packed)) sir_t;

/* USB Controller.  */
typedef struct usb_ctlr {
	u8 usb_usmod;
	u8 usb_usadr;
	u8 usb_uscom;
	u8 res1[1];
	u16 usb_usep1;
	u16 usb_usep2;
	u16 usb_usep3;
	u16 usb_usep4;
	u8 res2[4];
	u16 usb_usber;
	u8 res3[2];
	u16 usb_usbmr;
	u8 res4[1];
	u8 usb_usbs;
	u16 usb_ussft;
	u8 res5[2];
	u16 usb_usfrn;
	u8 res6[0x22];
} __attribute__ ((packed)) usb_t;

/* MCC */
typedef struct mcc {
	u32 mcce;		/* MCC event register */
	u32 mccm;		/* MCC mask register */
	u32 mccf;		/* MCC configuration register */
	u32 merl;		/* MCC emergency request level register */
	u8 res0[0xF0];
} __attribute__ ((packed)) mcc_t;

/* QE UCC Slow */
typedef struct ucc_slow {
	u32 gumr_l;		/* UCCx general mode register (low) */
	u32 gumr_h;		/* UCCx general mode register (high) */
	u16 upsmr;		/* UCCx protocol-specific mode register */
	u8 res0[0x2];
	u16 utodr;		/* UCCx transmit on demand register */
	u16 udsr;		/* UCCx data synchronization register */
	u16 ucce;		/* UCCx event register */
	u8 res1[0x2];
	u16 uccm;		/* UCCx mask register */
	u8 res2[0x1];
	u8 uccs;		/* UCCx status register */
	u8 res3[0x24];
	u16 utpt;
	u8 guemr;		/* UCC general extended mode register */
	u8 res4[0x200 - 0x091];
} __attribute__ ((packed)) ucc_slow_t;

typedef struct ucc_mii_mng {
	u32 miimcfg;		/* MII management configuration reg    */
	u32 miimcom;		/* MII management command reg          */
	u32 miimadd;		/* MII management address reg          */
	u32 miimcon;		/* MII management control reg          */
	u32 miimstat;		/* MII management status reg           */
	u32 miimind;		/* MII management indication reg       */
	u32 ifctl;		/* interface control reg               */
	u32 ifstat;		/* interface statux reg                */
} __attribute__ ((packed))uec_mii_t;

typedef struct ucc_ethernet {
	u32 maccfg1;		/* mac configuration reg. 1            */
	u32 maccfg2;		/* mac configuration reg. 2            */
	u32 ipgifg;		/* interframe gap reg.                 */
	u32 hafdup;		/* half-duplex reg.                    */
	u8 res1[0x10];
	u32 miimcfg;		/* MII management configuration reg    */
	u32 miimcom;		/* MII management command reg          */
	u32 miimadd;		/* MII management address reg          */
	u32 miimcon;		/* MII management control reg          */
	u32 miimstat;		/* MII management status reg           */
	u32 miimind;		/* MII management indication reg       */
	u32 ifctl;		/* interface control reg               */
	u32 ifstat;		/* interface statux reg                */
	u32 macstnaddr1;	/* mac station address part 1 reg      */
	u32 macstnaddr2;	/* mac station address part 2 reg      */
	u8 res2[0x8];
	u32 uempr;		/* UCC Ethernet Mac parameter reg      */
	u32 utbipar;		/* UCC tbi address reg                 */
	u16 uescr;		/* UCC Ethernet statistics control reg */
	u8 res3[0x180 - 0x15A];
	u32 tx64;		/* Total number of frames (including bad
				 * frames) transmitted that were exactly
				 * of the minimal length (64 for un tagged,
				 * 68 for tagged, or with length exactly
				 * equal to the parameter MINLength */
	u32 tx127;		/* Total number of frames (including bad
				 * frames) transmitted that were between
				 * MINLength (Including FCS length==4)
				 * and 127 octets */
	u32 tx255;		/* Total number of frames (including bad
				 * frames) transmitted that were between
				 * 128 (Including FCS length==4) and 255
				 * octets */
	u32 rx64;		/* Total number of frames received including
				 * bad frames that were exactly of the
				 * mninimal length (64 bytes) */
	u32 rx127;		/* Total number of frames (including bad
				 * frames) received that were between
				 * MINLength (Including FCS length==4)
				 * and 127 octets */
	u32 rx255;		/* Total number of frames (including
				 * bad frames) received that were between
				 * 128 (Including FCS length==4) and 255
				 * octets */
	u32 txok;		/* Total number of octets residing in frames
				 * that where involved in succesfull
				 * transmission */
	u16 txcf;		/* Total number of PAUSE control frames
				 *  transmitted by this MAC */
	u8 res4[0x2];
	u32 tmca;		/* Total number of frames that were transmitted
				 * succesfully with the group address bit set
				 * that are not broadcast frames */
	u32 tbca;		/* Total number of frames transmitted
				 * succesfully that had destination address
				 * field equal to the broadcast address */
	u32 rxfok;		/* Total number of frames received OK */
	u32 rxbok;		/* Total number of octets received OK */
	u32 rbyt;		/* Total number of octets received including
				 * octets in bad frames. Must be implemented
				 * in HW because it includes octets in frames
				 * that never even reach the UCC */
	u32 rmca;		/* Total number of frames that were received
				 * succesfully with the group address bit set
				 * that are not broadcast frames */
	u32 rbca;		/* Total number of frames received succesfully
				 * that had destination address equal to the
				 * broadcast address */
	u32 scar;		/* Statistics carry register */
	u32 scam;		/* Statistics caryy mask register */
	u8 res5[0x200 - 0x1c4];
} __attribute__ ((packed)) uec_t;

/* QE UCC Fast */
typedef struct ucc_fast {
	u32 gumr;		/* UCCx general mode register */
	u32 upsmr;		/* UCCx protocol-specific mode register  */
	u16 utodr;		/* UCCx transmit on demand register  */
	u8 res0[0x2];
	u16 udsr;		/* UCCx data synchronization register  */
	u8 res1[0x2];
	u32 ucce;		/* UCCx event register */
	u32 uccm;		/* UCCx mask register.  */
	u8 uccs;		/* UCCx status register */
	u8 res2[0x7];
	u32 urfb;		/* UCC receive FIFO base */
	u16 urfs;		/* UCC receive FIFO size */
	u8 res3[0x2];
	u16 urfet;		/* UCC receive FIFO emergency threshold */
	u16 urfset;		/* UCC receive FIFO special emergency
				 * threshold */
	u32 utfb;		/* UCC transmit FIFO base */
	u16 utfs;		/* UCC transmit FIFO size */
	u8 res4[0x2];
	u16 utfet;		/* UCC transmit FIFO emergency threshold */
	u8 res5[0x2];
	u16 utftt;		/* UCC transmit FIFO transmit threshold */
	u8 res6[0x2];
	u16 utpt;		/* UCC transmit polling timer */
	u8 res7[0x2];
	u32 urtry;		/* UCC retry counter register */
	u8 res8[0x4C];
	u8 guemr;		/* UCC general extended mode register */
	u8 res9[0x100 - 0x091];
	uec_t ucc_eth;
} __attribute__ ((packed)) ucc_fast_t;

/* QE UCC */
typedef struct ucc_common {
	u8 res1[0x90];
	u8 guemr;
	u8 res2[0x200 - 0x091];
} __attribute__ ((packed)) ucc_common_t;

typedef struct ucc {
	union {
		ucc_slow_t slow;
		ucc_fast_t fast;
		ucc_common_t common;
	};
} __attribute__ ((packed)) ucc_t;

/* MultiPHY UTOPIA POS Controllers (UPC) */
typedef struct upc {
	u32 upgcr;		/* UTOPIA/POS general configuration register */
	u32 uplpa;		/* UTOPIA/POS last PHY address */
	u32 uphec;		/* ATM HEC register */
	u32 upuc;		/* UTOPIA/POS UCC configuration */
	u32 updc1;		/* UTOPIA/POS device 1 configuration */
	u32 updc2;		/* UTOPIA/POS device 2 configuration  */
	u32 updc3;		/* UTOPIA/POS device 3 configuration */
	u32 updc4;		/* UTOPIA/POS device 4 configuration  */
	u32 upstpa;		/* UTOPIA/POS STPA threshold  */
	u8 res0[0xC];
	u32 updrs1_h;		/* UTOPIA/POS device 1 rate select  */
	u32 updrs1_l;		/* UTOPIA/POS device 1 rate select  */
	u32 updrs2_h;		/* UTOPIA/POS device 2 rate select  */
	u32 updrs2_l;		/* UTOPIA/POS device 2 rate select */
	u32 updrs3_h;		/* UTOPIA/POS device 3 rate select */
	u32 updrs3_l;		/* UTOPIA/POS device 3 rate select */
	u32 updrs4_h;		/* UTOPIA/POS device 4 rate select */
	u32 updrs4_l;		/* UTOPIA/POS device 4 rate select */
	u32 updrp1;		/* UTOPIA/POS device 1 receive priority low  */
	u32 updrp2;		/* UTOPIA/POS device 2 receive priority low  */
	u32 updrp3;		/* UTOPIA/POS device 3 receive priority low  */
	u32 updrp4;		/* UTOPIA/POS device 4 receive priority low  */
	u32 upde1;		/* UTOPIA/POS device 1 event */
	u32 upde2;		/* UTOPIA/POS device 2 event */
	u32 upde3;		/* UTOPIA/POS device 3 event */
	u32 upde4;		/* UTOPIA/POS device 4 event */
	u16 uprp1;
	u16 uprp2;
	u16 uprp3;
	u16 uprp4;
	u8 res1[0x8];
	u16 uptirr1_0;		/* Device 1 transmit internal rate 0 */
	u16 uptirr1_1;		/* Device 1 transmit internal rate 1 */
	u16 uptirr1_2;		/* Device 1 transmit internal rate 2 */
	u16 uptirr1_3;		/* Device 1 transmit internal rate 3 */
	u16 uptirr2_0;		/* Device 2 transmit internal rate 0 */
	u16 uptirr2_1;		/* Device 2 transmit internal rate 1 */
	u16 uptirr2_2;		/* Device 2 transmit internal rate 2 */
	u16 uptirr2_3;		/* Device 2 transmit internal rate 3 */
	u16 uptirr3_0;		/* Device 3 transmit internal rate 0 */
	u16 uptirr3_1;		/* Device 3 transmit internal rate 1 */
	u16 uptirr3_2;		/* Device 3 transmit internal rate 2 */
	u16 uptirr3_3;		/* Device 3 transmit internal rate 3 */
	u16 uptirr4_0;		/* Device 4 transmit internal rate 0 */
	u16 uptirr4_1;		/* Device 4 transmit internal rate 1 */
	u16 uptirr4_2;		/* Device 4 transmit internal rate 2 */
	u16 uptirr4_3;		/* Device 4 transmit internal rate 3 */
	u32 uper1;		/* Device 1 port enable register */
	u32 uper2;		/* Device 2 port enable register */
	u32 uper3;		/* Device 3 port enable register */
	u32 uper4;		/* Device 4 port enable register */
	u8 res2[0x150];
} __attribute__ ((packed)) upc_t;

/* SDMA */
typedef struct sdma {
	u32 sdsr;		/* Serial DMA status register */
	u32 sdmr;		/* Serial DMA mode register */
	u32 sdtr1;		/* SDMA system bus threshold register */
	u32 sdtr2;		/* SDMA secondary bus threshold register */
	u32 sdhy1;		/* SDMA system bus hysteresis register */
	u32 sdhy2;		/* SDMA secondary bus hysteresis register */
	u32 sdta1;		/* SDMA system bus address register */
	u32 sdta2;		/* SDMA secondary bus address register */
	u32 sdtm1;		/* SDMA system bus MSNUM register */
	u32 sdtm2;		/* SDMA secondary bus MSNUM register */
	u8 res0[0x10];
	u32 sdaqr;		/* SDMA address bus qualify register */
	u32 sdaqmr;		/* SDMA address bus qualify mask register */
	u8 res1[0x4];
	u32 sdwbcr;		/* SDMA CAM entries base register */
	u8 res2[0x38];
} __attribute__ ((packed)) sdma_t;

/* Debug Space */
typedef struct dbg {
	u32 bpdcr;		/* Breakpoint debug command register */
	u32 bpdsr;		/* Breakpoint debug status register */
	u32 bpdmr;		/* Breakpoint debug mask register */
	u32 bprmrr0;		/* Breakpoint request mode risc register 0 */
	u32 bprmrr1;		/* Breakpoint request mode risc register 1 */
	u8 res0[0x8];
	u32 bprmtr0;		/* Breakpoint request mode trb register 0 */
	u32 bprmtr1;		/* Breakpoint request mode trb register 1 */
	u8 res1[0x8];
	u32 bprmir;		/* Breakpoint request mode immediate register */
	u32 bprmsr;		/* Breakpoint request mode serial register */
	u32 bpemr;		/* Breakpoint exit mode register */
	u8 res2[0x48];
} __attribute__ ((packed)) dbg_t;

/*
 * RISC Special Registers (Trap and Breakpoint).  These are described in
 * the QE Developer's Handbook.
*/
typedef struct rsp {
	u32 tibcr[16];	/* Trap/instruction breakpoint control regs */
	u8 res0[64];
	u32 ibcr0;
	u32 ibs0;
	u32 ibcnr0;
	u8 res1[4];
	u32 ibcr1;
	u32 ibs1;
	u32 ibcnr1;
	u32 npcr;
	u32 dbcr;
	u32 dbar;
	u32 dbamr;
	u32 dbsr;
	u32 dbcnr;
	u8 res2[12];
	u32 dbdr_h;
	u32 dbdr_l;
	u32 dbdmr_h;
	u32 dbdmr_l;
	u32 bsr;
	u32 bor;
	u32 bior;
	u8 res3[4];
	u32 iatr[4];
	u32 eccr;		/* Exception control configuration register */
	u32 eicr;
	u8 res4[0x100-0xf8];
} __attribute__ ((packed)) rsp_t;

typedef struct qe_immap {
	qe_iram_t iram;		/* I-RAM */
	qe_ic_t ic;		/* Interrupt Controller */
	cp_qe_t cp;		/* Communications Processor */
	qe_mux_t qmx;		/* QE Multiplexer */
	qe_timers_t qet;	/* QE Timers */
	spi_t spi[0x2];		/* spi  */
	mcc_t mcc;		/* mcc */
	qe_brg_t brg;		/* brg */
	usb_t usb;		/* USB */
	si1_t si1;		/* SI */
	u8 res11[0x800];
	sir_t sir;		/* SI Routing Tables  */
	ucc_t ucc1;		/* ucc1 */
	ucc_t ucc3;		/* ucc3 */
	ucc_t ucc5;		/* ucc5 */
	ucc_t ucc7;		/* ucc7 */
	u8 res12[0x600];
	upc_t upc1;		/* MultiPHY UTOPIA POS Controller 1 */
	ucc_t ucc2;		/* ucc2 */
	ucc_t ucc4;		/* ucc4 */
	ucc_t ucc6;		/* ucc6 */
	ucc_t ucc8;		/* ucc8 */
	u8 res13[0x600];
	upc_t upc2;		/* MultiPHY UTOPIA POS Controller 2 */
	sdma_t sdma;		/* SDMA */
	dbg_t dbg;		/* Debug Space */
	rsp_t rsp[0x2];		/* RISC Special Registers
				 * (Trap and Breakpoint) */
	u8 res14[0x300];
	u8 res15[0x3A00];
	u8 res16[0x8000];	/* 0x108000 -  0x110000 */
	u8 muram[QE_MURAM_SIZE];
} __attribute__ ((packed)) qe_map_t;

extern qe_map_t *qe_immr;

#endif				/* __IMMAP_QE_H__ */
