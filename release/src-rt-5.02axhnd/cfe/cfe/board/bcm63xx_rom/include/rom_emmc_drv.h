/***************************************************************************
    Copyright 2000-2016 Broadcom Corporation

    <:label-BRCM:2016:DUAL/GPL:standard
    
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

#ifndef _FSBL_EMMC_H
#define _FSBL_EMMC_H

#include <lib_types.h>
#include "bcm63xx_boot.h"

/* *********************************************************************
   *  eMMC Flash Configuration                                         *
   *     Settings/Parameters to configure eMMC transfer options.       *
   *     The options can be changeable by eMMC device,                  *
   *     IP/chip version and board design.                             *
   ********************************************************************* */
/* Bus Mode : Voltage */
#define EMMC_BUS_VOLTAGE    BUS_VOLTAGE_33/* BUS_VOLTAGE_18, BUS_VOLTAGE_33 */
#if defined (_BCM94908_) || defined (_BCM963158_) || defined (_BCM96846_)
#define EMMC_HS_TIMING      HS_TIMING_HS/* HS_TIMING_FULL, HS_TIMING_HS */
#else //if defined (_BCM96858_) || defined (_BCM963138_)
#define EMMC_HS_TIMING      HS_TIMING_FULL/* HS_TIMING_FULL, HS_TIMING_HS */
#endif
/* ********************************************************************* */


#define SDHC_REG_SDMA                 0x00
#define SDHC_REG_BLKCNT_BLKSIZE       0x04
#define SDHC_REG_BLKSIZE_MASK         0xfff
#define SDHC_MAKE_BLK_REG(count, size, boundry) \
        ((count << 16) | ((boundry & 0x7) << 12) | (size & 0xfff))
#define SDHC_REG_ARGUMENT             0x08
#define SDHC_REG_CMD_MODE             0x0c
#define SDHC_MODE_DMA                 0x00000001
#define SDHC_MODE_BLK_CNT             0x00000002
#define SDHC_MODE_ACMD12              0x00000004
#define SDHC_MODE_ACMD23              0x00000008
#define SDHC_MODE_XFER_DIR_READ       0x00000010
#define SDHC_MODE_MULTI               0x00000020
#define SDHC_CMD_INDEX_MASK           0x3f000000
#define SDHC_CMD_INDEX_SHIFT          24
#define SDHC_CMD_DATA                 0x00200000
#define SDHC_CMD_CHK_INDEX            0x00100000
#define SDHC_CMD_CHK_CRC              0x00080000
#define SDHC_CMD_RESP_MASK            0x00030000
#define SDHC_CMD_RESP_NONE            0x00000000
#define SDHC_CMD_RESP_48_BSY          0x00030000
#define SDHC_CMD_RESP_48              0x00020000
#define SDHC_CMD_RESP_136             0x00010000
#define SDHC_REG_RESPONSE_01          0x10
#define SDHC_REG_RESPONSE_23          0x14
#define SDHC_REG_RESPONSE_45          0x18
#define SDHC_REG_RESPONSE_67          0x1c
#define SDHC_REG_BUFFER_PORT          0x20
#define SDHC_REG_STATE                0x24
#define SDHC_STATE_WP                 0x00080000
#define SDHC_STATE_DETECT             0x00040000
#define SDHC_STATE_STABLE             0x00020000
#define SDHC_STATE_INSTERTED          0x00010000
#define SDHC_STATE_BUF_READ_RDY       0x00000800
#define SDHC_STATE_BUF_WRITE_RDY      0x00000400
#define SDHC_STATE_CMD_INHIBIT_DAT    0x00000002
#define SDHC_STATE_CMD_INHIBIT        0x00000001
#define SDHC_REG_CONTROL_0            0x28
#define SDHC_BUS_VOLT_MASK            0x00000e00
#define SDHC_BUS_VOLT_SHIFT           9
#define SDHC_BUS_VOLT_3_3             7
#define SDHC_BUS_VOLT_3_0             6
#define SDHC_BUS_VOLT_1_8             5
#define SDHC_BUS_PWR                  0x00000100
#define SDHC_CTL1_8BIT                0x00000020
#define SDHC_CTL1_HIGH_SPEED          0x00000004
#define SDHC_CTL1_4BIT                0x00000002
#define SDHC_CTL1_LED                 0x00000001
#define SDHC_REG_CONTROL_1            0x2c
#define SDHC_SW_RESET_DAT             0x04000000
#define SDHC_SW_RESET_CMD             0x02000000
#define SDHC_SW_RESET_ALL             0x01000000
#define SDHC_TOCTL_TIMEOUT_MASK       0x000f0000
#define SDHC_TOCTL_TIMEOUT_SHIFT      16
#define SDHC_CLKCTL_FREQ_MASK         0x0000ff00
#define SDHC_CLKCTL_FREQ_SHIFT        8
#define SDHC_CLKCTL_FREQ_UPPER_MASK   0x000000c0
#define SDHC_CLKCTL_FREQ_UPPER_SHIFT  6
#define SDHC_CLKCTL_CGEN_SEL          0x00000020
#define SDHC_CLKCTL_SD_CLK_ENA        0x00000004
#define SDHC_CLKCTL_INTERN_CLK_STABLE 0x00000002
#define SDHC_CLKCTL_INTERN_CLK_ENA    0x00000001
#define SDHC_REG_INTR_STAT            0x30
#define SDHC_INT_CMD_COMPLETE         0x00000001
#define SDHC_INT_XFER_COMPLETE        0x00000002
#define SDHC_INT_XFER_DMA             0x00000008
#define SDHC_INT_WRITE_BUF            0x00000010
#define SDHC_INT_READ_BUF             0x00000020
#define SDHC_INT_CARD_INSERTED        0x00000040
#define SDHC_INT_CARD_REMOVE          0x00000080
#define SDHC_INT_CARD_INT             0x00000100
#define SDHC_INT_ERR                  0x00008000
#define SDHC_INT_ERR_CMD_TO           0x00010000
#define SDHC_INT_ERR_CMD_CRC          0x00020000
#define SDHC_INT_ERR_CMD_END_BIT      0x00040000
#define SDHC_INT_ERR_CMD_INDEX        0x00080000
#define SDHC_INT_ERR_DATA_TO          0x00100000
#define SDHC_INT_ERR_DATA_CRC         0x00200000
#define SDHC_INT_ERR_DATA_END_BIT     0x00400000
#define SDHC_INT_ERR_CURRENT_LIMIT    0x00800000
#define SDHC_INT_ERR_ACMD             0x01000000
#define SDHC_INT_ERR_ADMA             0x02000000
#define SDHC_INT_ERR_TUNING           0x04000000
#define SDHC_INT_ERR_MASK             0xffff0000
#define SDHC_REG_INTR_STAT_ENABLE     0x34
#define SDHC_REG_INTR_SIG_ENABLE      0x38
#define SDHC_REG_CONTROL_2            0x3c
#define SDHC_REG_CAPABILITIES_0       0x40
#define SDHC_CAP0_BASE_CLK_FREQ_MASK  0x0000ff00
#define SDHC_CAP0_BASE_CLK_FREQ_SHIFT 8
#define SDHC_REG_CAPABILITIES_1         0x44

#define SDHC_STAT_ERROR_MASK            0xffff8000

/* eMMC device state */
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

/* Device Status */
#define Idx_CST_ADDRESS_OUT_OF_RANGE 0x80000000
#define Idx_CST_ADDRESS_MISALIGN     0x40000000
#define Idx_CST_BLOCK_LEN_ERROR      0x20000000
#define Idx_CST_ERASE_SEQ_ERROR      0x10000000
#define Idx_CST_ERASE_PARAM          0x08000000
#define Idx_CST_WP_VIOLATION         0x04000000
#define Idx_CST_DEVICE_IS_LOCKED     0x02000000
#define Idx_CST_LOCK_UNLOCK_FAILED   0x01000000
#define Idx_CST_COM_CRC_ERROR        0x00800000
#define Idx_CST_ILLEGAL_COMMAND      0x00400000
#define Idx_CST_DEVICE_ECC_FAILED    0x00200000
#define Idx_CST_CC_ERROR             0x00100000
#define Idx_CST_ERROR                0x00080000
#define Idx_CST_CID_CSD_OVERWRITE    0x00010000
#define Idx_CST_WP_ERASE_SKIP        0x00008000
#define Idx_CST_ERASE_RESET          0x00002000
#define Idx_CST_CURRENT_STATE        0x00001E00
#define Idx_CST_READY_FOR_DATA       0x00000100
#define Idx_CST_SWITCH_ERROR         0x00000080
#define Idx_CST_EXCEPTION_EVENT      0x00000040
#define Idx_CST_APP_CMD              0x00000020

/* Register : Extended CSD */
/* Properties Segment */
#define Idx_ExtCSD_S_CMD_SET                    504
#define Idx_ExtCSD_HPI_FEATURES                 503
#define Idx_ExtCSD_BKOPS_SUPPORT                502
#define Idx_ExtCSD_MAX_PACKED_READS             501
#define Idx_ExtCSD_MAX_PACKED_WRITES            500
#define Idx_ExtCSD_DATA_TAG_SUPPORT             499
#define Idx_ExtCSD_TAG_UNIT_SIZE                498
#define Idx_ExtCSD_TAG_RES_SIZE                 497
#define Idx_ExtCSD_CONTEXT_CAPABILITIES         496
#define Idx_ExtCSD_LARGE_UNIT_SIZE_M1           495
#define Idx_ExtCSD_EXT_SUPPORT                  494
#define Idx_ExtCSD_CACHE_SIZE                   249
#define Idx_ExtCSD_GENERIC_CMD6_TIME            248
#define Idx_ExtCSD_POWER_OFF_LONG_TIME          247
#define Idx_ExtCSD_BKOPS_STATUS                 246
#define Idx_ExtCSD_CORRECTLY_PRG_SECTORS_NUM    242
#define Idx_ExtCSD_INI_TIMEOUT_AP               241
#define Idx_ExtCSD_PWR_CL_DDR_52_360            239
#define Idx_ExtCSD_PWR_CL_DDR_52_195            238
#define Idx_ExtCSD_PWR_CL_200_360               237
#define Idx_ExtCSD_PWR_CL_200_195               236
#define Idx_ExtCSD_MIN_PERF_DDR_W_8_52          235
#define Idx_ExtCSD_MIN_PERF_DDR_R_8_52          234
#define Idx_ExtCSD_TRIM_MULT                    232
#define Idx_ExtCSD_SEC_FEATURE_SUPPORT          231
#define Idx_ExtCSD_SEC_ERASE_MULT               230
#define Idx_ExtCSD_SEC_TRIM_MULT                229
#define Idx_ExtCSD_BOOT_INFO                    228
#define Idx_ExtCSD_BOOT_SIZE_MULT               226
#define Idx_ExtCSD_ACC_SIZE                     225
#define Idx_ExtCSD_HC_ERASE_GRP_SIZE            224
#define Idx_ExtCSD_ERASE_TIMEOUT_MULT           223
#define Idx_ExtCSD_REL_WR_SEC_C                 222
#define Idx_ExtCSD_HC_WP_GRP_SIZE               221
#define Idx_ExtCSD_S_C_VCC                      220
#define Idx_ExtCSD_S_C_VCCQ                     219
#define Idx_ExtCSD_S_A_TIMEOUT                  217
#define Idx_ExtCSD_SEC_COUNT                    212
#define Idx_ExtCSD_MIN_PERF_W_8_52              210
#define Idx_ExtCSD_MIN_PERF_R_8_52              209
#define Idx_ExtCSD_MIN_PERF_W_8_26_4_52         208
#define Idx_ExtCSD_MIN_PERF_R_8_26_4_52         207
#define Idx_ExtCSD_MIN_PERF_W_4_26              206
#define Idx_ExtCSD_MIN_PERF_R_4_26              205
#define Idx_ExtCSD_PWR_CL_26_360                203
#define Idx_ExtCSD_PWR_CL_52_360                202
#define Idx_ExtCSD_PWR_CL_26_195                201
#define Idx_ExtCSD_PWR_CL_52_195                200
#define Idx_ExtCSD_PARTITION_SWITCH_TIME        199
#define Idx_ExtCSD_OUT_OF_INTERRUPT_TIME        198
#define Idx_ExtCSD_DRIVER_STRENGTH              197
#define Idx_ExtCSD_DEVICE_TYPE                  196
#define Idx_ExtCSD_CSD_STRUCTURE                194
#define Idx_ExtCSD_EXT_CSD_REV                  192

/* Modes Segment */
#define Idx_ExtCSD_CMD_SET                      191
#define Idx_ExtCSD_CMD_SET_REV                  189
#define Idx_ExtCSD_POWER_CLASS                  187
#define Idx_ExtCSD_HS_TIMING                    185
#define Idx_ExtCSD_BUS_WIDTH                    183
#define Idx_ExtCSD_ERASED_MEM_CONT              181
#define Idx_ExtCSD_PARTITION_CONFIG             179
#define PCFG_BOOT_ACK                           0x40
#define PCFG_BOOT_PARTITION_ENABLE_MASK         0x38
#define PCFG_BOOT_PARTITION_ENABLE_NONE         0x00
#define PCFG_BOOT_PARTITION_ENABLE_BOOT1        0x08
#define PCFG_BOOT_PARTITION_ENABLE_BOOT2        0x10
#define PCFG_BOOT_PARTITION_ENABLE_DATA         0x38
#define PCFG_PARTITION_ACCESS_MASK              0x07
#define PCFG_PARTITION_ACCESS_DATA              0x00
#define PCFG_PARTITION_ACCESS_BOOT1             0x01
#define PCFG_PARTITION_ACCESS_BOOT2             0x02
#define PCFG_PARTITION_ACCESS_RPMB              0x03
#define PCFG_PARTITION_ACCESS_GP0               0x04
#define PCFG_PARTITION_ACCESS_GP1               0x05
#define PCFG_PARTITION_ACCESS_GP2               0x06
#define PCFG_PARTITION_ACCESS_GP3               0x07
#define Idx_ExtCSD_BOOT_CONFIG_PROT             178
#define Idx_ExtCSD_BOOT_BUS_CONDITIONS          177
#define Idx_ExtCSD_ERASE_GROUP_DEF              175
#define Idx_ExtCSD_BOOT_WP_STATUS               174
#define Idx_ExtCSD_BOOT_WP                      173
#define Idx_ExtCSD_USER_WP                      171
#define Idx_ExtCSD_FW_CONFIG                    169
#define Idx_ExtCSD_RPMB_SIZE_MULT               168
#define Idx_ExtCSD_WR_REL_SET                   167
#define Idx_ExtCSD_WR_REL_PARAM                 166
#define Idx_ExtCSD_SANITIZE_START               165
#define Idx_ExtCSD_BKOPS_START                  164
#define Idx_ExtCSD_BKOPS_EN                     163
#define Idx_ExtCSD_RST_n_FUNCTION               162
#define Idx_ExtCSD_HPI_MGMT                     161
#define Idx_ExtCSD_PARTITIONING_SUPPORT         160
#define Idx_ExtCSD_MAX_ENH_SIZE_MULT            157
#define Idx_ExtCSD_PARTITIONS_ATTRIBUTE         156
#define Idx_ExtCSD_PARTITION_SETTING_COMPLETED  155
#define Idx_ExtCSD_GP_SIZE_MULT                 143
#define Idx_ExtCSD_ENH_SIZE_MULT                140
#define Idx_ExtCSD_ENH_START_ADDR               136
#define Idx_ExtCSD_SEC_BAD_BLK_MGMNT            134
#define Idx_ExtCSD_TCASE_SUPPORT                132
#define Idx_ExtCSD_PERIODIC_WAKEUP              131
#define Idx_ExtCSD_PROGRAM_CID_CSD_DDR_SUPPORT  130
#define Idx_ExtCSD_VENDOR_SPECIFIC_FIELD         64
#define Idx_ExtCSD_NATIVE_SECTOR_SIZE            63
#define Idx_ExtCSD_USE_NATIVE_SECTOR             62
#define Idx_ExtCSD_DATA_SECTOR_SIZE              61
#define Idx_ExtCSD_INI_TIMEOUT_EMU               60
#define Idx_ExtCSD_CLASS_6_CTRL                  59
#define Idx_ExtCSD_DYNCAP_NEEDED                 58
#define Idx_ExtCSD_EXCEPTION_EVENTS_CTRL         56
#define Idx_ExtCSD_EXCEPTION_EVENTS_STATUS       54
#define Idx_ExtCSD_EXT_PARTITIONS_ATTRIBUTE      52
#define Idx_ExtCSD_CONTEXT_CONF                  37
#define Idx_ExtCSD_PACKED_COMMAND_STATUS         36
#define Idx_ExtCSD_PACKED_FAILURE_INDEX          35
#define Idx_ExtCSD_POWER_OFF_NOTIFICATION        34
#define Idx_ExtCSD_CACHE_CTRL                    33
#define Idx_ExtCSD_FLUSH_CACHE                   32

#define Idx_ExtCSD_ACC_CMD  0   /* EXT_CSD Access mode : Command Set */
#define Idx_ExtCSD_ACC_SET  1   /* EXT_CSD Access mode : Set Bits */
#define Idx_ExtCSD_ACC_CLR  2   /* EXT_CSD Access mode : Clear Bits */
#define Idx_ExtCSD_ACC_WRB  3   /* EXT_CSD Access mode : Write Byte */

/* OCR */
#define OCR_READY       0x80000000
#define OCR_SECTOR_MODE 0x40000000
#define OCR_VDD_33_34   0x00200000
#define OCR_VDD_17_195  0x00000080


//int rom_emmc_get_nvram_memcfg(uint32_t * memcfg );
int rom_emmc_get_nvram_memcfg(uint32_t * memcfg, unsigned char * dma_virt_addr );
int rom_emmc_init(void);
int rom_emmc_boot(unsigned char * dma_addr, unsigned long rom_param, cfe_rom_media_params *media_params);

#endif /* _FSBL_EMMC_H */
