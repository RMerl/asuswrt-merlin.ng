/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __DDR3_AXP_CONFIG_H
#define __DDR3_AXP_CONFIG_H

/*
 * DDR3_LOG_LEVEL Information
 *
 * Level 0: Provides an error code in a case of failure, RL, WL errors
 *          and other algorithm failure
 * Level 1: Provides the D-Unit setup (SPD/Static configuration)
 * Level 2: Provides the windows margin as a results of DQS centeralization
 * Level 3: Provides the windows margin of each DQ as a results of DQS
 *          centeralization
 */
#ifdef CONFIG_DDR_LOG_LEVEL
#define	DDR3_LOG_LEVEL	CONFIG_DDR_LOG_LEVEL
#else
#define	DDR3_LOG_LEVEL	0
#endif

#define DDR3_PBS        1

/* This flag allows the execution of SW WL/RL upon HW failure */
#define DDR3_RUN_SW_WHEN_HW_FAIL    1

/*
 * General Configurations
 *
 * The following parameters are required for proper setup:
 *
 * DDR_TARGET_FABRIC   - Set desired fabric configuration
 *                       (for sample@Reset fabfreq parameter)
 * DRAM_ECC            - Set ECC support 1/0
 * BUS_WIDTH           - 64/32 bit
 * CONFIG_SPD_EEPROM   - Enables auto detection of DIMMs and their timing values
 * DQS_CLK_ALIGNED     - Set this if CLK and DQS signals are aligned on board
 * MIXED_DIMM_STATIC   - Mixed DIMM + On board devices support (ODT registers
 *                       values are taken statically)
 * DDR3_TRAINING_DEBUG - Debug prints of internal code
 */
#define DDR_TARGET_FABRIC			5
/* Only enable ECC if the board selects it */
#ifdef CONFIG_BOARD_ECC_SUPPORT
#define DRAM_ECC				1
#else
#define DRAM_ECC				0
#endif

#ifdef CONFIG_DDR_32BIT
#define BUS_WIDTH                               32
#else
#define BUS_WIDTH				64
#endif

#undef DQS_CLK_ALIGNED
#undef MIXED_DIMM_STATIC
#define DDR3_TRAINING_DEBUG			0
#define REG_DIMM_SKIP_WL			0

/* Marvell boards specific configurations */
#if defined(DB_78X60_PCAC)
#undef CONFIG_SPD_EEPROM
#define STATIC_TRAINING
#endif

#if defined(DB_78X60_AMC)
#undef CONFIG_SPD_EEPROM
#undef  DRAM_ECC
#define DRAM_ECC				1
#endif

#ifdef CONFIG_SPD_EEPROM
/*
 * DIMM support parameters:
 * DRAM_2T - Set Desired 2T Mode - 0 - 1T, 0x1 - 2T, 0x2 - 3T
 * DIMM_CS_BITMAP - bitmap representing the optional CS in DIMMs
 * (0xF=CS0+CS1+CS2+CS3, 0xC=CS2+CS3...)
 */
#define DRAM_2T					0x0
#define DIMM_CS_BITMAP				0xF
#define DUNIT_SPD
#endif

#ifdef DRAM_ECC
/*
 * ECC support parameters:
 *
 * U_BOOT_START_ADDR, U_BOOT_SCRUB_SIZE - relevant when using ECC and need
 * to configure the scrubbing area
 */
#define TRAINING_SIZE				0x20000
#define U_BOOT_START_ADDR			0
#define U_BOOT_SCRUB_SIZE			0x1000000 /* TRAINING_SIZE */
#endif

/*
 * Registered DIMM Support - In case registered DIMM is attached,
 * please supply the following values:
 * (see JEDEC - JESD82-29A "Definition of the SSTE32882 Registering Clock
 * Driver with Parity and Quad Chip
 * Selects for DDR3/DDR3L/DDR3U RDIMM 1.5 V/1.35 V/1.25 V Applications")
 * RC0: Global Features Control Word
 * RC1: Clock Driver Enable Control Word
 * RC2: Timing Control Word
 * RC3-RC5 - taken from SPD
 * RC8: Additional IBT Setting Control Word
 * RC9: Power Saving Settings Control Word
 * RC10: Encoding for RDIMM Operating Speed
 * RC11: Operating Voltage VDD and VREFCA Control Word
 */
#define RDIMM_RC0				0
#define RDIMM_RC1				0
#define RDIMM_RC2				0
#define RDIMM_RC8				0
#define RDIMM_RC9				0
#define RDIMM_RC10				0x2
#define RDIMM_RC11				0x0

#if defined(MIXED_DIMM_STATIC) || !defined(CONFIG_SPD_EEPROM)
#define DUNIT_STATIC
#endif

#if defined(MIXED_DIMM_STATIC) || defined(CONFIG_SPD_EEPROM)
/*
 * This flag allows the user to change the dram refresh cycle in ps,
 * only in case of SPD or MIX DIMM topology
 */
#define TREFI_USER_EN

#ifdef TREFI_USER_EN
#define TREFI_USER				3900000
#endif
#endif

#ifdef CONFIG_SPD_EEPROM
/*
 * AUTO_DETECTION_SUPPORT - relevant ONLY for Marvell DB boards.
 * Enables I2C auto detection different options
 */
#if defined(CONFIG_DB_88F78X60) || defined(CONFIG_DB_88F78X60_REV2) || \
    defined(CONFIG_DB_784MP_GP)
#define AUTO_DETECTION_SUPPORT
#endif
#endif

#endif /* __DDR3_AXP_CONFIG_H */
