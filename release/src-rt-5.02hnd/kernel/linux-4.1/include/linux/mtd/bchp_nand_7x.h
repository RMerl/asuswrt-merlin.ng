#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
<:copyright-BRCM:2014:DUAL/GPL:standard

   Copyright (c) 2014 Broadcom 
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
*/

#ifndef BCHP_NAND_7x_H__
#define BCHP_NAND_7x_H__

#include <bcm_map_part.h>
#if defined (CONFIG_ARM) || defined (CONFIG_ARM64)
#define BRCMNAND_CTL_BASE                       ((uintptr_t)(NAND_REG_BASE))
#define BRCMNAND_INTR_BASE                      ((uintptr_t)(NAND_INTR_BASE))
#define BRCMNAND_CACHE_BASE                     ((uintptr_t)(NAND_CACHE_BASE))
#else
#define BRCMNAND_CTL_BASE                       ((uintptr_t)(NAND_REG_BASE & 0x0fffffff))
#define BRCMNAND_INTR_BASE                      ((uintptr_t)(NAND_INTR_BASE & 0x0fffffff))
#define BRCMNAND_CACHE_BASE                     ((uintptr_t)(NAND_CACHE_BASE & 0x0fffffff))
#endif
#define BRCMNAND_FLD_ADDR(FLD)                  \
    ((uintptr_t)(BRCMNAND_CTL_BASE + (offsetof(NandCtrlRegs,FLD))))

#define BCHP_NAND_REG_START                     ((uintptr_t)BRCMNAND_CTL_BASE)
#define BCHP_NAND_REG_END                       ((uintptr_t)(BCHP_NAND_REG_START + \
                                                 sizeof(NandCtrlRegs)))

/***************************************************************************
 *NAND - Nand Flash Control Registers
 ***************************************************************************/
#define BCHP_NAND_REVISION                      BRCMNAND_FLD_ADDR(NandRevision) /* NAND Revision */
#define BCHP_NAND_CMD_START                     BRCMNAND_FLD_ADDR(NandCmdStart) /* Nand Flash Command Start */
#define BCHP_NAND_CMD_EXT_ADDRESS               BRCMNAND_FLD_ADDR(NandCmdExtAddr) /* Nand Flash Command Extended Address */
#define BCHP_NAND_CMD_ADDRESS                   BRCMNAND_FLD_ADDR(NandCmdAddr) /* Nand Flash Command Address */
#define BCHP_NAND_CMD_END_ADDRESS               BRCMNAND_FLD_ADDR(NandCmdEndAddr) /* Nand Flash Command End Address */
#define BCHP_NAND_CS_NAND_SELECT                BRCMNAND_FLD_ADDR(NandNandBootConfig) /* Nand Flash EBI CS Select */
#define BCHP_NAND_CS_NAND_XOR                   BRCMNAND_FLD_ADDR(NandCsNandXor) /* Nand Flash EBI CS Address XOR with 1FC0 Control */
#define BCHP_NAND_SPARE_AREA_READ_OFS_0         BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs0) /* Nand Flash Spare Area Read Bytes 0-3 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_4         BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs4) /* Nand Flash Spare Area Read Bytes 4-7 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_8         BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs8) /* Nand Flash Spare Area Read Bytes 8-11 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_C         BRCMNAND_FLD_ADDR(NandSpareAreaReadOfsC) /* Nand Flash Spare Area Read Bytes 12-15 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0        BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs0) /* Nand Flash Spare Area Write Bytes 0-3 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4        BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs4) /* Nand Flash Spare Area Write Bytes 4-7 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8        BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs8) /* Nand Flash Spare Area Write Bytes 8-11 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C        BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfsC) /* Nand Flash Spare Area Write Bytes 12-15 */
#define BCHP_NAND_ACC_CONTROL                   BRCMNAND_FLD_ADDR(NandAccControl) /* Nand Flash Access Control */
#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_7_0
#define BCHP_NAND_CONFIG_EXT                    BRCMNAND_FLD_ADDR(NandConfigExt) /* Nand Flash Config Ext */
#endif
#define BCHP_NAND_CONFIG                        BRCMNAND_FLD_ADDR(NandConfig) /* Nand Flash Config */
#define BCHP_NAND_TIMING_1                      BRCMNAND_FLD_ADDR(NandTiming1) /* Nand Flash Timing Parameters 1 */
#define BCHP_NAND_TIMING_2                      BRCMNAND_FLD_ADDR(NandTiming2) /* Nand Flash Timing Parameters 2 */
#define BCHP_NAND_SEMAPHORE                     BRCMNAND_FLD_ADDR(NandSemaphore) /* Semaphore */
#define BCHP_NAND_FLASH_DEVICE_ID               BRCMNAND_FLD_ADDR(NandFlashDeviceId) /* Nand Flash Device ID */
#define BCHP_NAND_FLASH_DEVICE_ID_EXT           BRCMNAND_FLD_ADDR(NandFlashDeviceIdExt) /* Nand Flash Extended Device ID */
#define BCHP_NAND_BLOCK_LOCK_STATUS             BRCMNAND_FLD_ADDR(NandBlockLockStatus) /* Nand Flash Block Lock Status */
#define BCHP_NAND_INTFC_STATUS                  BRCMNAND_FLD_ADDR(NandIntfcStatus) /* Nand Flash Interface Status */
#define BCHP_NAND_ECC_CORR_EXT_ADDR             BRCMNAND_FLD_ADDR(NandEccCorrExtAddr) /* ECC Correctable Error Extended Address */
#define BCHP_NAND_ECC_CORR_ADDR                 BRCMNAND_FLD_ADDR(NandEccCorrAddr) /* ECC Correctable Error Address */
#define BCHP_NAND_ECC_UNC_EXT_ADDR              BRCMNAND_FLD_ADDR(NandEccUncExtAddr) /* ECC Uncorrectable Error Extended Address */
#define BCHP_NAND_ECC_UNC_ADDR                  BRCMNAND_FLD_ADDR(NandEccUncAddr) /* ECC Uncorrectable Error Address */
#define BCHP_NAND_READ_ERROR_COUNT              BRCMNAND_FLD_ADDR(NandReadErrorCount) /* Read Error Count */
#define BCHP_NAND_CORR_STAT_THRESHOLD           BRCMNAND_FLD_ADDR(NandCorrStatThreshold) /* Correctable Error Reporting Threshold */
#define BCHP_NAND_ONFI_STATUS                   BRCMNAND_FLD_ADDR(NandOnfiStatus) /* ONFI Status */
#define BCHP_NAND_ONFI_DEBUG_DATA               BRCMNAND_FLD_ADDR(NandOnfiDebugData) /* ONFI Debug Data */
#define BCHP_NAND_FLASH_READ_EXT_ADDR           BRCMNAND_FLD_ADDR(NandFlashReadExtAddr) /* Flash Read Data Extended Address */
#define BCHP_NAND_FLASH_READ_ADDR               BRCMNAND_FLD_ADDR(NandFlashReadAddr) /* Flash Read Data Address */
#define BCHP_NAND_PROGRAM_PAGE_EXT_ADDR         BRCMNAND_FLD_ADDR(NandProgramPageExtAddr) /* Page Program Extended Address */
#define BCHP_NAND_PROGRAM_PAGE_ADDR             BRCMNAND_FLD_ADDR(NandProgramPageAddr) /* Page Program Address */
#define BCHP_NAND_COPY_BACK_EXT_ADDR            BRCMNAND_FLD_ADDR(NandCopyBackExtAddr) /* Copy Back Extended Address */
#define BCHP_NAND_COPY_BACK_ADDR                BRCMNAND_FLD_ADDR(NandCopyBackAddr) /* Copy Back Address */
#define BCHP_NAND_BLOCK_ERASE_EXT_ADDR          BRCMNAND_FLD_ADDR(NandBlockEraseExtAddr) /* Block Erase Extended Address */
#define BCHP_NAND_BLOCK_ERASE_ADDR              BRCMNAND_FLD_ADDR(NandBlockEraseAddr) /* Block Erase Address */
#define BCHP_NAND_INV_READ_EXT_ADDR             BRCMNAND_FLD_ADDR(NandInvReadExtAddr) /* Flash Invalid Data Extended Address */
#define BCHP_NAND_INV_READ_ADDR                 BRCMNAND_FLD_ADDR(NandInvReadAddr) /* Flash Invalid Data Address */
#define BCHP_NAND_BLK_WR_PROTECT                BRCMNAND_FLD_ADDR(NandBlkWrProtect) /* Block Write Protect Enable and Size for EBI_CS0b */
#define BCHP_NAND_ACC_CONTROL_CS1               BRCMNAND_FLD_ADDR(NandAccControlCs1) /* Nand Flash Access Control */
#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_7_0
#define BCHP_NAND_CONFIG_EXT_CS1                BRCMNAND_FLD_ADDR(NandConfigExtCs1) /* Nand Flash Config Ext */
#endif
#define BCHP_NAND_CONFIG_CS1                    BRCMNAND_FLD_ADDR(NandConfigCs1) /* Nand Flash Config */
#define BCHP_NAND_TIMING_1_CS1                  BRCMNAND_FLD_ADDR(NandTiming1Cs1) /* Nand Flash Timing Parameters 1 */
#define BCHP_NAND_TIMING_2_CS1                  BRCMNAND_FLD_ADDR(NandTiming2Cs1) /* Nand Flash Timing Parameters 2 */
#define BCHP_NAND_ACC_CONTROL_CS2               BRCMNAND_FLD_ADDR(NandAccControlCs2) /* Nand Flash Access Control */
#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_7_0
#define BCHP_NAND_CONFIG_EXT_CS2                BRCMNAND_FLD_ADDR(NandConfigExtCs2) /* Nand Flash Config Ext */
#endif
#define BCHP_NAND_CONFIG_CS2                    BRCMNAND_FLD_ADDR(NandConfigCs2) /* Nand Flash Config */
#define BCHP_NAND_TIMING_1_CS2                  BRCMNAND_FLD_ADDR(NandTiming1Cs2) /* Nand Flash Timing Parameters 1 */
#define BCHP_NAND_TIMING_2_CS2                  BRCMNAND_FLD_ADDR(NandTiming2Cs2) /* Nand Flash Timing Parameters 2 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_10        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs10) /* Nand Flash Spare Area Read Bytes 16-19 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_14        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs14) /* Nand Flash Spare Area Read Bytes 20-23 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_18        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs18) /* Nand Flash Spare Area Read Bytes 24-27 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs1C) /* Nand Flash Spare Area Read Bytes 28-31 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_20        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs20) /* Nand Flash Spare Area Read Bytes 32-35 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_24        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs24) /* Nand Flash Spare Area Read Bytes 36-39 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_28        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs28) /* Nand Flash Spare Area Read Bytes 40-43 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_2C        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs2C) /* Nand Flash Spare Area Read Bytes 44-47 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_30        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs30) /* Nand Flash Spare Area Read Bytes 48-51*/
#define BCHP_NAND_SPARE_AREA_READ_OFS_34        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs34) /* Nand Flash Spare Area Read Bytes 52-55 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_38        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs38) /* Nand Flash Spare Area Read Bytes 56-59 */
#define BCHP_NAND_SPARE_AREA_READ_OFS_3C        BRCMNAND_FLD_ADDR(NandSpareAreaReadOfs3C) /* Nand Flash Spare Area Read Bytes 60-63 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_10       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs10) /* Nand Flash Spare Area Write Bytes 16-19 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_14       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs14) /* Nand Flash Spare Area Write Bytes 20-23 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_18       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs18) /* Nand Flash Spare Area Write Bytes 24-27 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_1C       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs1C) /* Nand Flash Spare Area Write Bytes 28-31 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_20       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs20) /* Nand Flash Spare Area Write Bytes 32-35 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_24       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs24) /* Nand Flash Spare Area Write Bytes 36-39 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_28       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs28) /* Nand Flash Spare Area Write Bytes 40-43 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_2C       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs2C) /* Nand Flash Spare Area Write Bytes 44-47 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_30       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs30) /* Nand Flash Spare Area Write Bytes 48-51 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_34       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs34) /* Nand Flash Spare Area Write Bytes 52-55 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_38       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs38) /* Nand Flash Spare Area Write Bytes 56-59 */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_3C       BRCMNAND_FLD_ADDR(NandSpareAreaWriteOfs3C) /* Nand Flash Spare Area Write Bytes 60-63 */
#define BCHP_NAND_LL_OP                         BRCMNAND_FLD_ADDR(NandLlOpNand) /* Nand Flash Low Level Operation */
#define BCHP_NAND_LL_RDDATA                     BRCMNAND_FLD_ADDR(NandLlRdData) /* Nand Flash Low Level Read Data */

/***************************************************************************
 *REVISION - NAND Revision
 ***************************************************************************/
/* NAND :: REVISION :: 8KB_PAGE_SUPPORT [31:31] */
#define BCHP_NAND_REVISION_8KB_PAGE_SUPPORT_MASK                   0x80000000
#define BCHP_NAND_REVISION_8KB_PAGE_SUPPORT_SHIFT                  31

/* NAND :: REVISION :: reserved0 [30:16] */
#define BCHP_NAND_REVISION_reserved0_MASK                          0x7fff0000
#define BCHP_NAND_REVISION_reserved0_SHIFT                         16

/* NAND :: REVISION :: MAJOR [15:08] */
#define BCHP_NAND_REVISION_MAJOR_MASK                              0x0000ff00
#define BCHP_NAND_REVISION_MAJOR_SHIFT                             8

/* NAND :: REVISION :: MINOR [07:00] */
#define BCHP_NAND_REVISION_MINOR_MASK                              0x000000ff
#define BCHP_NAND_REVISION_MINOR_SHIFT                             0

/***************************************************************************
 *CMD_START - Nand Flash Command Start
 ***************************************************************************/
/* NAND :: CMD_START :: reserved0 [31:05] */
#define BCHP_NAND_CMD_START_reserved_MASK                          0xffffffe0
#define BCHP_NAND_CMD_START_reserved_SHIFT                         5

/* NAND :: CMD_START :: OPCODE [04:00] */
#define BCHP_NAND_CMD_START_OPCODE_MASK                            0x1f
#define BCHP_NAND_CMD_START_OPCODE_SHIFT                           0
#define BCHP_NAND_CMD_START_OPCODE_NULL                            0
#define BCHP_NAND_CMD_START_OPCODE_PAGE_READ                       1
#define BCHP_NAND_CMD_START_OPCODE_SPARE_AREA_READ                 2
#define BCHP_NAND_CMD_START_OPCODE_STATUS_READ                     3
#define BCHP_NAND_CMD_START_OPCODE_PROGRAM_PAGE                    4
#define BCHP_NAND_CMD_START_OPCODE_PROGRAM_SPARE_AREA              5
#define BCHP_NAND_CMD_START_OPCODE_COPY_BACK                       6
#define BCHP_NAND_CMD_START_OPCODE_DEVICE_ID_READ                  7
#define BCHP_NAND_CMD_START_OPCODE_BLOCK_ERASE                     8
#define BCHP_NAND_CMD_START_OPCODE_FLASH_RESET                     9
#define BCHP_NAND_CMD_START_OPCODE_BLOCKS_LOCK                     10
#define BCHP_NAND_CMD_START_OPCODE_BLOCKS_LOCK_DOWN                11
#define BCHP_NAND_CMD_START_OPCODE_BLOCKS_UNLOCK                   12
#define BCHP_NAND_CMD_START_OPCODE_READ_BLOCKS_LOCK_STATUS         13
#define BCHP_NAND_CMD_START_OPCODE_PARAMETER_READ                  14
#define BCHP_NAND_CMD_START_OPCODE_PARAMETER_CHANGE_COL            15
#define BCHP_NAND_CMD_START_OPCODE_LOW_LEVEL_OP                    16

/***************************************************************************
 *CMD_EXT_ADDRESS - Nand Flash Command Extended Address
 ***************************************************************************/
/* NAND :: CMD_EXT_ADDRESS :: reserved0 [31:19] */
#define BCHP_NAND_CMD_EXT_ADDRESS_reserved0_MASK                   0xfff80000
#define BCHP_NAND_CMD_EXT_ADDRESS_reserved0_SHIFT                  19

/* NAND :: CMD_EXT_ADDRESS :: CS_SEL [18:16] */
#define BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_MASK                      0x00070000
#define BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT                     16

/* NAND :: CMD_EXT_ADDRESS :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_CMD_EXT_ADDRESS_EXT_ADDRESS_MASK                 0x0000ffff
#define BCHP_NAND_CMD_EXT_ADDRESS_EXT_ADDRESS_SHIFT                0

/***************************************************************************
 *CMD_ADDRESS - Nand Flash Command Address
 ***************************************************************************/
/* NAND :: CMD_ADDRESS :: ADDRESS [31:00] */
#define BCHP_NAND_CMD_ADDRESS_ADDRESS_MASK                         0xffffffff
#define BCHP_NAND_CMD_ADDRESS_ADDRESS_SHIFT                        0

/***************************************************************************
 *CMD_END_ADDRESS - Nand Flash Command End Address
 ***************************************************************************/
/* NAND :: CMD_END_ADDRESS :: ADDRESS [31:00] */
#define BCHP_NAND_CMD_END_ADDRESS_ADDRESS_MASK                     0xffffffff
#define BCHP_NAND_CMD_END_ADDRESS_ADDRESS_SHIFT                    0

/***************************************************************************
 *CS_NAND_SELECT - Nand Flash EBI CS Select
 ***************************************************************************/
/* NAND :: CS_NAND_SELECT :: CS_LOCK [31:31] */
#define BCHP_NAND_CS_NAND_SELECT_CS_LOCK_MASK                      0x80000000
#define BCHP_NAND_CS_NAND_SELECT_CS_LOCK_SHIFT                     31

/* NAND :: CS_NAND_SELECT :: AUTO_DEVICE_ID_CONFIG [30:30] */
#define BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_MASK        0x40000000
#define BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_SHIFT       30

/* NAND :: CS_NAND_SELECT :: reserved0 [29:29] */
#define BCHP_NAND_CS_NAND_SELECT_NAND_WP_MASK                    0x20000000
#define BCHP_NAND_CS_NAND_SELECT_NAND_WP_SHIFT                   29

/* NAND :: CS_NAND_SELECT :: WR_PROTECT_BLK0 [28:28] */
#define BCHP_NAND_CS_NAND_SELECT_WR_PROTECT_BLK0_MASK              0x10000000
#define BCHP_NAND_CS_NAND_SELECT_WR_PROTECT_BLK0_SHIFT             28

/* NAND :: CS_NAND_SELECT :: reserved1 [27:16] */
#define BCHP_NAND_CS_NAND_SELECT_reserved1_MASK                    0x0fff0000
#define BCHP_NAND_CS_NAND_SELECT_reserved1_SHIFT                   16

/* NAND :: CS_NAND_SELECT :: EBI_CS_7_USES_NAND [15:15] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_7_USES_NAND_MASK           0x00008000
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_7_USES_NAND_SHIFT          15

/* NAND :: CS_NAND_SELECT :: EBI_CS_6_USES_NAND [14:14] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_6_USES_NAND_MASK           0x00004000
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_6_USES_NAND_SHIFT          14

/* NAND :: CS_NAND_SELECT :: EBI_CS_5_USES_NAND [13:13] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_5_USES_NAND_MASK           0x00002000
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_5_USES_NAND_SHIFT          13

/* NAND :: CS_NAND_SELECT :: EBI_CS_4_USES_NAND [12:12] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_4_USES_NAND_MASK           0x00001000
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_4_USES_NAND_SHIFT          12

/* NAND :: CS_NAND_SELECT :: EBI_CS_3_USES_NAND [11:11] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_3_USES_NAND_MASK           0x00000800
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_3_USES_NAND_SHIFT          11

/* NAND :: CS_NAND_SELECT :: EBI_CS_2_USES_NAND [10:10] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_2_USES_NAND_MASK           0x00000400
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_2_USES_NAND_SHIFT          10

/* NAND :: CS_NAND_SELECT :: EBI_CS_1_USES_NAND [09:09] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_1_USES_NAND_MASK           0x00000200
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_1_USES_NAND_SHIFT          9

/* NAND :: CS_NAND_SELECT :: EBI_CS_0_USES_NAND [08:08] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_USES_NAND_MASK           0x00000100
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_USES_NAND_SHIFT          8

/* NAND :: CS_NAND_SELECT :: EBI_CS_7_SEL [07:07] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_7_SEL_MASK                 0x00000080
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_7_SEL_SHIFT                7

/* NAND :: CS_NAND_SELECT :: EBI_CS_6_SEL [06:06] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_6_SEL_MASK                 0x00000040
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_6_SEL_SHIFT                6

/* NAND :: CS_NAND_SELECT :: EBI_CS_5_SEL [05:05] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_5_SEL_MASK                 0x00000020
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_5_SEL_SHIFT                5

/* NAND :: CS_NAND_SELECT :: EBI_CS_4_SEL [04:04] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_4_SEL_MASK                 0x00000010
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_4_SEL_SHIFT                4

/* NAND :: CS_NAND_SELECT :: EBI_CS_3_SEL [03:03] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_3_SEL_MASK                 0x00000008
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_3_SEL_SHIFT                3

/* NAND :: CS_NAND_SELECT :: EBI_CS_2_SEL [02:02] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_2_SEL_MASK                 0x00000004
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_2_SEL_SHIFT                2

/* NAND :: CS_NAND_SELECT :: EBI_CS_1_SEL [01:01] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_1_SEL_MASK                 0x00000002
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_1_SEL_SHIFT                1

/* NAND :: CS_NAND_SELECT :: EBI_CS_0_SEL [00:00] */
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_MASK                 0x00000001
#define BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_SHIFT                0

/***************************************************************************
 *CS_NAND_XOR - Nand Flash EBI CS Address XOR with 1FC0 Control
 ***************************************************************************/
/* NAND :: CS_NAND_XOR :: ONLY_BLOCK_0_1FC0_XOR [31:31] */
#define BCHP_NAND_CS_NAND_XOR_ONLY_BLOCK_0_1FC0_XOR_MASK           0x80000000
#define BCHP_NAND_CS_NAND_XOR_ONLY_BLOCK_0_1FC0_XOR_SHIFT          31

/* NAND :: CS_NAND_XOR :: reserved0 [30:08] */
#define BCHP_NAND_CS_NAND_XOR_reserved0_MASK                       0x7fffff00
#define BCHP_NAND_CS_NAND_XOR_reserved0_SHIFT                      8

/* NAND :: CS_NAND_XOR :: EBI_CS_7_ADDR_1FC0_XOR [07:07] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_7_ADDR_1FC0_XOR_MASK          0x00000080
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_7_ADDR_1FC0_XOR_SHIFT         7

/* NAND :: CS_NAND_XOR :: EBI_CS_6_ADDR_1FC0_XOR [06:06] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_6_ADDR_1FC0_XOR_MASK          0x00000040
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_6_ADDR_1FC0_XOR_SHIFT         6

/* NAND :: CS_NAND_XOR :: EBI_CS_5_ADDR_1FC0_XOR [05:05] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_5_ADDR_1FC0_XOR_MASK          0x00000020
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_5_ADDR_1FC0_XOR_SHIFT         5

/* NAND :: CS_NAND_XOR :: EBI_CS_4_ADDR_1FC0_XOR [04:04] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_4_ADDR_1FC0_XOR_MASK          0x00000010
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_4_ADDR_1FC0_XOR_SHIFT         4

/* NAND :: CS_NAND_XOR :: EBI_CS_3_ADDR_1FC0_XOR [03:03] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_3_ADDR_1FC0_XOR_MASK          0x00000008
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_3_ADDR_1FC0_XOR_SHIFT         3

/* NAND :: CS_NAND_XOR :: EBI_CS_2_ADDR_1FC0_XOR [02:02] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_2_ADDR_1FC0_XOR_MASK          0x00000004
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_2_ADDR_1FC0_XOR_SHIFT         2

/* NAND :: CS_NAND_XOR :: EBI_CS_1_ADDR_1FC0_XOR [01:01] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_1_ADDR_1FC0_XOR_MASK          0x00000002
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_1_ADDR_1FC0_XOR_SHIFT         1

/* NAND :: CS_NAND_XOR :: EBI_CS_0_ADDR_1FC0_XOR [00:00] */
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_0_ADDR_1FC0_XOR_MASK          0x00000001
#define BCHP_NAND_CS_NAND_XOR_EBI_CS_0_ADDR_1FC0_XOR_SHIFT         0

/***************************************************************************
 *SPARE_AREA_READ_OFS_0 - Nand Flash Spare Area Read Bytes 0-3
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_0 :: BYTE_OFS_0 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_0_MASK            0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_0_SHIFT           24

/* NAND :: SPARE_AREA_READ_OFS_0 :: BYTE_OFS_1 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_1_MASK            0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_1_SHIFT           16

/* NAND :: SPARE_AREA_READ_OFS_0 :: BYTE_OFS_2 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_2_MASK            0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_2_SHIFT           8

/* NAND :: SPARE_AREA_READ_OFS_0 :: BYTE_OFS_3 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_3_MASK            0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_0_BYTE_OFS_3_SHIFT           0

/***************************************************************************
 *SPARE_AREA_READ_OFS_4 - Nand Flash Spare Area Read Bytes 4-7
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_4 :: BYTE_OFS_4 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_4_MASK            0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_4_SHIFT           24

/* NAND :: SPARE_AREA_READ_OFS_4 :: BYTE_OFS_5 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_5_MASK            0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_5_SHIFT           16

/* NAND :: SPARE_AREA_READ_OFS_4 :: BYTE_OFS_6 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_6_MASK            0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_6_SHIFT           8

/* NAND :: SPARE_AREA_READ_OFS_4 :: BYTE_OFS_7 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_7_MASK            0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_4_BYTE_OFS_7_SHIFT           0

/***************************************************************************
 *SPARE_AREA_READ_OFS_8 - Nand Flash Spare Area Read Bytes 8-11
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_8 :: BYTE_OFS_8 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_8_MASK            0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_8_SHIFT           24

/* NAND :: SPARE_AREA_READ_OFS_8 :: BYTE_OFS_9 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_9_MASK            0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_9_SHIFT           16

/* NAND :: SPARE_AREA_READ_OFS_8 :: BYTE_OFS_10 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_10_MASK           0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_10_SHIFT          8

/* NAND :: SPARE_AREA_READ_OFS_8 :: BYTE_OFS_11 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_11_MASK           0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_8_BYTE_OFS_11_SHIFT          0

/***************************************************************************
 *SPARE_AREA_READ_OFS_C - Nand Flash Spare Area Read Bytes 12-15
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_C :: BYTE_OFS_12 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_12_MASK           0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_12_SHIFT          24

/* NAND :: SPARE_AREA_READ_OFS_C :: BYTE_OFS_13 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_13_MASK           0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_13_SHIFT          16

/* NAND :: SPARE_AREA_READ_OFS_C :: BYTE_OFS_14 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_14_MASK           0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_14_SHIFT          8

/* NAND :: SPARE_AREA_READ_OFS_C :: BYTE_OFS_15 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_15_MASK           0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_C_BYTE_OFS_15_SHIFT          0

/***************************************************************************
 *SPARE_AREA_WRITE_OFS_0 - Nand Flash Spare Area Write Bytes 0-3
 ***************************************************************************/
/* NAND :: SPARE_AREA_WRITE_OFS_0 :: BYTE_OFS_0 [31:24] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_0_MASK           0xff000000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_0_SHIFT          24

/* NAND :: SPARE_AREA_WRITE_OFS_0 :: BYTE_OFS_1 [23:16] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_1_MASK           0x00ff0000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_1_SHIFT          16

/* NAND :: SPARE_AREA_WRITE_OFS_0 :: BYTE_OFS_2 [15:08] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_2_MASK           0x0000ff00
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_2_SHIFT          8

/* NAND :: SPARE_AREA_WRITE_OFS_0 :: BYTE_OFS_3 [07:00] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_3_MASK           0x000000ff
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_0_BYTE_OFS_3_SHIFT          0

/***************************************************************************
 *SPARE_AREA_WRITE_OFS_4 - Nand Flash Spare Area Write Bytes 4-7
 ***************************************************************************/
/* NAND :: SPARE_AREA_WRITE_OFS_4 :: BYTE_OFS_4 [31:24] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_4_MASK           0xff000000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_4_SHIFT          24

/* NAND :: SPARE_AREA_WRITE_OFS_4 :: BYTE_OFS_5 [23:16] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_5_MASK           0x00ff0000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_5_SHIFT          16

/* NAND :: SPARE_AREA_WRITE_OFS_4 :: BYTE_OFS_6 [15:08] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_6_MASK           0x0000ff00
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_6_SHIFT          8

/* NAND :: SPARE_AREA_WRITE_OFS_4 :: BYTE_OFS_7 [07:00] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_7_MASK           0x000000ff
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_4_BYTE_OFS_7_SHIFT          0

/***************************************************************************
 *SPARE_AREA_WRITE_OFS_8 - Nand Flash Spare Area Write Bytes 8-11
 ***************************************************************************/
/* NAND :: SPARE_AREA_WRITE_OFS_8 :: BYTE_OFS_8 [31:24] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_8_MASK           0xff000000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_8_SHIFT          24

/* NAND :: SPARE_AREA_WRITE_OFS_8 :: BYTE_OFS_9 [23:16] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_9_MASK           0x00ff0000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_9_SHIFT          16

/* NAND :: SPARE_AREA_WRITE_OFS_8 :: BYTE_OFS_10 [15:08] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_10_MASK          0x0000ff00
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_10_SHIFT         8

/* NAND :: SPARE_AREA_WRITE_OFS_8 :: BYTE_OFS_11 [07:00] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_11_MASK          0x000000ff
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_8_BYTE_OFS_11_SHIFT         0

/***************************************************************************
 *SPARE_AREA_WRITE_OFS_C - Nand Flash Spare Area Write Bytes 12-15
 ***************************************************************************/
/* NAND :: SPARE_AREA_WRITE_OFS_C :: BYTE_OFS_12 [31:24] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_12_MASK          0xff000000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_12_SHIFT         24

/* NAND :: SPARE_AREA_WRITE_OFS_C :: BYTE_OFS_13 [23:16] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_13_MASK          0x00ff0000
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_13_SHIFT         16

/* NAND :: SPARE_AREA_WRITE_OFS_C :: BYTE_OFS_14 [15:08] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_14_MASK          0x0000ff00
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_14_SHIFT         8

/* NAND :: SPARE_AREA_WRITE_OFS_C :: BYTE_OFS_15 [07:00] */
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_15_MASK          0x000000ff
#define BCHP_NAND_SPARE_AREA_WRITE_OFS_C_BYTE_OFS_15_SHIFT         0

/***************************************************************************
 *ACC_CONTROL - Nand Flash Access Control
 ***************************************************************************/
/* NAND :: ACC_CONTROL :: RD_ECC_EN [31:31] */
#define BCHP_NAND_ACC_CONTROL_RD_ECC_EN_MASK                       0x80000000
#define BCHP_NAND_ACC_CONTROL_RD_ECC_EN_SHIFT                      31

/* NAND :: ACC_CONTROL :: WR_ECC_EN [30:30] */
#define BCHP_NAND_ACC_CONTROL_WR_ECC_EN_MASK                       0x40000000
#define BCHP_NAND_ACC_CONTROL_WR_ECC_EN_SHIFT                      30

/* NAND :: ACC_CONTROL :: RD_ECC_BLK0_EN [29:29] */
#define BCHP_NAND_ACC_CONTROL_CE_CARE_MASK                         0x20000000
#define BCHP_NAND_ACC_CONTROL_CE_CARE_SHIFT                        29

/* NAND :: ACC_CONTROL :: reserve1 [28:28] */
#define BCHP_NAND_ACC_CONTROL_RESERVED1_MASK                       0x10000000
#define BCHP_NAND_ACC_CONTROL_RESERVED1_SHIFT                      28

/* NAND :: ACC_CONTROL :: RD_ERASED_ECC_EN [27:27] */
#define BCHP_NAND_ACC_CONTROL_RD_ERASED_ECC_EN_MASK                0x08000000
#define BCHP_NAND_ACC_CONTROL_RD_ERASED_ECC_EN_SHIFT               27

/* NAND :: ACC_CONTROL :: PARTIAL_PAGE_EN [26:26] */
#define BCHP_NAND_ACC_CONTROL_PARTIAL_PAGE_EN_MASK                 0x04000000
#define BCHP_NAND_ACC_CONTROL_PARTIAL_PAGE_EN_SHIFT                26

/* NAND :: ACC_CONTROL :: WR_PREEMPT_EN [25:25] */
#define BCHP_NAND_ACC_CONTROL_WR_PREEMPT_EN_MASK                   0x02000000
#define BCHP_NAND_ACC_CONTROL_WR_PREEMPT_EN_SHIFT                  25

/* NAND :: ACC_CONTROL :: PAGE_HIT_EN [24:24] */
#define BCHP_NAND_ACC_CONTROL_PAGE_HIT_EN_MASK                     0x01000000
#define BCHP_NAND_ACC_CONTROL_PAGE_HIT_EN_SHIFT                    24

/* NAND :: ACC_CONTROL :: PREFETCH_EN [23:23] */
#define BCHP_NAND_ACC_CONTROL_PREFETCH_EN_MASK                     0x00800000
#define BCHP_NAND_ACC_CONTROL_PREFETCH_EN_SHIFT                    23

/* NAND :: ACC_CONTROL :: CACHE_MODE_EN [22:22] */
#define BCHP_NAND_ACC_CONTROL_CACHE_MODE_EN_MASK                   0x00400000
#define BCHP_NAND_ACC_CONTROL_CACHE_MODE_EN_SHIFT                  22

/* NAND :: ACC_CONTROL :: reserve2 [21:21] */
#define BCHP_NAND_ACC_CONTROL_RESERVED2_MASK                       0x00200000
#define BCHP_NAND_ACC_CONTROL_RESERVED2_SHIFT                      21

/* NAND :: ACC_CONTROL :: ECC_LEVEL [20:16] */
#define BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK                       0x001f0000
#define BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT                      16

/* NAND :: ACC_CONTROL :: reserved3 [15:8] */
#define BCHP_NAND_ACC_CONTROL_RESERVED3_MASK                       0x0000ff00
#define BCHP_NAND_ACC_CONTROL_RESERVED3_SHIFT                      8

/* NAND :: ACC_CONTROL :: SECTOR_SIZE_1K [07:07] */
#define BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_MASK                  0x00000080
#define BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_SHIFT                 7

/* NAND :: ACC_CONTROL :: SPARE_AREA_SIZE [06:00] */
#define BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK                 0x0000007f
#define BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT                0

#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_7_0
/***************************************************************************
 *CONFIG EXT - Nand Flash Config Ext
 ***************************************************************************/
/* NAND :: CONFIG_EXT :: BLOCK_SIZE [11:4] */
#define BCHP_NAND_CONFIG_BLOCK_SIZE_MASK                           0x00000ff0
#define BCHP_NAND_CONFIG_BLOCK_SIZE_SHIFT                          4
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_8192KB                 10
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_4096KB                 9
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_2048KB                 8
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_1024KB                 7
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_512KB                  6
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_256KB                  5
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_128KB                  4
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_64KB                   3
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_32KB                   2
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_16KB                   1
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_8KB                    0

/* NAND :: CONFIG_EXT :: PAGE_SIZE [11:4] */
#define BCHP_NAND_CONFIG_PAGE_SIZE_MASK                            0x0000000f
#define BCHP_NAND_CONFIG_PAGE_SIZE_SHIFT                           0
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_512                     0
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_1KB                     1
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_2KB                     2
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_4KB                     3
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_8KB                     4
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_16KB                    5 

#endif

/***************************************************************************
 *CONFIG - Nand Flash Config
 ***************************************************************************/
/* NAND :: CONFIG :: CONFIG_LOCK [31:31] */
#define BCHP_NAND_CONFIG_CONFIG_LOCK_MASK                          0x80000000
#define BCHP_NAND_CONFIG_CONFIG_LOCK_SHIFT                         31

#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_7_0
/* NAND :: CONFIG :: BLOCK_SIZE [30:28] */
#define BCHP_NAND_CONFIG_BLOCK_SIZE_MASK                           0x70000000
#define BCHP_NAND_CONFIG_BLOCK_SIZE_SHIFT                          28
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_2048KB                 6
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_1024KB                 5
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_512KB                  4
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_256KB                  3
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_128KB                  2
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_16KB                   1
#define BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_8KB                    0
#endif

/* NAND :: CONFIG :: DEVICE_SIZE [27:24] */
#define BCHP_NAND_CONFIG_DEVICE_SIZE_MASK                          0x0f000000
#define BCHP_NAND_CONFIG_DEVICE_SIZE_SHIFT                         24
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_4MB                  0
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_8MB                  1
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_16MB                 2
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_32MB                 3
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_64MB                 4
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_128MB                5
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_256MB                6
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_512MB                7
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_1GB                  8
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_2GB                  9
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_4GB                  10
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_8GB                  11
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_16GB                 12
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_32GB                 13
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_64GB                 14
#define BCHP_NAND_CONFIG_DEVICE_SIZE_DVC_SIZE_128GB                15

/* NAND :: CONFIG :: DEVICE_WIDTH [23:23] */
#define BCHP_NAND_CONFIG_DEVICE_WIDTH_MASK                         0x00800000
#define BCHP_NAND_CONFIG_DEVICE_WIDTH_SHIFT                        23
#define BCHP_NAND_CONFIG_DEVICE_WIDTH_DVC_WIDTH_8                  0
#define BCHP_NAND_CONFIG_DEVICE_WIDTH_DVC_WIDTH_16                 1

/* NAND :: CONFIG :: reserved0 [22:22] */
#define BCHP_NAND_CONFIG_reserved0_MASK                            0x00400000
#define BCHP_NAND_CONFIG_reserved0_SHIFT                           22

#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_7_0
/* NAND :: CONFIG :: PAGE_SIZE [21:20] */
#define BCHP_NAND_CONFIG_PAGE_SIZE_MASK                            0x00300000
#define BCHP_NAND_CONFIG_PAGE_SIZE_SHIFT                           20
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_512                     0
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_2KB                     1
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_4KB                     2
#define BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_8KB                     3
#endif

/* NAND :: CONFIG :: reserved1 [19:19] */
#define BCHP_NAND_CONFIG_reserved1_MASK                            0x00080000
#define BCHP_NAND_CONFIG_reserved1_SHIFT                           19

/* NAND :: CONFIG :: FUL_ADR_BYTES [18:16] */
#define BCHP_NAND_CONFIG_FUL_ADR_BYTES_MASK                        0x00070000
#define BCHP_NAND_CONFIG_FUL_ADR_BYTES_SHIFT                       16

/* NAND :: CONFIG :: reserved2 [15:15] */
#define BCHP_NAND_CONFIG_reserved2_MASK                            0x00008000
#define BCHP_NAND_CONFIG_reserved2_SHIFT                           15

/* NAND :: CONFIG :: COL_ADR_BYTES [14:12] */
#define BCHP_NAND_CONFIG_COL_ADR_BYTES_MASK                        0x00007000
#define BCHP_NAND_CONFIG_COL_ADR_BYTES_SHIFT                       12

/* NAND :: CONFIG :: reserved3 [11:11] */
#define BCHP_NAND_CONFIG_reserved3_MASK                            0x00000800
#define BCHP_NAND_CONFIG_reserved3_SHIFT                           11

/* NAND :: CONFIG :: BLK_ADR_BYTES [10:08] */
#define BCHP_NAND_CONFIG_BLK_ADR_BYTES_MASK                        0x00000700
#define BCHP_NAND_CONFIG_BLK_ADR_BYTES_SHIFT                       8

/* NAND :: CONFIG :: reserved4 [07:00] */
#define BCHP_NAND_CONFIG_reserved4_MASK                            0x000000ff
#define BCHP_NAND_CONFIG_reserved4_SHIFT                           0

/***************************************************************************
 *TIMING_1 - Nand Flash Timing Parameters 1
 ***************************************************************************/
/* NAND :: TIMING_1 :: tWP [31:28] */
#define BCHP_NAND_TIMING_1_tWP_MASK                                0xf0000000
#define BCHP_NAND_TIMING_1_tWP_SHIFT                               28

/* NAND :: TIMING_1 :: tWH [27:24] */
#define BCHP_NAND_TIMING_1_tWH_MASK                                0x0f000000
#define BCHP_NAND_TIMING_1_tWH_SHIFT                               24

/* NAND :: TIMING_1 :: tRP [23:20] */
#define BCHP_NAND_TIMING_1_tRP_MASK                                0x00f00000
#define BCHP_NAND_TIMING_1_tRP_SHIFT                               20

/* NAND :: TIMING_1 :: tREH [19:16] */
#define BCHP_NAND_TIMING_1_tREH_MASK                               0x000f0000
#define BCHP_NAND_TIMING_1_tREH_SHIFT                              16

/* NAND :: TIMING_1 :: tCS [15:12] */
#define BCHP_NAND_TIMING_1_tCS_MASK                                0x0000f000
#define BCHP_NAND_TIMING_1_tCS_SHIFT                               12

/* NAND :: TIMING_1 :: tCLH [11:08] */
#define BCHP_NAND_TIMING_1_tCLH_MASK                               0x00000f00
#define BCHP_NAND_TIMING_1_tCLH_SHIFT                              8

/* NAND :: TIMING_1 :: tALH [07:04] */
#define BCHP_NAND_TIMING_1_tALH_MASK                               0x000000f0
#define BCHP_NAND_TIMING_1_tALH_SHIFT                              4

/* NAND :: TIMING_1 :: tADL [03:00] */
#define BCHP_NAND_TIMING_1_tADL_MASK                               0x0000000f
#define BCHP_NAND_TIMING_1_tADL_SHIFT                              0

/***************************************************************************
 *TIMING_2 - Nand Flash Timing Parameters 2
 ***************************************************************************/
/* NAND :: TIMING_2 :: CLK_SELECT [31:31] */
#define BCHP_NAND_TIMING_2_CLK_SELECT_MASK                         0x80000000
#define BCHP_NAND_TIMING_2_CLK_SELECT_SHIFT                        31
#define BCHP_NAND_TIMING_2_CLK_SELECT_CLK_108                      0
#define BCHP_NAND_TIMING_2_CLK_SELECT_CLK_216                      1

/* NAND :: TIMING_2 :: reserved0 [30:13] */
#define BCHP_NAND_TIMING_2_reserved0_MASK                          0x7fffe000
#define BCHP_NAND_TIMING_2_reserved0_SHIFT                         13

/* NAND :: TIMING_2 :: tWB [12:09] */
#define BCHP_NAND_TIMING_2_tWB_MASK                                0x00001e00
#define BCHP_NAND_TIMING_2_tWB_SHIFT                               9

/* NAND :: TIMING_2 :: tWHR [08:04] */
#define BCHP_NAND_TIMING_2_tWHR_MASK                               0x000001f0
#define BCHP_NAND_TIMING_2_tWHR_SHIFT                              4

/* NAND :: TIMING_2 :: tREAD [03:00] */
#define BCHP_NAND_TIMING_2_tREAD_MASK                              0x0000000f
#define BCHP_NAND_TIMING_2_tREAD_SHIFT                             0

/***************************************************************************
 *SEMAPHORE - Semaphore
 ***************************************************************************/
/* NAND :: SEMAPHORE :: reserved0 [31:08] */
#define BCHP_NAND_SEMAPHORE_reserved0_MASK                         0xffffff00
#define BCHP_NAND_SEMAPHORE_reserved0_SHIFT                        8

/* NAND :: SEMAPHORE :: semaphore_ctrl [07:00] */
#define BCHP_NAND_SEMAPHORE_semaphore_ctrl_MASK                    0x000000ff
#define BCHP_NAND_SEMAPHORE_semaphore_ctrl_SHIFT                   0

/***************************************************************************
 *FLASH_DEVICE_ID - Nand Flash Device ID
 ***************************************************************************/
/* NAND :: FLASH_DEVICE_ID :: BYTE_0 [31:24] */
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_0_MASK                      0xff000000
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_0_SHIFT                     24

/* NAND :: FLASH_DEVICE_ID :: BYTE_1 [23:16] */
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_1_MASK                      0x00ff0000
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_1_SHIFT                     16

/* NAND :: FLASH_DEVICE_ID :: BYTE_2 [15:08] */
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_2_MASK                      0x0000ff00
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_2_SHIFT                     8

/* NAND :: FLASH_DEVICE_ID :: BYTE_3 [07:00] */
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_3_MASK                      0x000000ff
#define BCHP_NAND_FLASH_DEVICE_ID_BYTE_3_SHIFT                     0

/***************************************************************************
 *FLASH_DEVICE_ID_EXT - Nand Flash Extended Device ID
 ***************************************************************************/
/* NAND :: FLASH_DEVICE_ID_EXT :: BYTE_4 [31:24] */
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_4_MASK                  0xff000000
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_4_SHIFT                 24

/* NAND :: FLASH_DEVICE_ID_EXT :: BYTE_5 [23:16] */
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_5_MASK                  0x00ff0000
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_5_SHIFT                 16

/* NAND :: FLASH_DEVICE_ID_EXT :: BYTE_6 [15:08] */
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_6_MASK                  0x0000ff00
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_6_SHIFT                 8

/* NAND :: FLASH_DEVICE_ID_EXT :: BYTE_7 [07:00] */
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_7_MASK                  0x000000ff
#define BCHP_NAND_FLASH_DEVICE_ID_EXT_BYTE_7_SHIFT                 0

/***************************************************************************
 *BLOCK_LOCK_STATUS - Nand Flash Block Lock Status
 ***************************************************************************/
/* NAND :: BLOCK_LOCK_STATUS :: reserved0 [31:08] */
#define BCHP_NAND_BLOCK_LOCK_STATUS_reserved0_MASK                 0xffffff00
#define BCHP_NAND_BLOCK_LOCK_STATUS_reserved0_SHIFT                8

/* NAND :: BLOCK_LOCK_STATUS :: STATUS [07:00] */
#define BCHP_NAND_BLOCK_LOCK_STATUS_STATUS_MASK                    0x000000ff
#define BCHP_NAND_BLOCK_LOCK_STATUS_STATUS_SHIFT                   0

/***************************************************************************
 *INTFC_STATUS - Nand Flash Interface Status
 ***************************************************************************/
/* NAND :: INTFC_STATUS :: CTLR_READY [31:31] */
#define BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK                     0x80000000
#define BCHP_NAND_INTFC_STATUS_CTLR_READY_SHIFT                    31

/* NAND :: INTFC_STATUS :: FLASH_READY [30:30] */
#define BCHP_NAND_INTFC_STATUS_FLASH_READY_MASK                    0x40000000
#define BCHP_NAND_INTFC_STATUS_FLASH_READY_SHIFT                   30

/* NAND :: INTFC_STATUS :: CACHE_VALID [29:29] */
#define BCHP_NAND_INTFC_STATUS_CACHE_VALID_MASK                    0x20000000
#define BCHP_NAND_INTFC_STATUS_CACHE_VALID_SHIFT                   29

/* NAND :: INTFC_STATUS :: SPARE_AREA_VALID [28:28] */
#define BCHP_NAND_INTFC_STATUS_SPARE_AREA_VALID_MASK               0x10000000
#define BCHP_NAND_INTFC_STATUS_SPARE_AREA_VALID_SHIFT              28

/* NAND :: INTFC_STATUS :: ERASED [27:27] */
#define BCHP_NAND_INTFC_STATUS_ERASED_MASK                         0x08000000
#define BCHP_NAND_INTFC_STATUS_ERASED_SHIFT                        27

/* NAND :: INTFC_STATUS :: PLANE_READY [26:26] */
#define BCHP_NAND_INTFC_STATUS_PLANE_READY_MASK                    0x04000000
#define BCHP_NAND_INTFC_STATUS_PLANE_READY_SHIFT                   26

/* NAND :: INTFC_STATUS :: reserved0 [25:08] */
#define BCHP_NAND_INTFC_STATUS_reserved0_MASK                      0x03ffff00
#define BCHP_NAND_INTFC_STATUS_reserved0_SHIFT                     8

/* NAND :: INTFC_STATUS :: FLASH_STATUS [07:00] */
#define BCHP_NAND_INTFC_STATUS_FLASH_STATUS_MASK                   0x000000ff
#define BCHP_NAND_INTFC_STATUS_FLASH_STATUS_SHIFT                  0

/***************************************************************************
 *ECC_CORR_EXT_ADDR - ECC Correctable Error Extended Address
 ***************************************************************************/
/* NAND :: ECC_CORR_EXT_ADDR :: reserved0 [31:19] */
#define BCHP_NAND_ECC_CORR_EXT_ADDR_reserved0_MASK                 0xfff80000
#define BCHP_NAND_ECC_CORR_EXT_ADDR_reserved0_SHIFT                19

/* NAND :: ECC_CORR_EXT_ADDR :: CS_SEL [18:16] */
#define BCHP_NAND_ECC_CORR_EXT_ADDR_CS_SEL_MASK                    0x00070000
#define BCHP_NAND_ECC_CORR_EXT_ADDR_CS_SEL_SHIFT                   16

/* NAND :: ECC_CORR_EXT_ADDR :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_ECC_CORR_EXT_ADDR_EXT_ADDRESS_MASK               0x0000ffff
#define BCHP_NAND_ECC_CORR_EXT_ADDR_EXT_ADDRESS_SHIFT              0

/***************************************************************************
 *ECC_CORR_ADDR - ECC Correctable Error Address
 ***************************************************************************/
/* NAND :: ECC_CORR_ADDR :: ADDRESS [31:00] */
#define BCHP_NAND_ECC_CORR_ADDR_ADDRESS_MASK                       0xffffffff
#define BCHP_NAND_ECC_CORR_ADDR_ADDRESS_SHIFT                      0

/***************************************************************************
 *ECC_UNC_EXT_ADDR - ECC Uncorrectable Error Extended Address
 ***************************************************************************/
/* NAND :: ECC_UNC_EXT_ADDR :: reserved0 [31:19] */
#define BCHP_NAND_ECC_UNC_EXT_ADDR_reserved0_MASK                  0xfff80000
#define BCHP_NAND_ECC_UNC_EXT_ADDR_reserved0_SHIFT                 19

/* NAND :: ECC_UNC_EXT_ADDR :: CS_SEL [18:16] */
#define BCHP_NAND_ECC_UNC_EXT_ADDR_CS_SEL_MASK                     0x00070000
#define BCHP_NAND_ECC_UNC_EXT_ADDR_CS_SEL_SHIFT                    16

/* NAND :: ECC_UNC_EXT_ADDR :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_ECC_UNC_EXT_ADDR_EXT_ADDRESS_MASK                0x0000ffff
#define BCHP_NAND_ECC_UNC_EXT_ADDR_EXT_ADDRESS_SHIFT               0

/***************************************************************************
 *ECC_UNC_ADDR - ECC Uncorrectable Error Address
 ***************************************************************************/
/* NAND :: ECC_UNC_ADDR :: ADDRESS [31:00] */
#define BCHP_NAND_ECC_UNC_ADDR_ADDRESS_MASK                        0xffffffff
#define BCHP_NAND_ECC_UNC_ADDR_ADDRESS_SHIFT                       0

/***************************************************************************
 *READ_ERROR_COUNT - Read Error Count
 ***************************************************************************/
/* NAND :: READ_ERROR_COUNT :: READ_ERROR_COUNT [31:00] */
#define BCHP_NAND_READ_ERROR_COUNT_READ_ERROR_COUNT_MASK           0xffffffff
#define BCHP_NAND_READ_ERROR_COUNT_READ_ERROR_COUNT_SHIFT          0

/***************************************************************************
 *CORR_STAT_THRESHOLD - Correctable Error Reporting Threshold
 ***************************************************************************/
/* NAND :: CORR_STAT_THRESHOLD :: reserved0 [31:30] */
#define BCHP_NAND_CORR_STAT_THRESHOLD_reserved0_MASK               0xc0000000
#define BCHP_NAND_CORR_STAT_THRESHOLD_reserved0_SHIFT              30

/* NAND :: CORR_STAT_THRESHOLD :: CORR_STAT_THRESHOLD [05:00] */
#define BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_MASK     0x0000003f
#define BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_SHIFT    0

/***************************************************************************
 *ONFI_STATUS - ONFI Status
 ***************************************************************************/
/* NAND :: ONFI_STATUS :: ONFI_DEBUG_SEL [31:28] */
#define BCHP_NAND_ONFI_STATUS_ONFI_DEBUG_SEL_MASK                  0xf0000000
#define BCHP_NAND_ONFI_STATUS_ONFI_DEBUG_SEL_SHIFT                 28

/* NAND :: ONFI_STATUS :: reserved0 [27:06] */
#define BCHP_NAND_ONFI_STATUS_reserved0_MASK                       0x0fffffc0
#define BCHP_NAND_ONFI_STATUS_reserved0_SHIFT                      6

/* NAND :: ONFI_STATUS :: ONFI_BAD_IDENT_PG2 [05:05] */
#define BCHP_NAND_ONFI_STATUS_ONFI_BAD_IDENT_PG2_MASK              0x00000020
#define BCHP_NAND_ONFI_STATUS_ONFI_BAD_IDENT_PG2_SHIFT             5

/* NAND :: ONFI_STATUS :: ONFI_BAD_IDENT_PG1 [04:04] */
#define BCHP_NAND_ONFI_STATUS_ONFI_BAD_IDENT_PG1_MASK              0x00000010
#define BCHP_NAND_ONFI_STATUS_ONFI_BAD_IDENT_PG1_SHIFT             4

/* NAND :: ONFI_STATUS :: ONFI_BAD_IDENT_PG0 [03:03] */
#define BCHP_NAND_ONFI_STATUS_ONFI_BAD_IDENT_PG0_MASK              0x00000008
#define BCHP_NAND_ONFI_STATUS_ONFI_BAD_IDENT_PG0_SHIFT             3

/* NAND :: ONFI_STATUS :: ONFI_CRC_ERROR_PG2 [02:02] */
#define BCHP_NAND_ONFI_STATUS_ONFI_CRC_ERROR_PG2_MASK              0x00000004
#define BCHP_NAND_ONFI_STATUS_ONFI_CRC_ERROR_PG2_SHIFT             2

/* NAND :: ONFI_STATUS :: ONFI_CRC_ERROR_PG1 [01:01] */
#define BCHP_NAND_ONFI_STATUS_ONFI_CRC_ERROR_PG1_MASK              0x00000002
#define BCHP_NAND_ONFI_STATUS_ONFI_CRC_ERROR_PG1_SHIFT             1

/* NAND :: ONFI_STATUS :: ONFI_CRC_ERROR_PG0 [00:00] */
#define BCHP_NAND_ONFI_STATUS_ONFI_CRC_ERROR_PG0_MASK              0x00000001
#define BCHP_NAND_ONFI_STATUS_ONFI_CRC_ERROR_PG0_SHIFT             0

/***************************************************************************
 *ONFI_DEBUG_DATA - ONFI Debug Data
 ***************************************************************************/
/* NAND :: ONFI_DEBUG_DATA :: ONFI_DEBUG_DATA [31:00] */
#define BCHP_NAND_ONFI_DEBUG_DATA_ONFI_DEBUG_DATA_MASK             0xffffffff
#define BCHP_NAND_ONFI_DEBUG_DATA_ONFI_DEBUG_DATA_SHIFT            0

/***************************************************************************
 *FLASH_READ_EXT_ADDR - Flash Read Data Extended Address
 ***************************************************************************/
/* NAND :: FLASH_READ_EXT_ADDR :: reserved0 [31:19] */
#define BCHP_NAND_FLASH_READ_EXT_ADDR_reserved0_MASK               0xfff80000
#define BCHP_NAND_FLASH_READ_EXT_ADDR_reserved0_SHIFT              19

/* NAND :: FLASH_READ_EXT_ADDR :: CS_SEL [18:16] */
#define BCHP_NAND_FLASH_READ_EXT_ADDR_CS_SEL_MASK                  0x00070000
#define BCHP_NAND_FLASH_READ_EXT_ADDR_CS_SEL_SHIFT                 16

/* NAND :: FLASH_READ_EXT_ADDR :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_FLASH_READ_EXT_ADDR_EXT_ADDRESS_MASK             0x0000ffff
#define BCHP_NAND_FLASH_READ_EXT_ADDR_EXT_ADDRESS_SHIFT            0

/***************************************************************************
 *FLASH_READ_ADDR - Flash Read Data Address
 ***************************************************************************/
/* NAND :: FLASH_READ_ADDR :: ADDRESS [31:00] */
#define BCHP_NAND_FLASH_READ_ADDR_ADDRESS_MASK                     0xffffffff
#define BCHP_NAND_FLASH_READ_ADDR_ADDRESS_SHIFT                    0

/***************************************************************************
 *PROGRAM_PAGE_EXT_ADDR - Page Program Extended Address
 ***************************************************************************/
/* NAND :: PROGRAM_PAGE_EXT_ADDR :: reserved0 [31:19] */
#define BCHP_NAND_PROGRAM_PAGE_EXT_ADDR_reserved0_MASK             0xfff80000
#define BCHP_NAND_PROGRAM_PAGE_EXT_ADDR_reserved0_SHIFT            19

/* NAND :: PROGRAM_PAGE_EXT_ADDR :: CS_SEL [18:16] */
#define BCHP_NAND_PROGRAM_PAGE_EXT_ADDR_CS_SEL_MASK                0x00070000
#define BCHP_NAND_PROGRAM_PAGE_EXT_ADDR_CS_SEL_SHIFT               16

/* NAND :: PROGRAM_PAGE_EXT_ADDR :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_PROGRAM_PAGE_EXT_ADDR_EXT_ADDRESS_MASK           0x0000ffff
#define BCHP_NAND_PROGRAM_PAGE_EXT_ADDR_EXT_ADDRESS_SHIFT          0

/***************************************************************************
 *PROGRAM_PAGE_ADDR - Page Program Address
 ***************************************************************************/
/* NAND :: PROGRAM_PAGE_ADDR :: ADDRESS [31:00] */
#define BCHP_NAND_PROGRAM_PAGE_ADDR_ADDRESS_MASK                   0xffffffff
#define BCHP_NAND_PROGRAM_PAGE_ADDR_ADDRESS_SHIFT                  0

/***************************************************************************
 *COPY_BACK_EXT_ADDR - Copy Back Extended Address
 ***************************************************************************/
/* NAND :: COPY_BACK_EXT_ADDR :: reserved0 [31:19] */
#define BCHP_NAND_COPY_BACK_EXT_ADDR_reserved0_MASK                0xfff80000
#define BCHP_NAND_COPY_BACK_EXT_ADDR_reserved0_SHIFT               19

/* NAND :: COPY_BACK_EXT_ADDR :: CS_SEL [18:16] */
#define BCHP_NAND_COPY_BACK_EXT_ADDR_CS_SEL_MASK                   0x00070000
#define BCHP_NAND_COPY_BACK_EXT_ADDR_CS_SEL_SHIFT                  16

/* NAND :: COPY_BACK_EXT_ADDR :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_COPY_BACK_EXT_ADDR_EXT_ADDRESS_MASK              0x0000ffff
#define BCHP_NAND_COPY_BACK_EXT_ADDR_EXT_ADDRESS_SHIFT             0

/***************************************************************************
 *COPY_BACK_ADDR - Copy Back Address
 ***************************************************************************/
/* NAND :: COPY_BACK_ADDR :: ADDRESS [31:00] */
#define BCHP_NAND_COPY_BACK_ADDR_ADDRESS_MASK                      0xffffffff
#define BCHP_NAND_COPY_BACK_ADDR_ADDRESS_SHIFT                     0

/***************************************************************************
 *BLOCK_ERASE_EXT_ADDR - Block Erase Extended Address
 ***************************************************************************/
/* NAND :: BLOCK_ERASE_EXT_ADDR :: reserved0 [31:19] */
#define BCHP_NAND_BLOCK_ERASE_EXT_ADDR_reserved0_MASK              0xfff80000
#define BCHP_NAND_BLOCK_ERASE_EXT_ADDR_reserved0_SHIFT             19

/* NAND :: BLOCK_ERASE_EXT_ADDR :: CS_SEL [18:16] */
#define BCHP_NAND_BLOCK_ERASE_EXT_ADDR_CS_SEL_MASK                 0x00070000
#define BCHP_NAND_BLOCK_ERASE_EXT_ADDR_CS_SEL_SHIFT                16

/* NAND :: BLOCK_ERASE_EXT_ADDR :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_BLOCK_ERASE_EXT_ADDR_EXT_ADDRESS_MASK            0x0000ffff
#define BCHP_NAND_BLOCK_ERASE_EXT_ADDR_EXT_ADDRESS_SHIFT           0

/***************************************************************************
 *BLOCK_ERASE_ADDR - Block Erase Address
 ***************************************************************************/
/* NAND :: BLOCK_ERASE_ADDR :: ADDRESS [31:00] */
#define BCHP_NAND_BLOCK_ERASE_ADDR_ADDRESS_MASK                    0xffffffff
#define BCHP_NAND_BLOCK_ERASE_ADDR_ADDRESS_SHIFT                   0

/***************************************************************************
 *INV_READ_EXT_ADDR - Flash Invalid Data Extended Address
 ***************************************************************************/
/* NAND :: INV_READ_EXT_ADDR :: reserved0 [31:19] */
#define BCHP_NAND_INV_READ_EXT_ADDR_reserved0_MASK                 0xfff80000
#define BCHP_NAND_INV_READ_EXT_ADDR_reserved0_SHIFT                19

/* NAND :: INV_READ_EXT_ADDR :: CS_SEL [18:16] */
#define BCHP_NAND_INV_READ_EXT_ADDR_CS_SEL_MASK                    0x00070000
#define BCHP_NAND_INV_READ_EXT_ADDR_CS_SEL_SHIFT                   16

/* NAND :: INV_READ_EXT_ADDR :: EXT_ADDRESS [15:00] */
#define BCHP_NAND_INV_READ_EXT_ADDR_EXT_ADDRESS_MASK               0x0000ffff
#define BCHP_NAND_INV_READ_EXT_ADDR_EXT_ADDRESS_SHIFT              0

/***************************************************************************
 *INV_READ_ADDR - Flash Invalid Data Address
 ***************************************************************************/
/* NAND :: INV_READ_ADDR :: ADDRESS [31:00] */
#define BCHP_NAND_INV_READ_ADDR_ADDRESS_MASK                       0xffffffff
#define BCHP_NAND_INV_READ_ADDR_ADDRESS_SHIFT                      0

/***************************************************************************
 *BLK_WR_PROTECT - Block Write Protect Enable and Size for EBI_CS0b
 ***************************************************************************/
/* NAND :: BLK_WR_PROTECT :: BLK_END_ADDR [31:00] */
#define BCHP_NAND_BLK_WR_PROTECT_BLK_END_ADDR_MASK                 0xffffffff
#define BCHP_NAND_BLK_WR_PROTECT_BLK_END_ADDR_SHIFT                0

/***************************************************************************
 *ACC_CONTROL_CS1 - Nand Flash Access Control
 ***************************************************************************/
/* NAND :: ACC_CONTROL_CS1 :: RD_ECC_EN [31:31] */
#define BCHP_NAND_ACC_CONTROL_CS1_RD_ECC_EN_MASK                   0x80000000
#define BCHP_NAND_ACC_CONTROL_CS1_RD_ECC_EN_SHIFT                  31

/* NAND :: ACC_CONTROL_CS1 :: WR_ECC_EN [30:30] */
#define BCHP_NAND_ACC_CONTROL_CS1_WR_ECC_EN_MASK                   0x40000000
#define BCHP_NAND_ACC_CONTROL_CS1_WR_ECC_EN_SHIFT                  30

/* NAND :: ACC_CONTROL :: RD_ECC_BLK0_EN [29:29] */
#define BCHP_NAND_ACC_CONTROL_CS1_CE_CARE_MASK                     0x20000000
#define BCHP_NAND_ACC_CONTROL_CS1_CE_CARE_SHIFT                    29

/* NAND :: ACC_CONTROL :: reserve1 [28:28] */
#define BCHP_NAND_ACC_CONTROL_CS1_RESERVED1_MASK                   0x10000000
#define BCHP_NAND_ACC_CONTROL_CS1_RESERVED1_SHIFT                  28

/* NAND :: ACC_CONTROL_CS1 :: RD_ERASED_ECC_EN [27:27] */
#define BCHP_NAND_ACC_CONTROL_CS1_RD_ERASED_ECC_EN_MASK            0x08000000
#define BCHP_NAND_ACC_CONTROL_CS1_RD_ERASED_ECC_EN_SHIFT           27

/* NAND :: ACC_CONTROL_CS1 :: PARTIAL_PAGE_EN [26:26] */
#define BCHP_NAND_ACC_CONTROL_CS1_PARTIAL_PAGE_EN_MASK             0x04000000
#define BCHP_NAND_ACC_CONTROL_CS1_PARTIAL_PAGE_EN_SHIFT            26

/* NAND :: ACC_CONTROL_CS1 :: WR_PREEMPT_EN [25:25] */
#define BCHP_NAND_ACC_CONTROL_CS1_WR_PREEMPT_EN_MASK               0x02000000
#define BCHP_NAND_ACC_CONTROL_CS1_WR_PREEMPT_EN_SHIFT              25

/* NAND :: ACC_CONTROL_CS1 :: PAGE_HIT_EN [24:24] */
#define BCHP_NAND_ACC_CONTROL_CS1_PAGE_HIT_EN_MASK                 0x01000000
#define BCHP_NAND_ACC_CONTROL_CS1_PAGE_HIT_EN_SHIFT                24

/* NAND :: ACC_CONTROL :: PREFETCH_EN [23:23] */
#define BCHP_NAND_ACC_CONTROL_CS1_PREFETCH_EN_MASK                 0x00800000
#define BCHP_NAND_ACC_CONTROL_CS1_PREFETCH_EN_SHIFT                23

/* NAND :: ACC_CONTROL :: CACHE_MODE_EN [22:22] */
#define BCHP_NAND_ACC_CONTROL_CS1_CACHE_MODE_EN_MASK               0x00400000
#define BCHP_NAND_ACC_CONTROL_CS1_CACHE_MODE_EN_SHIFT              22

/* NAND :: ACC_CONTROL :: reserve2 [21:21] */
#define BCHP_NAND_ACC_CONTROL_CS1_RESERVED2_MASK                   0x00200000
#define BCHP_NAND_ACC_CONTROL_CS1_RESERVED2_SHIFT                  21

/* NAND :: ACC_CONTROL :: ECC_LEVEL [20:16] */
#define BCHP_NAND_ACC_CONTROL_CS1_ECC_LEVEL_MASK                   0x001f0000
#define BCHP_NAND_ACC_CONTROL_CS1_ECC_LEVEL_SHIFT                  16

/* NAND :: ACC_CONTROL :: reserved3 [15:8] */
#define BCHP_NAND_ACC_CONTROL_CS1_RESERVED3_MASK                   0x0000ff00
#define BCHP_NAND_ACC_CONTROL_CS1_RESERVED3_SHIFT                  8

/* NAND :: ACC_CONTROL :: SECTOR_SIZE_1K [07:07] */
#define BCHP_NAND_ACC_CONTROL_CS1_SECTOR_SIZE_1K_MASK              0x00000080
#define BCHP_NAND_ACC_CONTROL_CS1_SECTOR_SIZE_1K_SHIFT             7

/* NAND :: ACC_CONTROL_CS1 :: SPARE_AREA_SIZE [06:00] */
#define BCHP_NAND_ACC_CONTROL_CS1_SPARE_AREA_SIZE_MASK             0x0000007f
#define BCHP_NAND_ACC_CONTROL_CS1_SPARE_AREA_SIZE_SHIFT            0

#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_7_0
/***************************************************************************
 *CONFIG_CS1 EXT - Nand Flash Config Ext
 ***************************************************************************/
/* NAND :: CONFIG_CS1_EXT :: BLOCK_SIZE [11:4] */
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_MASK                       0x00000ff0
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_SHIFT                      4
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_8192KB             10
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_4096KB             9
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_2048KB             8
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_1024KB             7
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_512KB              6
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_256KB              5
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_128KB              4
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_64KB               3
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_32KB               2
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_16KB               1
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_8KB                0

/* NAND :: CONFIG_CS1_EXT :: PAGE_SIZE [11:4] */
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_MASK                        0x0000000f
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_SHIFT                       0
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_512                 0
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_1KB                 1
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_2KB                 2
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_4KB                 3
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_8KB                 4
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_16KB                5 

#endif

/***************************************************************************
 *CONFIG_CS1 - Nand Flash Config
 ***************************************************************************/
/* NAND :: CONFIG_CS1 :: CONFIG_LOCK [31:31] */
#define BCHP_NAND_CONFIG_CS1_CONFIG_LOCK_MASK                      0x80000000
#define BCHP_NAND_CONFIG_CS1_CONFIG_LOCK_SHIFT                     31

#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_7_0
/* NAND :: CONFIG_CS1 :: BLOCK_SIZE [30:28] */
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_MASK                       0x70000000
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_SHIFT                      28
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_2048KB             6
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_1024KB             5
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_512KB              4
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_256KB              3
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_128KB              2
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_16KB               1
#define BCHP_NAND_CONFIG_CS1_BLOCK_SIZE_BK_SIZE_8KB                0
#endif

/* NAND :: CONFIG_CS1 :: DEVICE_SIZE [27:24] */
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_MASK                      0x0f000000
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_SHIFT                     24
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_4MB              0
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_8MB              1
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_16MB             2
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_32MB             3
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_64MB             4
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_128MB            5
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_256MB            6
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_512MB            7
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_1GB              8
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_2GB              9
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_4GB              10
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_8GB              11
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_16GB             12
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_32GB             13
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_64GB             14
#define BCHP_NAND_CONFIG_CS1_DEVICE_SIZE_DVC_SIZE_128GB            15

/* NAND :: CONFIG_CS1 :: DEVICE_WIDTH [23:23] */
#define BCHP_NAND_CONFIG_CS1_DEVICE_WIDTH_MASK                     0x00800000
#define BCHP_NAND_CONFIG_CS1_DEVICE_WIDTH_SHIFT                    23
#define BCHP_NAND_CONFIG_CS1_DEVICE_WIDTH_DVC_WIDTH_8              0
#define BCHP_NAND_CONFIG_CS1_DEVICE_WIDTH_DVC_WIDTH_16             1

/* NAND :: CONFIG_CS1 :: reserved0 [22:22] */
#define BCHP_NAND_CONFIG_CS1_reserved0_MASK                        0x00400000
#define BCHP_NAND_CONFIG_CS1_reserved0_SHIFT                       22

#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_7_0
/* NAND :: CONFIG_CS1 :: PAGE_SIZE [21:20] */
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_MASK                        0x00300000
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_SHIFT                       20
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_512                 0
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_2KB                 1
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_4KB                 2
#define BCHP_NAND_CONFIG_CS1_PAGE_SIZE_PG_SIZE_8KB                 3
#endif

/* NAND :: CONFIG_CS1 :: reserved1 [19:19] */
#define BCHP_NAND_CONFIG_CS1_reserved1_MASK                        0x00080000
#define BCHP_NAND_CONFIG_CS1_reserved1_SHIFT                       19

/* NAND :: CONFIG_CS1 :: FUL_ADR_BYTES [18:16] */
#define BCHP_NAND_CONFIG_CS1_FUL_ADR_BYTES_MASK                    0x00070000
#define BCHP_NAND_CONFIG_CS1_FUL_ADR_BYTES_SHIFT                   16

/* NAND :: CONFIG_CS1 :: reserved2 [15:15] */
#define BCHP_NAND_CONFIG_CS1_reserved2_MASK                        0x00008000
#define BCHP_NAND_CONFIG_CS1_reserved2_SHIFT                       15

/* NAND :: CONFIG_CS1 :: COL_ADR_BYTES [14:12] */
#define BCHP_NAND_CONFIG_CS1_COL_ADR_BYTES_MASK                    0x00007000
#define BCHP_NAND_CONFIG_CS1_COL_ADR_BYTES_SHIFT                   12

/* NAND :: CONFIG_CS1 :: reserved3 [11:11] */
#define BCHP_NAND_CONFIG_CS1_reserved3_MASK                        0x00000800
#define BCHP_NAND_CONFIG_CS1_reserved3_SHIFT                       11

/* NAND :: CONFIG_CS1 :: BLK_ADR_BYTES [10:08] */
#define BCHP_NAND_CONFIG_CS1_BLK_ADR_BYTES_MASK                    0x00000700
#define BCHP_NAND_CONFIG_CS1_BLK_ADR_BYTES_SHIFT                   8

/* NAND :: CONFIG_CS1 :: reserved4 [07:00] */
#define BCHP_NAND_CONFIG_CS1_reserved4_MASK                        0x000000ff
#define BCHP_NAND_CONFIG_CS1_reserved4_SHIFT                       0

/***************************************************************************
 *TIMING_1_CS1 - Nand Flash Timing Parameters 1
 ***************************************************************************/
/* NAND :: TIMING_1_CS1 :: tWP [31:28] */
#define BCHP_NAND_TIMING_1_CS1_tWP_MASK                            0xf0000000
#define BCHP_NAND_TIMING_1_CS1_tWP_SHIFT                           28

/* NAND :: TIMING_1_CS1 :: tWH [27:24] */
#define BCHP_NAND_TIMING_1_CS1_tWH_MASK                            0x0f000000
#define BCHP_NAND_TIMING_1_CS1_tWH_SHIFT                           24

/* NAND :: TIMING_1_CS1 :: tRP [23:20] */
#define BCHP_NAND_TIMING_1_CS1_tRP_MASK                            0x00f00000
#define BCHP_NAND_TIMING_1_CS1_tRP_SHIFT                           20

/* NAND :: TIMING_1_CS1 :: tREH [19:16] */
#define BCHP_NAND_TIMING_1_CS1_tREH_MASK                           0x000f0000
#define BCHP_NAND_TIMING_1_CS1_tREH_SHIFT                          16

/* NAND :: TIMING_1_CS1 :: tCS [15:12] */
#define BCHP_NAND_TIMING_1_CS1_tCS_MASK                            0x0000f000
#define BCHP_NAND_TIMING_1_CS1_tCS_SHIFT                           12

/* NAND :: TIMING_1_CS1 :: tCLH [11:08] */
#define BCHP_NAND_TIMING_1_CS1_tCLH_MASK                           0x00000f00
#define BCHP_NAND_TIMING_1_CS1_tCLH_SHIFT                          8

/* NAND :: TIMING_1_CS1 :: tALH [07:04] */
#define BCHP_NAND_TIMING_1_CS1_tALH_MASK                           0x000000f0
#define BCHP_NAND_TIMING_1_CS1_tALH_SHIFT                          4

/* NAND :: TIMING_1_CS1 :: tADL [03:00] */
#define BCHP_NAND_TIMING_1_CS1_tADL_MASK                           0x0000000f
#define BCHP_NAND_TIMING_1_CS1_tADL_SHIFT                          0

/***************************************************************************
 *TIMING_2_CS1 - Nand Flash Timing Parameters 2
 ***************************************************************************/
/* NAND :: TIMING_2_CS1 :: CLK_SELECT [31:31] */
#define BCHP_NAND_TIMING_2_CS1_CLK_SELECT_MASK                     0x80000000
#define BCHP_NAND_TIMING_2_CS1_CLK_SELECT_SHIFT                    31
#define BCHP_NAND_TIMING_2_CS1_CLK_SELECT_CLK_108                  0
#define BCHP_NAND_TIMING_2_CS1_CLK_SELECT_CLK_216                  1

/* NAND :: TIMING_2_CS1 :: reserved0 [30:13] */
#define BCHP_NAND_TIMING_2_CS1_reserved0_MASK                      0x7fffe000
#define BCHP_NAND_TIMING_2_CS1_reserved0_SHIFT                     13

/* NAND :: TIMING_2_CS1 :: tWB [12:09] */
#define BCHP_NAND_TIMING_2_CS1_tWB_MASK                            0x00001e00
#define BCHP_NAND_TIMING_2_CS1_tWB_SHIFT                           9

/* NAND :: TIMING_2_CS1 :: tWHR [08:04] */
#define BCHP_NAND_TIMING_2_CS1_tWHR_MASK                           0x000001f0
#define BCHP_NAND_TIMING_2_CS1_tWHR_SHIFT                          4

/* NAND :: TIMING_2_CS1 :: tREAD [03:00] */
#define BCHP_NAND_TIMING_2_CS1_tREAD_MASK                          0x0000000f
#define BCHP_NAND_TIMING_2_CS1_tREAD_SHIFT                         0

/***************************************************************************
 *SPARE_AREA_READ_OFS_10 - Nand Flash Spare Area Read Bytes 16-19
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_10 :: BYTE_OFS_16 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_16_MASK          0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_16_SHIFT         24

/* NAND :: SPARE_AREA_READ_OFS_10 :: BYTE_OFS_17 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_17_MASK          0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_17_SHIFT         16

/* NAND :: SPARE_AREA_READ_OFS_10 :: BYTE_OFS_18 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_18_MASK          0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_18_SHIFT         8

/* NAND :: SPARE_AREA_READ_OFS_10 :: BYTE_OFS_19 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_19_MASK          0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_10_BYTE_OFS_19_SHIFT         0

/***************************************************************************
 *SPARE_AREA_READ_OFS_14 - Nand Flash Spare Area Read Bytes 20-23
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_14 :: BYTE_OFS_20 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_20_MASK          0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_20_SHIFT         24

/* NAND :: SPARE_AREA_READ_OFS_14 :: BYTE_OFS_21 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_21_MASK          0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_21_SHIFT         16

/* NAND :: SPARE_AREA_READ_OFS_14 :: BYTE_OFS_22 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_22_MASK          0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_22_SHIFT         8

/* NAND :: SPARE_AREA_READ_OFS_14 :: BYTE_OFS_23 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_23_MASK          0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_14_BYTE_OFS_23_SHIFT         0

/***************************************************************************
 *SPARE_AREA_READ_OFS_18 - Nand Flash Spare Area Read Bytes 24-27
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_18 :: BYTE_OFS_24 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_24_MASK          0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_24_SHIFT         24

/* NAND :: SPARE_AREA_READ_OFS_18 :: BYTE_OFS_25 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_25_MASK          0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_25_SHIFT         16

/* NAND :: SPARE_AREA_READ_OFS_18 :: BYTE_OFS_26 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_26_MASK          0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_26_SHIFT         8

/* NAND :: SPARE_AREA_READ_OFS_18 :: BYTE_OFS_27 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_27_MASK          0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_18_BYTE_OFS_27_SHIFT         0

/***************************************************************************
 *SPARE_AREA_READ_OFS_1C - Nand Flash Spare Area Read Bytes 28-31
 ***************************************************************************/
/* NAND :: SPARE_AREA_READ_OFS_1C :: BYTE_OFS_28 [31:24] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_28_MASK          0xff000000
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_28_SHIFT         24

/* NAND :: SPARE_AREA_READ_OFS_1C :: BYTE_OFS_29 [23:16] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_29_MASK          0x00ff0000
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_29_SHIFT         16

/* NAND :: SPARE_AREA_READ_OFS_1C :: BYTE_OFS_30 [15:08] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_30_MASK          0x0000ff00
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_30_SHIFT         8

/* NAND :: SPARE_AREA_READ_OFS_1C :: BYTE_OFS_31 [07:00] */
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_31_MASK          0x000000ff
#define BCHP_NAND_SPARE_AREA_READ_OFS_1C_BYTE_OFS_31_SHIFT         0

/***************************************************************************
 *LL_OP - Nand Flash Low Level Operation
 ***************************************************************************/
/* NAND :: LL_OP :: RETURN_IDLE [31:31] */
#define BCHP_NAND_LL_OP_RETURN_IDLE_MASK                           0x80000000
#define BCHP_NAND_LL_OP_RETURN_IDLE_SHIFT                          31

/* NAND :: LL_OP :: reserved0 [30:20] */
#define BCHP_NAND_LL_OP_reserved0_MASK                             0x7ff00000
#define BCHP_NAND_LL_OP_reserved0_SHIFT                            20

/* NAND :: LL_OP :: CLE [19:19] */
#define BCHP_NAND_LL_OP_CLE_MASK                                   0x00080000
#define BCHP_NAND_LL_OP_CLE_SHIFT                                  19

/* NAND :: LL_OP :: ALE [18:18] */
#define BCHP_NAND_LL_OP_ALE_MASK                                   0x00040000
#define BCHP_NAND_LL_OP_ALE_SHIFT                                  18

/* NAND :: LL_OP :: WE [17:17] */
#define BCHP_NAND_LL_OP_WE_MASK                                    0x00020000
#define BCHP_NAND_LL_OP_WE_SHIFT                                   17

/* NAND :: LL_OP :: RE [16:16] */
#define BCHP_NAND_LL_OP_RE_MASK                                    0x00010000
#define BCHP_NAND_LL_OP_RE_SHIFT                                   16

/* NAND :: LL_OP :: DATA [15:00] */
#define BCHP_NAND_LL_OP_DATA_MASK                                  0x0000ffff
#define BCHP_NAND_LL_OP_DATA_SHIFT                                 0

/***************************************************************************
 *LL_RDDATA - Nand Flash Low Level Read Data
 ***************************************************************************/
/* NAND :: LL_RDDATA :: reserved0 [31:16] */
#define BCHP_NAND_LL_RDDATA_reserved0_MASK                         0xffff0000
#define BCHP_NAND_LL_RDDATA_reserved0_SHIFT                        16

/* NAND :: LL_RDDATA :: DATA [15:00] */
#define BCHP_NAND_LL_RDDATA_DATA_MASK                              0x0000ffff
#define BCHP_NAND_LL_RDDATA_DATA_SHIFT                             0

/***************************************************************************
 *FLASH_CACHE%i - Flash Cache Buffer Read Access
 ***************************************************************************/
#define BCHP_NAND_FLASH_CACHEi_ARRAY_BASE                          BRCMNAND_CACHE_BASE
#define BCHP_NAND_FLASH_CACHEi_ARRAY_START                         0
#define BCHP_NAND_FLASH_CACHEi_ARRAY_END                           127
#define BCHP_NAND_FLASH_CACHEi_ARRAY_ELEMENT_SIZE                  32

/***************************************************************************
 *FLASH_CACHE%i - Flash Cache Buffer Read Access
 ***************************************************************************/
/* NAND :: FLASH_CACHEi :: WORD [31:00] */
#define BCHP_NAND_FLASH_CACHEi_WORD_MASK                           0xffffffff
#define BCHP_NAND_FLASH_CACHEi_WORD_SHIFT                          0

/***************************************************************************
 *NAND FLASH INTR STATUS register
 ***************************************************************************/

#define BCHP_HIF_INTR2_CPU_STATUS                                  (BRCMNAND_INTR_BASE)
#define BCHP_HIF_INTR2_CPU_STATUS_NAND_UNC_INTR_MASK               (0x1<<6)
#define BCHP_HIF_INTR2_CPU_STATUS_NAND_CORR_INTR_MASK              (0x1<<7)

#endif /* #ifndef BCHP_NAND_7x_H__ */

/* End of File */
#endif
