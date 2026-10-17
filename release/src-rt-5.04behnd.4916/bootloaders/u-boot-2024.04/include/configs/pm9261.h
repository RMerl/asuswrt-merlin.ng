/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 *
 * Configuation settings for the RONETIX PM9261 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */

#include <asm/hardware.h>
/* ARM asynchronous clock */

#define MASTER_PLL_DIV		15
#define MASTER_PLL_MUL		162
#define MAIN_PLL_DIV		2
#define CFG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CFG_SYS_AT91_MAIN_CLOCK	18432000

/* clocks */
/* CKGR_MOR - enable main osc. */
#define CFG_SYS_MOR_VAL						\
		(AT91_PMC_MOR_MOSCEN |					\
		 (255 << 8))		/* Main Oscillator Start-up Time */
#define CFG_SYS_PLLAR_VAL						\
		(AT91_PMC_PLLAR_29 | /* Bit 29 must be 1 when prog */ \
		 AT91_PMC_PLLXR_OUT(3) |						\
		 ((MASTER_PLL_MUL - 1) << 16) | (MASTER_PLL_DIV))

/* PCK/2 = MCK Master Clock from PLLA */
#define	CFG_SYS_MCKR1_VAL		\
		(AT91_PMC_MCKR_CSS_SLOW |	\
		 AT91_PMC_MCKR_PRES_1 |	\
		 AT91_PMC_MCKR_MDIV_2)

/* PCK/2 = MCK Master Clock from PLLA */
#define	CFG_SYS_MCKR2_VAL		\
		(AT91_PMC_MCKR_CSS_PLLA |	\
		 AT91_PMC_MCKR_PRES_1 |	\
		 AT91_PMC_MCKR_MDIV_2)

/* define PDC[31:16] as DATA[31:16] */
#define CFG_SYS_PIOC_PDR_VAL1	0xFFFF0000
/* no pull-up for D[31:16] */
#define CFG_SYS_PIOC_PPUDR_VAL	0xFFFF0000

/* EBI_CSA, no pull-ups for D[15:0], CS1 SDRAM, CS3 NAND Flash */
#define CFG_SYS_MATRIX_EBICSA_VAL		\
	(AT91_MATRIX_CSA_DBPUC | AT91_MATRIX_CSA_EBI_CS1A)

/* SDRAM */
/* SDRAMC_MR Mode register */
#define CFG_SYS_SDRC_MR_VAL1		AT91_SDRAMC_MODE_NORMAL
/* SDRAMC_TR - Refresh Timer register */
#define CFG_SYS_SDRC_TR_VAL1		0x13C
/* SDRAMC_CR - Configuration register*/
#define CFG_SYS_SDRC_CR_VAL							\
		(AT91_SDRAMC_NC_9 |						\
		 AT91_SDRAMC_NR_13 |						\
		 AT91_SDRAMC_NB_4 |						\
		 AT91_SDRAMC_CAS_3 |						\
		 AT91_SDRAMC_DBW_32 |						\
		 (1 <<  8) |		/* Write Recovery Delay */		\
		 (7 << 12) |		/* Row Cycle Delay */			\
		 (3 << 16) |		/* Row Precharge Delay */		\
		 (2 << 20) |		/* Row to Column Delay */		\
		 (5 << 24) |		/* Active to Precharge Delay */		\
		 (1 << 28))		/* Exit Self Refresh to Active Delay */

/* Memory Device Register -> SDRAM */
#define CFG_SYS_SDRC_MDR_VAL		AT91_SDRAMC_MD_SDRAM
#define CFG_SYS_SDRC_MR_VAL2		AT91_SDRAMC_MODE_PRECHARGE
#define CFG_SYS_SDRAM_VAL1		0		/* SDRAM_BASE */
#define CFG_SYS_SDRC_MR_VAL3		AT91_SDRAMC_MODE_REFRESH
#define CFG_SYS_SDRAM_VAL2		0		/* SDRAM_BASE */
#define CFG_SYS_SDRAM_VAL3		0		/* SDRAM_BASE */
#define CFG_SYS_SDRAM_VAL4		0		/* SDRAM_BASE */
#define CFG_SYS_SDRAM_VAL5		0		/* SDRAM_BASE */
#define CFG_SYS_SDRAM_VAL6		0		/* SDRAM_BASE */
#define CFG_SYS_SDRAM_VAL7		0		/* SDRAM_BASE */
#define CFG_SYS_SDRAM_VAL8		0		/* SDRAM_BASE */
#define CFG_SYS_SDRAM_VAL9		0		/* SDRAM_BASE */
#define CFG_SYS_SDRC_MR_VAL4		AT91_SDRAMC_MODE_LMR
#define CFG_SYS_SDRAM_VAL10		0		/* SDRAM_BASE */
#define CFG_SYS_SDRC_MR_VAL5		AT91_SDRAMC_MODE_NORMAL
#define CFG_SYS_SDRAM_VAL11		0		/* SDRAM_BASE */
#define CFG_SYS_SDRC_TR_VAL2		1200		/* SDRAM_TR */
#define CFG_SYS_SDRAM_VAL12		0		/* SDRAM_BASE */

/* setup SMC0, CS0 (NOR Flash) - 16-bit, 15 WS */
#define CFG_SYS_SMC0_SETUP0_VAL					\
		(AT91_SMC_SETUP_NWE(10) | AT91_SMC_SETUP_NCS_WR(10) |	\
		 AT91_SMC_SETUP_NRD(10) | AT91_SMC_SETUP_NCS_RD(10))
#define CFG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_PULSE_NWE(11) | AT91_SMC_PULSE_NCS_WR(11) |	\
		 AT91_SMC_PULSE_NRD(11) | AT91_SMC_PULSE_NCS_RD(11))
#define CFG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_CYCLE_NWE(22) | AT91_SMC_CYCLE_NRD(22))
#define CFG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |	\
		 AT91_SMC_MODE_DBW_16 |				\
		 AT91_SMC_MODE_TDF |				\
		 AT91_SMC_MODE_TDF_CYCLE(6))

/* user reset enable */
#define CFG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_CR_PROCRST |		\
		AT91_RSTC_MR_ERSTL(1) |	\
		AT91_RSTC_MR_ERSTL(2))

/* Disable Watchdog */
#define CFG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_MR_WDIDLEHLT | AT91_WDT_MR_WDDBGHLT |	\
		 AT91_WDT_MR_WDV(0xfff) |					\
		 AT91_WDT_MR_WDDIS |				\
		 AT91_WDT_MR_WDD(0xfff))

/* SDRAM */
#define PHYS_SDRAM				0x20000000
#define PHYS_SDRAM_SIZE				0x04000000	/* 64 megs */

/* NAND flash */
#define CFG_SYS_NAND_BASE			0x40000000
/* our ALE is AD22 */
#define CFG_SYS_NAND_MASK_ALE		(1 << 22)
/* our CLE is AD21 */
#define CFG_SYS_NAND_MASK_CLE		(1 << 21)
#define CFG_SYS_NAND_ENABLE_PIN		GPIO_PIN_PC(14)
#define CFG_SYS_NAND_READY_PIN		GPIO_PIN_PA(16)

/* NOR flash */
#define PHYS_FLASH_1				0x10000000
#define CFG_SYS_FLASH_BASE			PHYS_FLASH_1

/* USB */
#define CFG_SYS_USB_OHCI_REGS_BASE		0x00500000

#define CFG_EXTRA_ENV_SETTINGS				\
	"partition=nand0,0\0"					\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "		\
		"fbcon=rotate:3 "				\
		"nfsroot=$(serverip):$(rootpath) $(mtdparts)\0"	\
	"addip=setenv bootargs $(bootargs) "			\
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"\
		":$(hostname):eth0:off\0"			\
	"ramboot=tftpboot 0x22000000 vmImage;"			\
		"run ramargs;run addip;bootm 22000000\0"	\
	"nfsboot=tftpboot 0x22000000 vmImage;"			\
		"run nfsargs;run addip;bootm 22000000\0"	\
	"flashboot=run ramargs;run addip;bootm 0x10050000\0"	\
	""

#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM

#endif
