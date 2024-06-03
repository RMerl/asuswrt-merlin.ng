/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2004-2011 Freescale Semiconductor, Inc.
 *
 * MPC83xx Internal Memory Map
 *
 * Contributors:
 *	Dave Liu <daveliu@freescale.com>
 *	Tanya Jiang <tanya.jiang@freescale.com>
 *	Mandy Lavi <mandy.lavi@freescale.com>
 *	Eran Liberty <liberty@freescale.com>
 */
#ifndef __IMMAP_83xx__
#define __IMMAP_83xx__

#include <fsl_immap.h>
#include <asm/types.h>
#include <asm/fsl_i2c.h>
#include <asm/mpc8xxx_spi.h>
#include <asm/fsl_lbc.h>
#include <asm/fsl_dma.h>

/*
 * Local Access Window
 */
typedef struct law83xx {
	u32 bar;		/* LBIU local access window base address register */
	u32 ar;			/* LBIU local access window attribute register */
} law83xx_t;

/*
 * System configuration registers
 */
typedef struct sysconf83xx {
	u32 immrbar;		/* Internal memory map base address register */
	u8 res0[0x04];
	u32 altcbar;		/* Alternate configuration base address register */
	u8 res1[0x14];
	law83xx_t lblaw[4];	/* LBIU local access window */
	u8 res2[0x20];
	law83xx_t pcilaw[2];	/* PCI local access window */
	u8 res3[0x10];
	law83xx_t pcielaw[2];	/* PCI Express local access window */
	u8 res4[0x10];
	law83xx_t ddrlaw[2];	/* DDR local access window */
	u8 res5[0x50];
	u32 sgprl;		/* System General Purpose Register Low */
	u32 sgprh;		/* System General Purpose Register High */
	u32 spridr;		/* System Part and Revision ID Register */
	u8 res6[0x04];
	u32 spcr;		/* System Priority Configuration Register */
	u32 sicrl;		/* System I/O Configuration Register Low */
	u32 sicrh;		/* System I/O Configuration Register High */
	u8 res7[0x04];
	u32 sidcr0;		/* System I/O Delay Configuration Register 0 */
	u32 sidcr1;		/* System I/O Delay Configuration Register 1 */
	u32 ddrcdr;		/* DDR Control Driver Register */
	u32 ddrdsr;		/* DDR Debug Status Register */
	u32 obir;		/* Output Buffer Impedance Register */
	u8 res8[0xC];
	u32 pecr1;		/* PCI Express control register 1 */
#if defined(CONFIG_ARCH_MPC830X)
	u32 sdhccr;		/* eSDHC Control Registers for MPC830x */
#else
	u32 pecr2;		/* PCI Express control register 2 */
#endif
#if defined(CONFIG_ARCH_MPC8309)
	u32 can_dbg_ctrl;
	u32 res9a;
	u32 gpr1;
	u8 res9b[0xAC];
#else
	u8 res9[0xB8];
#endif
} sysconf83xx_t;

/*
 * Watch Dog Timer (WDT) Registers
 */
typedef struct wdt83xx {
	u8 res0[4];
	u32 swcrr;		/* System watchdog control register */
	u32 swcnr;		/* System watchdog count register */
	u8 res1[2];
	u16 swsrr;		/* System watchdog service register */
	u8 res2[0xF0];
} wdt83xx_t;

/*
 * RTC/PIT Module Registers
 */
typedef struct rtclk83xx {
	u32 cnr;		/* control register */
	u32 ldr;		/* load register */
	u32 psr;		/* prescale register */
	u32 ctr;		/* counter value field register */
	u32 evr;		/* event register */
	u32 alr;		/* alarm register */
	u8 res0[0xE8];
} rtclk83xx_t;

/*
 * Global timer module
 */
typedef struct gtm83xx {
	u8 cfr1;		/* Timer1/2 Configuration */
	u8 res0[3];
	u8 cfr2;		/* Timer3/4 Configuration */
	u8 res1[11];
	u16 mdr1;		/* Timer1 Mode Register */
	u16 mdr2;		/* Timer2 Mode Register */
	u16 rfr1;		/* Timer1 Reference Register */
	u16 rfr2;		/* Timer2 Reference Register */
	u16 cpr1;		/* Timer1 Capture Register */
	u16 cpr2;		/* Timer2 Capture Register */
	u16 cnr1;		/* Timer1 Counter Register */
	u16 cnr2;		/* Timer2 Counter Register */
	u16 mdr3;		/* Timer3 Mode Register */
	u16 mdr4;		/* Timer4 Mode Register */
	u16 rfr3;		/* Timer3 Reference Register */
	u16 rfr4;		/* Timer4 Reference Register */
	u16 cpr3;		/* Timer3 Capture Register */
	u16 cpr4;		/* Timer4 Capture Register */
	u16 cnr3;		/* Timer3 Counter Register */
	u16 cnr4;		/* Timer4 Counter Register */
	u16 evr1;		/* Timer1 Event Register */
	u16 evr2;		/* Timer2 Event Register */
	u16 evr3;		/* Timer3 Event Register */
	u16 evr4;		/* Timer4 Event Register */
	u16 psr1;		/* Timer1 Prescaler Register */
	u16 psr2;		/* Timer2 Prescaler Register */
	u16 psr3;		/* Timer3 Prescaler Register */
	u16 psr4;		/* Timer4 Prescaler Register */
	u8 res[0xC0];
} gtm83xx_t;

/*
 * Integrated Programmable Interrupt Controller
 */
typedef struct ipic83xx {
	u32 sicfr;		/* System Global Interrupt Configuration Register */
	u32 sivcr;		/* System Global Interrupt Vector Register */
	u32 sipnr_h;		/* System Internal Interrupt Pending Register - High */
	u32 sipnr_l;		/* System Internal Interrupt Pending Register - Low */
	u32 siprr_a;		/* System Internal Interrupt Group A Priority Register */
	u32 siprr_b;		/* System Internal Interrupt Group B Priority Register */
	u32 siprr_c;		/* System Internal Interrupt Group C Priority Register */
	u32 siprr_d;		/* System Internal Interrupt Group D Priority Register */
	u32 simsr_h;		/* System Internal Interrupt Mask Register - High */
	u32 simsr_l;		/* System Internal Interrupt Mask Register - Low */
	u32 sicnr;		/* System Internal Interrupt Control Register */
	u32 sepnr;		/* System External Interrupt Pending Register */
	u32 smprr_a;		/* System Mixed Interrupt Group A Priority Register */
	u32 smprr_b;		/* System Mixed Interrupt Group B Priority Register */
	u32 semsr;		/* System External Interrupt Mask Register */
	u32 secnr;		/* System External Interrupt Control Register */
	u32 sersr;		/* System Error Status Register */
	u32 sermr;		/* System Error Mask Register */
	u32 sercr;		/* System Error Control Register */
	u32 sepcr;		/* System External Interrupt Polarity Control Register */
	u32 sifcr_h;		/* System Internal Interrupt Force Register - High */
	u32 sifcr_l;		/* System Internal Interrupt Force Register - Low */
	u32 sefcr;		/* System External Interrupt Force Register */
	u32 serfr;		/* System Error Force Register */
	u32 scvcr;		/* System Critical Interrupt Vector Register */
	u32 smvcr;		/* System Management Interrupt Vector Register */
	u8 res[0x98];
} ipic83xx_t;

/*
 * System Arbiter Registers
 */
typedef struct arbiter83xx {
	u32 acr;		/* Arbiter Configuration Register */
	u32 atr;		/* Arbiter Timers Register */
	u8 res[4];
	u32 aer;		/* Arbiter Event Register */
	u32 aidr;		/* Arbiter Interrupt Definition Register */
	u32 amr;		/* Arbiter Mask Register */
	u32 aeatr;		/* Arbiter Event Attributes Register */
	u32 aeadr;		/* Arbiter Event Address Register */
	u32 aerr;		/* Arbiter Event Response Register */
	u8 res1[0xDC];
} arbiter83xx_t;

/*
 * Reset Module
 */
typedef struct reset83xx {
	u32 rcwl;		/* Reset Configuration Word Low Register */
	u32 rcwh;		/* Reset Configuration Word High Register */
	u8 res0[8];
	u32 rsr;		/* Reset Status Register */
	u32 rmr;		/* Reset Mode Register */
	u32 rpr;		/* Reset protection Register */
	u32 rcr;		/* Reset Control Register */
	u32 rcer;		/* Reset Control Enable Register */
	u8 res1[0xDC];
} reset83xx_t;

/*
 * Clock Module
 */
typedef struct clk83xx {
	u32 spmr;		/* system PLL mode Register */
	u32 occr;		/* output clock control Register */
	u32 sccr;		/* system clock control Register */
	u8 res0[0xF4];
} clk83xx_t;

/*
 * Power Management Control Module
 */
typedef struct pmc83xx {
	u32 pmccr;		/* PMC Configuration Register */
	u32 pmcer;		/* PMC Event Register */
	u32 pmcmr;		/* PMC Mask Register */
	u32 pmccr1;		/* PMC Configuration Register 1 */
	u32 pmccr2;		/* PMC Configuration Register 2 */
	u8 res0[0xEC];
} pmc83xx_t;

/*
 * General purpose I/O module
 */
typedef struct gpio83xx {
	u32 dir;		/* direction register */
	u32 odr;		/* open drain register */
	u32 dat;		/* data register */
	u32 ier;		/* interrupt event register */
	u32 imr;		/* interrupt mask register */
	u32 icr;		/* external interrupt control register */
	u8 res0[0xE8];
} gpio83xx_t;

/*
 * QE Ports Interrupts Registers
 */
typedef struct qepi83xx {
	u8 res0[0xC];
	u32 qepier;		/* QE Ports Interrupt Event Register */
	u32 qepimr;		/* QE Ports Interrupt Mask Register */
	u32 qepicr;		/* QE Ports Interrupt Control Register */
	u8 res1[0xE8];
} qepi83xx_t;

/*
 * QE Parallel I/O Ports
 */
typedef struct gpio_n {
	u32 podr;		/* Open Drain Register */
	u32 pdat;		/* Data Register */
	u32 dir1;		/* direction register 1 */
	u32 dir2;		/* direction register 2 */
	u32 ppar1;		/* Pin Assignment Register 1 */
	u32 ppar2;		/* Pin Assignment Register 2 */
} gpio_n_t;

typedef struct qegpio83xx {
	gpio_n_t ioport[0x7];
	u8 res0[0x358];
} qepio83xx_t;

/*
 * QE Secondary Bus Access Windows
 */
typedef struct qesba83xx {
	u32 lbmcsar;		/* Local bus memory controller start address */
	u32 sdmcsar;		/* Secondary DDR memory controller start address */
	u8 res0[0x38];
	u32 lbmcear;		/* Local bus memory controller end address */
	u32 sdmcear;		/* Secondary DDR memory controller end address */
	u8 res1[0x38];
	u32 lbmcar;		/* Local bus memory controller attributes */
	u32 sdmcar;		/* Secondary DDR memory controller attributes */
	u8 res2[0x378];
} qesba83xx_t;

/*
 * DDR Memory Controller Memory Map for DDR1
 * The structure of DDR2, or DDR3 is defined in fsl_immap.h
 */
#if !defined(CONFIG_SYS_FSL_DDR2) && !defined(CONFIG_SYS_FSL_DDR3)
typedef struct ddr_cs_bnds {
	u32 csbnds;
	u8 res0[4];
} ddr_cs_bnds_t;

typedef struct ddr83xx {
	ddr_cs_bnds_t csbnds[4];/* Chip Select x Memory Bounds */
	u8 res0[0x60];
	u32 cs_config[4];	/* Chip Select x Configuration */
	u8 res1[0x70];
	u32 timing_cfg_3;	/* SDRAM Timing Configuration 3 */
	u32 timing_cfg_0;	/* SDRAM Timing Configuration 0 */
	u32 timing_cfg_1;	/* SDRAM Timing Configuration 1 */
	u32 timing_cfg_2;	/* SDRAM Timing Configuration 2 */
	u32 sdram_cfg;		/* SDRAM Control Configuration */
	u32 sdram_cfg2;		/* SDRAM Control Configuration 2 */
	u32 sdram_mode;		/* SDRAM Mode Configuration */
	u32 sdram_mode2;	/* SDRAM Mode Configuration 2 */
	u32 sdram_md_cntl;	/* SDRAM Mode Control */
	u32 sdram_interval;	/* SDRAM Interval Configuration */
	u32 ddr_data_init;	/* SDRAM Data Initialization */
	u8 res2[4];
	u32 sdram_clk_cntl;	/* SDRAM Clock Control */
	u8 res3[0x14];
	u32 ddr_init_addr;	/* DDR training initialization address */
	u32 ddr_init_ext_addr;	/* DDR training initialization extended address */
	u8 res4[0xAA8];
	u32 ddr_ip_rev1;	/* DDR IP block revision 1 */
	u32 ddr_ip_rev2;	/* DDR IP block revision 2 */
	u8 res5[0x200];
	u32 data_err_inject_hi;	/* Memory Data Path Error Injection Mask High */
	u32 data_err_inject_lo;	/* Memory Data Path Error Injection Mask Low */
	u32 ecc_err_inject;	/* Memory Data Path Error Injection Mask ECC */
	u8 res6[0x14];
	u32 capture_data_hi;	/* Memory Data Path Read Capture High */
	u32 capture_data_lo;	/* Memory Data Path Read Capture Low */
	u32 capture_ecc;	/* Memory Data Path Read Capture ECC */
	u8 res7[0x14];
	u32 err_detect;		/* Memory Error Detect */
	u32 err_disable;	/* Memory Error Disable */
	u32 err_int_en;		/* Memory Error Interrupt Enable */
	u32 capture_attributes;	/* Memory Error Attributes Capture */
	u32 capture_address;	/* Memory Error Address Capture */
	u32 capture_ext_address;/* Memory Error Extended Address Capture */
	u32 err_sbe;		/* Memory Single-Bit ECC Error Management */
	u8 res8[0xA4];
	u32 debug_reg;
	u8 res9[0xFC];
} ddr83xx_t;
#endif

/*
 * DUART
 */
typedef struct duart83xx {
	u8 urbr_ulcr_udlb;	/* combined register for URBR, UTHR and UDLB */
	u8 uier_udmb;		/* combined register for UIER and UDMB */
	u8 uiir_ufcr_uafr;	/* combined register for UIIR, UFCR and UAFR */
	u8 ulcr;		/* line control register */
	u8 umcr;		/* MODEM control register */
	u8 ulsr;		/* line status register */
	u8 umsr;		/* MODEM status register */
	u8 uscr;		/* scratch register */
	u8 res0[8];
	u8 udsr;		/* DMA status register */
	u8 res1[3];
	u8 res2[0xEC];
} duart83xx_t;

/*
 * DMA/Messaging Unit
 */
typedef struct dma83xx {
	u32 res0[0xC];		/* 0x0-0x29 reseverd */
	u32 omisr;		/* 0x30 Outbound message interrupt status register */
	u32 omimr;		/* 0x34 Outbound message interrupt mask register */
	u32 res1[0x6];		/* 0x38-0x49 reserved */
	u32 imr0;		/* 0x50 Inbound message register 0 */
	u32 imr1;		/* 0x54 Inbound message register 1 */
	u32 omr0;		/* 0x58 Outbound message register 0 */
	u32 omr1;		/* 0x5C Outbound message register 1 */
	u32 odr;		/* 0x60 Outbound doorbell register */
	u32 res2;		/* 0x64-0x67 reserved */
	u32 idr;		/* 0x68 Inbound doorbell register */
	u32 res3[0x5];		/* 0x6C-0x79 reserved */
	u32 imisr;		/* 0x80 Inbound message interrupt status register */
	u32 imimr;		/* 0x84 Inbound message interrupt mask register */
	u32 res4[0x1E];		/* 0x88-0x99 reserved */
	struct fsl_dma dma[4];
} dma83xx_t;

/*
 * PCI Software Configuration Registers
 */
typedef struct pciconf83xx {
	u32 config_address;
	u32 config_data;
	u32 int_ack;
	u8 res[116];
} pciconf83xx_t;

/*
 * PCI Outbound Translation Register
 */
typedef struct pci_outbound_window {
	u32 potar;
	u8 res0[4];
	u32 pobar;
	u8 res1[4];
	u32 pocmr;
	u8 res2[4];
} pot83xx_t;

/*
 * Sequencer
 */
typedef struct ios83xx {
	pot83xx_t pot[6];
	u8 res0[0x60];
	u32 pmcr;
	u8 res1[4];
	u32 dtcr;
	u8 res2[4];
} ios83xx_t;

/*
 * PCI Controller Control and Status Registers
 */
typedef struct pcictrl83xx {
	u32 esr;
	u32 ecdr;
	u32 eer;
	u32 eatcr;
	u32 eacr;
	u32 eeacr;
	u32 edlcr;
	u32 edhcr;
	u32 gcr;
	u32 ecr;
	u32 gsr;
	u8 res0[12];
	u32 pitar2;
	u8 res1[4];
	u32 pibar2;
	u32 piebar2;
	u32 piwar2;
	u8 res2[4];
	u32 pitar1;
	u8 res3[4];
	u32 pibar1;
	u32 piebar1;
	u32 piwar1;
	u8 res4[4];
	u32 pitar0;
	u8 res5[4];
	u32 pibar0;
	u8 res6[4];
	u32 piwar0;
	u8 res7[132];
} pcictrl83xx_t;

/*
 * USB
 */
typedef struct usb83xx {
	u8 fixme[0x1000];
} usb83xx_t;

/*
 * TSEC
 */
typedef struct tsec83xx {
	u8 fixme[0x1000];
} tsec83xx_t;

/*
 * Security
 */
typedef struct security83xx {
	u8 fixme[0x10000];
} security83xx_t;

/*
 *  PCI Express
 */
struct pex_inbound_window {
	u32 ar;
	u32 tar;
	u32 barl;
	u32 barh;
};

struct pex_outbound_window {
	u32 ar;
	u32 bar;
	u32 tarl;
	u32 tarh;
};

struct pex_csb_bridge {
	u32 pex_csb_ver;
	u32 pex_csb_cab;
	u32 pex_csb_ctrl;
	u8 res0[8];
	u32 pex_dms_dstmr;
	u8 res1[4];
	u32 pex_cbs_stat;
	u8 res2[0x20];
	u32 pex_csb_obctrl;
	u32 pex_csb_obstat;
	u8 res3[0x98];
	u32 pex_csb_ibctrl;
	u32 pex_csb_ibstat;
	u8 res4[0xb8];
	u32 pex_wdma_ctrl;
	u32 pex_wdma_addr;
	u32 pex_wdma_stat;
	u8 res5[0x94];
	u32 pex_rdma_ctrl;
	u32 pex_rdma_addr;
	u32 pex_rdma_stat;
	u8 res6[0xd4];
	u32 pex_ombcr;
	u32 pex_ombdr;
	u8 res7[0x38];
	u32 pex_imbcr;
	u32 pex_imbdr;
	u8 res8[0x38];
	u32 pex_int_enb;
	u32 pex_int_stat;
	u32 pex_int_apio_vec1;
	u32 pex_int_apio_vec2;
	u8 res9[0x10];
	u32 pex_int_ppio_vec1;
	u32 pex_int_ppio_vec2;
	u32 pex_int_wdma_vec1;
	u32 pex_int_wdma_vec2;
	u32 pex_int_rdma_vec1;
	u32 pex_int_rdma_vec2;
	u32 pex_int_misc_vec;
	u8 res10[4];
	u32 pex_int_axi_pio_enb;
	u32 pex_int_axi_wdma_enb;
	u32 pex_int_axi_rdma_enb;
	u32 pex_int_axi_misc_enb;
	u32 pex_int_axi_pio_stat;
	u32 pex_int_axi_wdma_stat;
	u32 pex_int_axi_rdma_stat;
	u32 pex_int_axi_misc_stat;
	u8 res11[0xa0];
	struct pex_outbound_window pex_outbound_win[4];
	u8 res12[0x100];
	u32 pex_epiwtar0;
	u32 pex_epiwtar1;
	u32 pex_epiwtar2;
	u32 pex_epiwtar3;
	u8 res13[0x70];
	struct pex_inbound_window pex_inbound_win[4];
};

typedef struct pex83xx {
	u8 pex_cfg_header[0x404];
	u32 pex_ltssm_stat;
	u8 res0[0x30];
	u32 pex_ack_replay_timeout;
	u8 res1[4];
	u32 pex_gclk_ratio;
	u8 res2[0xc];
	u32 pex_pm_timer;
	u32 pex_pme_timeout;
	u8 res3[4];
	u32 pex_aspm_req_timer;
	u8 res4[0x18];
	u32 pex_ssvid_update;
	u8 res5[0x34];
	u32 pex_cfg_ready;
	u8 res6[0x24];
	u32 pex_bar_sizel;
	u8 res7[4];
	u32 pex_bar_sel;
	u8 res8[0x20];
	u32 pex_bar_pf;
	u8 res9[0x88];
	u32 pex_pme_to_ack_tor;
	u8 res10[0xc];
	u32 pex_ss_intr_mask;
	u8 res11[0x25c];
	struct pex_csb_bridge bridge;
	u8 res12[0x160];
} pex83xx_t;

/*
 * SATA
 */
typedef struct sata83xx {
	u8 fixme[0x1000];
} sata83xx_t;

/*
 * eSDHC
 */
typedef struct sdhc83xx {
	u8 fixme[0x1000];
} sdhc83xx_t;

/*
 * SerDes
 */
typedef struct serdes83xx {
	u32 srdscr0;
	u32 srdscr1;
	u32 srdscr2;
	u32 srdscr3;
	u32 srdscr4;
	u8 res0[0xc];
	u32 srdsrstctl;
	u8 res1[0xdc];
} serdes83xx_t;

/*
 * On Chip ROM
 */
typedef struct rom83xx {
#if defined(CONFIG_ARCH_MPC8309)
	u8 mem[0x8000];
#else
	u8 mem[0x10000];
#endif
} rom83xx_t;

/*
 * TDM
 */
typedef struct tdm83xx {
	u8 fixme[0x200];
} tdm83xx_t;

/*
 * TDM DMAC
 */
typedef struct tdmdmac83xx {
	u8 fixme[0x2000];
} tdmdmac83xx_t;

#if defined(CONFIG_ARCH_MPC834X)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[2];	/* General purpose I/O module */
	u8			res0[0x200];
	u8			dll_ddr[0x100];
	u8			dll_lbc[0x100];
	u8			res1[0xE00];
#if defined(CONFIG_SYS_FSL_DDR2) || defined(CONFIG_SYS_FSL_DDR3)
	struct ccsr_ddr		ddr;	/* DDR Memory Controller Memory */
#else
	ddr83xx_t		ddr;	/* DDR Memory Controller Memory */
#endif
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res2[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res3[0x900];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res4[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[2];	/* PCI Software Configuration Registers */
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[2];	/* PCI Controller Control and Status Registers */
	u8			res5[0x19900];
	usb83xx_t		usb[2];
	tsec83xx_t		tsec[2];
	u8			res6[0xA000];
	security83xx_t		security;
	u8			res7[0xC0000];
} immap_t;

#ifndef	CONFIG_ARCH_MPC834X
#ifdef CONFIG_HAS_FSL_MPH_USB
#define CONFIG_SYS_MPC83xx_USB1_OFFSET  0x22000	/* use the MPH controller */
#define CONFIG_SYS_MPC83xx_USB2_OFFSET	0
#else
#define CONFIG_SYS_MPC83xx_USB1_OFFSET	0
#define CONFIG_SYS_MPC83xx_USB2_OFFSET  0x23000	/* use the DR controller */
#endif
#else
#define CONFIG_SYS_MPC83xx_USB1_OFFSET	0x22000
#define CONFIG_SYS_MPC83xx_USB2_OFFSET  0x23000
#endif

#elif defined(CONFIG_ARCH_MPC8313)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[1];	/* General purpose I/O module */
	u8			res0[0x1300];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res1[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res2[0x900];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res3[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res4[0x80];
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res5[0x1aa00];
	usb83xx_t		usb[1];
	tsec83xx_t		tsec[2];
	u8			res6[0xA000];
	security83xx_t		security;
	u8			res7[0xC0000];
} immap_t;

#elif defined(CONFIG_ARCH_MPC8315)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[1];	/* General purpose I/O module */
	u8			res0[0x1300];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[1];		/* I2C Controllers */
	u8			res1[0x1400];
	duart83xx_t		duart[2];	/* DUART */
	u8			res2[0x900];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res3[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res4[0x80];
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res5[0xa00];
	pex83xx_t		pciexp[2];	/* PCI Express Controller */
	u8			res6[0xb000];
	tdm83xx_t		tdm;		/* TDM Controller */
	u8			res7[0x1e00];
	sata83xx_t		sata[2];	/* SATA Controller */
	u8			res8[0x9000];
	usb83xx_t		usb[1];		/* USB DR Controller */
	tsec83xx_t		tsec[2];
	u8			res9[0x6000];
	tdmdmac83xx_t		tdmdmac;	/* TDM DMAC */
	u8			res10[0x2000];
	security83xx_t		security;
	u8			res11[0xA3000];
	serdes83xx_t		serdes[1];	/* SerDes Registers */
	u8			res12[0x1CF00];
} immap_t;

#elif defined(CONFIG_ARCH_MPC8308)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[1];		/* Global Timers Module */
	u8			res0[0x100];
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[1];	/* General purpose I/O module */
	u8			res1[0x1300];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res2[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res3[0x900];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res4[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	u8			res5[0x1000];
	pex83xx_t		pciexp[1];	/* PCI Express Controller */
	u8			res6[0x19000];
	usb83xx_t		usb[1];		/* USB DR Controller */
	tsec83xx_t		tsec[2];
	u8			res7[0x6000];
	tdmdmac83xx_t		tdmdmac;	/* TDM DMAC */
	sdhc83xx_t		sdhc;		/* SDHC Controller */
	u8			res8[0xb4000];
	serdes83xx_t		serdes[1];	/* SerDes Registers */
	u8			res9[0x1CF00];
} immap_t;

#elif defined(CONFIG_ARCH_MPC837X)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[2];	/* General purpose I/O module */
	u8			res0[0x1200];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res1[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res2[0x900];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res3[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res4[0x80];
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res5[0xa00];
	pex83xx_t		pciexp[2];	/* PCI Express Controller */
	u8			res6[0xd000];
	sata83xx_t		sata[4];	/* SATA Controller */
	u8			res7[0x7000];
	usb83xx_t		usb[1];		/* USB DR Controller */
	tsec83xx_t		tsec[2];
	u8			res8[0x8000];
	sdhc83xx_t		sdhc;		/* SDHC Controller */
	u8			res9[0x1000];
	security83xx_t		security;
	u8			res10[0xA3000];
	serdes83xx_t		serdes[2];	/* SerDes Registers */
	u8			res11[0xCE00];
	rom83xx_t		rom;		/* On Chip ROM */
} immap_t;

#elif defined(CONFIG_ARCH_MPC8360)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	u8			res0[0x200];
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	qepi83xx_t		qepi;		/* QE Ports Interrupts Registers */
	u8			res1[0x300];
	u8			dll_ddr[0x100];
	u8			dll_lbc[0x100];
	u8			res2[0x200];
	qepio83xx_t		qepio;		/* QE Parallel I/O ports */
	qesba83xx_t		qesba;		/* QE Secondary Bus Access Windows */
	u8			res3[0x400];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res4[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res5[0x900];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res6[0x2000];
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res7[128];
	ios83xx_t		ios;		/* Sequencer (IOS) */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res8[0x4A00];
	ddr83xx_t		ddr_secondary;	/* Secondary DDR Memory Controller Memory Map */
	u8			res9[0x22000];
	security83xx_t		security;
	u8			res10[0xC0000];
	u8			qe[0x100000];	/* QE block */
} immap_t;

#elif defined(CONFIG_ARCH_MPC832X)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	qepi83xx_t		qepi;		/* QE Ports Interrupts Registers */
	u8			res0[0x300];
	u8			dll_ddr[0x100];
	u8			dll_lbc[0x100];
	u8			res1[0x200];
	qepio83xx_t		qepio;		/* QE Parallel I/O ports */
	u8			res2[0x800];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res3[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res4[0x900];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res5[0x2000];
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res6[128];
	ios83xx_t		ios;		/* Sequencer (IOS) */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res7[0x27A00];
	security83xx_t		security;
	u8			res8[0xC0000];
	u8			qe[0x100000];	/* QE block */
} immap_t;
#elif defined(CONFIG_ARCH_MPC8309)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[2];	/* General purpose I/O module */
	u8			res0[0x500];	/* res0 1.25 KBytes added for 8309 */
	qepi83xx_t		qepi;		/* QE Ports Interrupts Registers */
	qepio83xx_t		qepio;		/* QE Parallel I/O ports */
	u8			res1[0x800];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res2[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res3[0x200];
	duart83xx_t		duart1[2];	/* DUART */
	u8			res4[0x500];
	fsl_lbc_t		im_lbc;		/* Local Bus Controller Regs */
	u8			res5[0x1000];
	u8			spi[0x100];
	u8			res6[0xf00];
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Configuration Registers */
	u8			res7[0x80];
	ios83xx_t		ios;		/* Sequencer (IOS) */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Control & Status Registers */
	u8			res8[0x13A00];
	u8			can1[0x1000];	/* Flexcan 1 */
	u8			can2[0x1000];	/* Flexcan 2 */
	u8			res9[0x5000];
	usb83xx_t		usb;
	u8			res10[0x5000];
	u8			can3[0x1000];	/* Flexcan 3 */
	u8			can4[0x1000];	/* Flexcan 4 */
	u8			res11[0x1000];
	u8			dma1[0x2000];	/* DMA */
	sdhc83xx_t		sdhc;		/* SDHC Controller */
	u8			res12[0xC1000];
	rom83xx_t		rom;		/* On Chip ROM */
	u8			res13[0x8000];
	u8			qe[0x100000];	/* QE block */
	u8			res14[0xE00000];/* Added for 8309 */
} immap_t;
#endif

#define CONFIG_SYS_MPC8xxx_DDR_OFFSET	(0x2000)
#define CONFIG_SYS_FSL_DDR_ADDR \
			(CONFIG_SYS_IMMR + CONFIG_SYS_MPC8xxx_DDR_OFFSET)
#define CONFIG_SYS_MPC83xx_DMA_OFFSET	(0x8000)
#define CONFIG_SYS_MPC83xx_DMA_ADDR \
			(CONFIG_SYS_IMMR + CONFIG_SYS_MPC83xx_DMA_OFFSET)
#define CONFIG_SYS_MPC83xx_ESDHC_OFFSET	(0x2e000)
#define CONFIG_SYS_MPC83xx_ESDHC_ADDR \
			(CONFIG_SYS_IMMR + CONFIG_SYS_MPC83xx_ESDHC_OFFSET)

#ifndef CONFIG_SYS_MPC83xx_USB1_OFFSET
#define CONFIG_SYS_MPC83xx_USB1_OFFSET  0x23000
#endif
#define CONFIG_SYS_MPC83xx_USB1_ADDR \
			(CONFIG_SYS_IMMR + CONFIG_SYS_MPC83xx_USB1_OFFSET)
#if defined(CONFIG_ARCH_MPC834X)
#define CONFIG_SYS_MPC83xx_USB2_ADDR \
			(CONFIG_SYS_IMMR + CONFIG_SYS_MPC83xx_USB2_OFFSET)
#endif
#define CONFIG_SYS_LBC_ADDR (&((immap_t *)CONFIG_SYS_IMMR)->im_lbc)

#define CONFIG_SYS_TSEC1_OFFSET		0x24000
#define CONFIG_SYS_MDIO1_OFFSET		0x24000

#define TSEC_BASE_ADDR		(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC1_OFFSET)
#define MDIO_BASE_ADDR		(CONFIG_SYS_IMMR + CONFIG_SYS_MDIO1_OFFSET)
#endif				/* __IMMAP_83xx__ */
