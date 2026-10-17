/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9263EK board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/stringify.h>

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CFG_SYS_AT91_MAIN_CLOCK	16367660 /* 16.367 MHz crystal */
#define CFG_SYS_AT91_SLOW_CLOCK	32768

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CFG_SYS_SDRAM_SIZE		0x04000000

#define CFG_SYS_INIT_RAM_SIZE	(16 * 1024)
#define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM1

/* NOR flash, if populated */
#ifdef CONFIG_SYS_USE_NORFLASH
#define PHYS_FLASH_1				0x10000000
#define CFG_SYS_FLASH_BASE			PHYS_FLASH_1

/* Address and size of Primary Environment Sector */

#define CFG_EXTRA_ENV_SETTINGS	\
	"monitor_base=" __stringify(CONFIG_SYS_MONITOR_BASE) "\0" \
	"update=" \
		"protect off ${monitor_base} +${filesize};" \
		"erase ${monitor_base} +${filesize};" \
		"cp.b ${fileaddr} ${monitor_base} ${filesize};" \
		"protect on ${monitor_base} +${filesize}\0"

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#define MASTER_PLL_MUL		171
#define MASTER_PLL_DIV		14
#define MASTER_PLL_OUT		3

/* clocks */
#define CFG_SYS_MOR_VAL						\
		(AT91_PMC_MOR_MOSCEN | AT91_PMC_MOR_OSCOUNT(255))
#define CFG_SYS_PLLAR_VAL					\
	(AT91_PMC_PLLAR_29 |					\
	AT91_PMC_PLLXR_OUT(MASTER_PLL_OUT) |			\
	AT91_PMC_PLLXR_PLLCOUNT(63) |				\
	AT91_PMC_PLLXR_MUL(MASTER_PLL_MUL - 1) |		\
	AT91_PMC_PLLXR_DIV(MASTER_PLL_DIV))

/* PCK/2 = MCK Master Clock from PLLA */
#define	CFG_SYS_MCKR1_VAL		\
	(AT91_PMC_MCKR_CSS_SLOW | AT91_PMC_MCKR_PRES_1 |	\
	 AT91_PMC_MCKR_MDIV_2)

/* PCK/2 = MCK Master Clock from PLLA */
#define	CFG_SYS_MCKR2_VAL		\
	(AT91_PMC_MCKR_CSS_PLLA | AT91_PMC_MCKR_PRES_1 |	\
	AT91_PMC_MCKR_MDIV_2)

/* define PDC[31:16] as DATA[31:16] */
#define CFG_SYS_PIOD_PDR_VAL1	0xFFFF0000
/* no pull-up for D[31:16] */
#define CFG_SYS_PIOD_PPUDR_VAL	0xFFFF0000
/* EBI0_CSA, CS1 SDRAM, CS3 NAND Flash, 3.3V memories */
#define CFG_SYS_MATRIX_EBICSA_VAL					\
	(AT91_MATRIX_CSA_DBPUC | AT91_MATRIX_CSA_VDDIOMSEL_3_3V |	\
	 AT91_MATRIX_CSA_EBI_CS1A)

/* SDRAM */
/* SDRAMC_MR Mode register */
#define CFG_SYS_SDRC_MR_VAL1		0
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
		 (2 << 16) |		/* Row Precharge Delay */		\
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
#define CFG_SYS_SMC0_SETUP0_VAL				\
	(AT91_SMC_SETUP_NWE(10) | AT91_SMC_SETUP_NCS_WR(10) |	\
	 AT91_SMC_SETUP_NRD(10) | AT91_SMC_SETUP_NCS_RD(10))
#define CFG_SYS_SMC0_PULSE0_VAL				\
	(AT91_SMC_PULSE_NWE(11) | AT91_SMC_PULSE_NCS_WR(11) |	\
	 AT91_SMC_PULSE_NRD(11) | AT91_SMC_PULSE_NCS_RD(11))
#define CFG_SYS_SMC0_CYCLE0_VAL	\
	(AT91_SMC_CYCLE_NWE(22) | AT91_SMC_CYCLE_NRD(22))
#define CFG_SYS_SMC0_MODE0_VAL				\
	(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |		\
	 AT91_SMC_MODE_DBW_16 |					\
	 AT91_SMC_MODE_TDF | AT91_SMC_MODE_TDF_CYCLE(6))

/* user reset enable */
#define CFG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_MR_URSTEN |		\
		AT91_RSTC_MR_ERSTL(15))

/* Disable Watchdog */
#define CFG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_MR_WDIDLEHLT | AT91_WDT_MR_WDDBGHLT |	\
		 AT91_WDT_MR_WDV(0xfff) |			\
		 AT91_WDT_MR_WDDIS |				\
		 AT91_WDT_MR_WDD(0xfff))

#endif
#include <linux/stringify.h>
#endif

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE			ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE		(1 << 22)
#define CFG_SYS_NAND_ENABLE_PIN		AT91_PIN_PD15
#define CFG_SYS_NAND_READY_PIN		AT91_PIN_PA22
#endif

/* USB */
#define CFG_SYS_USB_OHCI_REGS_BASE		0x00a00000	/* AT91SAM9263_UHP_BASE */

#endif
