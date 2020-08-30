/***************************************************************************
 <:copyright-BRCM:2016:DUAL/GPL:standard
 
    Copyright (c) 2016 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:> 
 ***************************************************************************/

#ifndef _CFE_DEV_EMMCFLASH_H
#define _CFE_DEV_EMMCFLASH_H

#ifndef __ASSEMBLER__
#include "lib_types.h"
#include "cfe_iocb.h"
#include "cfe_device.h"
#include "bsp_config.h"
#include "bcm_hwdefs.h"
#include "bcmTag.h"
#include "dev_bcm63xx_emmc_common.h"
#endif

/*==================================================================
   *  eMMC Flash Configuration                                         *
   *     Settings/Parameters to configure eMMC transfer options.       *
   *     The options can be changable by eMMC device,                  *
   *     IP/chip version and board design.                             *
==================================================================*/
/*--------------------------------------------------------*/
/* eMMC Predefinition                                     */
/*--------------------------------------------------------*/
#define EMMC_ON     1
#define EMMC_OFF    0
#define EMMC_ON_SDIO0   0
#define EMMC_ON_SDIO1   1
#define EMMC_PART_ATTR_VAL_DEF  0
#define EMMC_PART_ATTR_VAL_ENH  1
#define EMMC_PART_ATTR_VAL_SYS  1
#define EMMC_PART_ATTR_VAL_NOP  2
#define EMMC_BOOT_DISABLE       0
#define EMMC_BOOT_FROM_BOOT1    1
#define EMMC_BOOT_FROM_BOOT2    2
#define EMMC_BOOT_FROM_LAST     3
#define EMMC_BOOT_FROM_LAST_ACCESS 4
#define EMMC_CFE_BOOT_1M    1
#define EMMC_CFE_BOOT_2M    2
#define EMMC_CFE_BOOT_4M    4

/*--------------------------------------------------------*/
/* eMMC Debugging Options                                 */
/*--------------------------------------------------------*/
/* Basic Debugging */

#define DEBUG_EMMC                  EMMC_OFF     // defualt=EMMC_OFF
#define DEBUG_EMMC_CMD              EMMC_OFF    // defualt=EMMC_OFF, To debug eMMC command.
#define DEBUG_EMMC_CONFIG           EMMC_OFF    // defualt=EMMC_OFF, To debug eMMC configuration data.
#define DEBUG_EMMC_INIT             EMMC_OFF    // defualt=EMMC_OFF, To debug eMMC initailization.
#define DEBUG_EMMC_DRV              EMMC_OFF    // defualt=EMMC_OFF, To debug driver code.
#define DEBUG_EMMC_DRV_DISPATCH     EMMC_OFF   // default=EMMC_OFF, To display driver dispatch functions.
/* Performance Debugging */
#define DEBUG_EMMC_HOST_INT         EMMC_OFF    // defualt=EMMC_OFF, To check Host Interrupt for EMMC_TIMEOUT_CNT.
#define DEBUG_EMMC_SLEEP_TIMER      EMMC_OFF    // defualt=EMMC_OFF, To check actual time of cfe_usleep( ).
#define DEBUG_EMMC_SPEED            EMMC_OFF    // defualt=EMMC_OFF, To display time/speed to read/write
/* Etc */
#define DEBUG_BUFFDATA              EMMC_OFF

/*--------------------------------------------------------*/
/* Customer Options for eMMC                              */
/* 1. EMMC_BOOT_CLK                                   */
/* 2. EMMC_BOOT_PART                                  */
/* 3. EMMC_CFE_BOOT_SIZE                              */
/* 4. EMMC_PARTITION_CTRL                             */
/*--------------------------------------------------------*/
/* eMMC reset clock with CMD0 0x00000000(GO_IDLE_STATE) */
#define EMMC_BOOT_CLK   0xF0        // Initial Clock for Boot, Recommend 0xF0 for many hardware case
/*
 // 0xFF = Base clock div 510. (100MHz / 510 = 195KHz) for CID freq(fOD:0~400KHz)
 // 0xF0 = Base clock div 480. (100MHz / 480 = 208KHz)
 // 0xE0 = Base clock div 448. (100MHz / 448 = 223KHz)
 // 0xD0 = Base clock div 416. (100MHz / 416 = 240KHz)
 // 0xC0 = Base clock div 384. (100MHz / 384 = 260KHz)
 // 0xB0 = Base clock div 352. (100MHz / 352 = 284KHz)
 // 0xA0 = Base clock div 320. (100MHz / 320 = 313KHz)
 // 0x90 = Base clock div 288. (100MHz / 288 = 347KHz)
 // 0x80 = Base clock div 256. (100MHz / 256 = 391KHz), Max freq. for eMMC
 // 0x40 = Base clock div 128. (100MHz / 128 = 781KHz)
 // 0x01 = Base clock div   2. (100MHz /   2 =  50MHz)
 // Hynix OK at 0x90, 0x80, Toshiba OK at 0xA0
*/

/*--------------------------------------------------------*/
#define EMMC_BOOT_PART      EMMC_BOOT_FROM_BOOT1
/* PARTITION_CONFIG.BOOT_PARTITION_ENABLE */
/*
 * EMMC_BOOT_DISABLE    : don't support eMMC boot FOR secondary flash only
 * EMMC_BOOT_FROM_BOOT1 : always boot from BOOT1  FOR boot from eMMC (default) to support as secondary flash
 * EMMC_BOOT_FROM_BOOT2 : always boot from BOOT2  FOR boot from eMMC
 * EMMC_BOOT_FROM_LAST  : Boot from last BOOT partition which is selected using command 'CFE> flash -emmcbootpart=1'.
 *                      : 1=BOOT1, 2=BOOT2, Other=NO_BOOT
 * EMMC_BOOT_FROM_LAST_ACCESS  : Boot from last access BOOT partition. Same as previous version but for test.
 */

/*--------------------------------------------------------*/
/* Select CFE BOOT partition size for eMMC BOOT PARTITITON */
#define EMMC_CFE_BOOT_SIZE      EMMC_CFE_BOOT_2M
/*
 * EMMC_CFE_BOOT_2M : Basic size, 1.5M CFE + 512K BSECK reload, Most eMMC
 * EMMC_CFE_BOOT_1M : Small size, 0.75M CFE + 256K BSECK reload
 * EMMC_CFE_BOOT_4M : Big size,   3M CFE + 1M BSECK reload
 */

/*--------------------------------------------------------*/
/* eMMC Physical Partition in Data Partition Area : Default OFF */
#define EMMC_PARTITION_CTRL EMMC_OFF    /* Top Level Partition Control */

#define EMMC_GP1_PARTITION  EMMC_OFF        /* GP1 Partition in Data Area */
#define EMMC_GP2_PARTITION  EMMC_OFF        /* GP2 Partition in Data Area */
#define EMMC_GP3_PARTITION  EMMC_OFF        /* GP3 Partition in Data Area */
#define EMMC_GP4_PARTITION  EMMC_OFF        /* GP4 Partition in Data Area */
#define EMMC_ENH_PARTITION  EMMC_OFF        /* ENH Partition in Data Area */
#define EMMC_PHY_PARTITION  ( EMMC_PARTITION_CTRL & \
                                    ( EMMC_GP1_PARTITION | EMMC_GP2_PARTITION \
                                    | EMMC_GP3_PARTITION | EMMC_GP4_PARTITION \
                                    | EMMC_ENH_PARTITION ) )
                                /* Do Physical Partition in Data Area */
                                /* If partition is done, only logical partition works. */

/*--------------------------------------------------------*/
/* Parameters for eMMC Physical Partition in Data Area    */
/*  !!! Caution !!!                                       */    
/*  Customer should find proper parameters using tool or own method. */
/*  If parameters are not proper or eMMC is partitioned already,     */
/*   CFE won't partition evne if EMMC_PHYS_PARTITION is ON.       */
/*--------------------------------------------------------*/
/* Partition Size[MB] : must be multiple of HC_WP_GRP_SIZE*HC_ERASE_GRP_SIZE*512K. */
/*                      if size is 0, no partition even if partition ON            */
/*                      Sum of size must be under MAX_ENH_SIZE_MULT                */
/*                      if size is more than MAX_ENH_SIZE_MULT, no partition.      */
#define EMMC_PART_SIZE_GP1      256     /* [MB] */
#define EMMC_PART_SIZE_GP2      256     /* [MB] */
#define EMMC_PART_SIZE_GP3      128     /* [MB] */
#define EMMC_PART_SIZE_GP4      128     /* [MB] */
#define EMMC_PART_SIZE_ENH      256     /* [MB] */
/* Partition Enhanced Data : Start Address  */
#define EMMC_PART_ENH_ADDR      0x0000000000000000      /* 0x0000_0000_0010_0000 @ 1MB */
/* Partition Attribute : JEDEC 4.41     */
#define EMMC_PART_ATTR_GP1      EMMC_PART_ATTR_VAL_ENH      /* DFT(default), ENH(Enhanced) */
#define EMMC_PART_ATTR_GP2      EMMC_PART_ATTR_VAL_ENH      /* DFT(default), ENH(Enhanced) */
#define EMMC_PART_ATTR_GP3      EMMC_PART_ATTR_VAL_ENH      /* DFT(default), ENH(Enhanced) */
#define EMMC_PART_ATTR_GP4      EMMC_PART_ATTR_VAL_ENH      /* DFT(default), ENH(Enhanced) */
#define EMMC_PART_ATTR_ENH      EMMC_PART_ATTR_VAL_ENH      /* DFT(default), ENH(Enhanced) */
#define EMMC_PART_ATTR_ALL      ( EMMC_PART_ATTR_GP1 | EMMC_PART_ATTR_GP2 \
                                    | EMMC_PART_ATTR_GP3 | EMMC_PART_ATTR_GP4 \
                                    | EMMC_PART_ATTR_ENH )
                                    /* Set attribute in Data Area */

/* Partition Attribute : JEDEC 4.5     */
#define EMMC_PART_ATTR_EXT_GP1  EMMC_PART_ATTR_VAL_DEF      /* DFT(default), SYS(System code), NOP(Non-persistent) */
#define EMMC_PART_ATTR_EXT_GP2  EMMC_PART_ATTR_VAL_DEF      /* DFT(default), SYS(System code), NOP(Non-persistent) */
#define EMMC_PART_ATTR_EXT_GP3  EMMC_PART_ATTR_VAL_DEF      /* DFT(default), SYS(System code), NOP(Non-persistent) */
#define EMMC_PART_ATTR_EXT_GP4  EMMC_PART_ATTR_VAL_DEF      /* DFT(default), SYS(System code), NOP(Non-persistent) */
#define EMMC_PART_ATTR_EXT      ( EMMC_PART_ATTR_EXT_GP1 | EMMC_PART_ATTR_EXT_GP2 \
                                    | EMMC_PART_ATTR_EXT_GP3 | EMMC_PART_ATTR_EXT_GP4 )
                                    /* Set attribute in Data Area */

/*--------------------------------------------------------*/
/* Select SDIO port for eMMC to debug SDIO_0 at Case C & D */
#define EMMC_USE_SDIO_0     EMMC_ON
#define EMMC_SDIO_PORT      EMMC_ON_SDIO0

/*--------------------------------------------------------*/

/*  *********************************************************************
    *  Configuration
    ********************************************************************* */
//#define EMMC_BASE 0x10000000
//#define EMMC_CACHE_SIZE 512   /*Currently we are using 512B cache*/

/*  *********************************************************************
    *  Macros for defining custom sector tables
    ********************************************************************* */


/*  *********************************************************************
    *  Structures
    ********************************************************************* */
#ifndef __ASSEMBLER__


/*  *********************************************************************
    *  Function Definition
    ********************************************************************* */
    
    
/*  *********************************************************************
    *  Structure & Definition for eMMC device parameters & configration
    ********************************************************************* */
// Predefined value
#define EMMC_ON     1
#define EMMC_OFF    0
#define EMMC_WRITE  1
#define EMMC_READ   0

// eMMC Version
#define JESD84_V42    2
#define JESD84_V43    3
#define JESD84_V44    4
#define JESD84_V441   5
#define JESD84_V45    6
#define JESD84_V451   7
#define JESD84_V50    8

// ARASAN IP Version
#define ARASAN_IP_V9_98     0x00099800
#define ARASAN_IP_V10_7A    0x00107A00
#define ARASAN_IP_V10_9     0x00109000

// EMMC_BUS_SPEED
#define EMMC_BUS_SPEED_FULL     0
#define EMMC_BUS_SPEED_HS       1
#define EMMC_BUS_SPEED_HS200    2
//HS_TIMING_OPTION
#define HS_TIMING_FULL  0  // 0 ~  26MHz, Backwards Compatibility
#define HS_TIMING_HS    1  // 0 ~  52MHz, High Speed Mode
#define HS_TIMING_HS200 2  // 0 ~ 200MHz, HS200 Mode
// BUS_FREQ_OPTION
#define BUS_FREQ_100MHZ 0  // 100MHz/2^1, Base clock = 100MHz
#define BUS_FREQ_50MHZ  1  // 100MHz/2^1, Base clock = 100MHz
#define BUS_FREQ_25MHZ  2
#define BUS_FREQ_13MHZ  4
#define BUS_FREQ_06MHZ  8
#define BUS_FREQ_03MHZ  16
// BUS_WIDTH_OPTION
#define BUS_WIDTH_1BIT  0
#define BUS_WIDTH_4BIT  1
#define BUS_WIDTH_8BIT  2
// BUS_VOLTAGE_OPTION
#define BUS_VOLTAGE_33  7   // 3.3 volts
#define BUS_VOLTAGE_30  6   // 3.0 volts
#define BUS_VOLTAGE_18  5   // 1.8 volts

// IO_PAD_TUING OPTION
#define IO_PAD_TUNING_OFF   0
#define IO_PAD_TUNING_1     1   // Sample 1
#define IO_PAD_TUNING_2     2   // Sample 2
#define IO_PAD_TUNING_3     3   // Sample 3

// PIN_MUX_PAD
#define PIN_MUX_PAD_PULL_NONE 0
#define PIN_MUX_PAD_PULL_DOWN 1
#define PIN_MUX_PAD_PULL_UP   2

// EMMC_ON_SDIO
#define EMMC_ON_SDIO0         0
#define EMMC_ON_SDIO1         1

// SDIO Application Case : SDIO_APP
/*
0 : No SDIO
A : [A0] SD_Card
    [A1] eMMC(4-bit)
B : [B0] SD_Card
    [B1] eMMC(Boot)
C : [C0] SD_Card + eMMC(Boot)
    [C1] eMMC(4-bit) + eMMC(Boot)
D : [D0] SD_Card + eMMC(Boot)
    [D1] eMMC(4-bit) + eMMC(Boot)
    [D2] SD_Card + SD_Card
E : [E0] SD_Card + eMMC(Boot)
    [E1] eMMC(8-bit) + eMMC(Boot)
    [E2] SD_Card + SD_Card
*/
#define SDIO_APP_CASE_A0    0x0000
#define SDIO_APP_CASE_A1    0x0001
#define SDIO_APP_CASE_B0    0x0010
#define SDIO_APP_CASE_B1    0x0011
#define SDIO_APP_CASE_C0    0x0020
#define SDIO_APP_CASE_C1    0x0021
#define SDIO_APP_CASE_D0    0x0030
#define SDIO_APP_CASE_D1    0x0031
#define SDIO_APP_CASE_D2    0x0032
#define SDIO_APP_CASE_E0    0x0040
#define SDIO_APP_CASE_E1    0x0041
#define SDIO_APP_CASE_E2    0x0042


/* New Options for Each Chip */
/*==================================================================*/
/*                                                                  */
/* CFE engineer tuning parameters.                                  */
/*                                                                  */
/*==================================================================*/
/*--------------------------------------------------------*/
/* [Step 0] Select Chip & set-up options.                 */
/*   : Different STB chip with different IP needs different options. */
/*--------------------------------------------------------*/
// /* BCM7425
// #define BCM7425A0  0x74250000
// #define BCM7425B0  0x74250010
// #define 7BCM425B2  0x74250012
// // BCM7425
// #define BCM7429A0  0x74290000
// #define BCM7429B0  0x74290010
// // BCM7435
// #define BCM7435A0  0x74350000
// #define BCM7435B0  0x74350010
// // BCM7445
// #define BCM7445A0  0x74450000
// */
// /*
//   - Platform : @ 'tftpboot/cfe' folder
//     Case 0 : 7358a0 (No SDIO)
//     Case A : 7231b0, 7344b0, 7346b0, 7360a0, 7362a0, 7552b0
//     Case B : 7563a0
//     Case C : 7425b0, 7435b0, 7576, 7584a0
//     Case D : 7429b0(7241b0)
// */
// /* Compile option
// #define ARASAN_IP_VER             ARASAN_IP_V9_98
// #define EMMC_NEW_RDB          deleted at SDIO_0_HOST_CMD_MODE
// #define EMMC_SCB_SEQ_EN       EMMC_OFF
// #define EMMC_BUS_SPEED        Max clock bus speed : 25MHz or 50MHz
// #define EMMC_HIF_PIN_CTRL     HIF_PIN_MUX control
// #define EMMC_PULL_UP          Some chip initial pin mux pad is not pull up.
// #define EMMC_IO_PAD_4_HS      Not necessary.
// #define SDIO_ADDR_OFFSET          0
// #define EMMC_BUS_WIDTH            IO bus width : 4-bit or 8-bit
// */
// 
// /*------------------------------------*/
// /* SDIO Application Case : 0 (No SDIO) */
// #if( BCHP_CHIP == 7358 )
// #define ARASAN_IP_VER             ARASAN_IP_V9_98
// #define EMMC_SCB_SEQ_EN       EMMC_OFF
// #define EMMC_BUS_SPEED        EMMC_BUS_SPEED_FULL
// #define EMMC_HIF_PIN_CTRL     EMMC_OFF
// #define EMMC_IO_PAD_4_HS      IO_PAD_TUNING_OFF
// #define SDIO_ADDR_OFFSET          0
// #define EMMC_BUS_WIDTH            BUS_WIDTH_4BIT
// #endif
// /*------------------------------------*/
// 
// /*------------------------------------*/
// /* SDIO Application Case : A (7231b0, 7344b0, 7346b0, 7360a0, 7362a0, 7552b0) */
// #if( (BCHP_CHIP == 7230) || (BCHP_CHIP == 7231) )
// #define ARASAN_IP_VER             ARASAN_IP_V9_98
// #define EMMC_SCB_SEQ_EN       EMMC_ON
// #define EMMC_BUS_SPEED        EMMC_BUS_SPEED_HS
// #define EMMC_HIF_PIN_CTRL     EMMC_OFF
// #define EMMC_IO_PAD_4_HS      IO_PAD_TUNING_OFF
// #define SDIO_ADDR_OFFSET          0
// #define EMMC_BUS_WIDTH            BUS_WIDTH_4BIT
// #define EMMC_BOOT_SEL              EMMC_OFF
// #endif
// 
// #if( (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) )
// #define ARASAN_IP_VER             ARASAN_IP_V9_98
// #define EMMC_SCB_SEQ_EN       EMMC_ON
// #define EMMC_BUS_SPEED        EMMC_BUS_SPEED_HS
// #define EMMC_HIF_PIN_CTRL     EMMC_OFF
// #define EMMC_IO_PAD_4_HS      IO_PAD_TUNING_OFF
// #define SDIO_ADDR_OFFSET          0
// #define EMMC_BUS_WIDTH            BUS_WIDTH_4BIT
// #define EMMC_BOOT_SEL              EMMC_OFF
// #endif
// 
// #if( (BCHP_CHIP == 7360) || (BCHP_CHIP == 7552) )
// #define ARASAN_IP_VER             ARASAN_IP_V10_7A
// #define EMMC_SCB_SEQ_EN       EMMC_ON
// #define EMMC_BUS_SPEED        EMMC_BUS_SPEED_HS
// #define EMMC_HIF_PIN_CTRL     EMMC_OFF
// #define EMMC_IO_PAD_4_HS      IO_PAD_TUNING_OFF
// #define SDIO_ADDR_OFFSET          0
// #define EMMC_BUS_WIDTH            BUS_WIDTH_4BIT
// #define EMMC_BOOT_SEL              EMMC_OFF
// #endif
// 
// #if( BCHP_CHIP == 7362 )
// #define ARASAN_IP_VER             ARASAN_IP_V10_9
// #define EMMC_SCB_SEQ_EN       EMMC_ON
// #define EMMC_BUS_SPEED        EMMC_BUS_SPEED_HS
// #define EMMC_HIF_PIN_CTRL     EMMC_OFF
// #define EMMC_IO_PAD_4_HS      IO_PAD_TUNING_OFF
// #define SDIO_ADDR_OFFSET          0
// #define EMMC_BUS_WIDTH            BUS_WIDTH_4BIT
// #define EMMC_BOOT_SEL              EMMC_OFF
// #endif
// /*------------------------------------*/
// 
// /*------------------------------------*/
// /* SDIO Application Case : B (7563a0) */
// #if( BCHP_CHIP == 7563 )
// #define ARASAN_IP_VER             ARASAN_IP_V10_9
// #define EMMC_SCB_SEQ_EN       EMMC_ON
// #define EMMC_BUS_SPEED        EMMC_BUS_SPEED_FULL
// #define EMMC_HIF_PIN_CTRL     EMMC_ON
// #define EMMC_IO_PAD_4_HS      IO_PAD_TUNING_OFF
// #define SDIO_ADDR_OFFSET          0
// #define EMMC_BUS_WIDTH            BUS_WIDTH_8BIT
// #define EMMC_BOOT_SEL              EMMC_ON
// #endif
// /*------------------------------------*/
// 
// /*------------------------------------*/
// /* SDIO Application Case : C (7425b0, 7435b0, 7576, 7584a0) */
// /* BCM7425  */
// #if( BCHP_CHIP == 7425 && defined(BCHP_REV_A0) )
//  #define ARASAN_IP_VER            ARASAN_IP_V9_98
//  #define EMMC_SCB_SEQ_EN      EMMC_ON
//  #define EMMC_BUS_SPEED       EMMC_BUS_SPEED_FULL
//  #define EMMC_HIF_PIN_CTRL    EMMC_OFF
// #define EMMC_IO_PAD_4_HS      IO_PAD_TUNING_OFF
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_4BIT
// #else
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// 
// #if( BCHP_CHIP == 7425 && ( defined(BCHP_REV_B0) || defined(BCHP_REV_B1) || defined(BCHP_REV_B2) ) )
// #define ARASAN_IP_VER           ARASAN_IP_V10_7A
// #define EMMC_SCB_SEQ_EN     EMMC_ON
// #define EMMC_BUS_SPEED      EMMC_BUS_SPEED_HS
// #define EMMC_HIF_PIN_CTRL   EMMC_OFF
// #define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_1
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_4BIT
// #else
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// 
// /* BCM7435 */
// #if( BCHP_CHIP == 7435 && defined(BCHP_REV_A0) )
// #define ARASAN_IP_VER           ARASAN_IP_V10_9
// #define EMMC_SCB_SEQ_EN     EMMC_ON
// #define EMMC_BUS_SPEED      EMMC_BUS_SPEED_FULL
// #define EMMC_HIF_PIN_CTRL   EMMC_OFF
// #define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_OFF
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_4BIT
// #else
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// 
// #if( BCHP_CHIP == 7435 && defined(BCHP_REV_B0) )
// #define ARASAN_IP_VER           ARASAN_IP_V10_9
// #define EMMC_NEW_RDB        EMMC_ON
// #define EMMC_SCB_SEQ_EN     EMMC_ON
// #define EMMC_BUS_SPEED      EMMC_BUS_SPEED_HS
// #define EMMC_HIF_PIN_CTRL   EMMC_OFF
// #define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_1
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_4BIT
// #else
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// 
// /* BCM7584, BCM7576 */
// #if( BCHP_CHIP == 7584 || BCHP_CHIP == 7576 )
// #define ARASAN_IP_VER           ARASAN_IP_V10_9
// #define EMMC_SCB_SEQ_EN     EMMC_ON
// #define EMMC_BUS_SPEED      EMMC_BUS_SPEED_HS
// #define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_1
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define EMMC_HIF_PIN_CTRL   EMMC_OFF
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_4BIT
// #else
// #define EMMC_HIF_PIN_CTRL   EMMC_ON
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// /*------------------------------------*/
// 
// /*------------------------------------*/
// /* SDIO Application Case : D (7429b0(7241b0)) */
// /* BCM7429 */
// #if( BCHP_CHIP == 7429 && defined(BCHP_REV_A0) )
// #define ARASAN_IP_VER           ARASAN_IP_V10_7A
// #define EMMC_SCB_SEQ_EN     EMMC_ON
// #define EMMC_BUS_SPEED      EMMC_BUS_SPEED_HS
// #define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_1
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define EMMC_HIF_PIN_CTRL   EMMC_OFF
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_4BIT
// #else
// #define EMMC_HIF_PIN_CTRL   EMMC_ON
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// 
// #if( BCHP_CHIP == 7429 && defined(BCHP_REV_B0) )
// #define ARASAN_IP_VER           ARASAN_IP_V10_9
// #define EMMC_SCB_SEQ_EN     EMMC_ON
// #define EMMC_BUS_SPEED      EMMC_BUS_SPEED_HS
// #define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_1
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define EMMC_HIF_PIN_CTRL   EMMC_OFF
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_4BIT
// #else
// #define EMMC_HIF_PIN_CTRL   EMMC_ON
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// /*------------------------------------*/
// 
// 
// /*------------------------------------*/
// /* SDIO Application Case : E */
// /* BCM7445 */
// #if( BCHP_CHIP == 7445 && defined(BCHP_REV_A0) )
// #define ARASAN_IP_VER           ARASAN_IP_V10_9
// #define EMMC_SCB_SEQ_EN     EMMC_ON
// #define EMMC_BUS_SPEED      EMMC_BUS_SPEED_HS
// #define EMMC_HIF_PIN_CTRL   EMMC_OFF
// #define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_1
// #if( EMMC_SDIO_PORT == EMMC_ON_SDIO0 )
// #define SDIO_ADDR_OFFSET        0
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #else
// #define SDIO_ADDR_OFFSET        (BCHP_SDIO_1_HOST_SDMA-BCHP_SDIO_0_HOST_SDMA)
// #define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
// #endif
// #define EMMC_BOOT_SEL            EMMC_ON
// #endif
// /*------------------------------------*/

#if defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM963138_) || \
    defined (_BCM963158_) || defined (_BCM96846_) || defined (_BCM96856_)
#define ARASAN_IP_VER       ARASAN_IP_V10_9
#define EMMC_SCB_SEQ_EN     EMMC_ON
#define EMMC_HIF_PIN_CTRL   EMMC_OFF
#define EMMC_IO_PAD_4_HS    IO_PAD_TUNING_OFF
#if defined (_BCM96858_) || defined (_BCM963138_) || defined (_BCM96856_)
#define EMMC_BUS_SPEED      EMMC_BUS_SPEED_FULL
#else
#define EMMC_BUS_SPEED      EMMC_BUS_SPEED_HS
#endif
#define SDIO_ADDR_OFFSET        0
#define EMMC_BUS_WIDTH          BUS_WIDTH_8BIT
#define EMMC_BOOT_SEL           EMMC_ON
#define BOOT_FROM_EMMC          1
#endif

/*--------------------------------------------------------*/


/*--------------------------------------------------------*/
/* [Step 1] Data by STB chip specification                */
/*   : Different STB chip with different IP needs different options. */
/*--------------------------------------------------------*/
/* [Step 1-1] Pin Mux Control : Only 7429 Needs.  */
/* [Step 1-2] EMMC_NEW_RDB                    */
/*              SDIO_1 is using SDIO_0 RDB information(MASK, SHIFT, DEFAULT).
                Several registers of 7429B0 SDIO_0 was changed.
                7429B0 needs EMMC_NEW_RDB option.
*/
/* [Step 1-3] eMMC Revision                     */
#if( ARASAN_IP_VER == ARASAN_IP_V9_98 )
#define EMMC_VERSION  JESD84_V44
#elif( (ARASAN_IP_VER == ARASAN_IP_V10_7A) && (ARASAN_IP_VER == ARASAN_IP_V10_9) )
#define EMMC_VERSION  JESD84_V441
#endif
/*--------------------------------------------------------*/

/*--------------------------------------------------------*/
/* [Step 2] Data by eMMC device specification             */
/*--------------------------------------------------------*/
/* [Step 2-1] eMMC Mode : From 0x00020000, 0x00030000, ..., 0xFFFF0000 */
#define EMMC_RCA                0x00020000
/*#define EMMC_NUM_DEVICE         1           : The number of eMMC device on board */
/*#define EMMC_OCR_SECTOR_MODE    EMMC_ON     // ON(>2GB), OFF(<=2GB), Hynix H26M42001FMR(8GB), H26M54001DQR(16GB)*/
/*--------------------------------------------------------*/

/*--------------------------------------------------------*/
/* [Step 3] Data by board configuration                   */
/*   : This depends on board design.                      */
/*--------------------------------------------------------*/
/* [Step 3-1] Bus Mode : Speed                  */
/* EMMC_HS_TIMING : HS_TIMING_HS, HS_TIMING_FULL */
/* EMMC_BUS_FREQ  : BUS_FREQ_52MHZ, BUS_FREQ_26MHZ, BUS_FREQ_13MHZ, BUS_FREQ_06MHZ, BUS_FREQ_03MHZ */
/* EMMC_HOST_HS   : EMMC_ON, EMMC_OFF */
#if( EMMC_BUS_SPEED == EMMC_BUS_SPEED_HS )
#define EMMC_HS_TIMING      HS_TIMING_HS
#define EMMC_BUS_FREQ       BUS_FREQ_50MHZ
#define EMMC_HOST_HS        EMMC_ON
#else
#define EMMC_HS_TIMING      HS_TIMING_FULL
#define EMMC_BUS_FREQ       BUS_FREQ_25MHZ
#define EMMC_HOST_HS        EMMC_OFF
#endif
/* [Step 3-2] Bus Mode : Width - depends on board configuration. It should be tuned by customer. */
/*#define EMMC_BUS_WIDTH      BUS_WIDTH_8BIT      // BUS_WIDTH_8BIT, BUS_WIDTH_4BIT, BUS_WIDTH_1BIT */

/* [Step 3-3] Bus Mode : Voltage - depends on board configuration. It should be tuned by customer. */
#define EMMC_BUS_VOLTAGE    BUS_VOLTAGE_33      // BUS_VOLTAGE_33, BUS_VOLTAGE_30, BUS_VOLTAGE_18
/*--------------------------------------------------------*/

/*--------------------------------------------------------*/
/* [Step 4] Sleep Time Tuning by CFE configuration        */
/*   : This setting depends on eMMC reading response time,*/
/*     writing transfer time, bus freq. and board design. */
/*   : It should be tuned by customer.                    */
/*--------------------------------------------------------*/
/* [Step 4-1] TIMEOUT_CNT               // Timeout counter for waiting command and transfer. */
//#define EMMC_TIMEOUT_CNT        10000000    // 10[sec], To get the value(max) using DEBUG_EMMC_HOST_INT
#define EMMC_TIMEOUT_CNT        3000000     // 3[s]

/* [Step 4-2] BOOT_SLEEP                 // Sleep(waiting) time in Boot Mode, [Debug] Check CMD7/... error */
#define EMMC_BOOT_SLEEP         2       // [ms], 2~10[ms]

/* [Step 4-3] INIT_CMDS_SLEEP         // Sleep(waiting) time in DeviceID Mode(CMD0/1/2/3) & DataTransfer Mode(CMD9/7/8) */
#define EMMC_INIT_CMDS_SLEEP    100    // [us], 0~100 : Initial sleep time

/* [Step 4-4] INT_SLEEP                    // For all Interrupt Sleep after setting up transfer mode in 'emmc_Setup_BusFreqWidth( )'. */
#define EMMC_INT_SLEEP          1      // [us], 0~100 : Initial sleep time
/*--------------------------------------------------------*/

/*--------------------------------------------------------*/
/* [Step 5] IO_Pad Tuning using Oscilloscope by STB Chip  */
/*   : In(Input)/Out(Write) Delay Control.                */
/*   : This depends on IO pad performance of STB chip.    */
/*--------------------------------------------------------*/
#if( EMMC_IO_PAD_4_HS == IO_PAD_TUNING_1 )
#define BCM_EMMC_IN_DLY_ON      1       // ENABLE, DISABLE
#define BCM_EMMC_IN_DLY_CTRL    3       // 0 ~ 3(default)   
#define BCM_EMMC_IN_DLY_DLY     56      // 0(default) ~ 63
#define BCM_EMMC_OUT_DLY_ON     1       // ENABLE, DISABLE
#define BCM_EMMC_OUT_DLY_CTRL   3       // 0 ~ 3(default)
#define BCM_EMMC_OUT_DLY_DLY    4       // 0(default) ~ 15
#elif( EMMC_IO_PAD_4_HS == IO_PAD_TUNING_2 )
#define BCM_EMMC_IN_DLY_ON      1       // ENABLE, DISABLE
#define BCM_EMMC_IN_DLY_CTRL    3       // 0 ~ 3(default)   
#define BCM_EMMC_IN_DLY_DLY     16      // 0(default) ~ 63
#define BCM_EMMC_OUT_DLY_ON     1       // ENABLE, DISABLE
#define BCM_EMMC_OUT_DLY_CTRL   3       // 0 ~ 3(default)
#define BCM_EMMC_OUT_DLY_DLY    4       // 0(default) ~ 15
#elif( EMMC_IO_PAD_4_HS == IO_PAD_TUNING_3 )
#define BCM_EMMC_IN_DLY_ON      1       // ENABLE, DISABLE
#define BCM_EMMC_IN_DLY_CTRL    3       // 0 ~ 3(default)   
#define BCM_EMMC_IN_DLY_DLY     16      // 0(default) ~ 63
#define BCM_EMMC_OUT_DLY_ON     1       // ENABLE, DISABLE
#define BCM_EMMC_OUT_DLY_CTRL   3       // 0 ~ 3(default)
#define BCM_EMMC_OUT_DLY_DLY    10       // 0(default) ~ 15
#else
#define BCM_EMMC_IN_DLY_ON      0       // ENABLE, DISABLE
#define BCM_EMMC_IN_DLY_CTRL    3       // 0 ~ 3(default)   
#define BCM_EMMC_IN_DLY_DLY     0       // 0(default) ~ 63
#define BCM_EMMC_OUT_DLY_ON     0       // ENABLE, DISABLE
#define BCM_EMMC_OUT_DLY_CTRL   3       // 0 ~ 3(default)
#define BCM_EMMC_OUT_DLY_DLY    0       // 0(default) ~ 15
#endif

#define EMMC_CMD1_TIMEOUT_CNT    100      // 1[sec] = 10ms delay * 100 times
/*--------------------------------------------------------*/
/*==================================================================*/


/*==================================================================*/
/*--------------------------------------------------------*/
// eMMC device state
#define CST_ADDRESS_OUT_OF_RANGE_MASK  0x80000000
#define CST_ADDRESS_MISALIGN_MASK      0x40000000
#define CST_BLOCK_LEN_ERROR_MASK       0x20000000
#define CST_ERASE_SEQ_ERROR_MASK       0x10000000
#define CST_ERASE_PARAM_MASK           0x08000000
#define CST_WP_VIOLATION_MASK          0x04000000
#define CST_DEVICE_IS_LOCKED_MASK      0x02000000
#define CST_LOCK_UNLOCK_FAILED_MASK    0x01000000
#define CST_COM_CRC_ERROR_MASK         0x00800000
#define CST_ILLEGAL_COMMAND_MASK       0x00400000
#define CST_DEVICE_ECC_FAILED_MASK     0x00200000
#define CST_CC_ERROR_MASK              0x00100000
#define CST_ERROR_MASK                 0x00080000
#define CST_CID_CSD_OVERWRITE_MASK     0x00010000
#define CST_WP_ERASE_SKIP_MASK         0x00008000
#define CST_ERASE_RESET_MASK           0x00002000
#define CST_CURRENT_STATE_MASK         0x00001E00
#define CST_READY_FOR_DATA_MASK        0x00000100
#define CST_SWITCH_ERROR_MASK          0x00000080
#define CST_EXCEPTION_EVENT_MASK       0x00000040
#define CST_APP_CMD_MASK               0x00000020
#define CST_ADDRESS_OUT_OF_RANGE_SHIFT 31
#define CST_ADDRESS_MISALIGN_SHIFT     30
#define CST_BLOCK_LEN_ERROR_SHIFT      29
#define CST_ERASE_SEQ_ERROR_SHIFT      28
#define CST_ERASE_PARAM_SHIFT          27
#define CST_WP_VIOLATION_SHIFT         26
#define CST_DEVICE_IS_LOCKED_SHIFT     25
#define CST_LOCK_UNLOCK_FAILED_SHIFT   24
#define CST_COM_CRC_ERROR_SHIFT        23
#define CST_ILLEGAL_COMMAND_SHIFT      22
#define CST_DEVICE_ECC_FAILED_SHIFT    21
#define CST_CC_ERROR_SHIFT             20
#define CST_ERROR_SHIFT                19
#define CST_CID_CSD_OVERWRITE_SHIFT    16
#define CST_WP_ERASE_SKIP_SHIFT        15
#define CST_ERASE_RESET_SHIFT          13
#define CST_CURRENT_STATE_SHIFT         9
#define CST_READY_FOR_DATA_SHIFT        8
#define CST_SWITCH_ERROR_SHIFT          7
#define CST_EXCEPTION_EVENT_SHIFT       6
#define CST_APP_CMD_SHIFT               5
#define CST_STATE_SLP  10
#define CST_STATE_BTST  9
#define CST_STATE_DIS   8
#define CST_STATE_PRG   7
#define CST_STATE_RCV   6
#define CST_STATE_DATA  5
#define CST_STATE_TRAN  4
#define CST_STATE_STBY  3
#define CST_STATE_IDEN  2
#define CST_STATE_REDY  1
#define CST_STATE_IDLE  0

// Device Status
#define Idx_CST_ADDRESS_OUT_OF_RANGE 0x80000000  // '[    31]
#define Idx_CST_ADDRESS_MISALIGN     0x40000000  // '[    30]
#define Idx_CST_BLOCK_LEN_ERROR      0x20000000  // '[    29]
#define Idx_CST_ERASE_SEQ_ERROR      0x10000000  // '[    28]
#define Idx_CST_ERASE_PARAM          0x08000000  // '[    27]
#define Idx_CST_WP_VIOLATION         0x04000000  // '[    26]
#define Idx_CST_DEVICE_IS_LOCKED     0x02000000  // '[    25]
#define Idx_CST_LOCK_UNLOCK_FAILED   0x01000000  // '[    24]
#define Idx_CST_COM_CRC_ERROR        0x00800000  // '[    23]
#define Idx_CST_ILLEGAL_COMMAND      0x00400000  // '[    22]
#define Idx_CST_DEVICE_ECC_FAILED    0x00200000  // '[    21]
#define Idx_CST_CC_ERROR             0x00100000  // '[    20]
#define Idx_CST_ERROR                0x00080000  // '[    19]
#define Idx_CST_CID_CSD_OVERWRITE    0x00010000  // '[    16]
#define Idx_CST_WP_ERASE_SKIP        0x00008000  // '[    15]
#define Idx_CST_ERASE_RESET          0x00002000  // '[    13]
#define Idx_CST_CURRENT_STATE        0x00001E00  // '[12 : 9]
#define Idx_CST_READY_FOR_DATA       0x00000100  // '[     8]
#define Idx_CST_SWITCH_ERROR         0x00000080  // '[     7]
#define Idx_CST_EXCEPTION_EVENT      0x00000040  // '[     6]
#define Idx_CST_APP_CMD              0x00000020  // '[     5]

// Register : Extended CSD
// Properties Segment
#define Idx_ExtCSD_S_CMD_SET                    504  // '( 0)'( 1)
#define Idx_ExtCSD_HPI_FEATURES                 503  // '( 0)'( 1)
#define Idx_ExtCSD_BKOPS_SUPPORT                502  // '( 0)'( 1)
#define Idx_ExtCSD_MAX_PACKED_READS             501  // '( 0)'( 1)
#define Idx_ExtCSD_MAX_PACKED_WRITES            500  // '( 0)'( 1)
#define Idx_ExtCSD_DATA_TAG_SUPPORT             499  // '( 0)'( 1)
#define Idx_ExtCSD_TAG_UNIT_SIZE                498  // '( 0)'( 1)
#define Idx_ExtCSD_TAG_RES_SIZE                 497  // '( 0)'( 1)
#define Idx_ExtCSD_CONTEXT_CAPABILITIES         496  // '( 0)'( 1)
#define Idx_ExtCSD_LARGE_UNIT_SIZE_M1           495  // '( 0)'( 1)
#define Idx_ExtCSD_EXT_SUPPORT                  494  // '( 0)'( 1)
#define Idx_ExtCSD_CACHE_SIZE                   249  // '( 3)'( 4)
#define Idx_ExtCSD_GENERIC_CMD6_TIME            248  // '( 0)'( 1)
#define Idx_ExtCSD_POWER_OFF_LONG_TIME          247  // '( 0)'( 1)
#define Idx_ExtCSD_BKOPS_STATUS                 246  // '( 1)'( 2)
#define Idx_ExtCSD_CORRECTLY_PRG_SECTORS_NUM    242  // '( 3)'( 4)
#define Idx_ExtCSD_INI_TIMEOUT_AP               241  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_DDR_52_360            239  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_DDR_52_195            238  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_200_360               237  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_200_195               236  // '( 0)'( 1)
#define Idx_ExtCSD_MIN_PERF_DDR_W_8_52          235  // '( 0)'( 1)
#define Idx_ExtCSD_MIN_PERF_DDR_R_8_52          234  // '( 0)'( 1)
#define Idx_ExtCSD_TRIM_MULT                    232  // '( 0)'( 1)
#define Idx_ExtCSD_SEC_FEATURE_SUPPORT          231  // '( 0)'( 1)
#define Idx_ExtCSD_SEC_ERASE_MULT               230  // '( 0)'( 1)
#define Idx_ExtCSD_SEC_TRIM_MULT                229  // '( 0)'( 1)
#define Idx_ExtCSD_BOOT_INFO                    228  // '( 0)'( 1)
#define Idx_ExtCSD_BOOT_SIZE_MULT               226  // '( 0)'( 1)
#define Idx_ExtCSD_ACC_SIZE                     225  // '( 0)'( 1)
#define Idx_ExtCSD_HC_ERASE_GRP_SIZE            224  // '( 0)'( 1)
#define Idx_ExtCSD_ERASE_TIMEOUT_MULT           223  // '( 0)'( 1)
#define Idx_ExtCSD_REL_WR_SEC_C                 222  // '( 0)'( 1)
#define Idx_ExtCSD_HC_WP_GRP_SIZE               221  // '( 0)'( 1)
#define Idx_ExtCSD_S_C_VCC                      220  // '( 0)'( 1)
#define Idx_ExtCSD_S_C_VCCQ                     219  // '( 0)'( 1)
#define Idx_ExtCSD_S_A_TIMEOUT                  217  // '( 0)'( 1)
#define Idx_ExtCSD_SEC_COUNT                    212  // '( 3)'( 4)
#define Idx_ExtCSD_MIN_PERF_W_8_52              210  // '( 0)'( 1)
#define Idx_ExtCSD_MIN_PERF_R_8_52              209  // '( 0)'( 1)
#define Idx_ExtCSD_MIN_PERF_W_8_26_4_52         208  // '( 0)'( 1)
#define Idx_ExtCSD_MIN_PERF_R_8_26_4_52         207  // '( 0)'( 1)
#define Idx_ExtCSD_MIN_PERF_W_4_26              206  // '( 0)'( 1)
#define Idx_ExtCSD_MIN_PERF_R_4_26              205  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_26_360                203  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_52_360                202  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_26_195                201  // '( 0)'( 1)
#define Idx_ExtCSD_PWR_CL_52_195                200  // '( 0)'( 1)
#define Idx_ExtCSD_PARTITION_SWITCH_TIME        199  // '( 0)'( 1)
#define Idx_ExtCSD_OUT_OF_INTERRUPT_TIME        198  // '( 0)'( 1)
#define Idx_ExtCSD_DRIVER_STRENGTH              197  // '( 0)'( 1)
#define Idx_ExtCSD_DEVICE_TYPE                  196  // '( 0)'( 1)
#define Idx_ExtCSD_CSD_STRUCTURE                194  // '( 0)'( 1)
#define Idx_ExtCSD_EXT_CSD_REV                  192  // '( 0)'( 1)
// Modes Segment
#define Idx_ExtCSD_CMD_SET                      191  // '( 0)'( 1)
#define Idx_ExtCSD_CMD_SET_REV                  189  // '( 0)'( 1)
#define Idx_ExtCSD_POWER_CLASS                  187  // '( 0)'( 1)
#define Idx_ExtCSD_HS_TIMING                    185  // '( 0)'( 1)
#define Idx_ExtCSD_BUS_WIDTH                    183  // '( 0)'( 1)
#define Idx_ExtCSD_ERASED_MEM_CONT              181  // '( 0)'( 1)
#define Idx_ExtCSD_PARTITION_CONFIG             179  // '( 0)'( 1)
#define Idx_ExtCSD_BOOT_CONFIG_PROT             178  // '( 0)'( 1)
#define Idx_ExtCSD_BOOT_BUS_CONDITIONS          177  // '( 0)'( 1)
#define Idx_ExtCSD_ERASE_GROUP_DEF              175  // '( 0)'( 1)
#define Idx_ExtCSD_BOOT_WP_STATUS               174  // '( 0)'( 1)
#define Idx_ExtCSD_BOOT_WP                      173  // '( 0)'( 1)
#define Idx_ExtCSD_USER_WP                      171  // '( 0)'( 1)
#define Idx_ExtCSD_FW_CONFIG                    169  // '( 0)'( 1)
#define Idx_ExtCSD_RPMB_SIZE_MULT               168  // '( 0)'( 1)
#define Idx_ExtCSD_WR_REL_SET                   167  // '( 0)'( 1)
#define Idx_ExtCSD_WR_REL_PARAM                 166  // '( 0)'( 1)
#define Idx_ExtCSD_SANITIZE_START               165  // '( 0)'( 1)
#define Idx_ExtCSD_BKOPS_START                  164  // '( 0)'( 1)
#define Idx_ExtCSD_BKOPS_EN                     163  // '( 0)'( 1)
#define Idx_ExtCSD_RST_n_FUNCTION               162  // '( 0)'( 1)
#define Idx_ExtCSD_HPI_MGMT                     161  // '( 0)'( 1)
#define Idx_ExtCSD_PARTITIONING_SUPPORT         160  // '( 0)'( 1)
#define Idx_ExtCSD_MAX_ENH_SIZE_MULT            157  // '( 2)'( 3)
#define Idx_ExtCSD_PARTITIONS_ATTRIBUTE         156  // '( 0)'( 1)
#define Idx_ExtCSD_PARTITION_SETTING_COMPLETED  155  // '( 0)'( 1)
#define Idx_ExtCSD_GP_SIZE_MULT                 143  // '(11)'(12)
#define Idx_ExtCSD_ENH_SIZE_MULT                140  // '( 2)'( 3)
#define Idx_ExtCSD_ENH_START_ADDR               136  // '( 3)'( 4)
#define Idx_ExtCSD_SEC_BAD_BLK_MGMNT            134  // '( 0)'( 1)
#define Idx_ExtCSD_TCASE_SUPPORT                132  // '( 0)'( 1)
#define Idx_ExtCSD_PERIODIC_WAKEUP              131  // '( 0)'( 1)
#define Idx_ExtCSD_PROGRAM_CID_CSD_DDR_SUPPORT  130  // '( 0)'( 1)
#define Idx_ExtCSD_VENDOR_SPECIFIC_FIELD         64  // '(63)'(64)
#define Idx_ExtCSD_NATIVE_SECTOR_SIZE            63  // '( 0)'( 1)
#define Idx_ExtCSD_USE_NATIVE_SECTOR             62  // '( 0)'( 1)
#define Idx_ExtCSD_DATA_SECTOR_SIZE              61  // '( 0)'( 1)
#define Idx_ExtCSD_INI_TIMEOUT_EMU               60  // '( 0)'( 1)
#define Idx_ExtCSD_CLASS_6_CTRL                  59  // '( 0)'( 1)
#define Idx_ExtCSD_DYNCAP_NEEDED                 58  // '( 0)'( 1)
#define Idx_ExtCSD_EXCEPTION_EVENTS_CTRL         56  // '( 1)'( 2)
#define Idx_ExtCSD_EXCEPTION_EVENTS_STATUS       54  // '( 1)'( 2)
#define Idx_ExtCSD_EXT_PARTITIONS_ATTRIBUTE      52  // '( 1)'( 2)
#define Idx_ExtCSD_CONTEXT_CONF                  37  // '(14)'(15)
#define Idx_ExtCSD_PACKED_COMMAND_STATUS         36  // '( 0)'( 1)
#define Idx_ExtCSD_PACKED_FAILURE_INDEX          35  // '( 0)'( 1)
#define Idx_ExtCSD_POWER_OFF_NOTIFICATION        34  // '( 0)'( 1)
#define Idx_ExtCSD_CACHE_CTRL                    33  // '( 0)'( 1)
#define Idx_ExtCSD_FLUSH_CACHE                   32  // '( 0)'( 1)

#define Idx_ExtCSD_ACC_CMD  0   // EXT_CSD Access mode : Command Set
#define Idx_ExtCSD_ACC_SET  1   // EXT_CSD Access mode : Set Bits
#define Idx_ExtCSD_ACC_CLR  2   // EXT_CSD Access mode : Clear Bits
#define Idx_ExtCSD_ACC_WRB  3   // EXT_CSD Access mode : Write Byte

#define EMMC_PART_DATA  0   // Data Partition in EMMC
#define EMMC_PART_BOOT1 1   // Boot1 Partition in EMMC
#define EMMC_PART_BOOT2 2   // Boot2 Partition in EMMC
#define EMMC_PART_RPMB  3   // Replay Protected Memory Block in EMMC
#define EMMC_PART_GP1   4   // General Purpose 1 Partition in EMMC 
#define EMMC_PART_GP2   5   // General Purpose 2 Partition in EMMC 
#define EMMC_PART_GP3   6   // General Purpose 3 Partition in EMMC 
#define EMMC_PART_GP4   7   // General Purpose 4 Partition in EMMC

enum EMMC_BLOCK_ACCESS_ATTR { EMMC_ACCESS_FIRST_BLOCK=0, EMMC_ACCESS_MIDDLE_BLOCK, EMMC_ACCESS_LAST_BLOCK, EMMC_ACCESS_LESS_THAN_BLOCK };
enum EMMC_WRITE_BLOCK_ATTR  { EMMC_WRITE_FIRST_BLOCK=0,  EMMC_WRITE_MIDDLE_BLOCK,  EMMC_WRITE_LAST_BLOCK,  EMMC_WRITE_LESS_THAN_BLOCK  };
enum EMMC_READ_BLOCK_ATTR   { EMMC_READ_FIRST_BLOCK=0,   EMMC_READ_MIDDLE_BLOCK,   EMMC_READ_LAST_BLOCK,   EMMC_READ_LESS_THAN_BLOCK,   EMMC_READ_MIDDLE_BLOCK_4_WRITE   };
enum EMMC_BOOT_PART_EN_ATTR { EMMC_NO_BOOT=0,            EMMC_BOOT1_4_BOOT=1,      EMMC_BOOT2_4_BOOT=2,    EMMC_USER_4_BOOT=7          };
enum EMMC_PART_ACCESS_ATTR  { EMMC_NO_ACCESS_BOOT=0, EMMC_RW_BOOT_PART_1=1, EMMC_RW_BOOT_PART_2=2, EMMC_RW_RPMB=3, EMMC_RW_GP1=4, EMMC_RW_GP2=5, EMMC_RW_GP3=6, EMMC_RW_GP4=7 };


typedef struct emmcdev_cid_s {
    // Registers
    uint8_t            MID;
    uint8_t            CBX;
    uint8_t            OID;
    char               PNM[6];
    uint8_t            PRV;
    uint32_t           PSN;
    uint8_t            MDT;
} emmcdev_cid_t;

typedef struct emmcdev_csd_s {
    // Registers
    uint32_t           RESP67;
    uint32_t           RESP45;
    uint32_t           RESP23;
    uint32_t           RESP01;
    // Fields
    uint8_t            CSD_STRUCTURE     ;
    uint8_t            SPEC_VERS         ;
    uint8_t            TAAC              ;
    uint8_t            NSAC              ;
    uint8_t            TRAN_SPEED        ;
    uint16_t           CCC               ;
    uint8_t            READ_BL_LEN       ;
    uint8_t            READ_BL_PARTIAL   ;
    uint8_t            WRITE_BLK_MISALIGN;
    uint8_t            READ_BLK_MISALIGN ;
    uint8_t            DSR_IMP           ;
    uint16_t           C_SIZE            ;
    uint8_t            VDD_R_CURR_MIN    ;
    uint8_t            VDD_R_CURR_MAX    ;
    uint8_t            VDD_W_CURR_MIN    ;
    uint8_t            VDD_W_CURR_MAX    ;
    uint8_t            C_SIZE_MULT       ;
    uint8_t            ERASE_GRP_SIZE    ;
    uint8_t            ERASE_GRP_MULT    ;
    uint8_t            WP_GRP_SIZE       ;
    uint8_t            WP_GRP_ENABLE     ;
    uint8_t            DEFAULT_ECC       ;
    uint8_t            R2W_FACTOR        ;
    uint8_t            WRITE_BL_LEN      ;
    uint8_t            WRITE_BL_PARTIAL  ;
    uint8_t            CONTENT_PROT_APP  ;
    uint8_t            FILE_FORMAT_GRP   ;
    uint8_t            COPY              ;
    uint8_t            PERM_WRITE_PROTECT;
    uint8_t            TMP_WRITE_PROTECT ;
    uint8_t            FILE_FORMAT       ;
    uint8_t            ECC               ;
} emmcdev_csd_t;


typedef struct emmcdev_extcsd_s {
    // Properties Segment
    uint8_t            S_CMD_SET                  ;
    uint8_t            HPI_FEATURES               ;
    uint8_t            BKOPS_SUPPORT              ;
    uint8_t            MAX_PACKED_READS           ;
    uint8_t            MAX_PACKED_WRITES          ;
    uint8_t            DATA_TAG_SUPPORT           ;
    uint8_t            TAG_UNIT_SIZE              ;
    uint8_t            TAG_RES_SIZE               ;
    uint8_t            CONTEXT_CAPABILITIES       ;
    uint8_t            LARGE_UNIT_SIZE_M1         ;
    uint8_t            EXT_SUPPORT                ;
    uint32_t           CACHE_SIZE                 ;
    uint8_t            GENERIC_CMD6_TIME          ;
    uint8_t            POWER_OFF_LONG_TIME        ;
    uint8_t            BKOPS_STATUS               ;
    uint32_t           CORRECTLY_PRG_SECTORS_NUM  ;
    uint8_t            INI_TIMEOUT_AP             ;
    uint8_t            PWR_CL_DDR_52_360          ;
    uint8_t            PWR_CL_DDR_52_195          ;
    uint8_t            PWR_CL_200_360             ;
    uint8_t            PWR_CL_200_195             ;
    uint8_t            MIN_PERF_DDR_W_8_52        ;
    uint8_t            MIN_PERF_DDR_R_8_52        ;
    uint8_t            TRIM_MULT                  ;
    uint8_t            SEC_FEATURE_SUPPORT        ;
    uint8_t            SEC_ERASE_MULT             ;
    uint8_t            SEC_TRIM_MULT              ;
    uint8_t            BOOT_INFO                  ;
    uint8_t            BOOT_SIZE_MULT             ;
    uint8_t            ACC_SIZE                   ;
    uint8_t            HC_ERASE_GRP_SIZE          ;
    uint8_t            ERASE_TIMEOUT_MULT         ;
    uint8_t            REL_WR_SEC_C               ;
    uint8_t            HC_WP_GRP_SIZE             ;
    uint8_t            S_C_VCC                    ;
    uint8_t            S_C_VCCQ                   ;
    uint8_t            S_A_TIMEOUT                ;
    uint32_t           SEC_COUNT                  ;
    uint8_t            MIN_PERF_W_8_52            ;
    uint8_t            MIN_PERF_R_8_52            ;
    uint8_t            MIN_PERF_W_8_26_4_52       ;
    uint8_t            MIN_PERF_R_8_26_4_52       ;
    uint8_t            MIN_PERF_W_4_26            ;
    uint8_t            MIN_PERF_R_4_26            ;
    uint8_t            PWR_CL_26_360              ;
    uint8_t            PWR_CL_52_360              ;
    uint8_t            PWR_CL_26_195              ;
    uint8_t            PWR_CL_52_195              ;
    uint8_t            PARTITION_SWITCH_TIME      ;
    uint8_t            OUT_OF_INTERRUPT_TIME      ;
    uint8_t            DRIVER_STRENGTH            ;
    uint8_t            DEVICE_TYPE                ;
    uint8_t            CSD_STRUCTURE              ;
    uint8_t            EXT_CSD_REV                ;
    // Modes Segment
    uint8_t            CMD_SET                    ;
    uint8_t            CMD_SET_REV                ;
    uint8_t            POWER_CLASS                ;
    uint8_t            HS_TIMING                  ;
    uint8_t            BUS_WIDTH                  ;
    uint8_t            ERASED_MEM_CONT            ;
    uint8_t            PARTITION_CONFIG           ;
    uint8_t            BOOT_CONFIG_PROT           ;
    uint8_t            BOOT_BUS_CONDITIONS        ;
    uint8_t            ERASE_GROUP_DEF            ;
    uint8_t            BOOT_WP_STATUS             ;
    uint8_t            BOOT_WP                    ;
    uint8_t            USER_WP                    ;
    uint8_t            FW_CONFIG                  ;
    uint8_t            RPMB_SIZE_MULT             ;
    uint8_t            WR_REL_SET                 ;
    uint8_t            WR_REL_PARAM               ;
    uint8_t            SANITIZE_START             ;
    uint8_t            BKOPS_START                ;
    uint8_t            BKOPS_EN                   ;
    uint8_t            RST_n_FUNCTION             ;
    uint8_t            HPI_MGMT                   ;
    uint8_t            PARTITIONING_SUPPORT       ;
    uint32_t           MAX_ENH_SIZE_MULT          ;
    uint8_t            PARTITIONS_ATTRIBUTE       ;
    uint8_t            PARTITION_SETTING_COMPLETED;
    uint32_t           GP_SIZE_MULT[4]            ;
    uint32_t           ENH_SIZE_MULT              ;
    uint32_t           ENH_START_ADDR             ;
    uint8_t            SEC_BAD_BLK_MGMNT          ;
    uint8_t            TCASE_SUPPORT              ;
    uint8_t            PERIODIC_WAKEUP            ;
    uint8_t            PROGRAM_CID_CSD_DDR_SUPPORT;
    uint8_t            VENDOR_SPECIFIC_FIELD[64]  ;
    uint8_t            NATIVE_SECTOR_SIZE         ;
    uint8_t            USE_NATIVE_SECTOR          ;
    uint8_t            DATA_SECTOR_SIZE           ;
    uint8_t            INI_TIMEOUT_EMU            ;
    uint8_t            CLASS_6_CTRL               ;
    uint8_t            DYNCAP_NEEDED              ;
    uint16_t           EXCEPTION_EVENTS_CTRL      ;
    uint16_t           EXCEPTION_EVENTS_STATUS    ;
    uint16_t           EXT_PARTITIONS_ATTRIBUTE   ;
    uint8_t            CONTEXT_CONF[15]           ;
    uint8_t            PACKED_COMMAND_STATUS      ;
    uint8_t            PACKED_FAILURE_INDEX       ;
    uint8_t            POWER_OFF_NOTIFICATION     ;
    uint8_t            CACHE_CTRL                 ;
    uint8_t            FLUSH_CACHE                ;
} emmcdev_extcsd_t;


typedef struct emmcdev_config_s {
    //--------------------------------------------
    // < CFE Control >
    // CFE parameters
    uint8_t             CFEBootMode;
    uint8_t             CFEBigEndian;
    // User Configuration for eMMC Device
    uint8_t             OCR_SectorMode;
    uint8_t             SDMA_On;
    // Sleep Control
    uint32_t            BootModeHostSleep;  // [ms]
    uint32_t            InterruptSleep;     // [us]
    uint32_t            ReadBufSleep;       // [us]
    uint32_t            WriteBufSleep;      // [us] 
    //--------------------------------------------
    // < eMMC Control >
    // Bus Mode
    uint32_t            HSTiming;
    uint8_t             HostHS_On;
    uint16_t            BusFreq;        // User configuration
    uint8_t             BusWidth;       // User configuration
    uint8_t             BusVoltage;     // User configuration
    // Genral
    uint32_t            GenCmd6Timeout;
    //uint32_t            BusDDR_On;
    // Partition Access Info
    uint8_t             BootPartitionEnable;
    uint8_t             PartitionAccess;
    uint16_t            PartitionSwitchTime;
    // Partition Info
    uint8_t             PartitionCompleted;
    uint8_t             EraseGroupDef;
    // Partition Size : Physical Size [B]
    uint64_t            DataSize;
    uint32_t            Boot1Size;
    uint32_t            Boot2Size;
    uint32_t            RPMBSize;
    uint64_t            MaxEnhSize;
    uint64_t            GP1Size;
    uint64_t            GP2Size;
    uint64_t            GP3Size;
    uint64_t            GP4Size;
    uint64_t            DataEnhSize;
    uint64_t            DataEnhAddr;
    // Block Size & Control
    uint16_t            ReadBlkLen;
    uint8_t             ReadBlkLenBit4Addr;
    uint16_t            WriteBlkLen;
    uint8_t             WriteBlkLenBit4Addr;
    //uint32_t            ReadUnitSize;
    //uint32_t            WriteUnitSize;
    //uint8_t             HighCap_On;
    uint32_t            AccessSize;
    uint32_t            CacheSize;
    uint32_t            HcEraseUnitSize;
    uint32_t            HcWpGrpSize;
    uint16_t            HcEraseTimeout;
    uint16_t            HcReadTimeout;
    uint16_t            HcWriteTimeout;
    uint32_t            EraseUnitSize;
    uint32_t            WpGrpSize;
    uint16_t            EraseTimeout;
    uint16_t            ReadTimeout;
    uint16_t            WriteTimeout;
    // HPI Flag
    uint8_t             HPI_On;
    //--------------------------------------------
    // In(Read)/Out(Write) Delay Control for Chip IO Pad
    uint8_t             HostInDly_On;
    uint8_t             HostInDly_Ctrl;
    uint8_t             HostInDly_Dly;
    uint8_t             HostOutDly_On;
    uint8_t             HostOutDly_Ctrl;
    uint8_t             HostOutDly_Dly;
} emmcdev_config_t;


typedef struct emmcdev_s {
    // Registers in eMMC device
    uint32_t            OCR;
    emmcdev_cid_t       CID;
    emmcdev_csd_t       CSD;
    emmcdev_extcsd_t    ExtCSD;
    uint32_t            RCA;
    uint32_t            DSR;
    uint32_t            CST;    // status
    // Configuration from eMMC device register data
    emmcdev_config_t    config;
} emmcdev_t;


/*
 * Partition structure - use this to define a emmc flash "partition."
 * The partitions are assigned in order from the beginning of the emmc flash.
 * The special size '0' means 'fill to end of emmc flash', and you can
 * have more partitions after that which are aligned with the top
 * of the eMMC FLASH.
 * Therefore if you have a 1MB flash and set up
 * partitions for 256KB, 0, 128KB, the 128KB part will be aligned
 * to the top of the flash and the middle block will be 768KB.
 * Partitions can be on byte boundaries.
 */

/* Structure which specifies logical flash partitions */
typedef struct emmcflash_logicalpart_spec_t{
    uint64_t   fp_size;
    char       fp_name[EMMC_MAX_PART_NAME];
    int        fp_partition;    // Partitiion attribute
    uint64_t   fp_offset_bytes;
} emmcflash_logicalpart_spec_t;

/*
 * Probe structure - this is used when we want to describe to the flash
 * driver the layout of our physical flash partition
 */
typedef struct emmcflash_probe_t {
    /* For flash_info_t : getting value by emmc_get_config( ) */
    unsigned int    flash_type;         /* N/A FLASH_TYPE_xxx */
    uint64_t        flash_phy_addr;     /* N/A Physical address of eMMC partition */
    uint64_t        flash_phy_size;     /* Physical size of eMMC partition */
    uint32_t        flash_block_size;   /* block(sector) size in emmc_config */

    /* eMMC registers & configuration */
    emmcdev_t       emmc_config;

    /* CFE partition information in eMMC partition. */
    int                 flash_part_attr;    /* EMMC_PART_ACCESS_ATTR physical partition type */
    int                 flash_nparts;       /* All logical paritions. Zero means not partitioned. */
    uint64_t            flash_log_size;     /* total size of logical partitions */
    emmcflash_logicalpart_spec_t    flash_part_spec[EMMC_FLASH_MAX_LOGICAL_PARTS];
    
    /* The following are used for whacky, weird flashes */
    int             (*flash_ioctl_hook)(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
    
} emmcflash_probe_t;


/*  *********************************************************************
    *  PRIVATE STRUCTURES
    *
    *  These structures are actually the "property" of the eMMC flash driver.  
    ********************************************************************* */

typedef struct emmcflashdev_s emmcflashdev_t;

/* Structure which holds the emmc cfe partition configuration */
typedef struct emmcflash_cfepart_cfg_s {
    emmcflashdev_t  *fp_dev;
    uint64_t        fp_offset_bytes;
    uint64_t        fp_size;
    int             fp_partition;
} emmcflash_cfepart_cfg_t;

struct emmcflashdev_s {
    /* Input parameters */
    emmcflash_probe_t   fd_probe;        /* probe information from 'Bcm97xxx_devs.c' */
    /* Property parameters for usage */ 
    uint8_t         *fd_sectorbuffer;   /* sector copy buffer for read/write */
    emmcflash_cfepart_cfg_t fd_part_cfg[EMMC_FLASH_MAX_LOGICAL_PARTS];    /* To attach device */
    uint64_t        fd_pttlsize;        /* Physical total size of all partition in eMMC */
    uint64_t        fd_lttlsize;        /* Logical total size of all partition in eMMC which CFE uses. */
};


typedef struct
{
    uint64_t new_range_base;            // Block address in eMMC
    uint32_t first_block_data_offset;   // [Byte] in first blcok
    uint32_t last_block_data_offset;    // [Byte] in last blcok
    uint32_t num_block_access;          // to r/w
}emmc_range_descriptor_t;


typedef struct
{
    uint32_t block_attr;
    uint64_t block_offset;  // offset in flash physical partition
    uint64_t block_address; // address[Byte]
    uint32_t block_size;    // To r/w
    uint32_t block_cnt;
    uint8_t  *read_buffer;
    uint8_t  *data_buffer;
    uint32_t data_offset;
    uint64_t data_size;     // total data size[B] to r/w
    uint32_t data_copy_len; // data size[B] to r/w 
    uint32_t dma_address;
}emmc_data_descriptor_t;



/********************************************************************/
// Function Declaration : call by 'Bcm97xxx_devs.c'
void emmc_set_device_name( void );
char * emmc_get_manufacturer_name( int mid );
int emmc_Initialize( emmcdev_t *emmcdev );
uint64_t emmc_get_dev_phy_size( emmcflash_probe_t *fd_probe );
uint64_t emmc_get_dev_log_size( emmcflash_probe_t *fd_probe );
int emmc_flash_image( PFILE_TAG pTag, uint8_t *imagePtr );
int emmc_nvram_get(PNVRAM_DATA pNvramData);
int emmc_nvram_set(PNVRAM_DATA pNvramData);
int emmc_load_bootfs_file( const char* fname, unsigned int fnsize,
               unsigned char** file, unsigned int* file_size );
int emmc_go_idle(void); 
int emmc_init_dev( void );
int emmc_get_info(void);
int emmc_erase_psi(void);
int emmc_boot_os_image(int imageNum);
int emmc_read_part_words(char * flashdev, unsigned int offset, unsigned int num_words);
int emmc_write_part_words(char * flashdev, unsigned int offset, unsigned int write_val);
int emmc_erase_partition(char * flashdev);
int emmc_erase_all(int leave_blank);
int emmc_erase_img( int img_num , int erase_cferom_nvram);
int emmc_dump_bootfs(char * flashdev, char * filename);
int emmc_format_gpt_parts( int emmcPhysPartAttr );
int emmc_dump_gpt_dataPhysPart(void);
int emmc_format_gpt_dataPhysPart(unsigned int bootfs_sizekb, unsigned int rootfs_sizekb, unsigned int data_sizekb,
                    unsigned int misc1_sizekb, unsigned int misc2_sizekb, unsigned int misc3_sizekb, unsigned int misc4_sizekb);
void emmc_allow_img_update_repartitions(int val);
int emmc_get_img_update_repart_web_var( char * var_value, int * length );

char *get_emmc_chosen_root(void);
void get_emmc_boot_cfe_version(char **version, int *size);

#if EMMC_PARTITION_CTRL
int emmc_Partition_DataArea( emmcdev_t *emmcdev );
#endif
/********************************************************************/


#endif

#endif
