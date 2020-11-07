/*---------------------------------------------------------------------------
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 ------------------------------------------------------------------------- */
#ifndef BSL_H__
#define BSL_H__

/**
 * m = memory, c = core, r = register, f = field, d = data.
 */
#if !defined(GET_FIELD) && !defined(SET_FIELD)
#define BRCM_ALIGN(c,r,f)   c##_##r##_##f##_ALIGN
#define BRCM_BITS(c,r,f)    c##_##r##_##f##_BITS
#define BRCM_MASK(c,r,f)    c##_##r##_##f##_MASK
#define BRCM_SHIFT(c,r,f)   c##_##r##_##f##_SHIFT

#define GET_FIELD(m,c,r,f) \
	((((m) & BRCM_MASK(c,r,f)) >> BRCM_SHIFT(c,r,f)) << BRCM_ALIGN(c,r,f))

#define SET_FIELD(m,c,r,f,d) \
	((m) = (((m) & ~BRCM_MASK(c,r,f)) | ((((d) >> BRCM_ALIGN(c,r,f)) << \
	 BRCM_SHIFT(c,r,f)) & BRCM_MASK(c,r,f))) \
	)

#define SET_TYPE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##d)
#define SET_NAME_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##r##_##f##_##d)
#define SET_VALUE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,d)

#endif /* GET & SET */

/***************************************************************************
 *registers
 ***************************************************************************/
#define BSL_CHIP_ADDRESS_REG                     0x00000000 /* BSL Device Address for register access */
#define BSL_CHIP_ADDRESS_IRAM                    0x00000004 /* BSL Device Address for instruction memory access */
#define BSL_CHIP_ADDRESS_DARAM                   0x00000008 /* BSL Device Address for data memory access */
#define BSL_BSL_MODE_CONFIG                      0x00000010 /* BSL Operating Mode Configuration */
#define BSL_BSL_PARAM                            0x00000014 /* BSL Parameter Register */
#define BSL_BSL_ATE_MODE_CONFIG                  0x00000018 /* BSL High speed configuration for ATE */
#define BSL_BSL_I2C_IO_CONFIG                    0x0000001c /* BSL I2C IO drive strength configuration */
#define BSL_BSL_ALIGN_EPU_CYC_DEC                0x00000020 /* BSL decode to align to EPU for BSL RMA accesses */
#define BSL_BSL_CRC32_RESULTS                    0x00000024 /* BSL PMD IRAM WRITE CRC32 results */
#define BSL_BSL_CRC32_POSTD_RESULTS              0x00000028 /* BSL PMD POST DECRYPTION IRAM WRITE CRC32 results */

/***************************************************************************
 *CHIP_ADDRESS_REG - BSL Device Address for register access
 ***************************************************************************/
/* BSL :: CHIP_ADDRESS_REG :: reserved0 [31:10] */
#define BSL_CHIP_ADDRESS_REG_reserved0_MASK                        0xfffffc00
#define BSL_CHIP_ADDRESS_REG_reserved0_ALIGN                       0
#define BSL_CHIP_ADDRESS_REG_reserved0_BITS                        22
#define BSL_CHIP_ADDRESS_REG_reserved0_SHIFT                       10

/* BSL :: CHIP_ADDRESS_REG :: CHIP_ADDRESS_REG [09:00] */
#define BSL_CHIP_ADDRESS_REG_CHIP_ADDRESS_REG_MASK                 0x000003ff
#define BSL_CHIP_ADDRESS_REG_CHIP_ADDRESS_REG_ALIGN                0
#define BSL_CHIP_ADDRESS_REG_CHIP_ADDRESS_REG_BITS                 10
#define BSL_CHIP_ADDRESS_REG_CHIP_ADDRESS_REG_SHIFT                0
#define BSL_CHIP_ADDRESS_REG_CHIP_ADDRESS_REG_DEFAULT              80

/***************************************************************************
 *CHIP_ADDRESS_IRAM - BSL Device Address for instruction memory access
 ***************************************************************************/
/* BSL :: CHIP_ADDRESS_IRAM :: reserved0 [31:10] */
#define BSL_CHIP_ADDRESS_IRAM_reserved0_MASK                       0xfffffc00
#define BSL_CHIP_ADDRESS_IRAM_reserved0_ALIGN                      0
#define BSL_CHIP_ADDRESS_IRAM_reserved0_BITS                       22
#define BSL_CHIP_ADDRESS_IRAM_reserved0_SHIFT                      10

/* BSL :: CHIP_ADDRESS_IRAM :: CHIP_ADDRESS_IRAM [09:00] */
#define BSL_CHIP_ADDRESS_IRAM_CHIP_ADDRESS_IRAM_MASK               0x000003ff
#define BSL_CHIP_ADDRESS_IRAM_CHIP_ADDRESS_IRAM_ALIGN              0
#define BSL_CHIP_ADDRESS_IRAM_CHIP_ADDRESS_IRAM_BITS               10
#define BSL_CHIP_ADDRESS_IRAM_CHIP_ADDRESS_IRAM_SHIFT              0
#define BSL_CHIP_ADDRESS_IRAM_CHIP_ADDRESS_IRAM_DEFAULT            81

/***************************************************************************
 *CHIP_ADDRESS_DARAM - BSL Device Address for data memory access
 ***************************************************************************/
/* BSL :: CHIP_ADDRESS_DARAM :: reserved0 [31:10] */
#define BSL_CHIP_ADDRESS_DARAM_reserved0_MASK                      0xfffffc00
#define BSL_CHIP_ADDRESS_DARAM_reserved0_ALIGN                     0
#define BSL_CHIP_ADDRESS_DARAM_reserved0_BITS                      22
#define BSL_CHIP_ADDRESS_DARAM_reserved0_SHIFT                     10

/* BSL :: CHIP_ADDRESS_DARAM :: CHIP_ADDRESS_DARAM [09:00] */
#define BSL_CHIP_ADDRESS_DARAM_CHIP_ADDRESS_DARAM_MASK             0x000003ff
#define BSL_CHIP_ADDRESS_DARAM_CHIP_ADDRESS_DARAM_ALIGN            0
#define BSL_CHIP_ADDRESS_DARAM_CHIP_ADDRESS_DARAM_BITS             10
#define BSL_CHIP_ADDRESS_DARAM_CHIP_ADDRESS_DARAM_SHIFT            0
#define BSL_CHIP_ADDRESS_DARAM_CHIP_ADDRESS_DARAM_DEFAULT          82

/***************************************************************************
 *BSL_MODE_CONFIG - BSL Operating Mode Configuration
 ***************************************************************************/
/* BSL :: BSL_MODE_CONFIG :: reserved0 [31:17] */
#define BSL_BSL_MODE_CONFIG_reserved0_MASK                         0xfffe0000
#define BSL_BSL_MODE_CONFIG_reserved0_ALIGN                        0
#define BSL_BSL_MODE_CONFIG_reserved0_BITS                         15
#define BSL_BSL_MODE_CONFIG_reserved0_SHIFT                        17

/* BSL :: BSL_MODE_CONFIG :: CFG_BSL_BYPASS_DB [16:16] */
#define BSL_BSL_MODE_CONFIG_CFG_BSL_BYPASS_DB_MASK                 0x00010000
#define BSL_BSL_MODE_CONFIG_CFG_BSL_BYPASS_DB_ALIGN                0
#define BSL_BSL_MODE_CONFIG_CFG_BSL_BYPASS_DB_BITS                 1
#define BSL_BSL_MODE_CONFIG_CFG_BSL_BYPASS_DB_SHIFT                16
#define BSL_BSL_MODE_CONFIG_CFG_BSL_BYPASS_DB_DEFAULT              0

/* BSL :: BSL_MODE_CONFIG :: reserved1 [15:02] */
#define BSL_BSL_MODE_CONFIG_reserved1_MASK                         0x0000fffc
#define BSL_BSL_MODE_CONFIG_reserved1_ALIGN                        0
#define BSL_BSL_MODE_CONFIG_reserved1_BITS                         14
#define BSL_BSL_MODE_CONFIG_reserved1_SHIFT                        2

/* BSL :: BSL_MODE_CONFIG :: CFG_10BIT_MODE [01:01] */
#define BSL_BSL_MODE_CONFIG_CFG_10BIT_MODE_MASK                    0x00000002
#define BSL_BSL_MODE_CONFIG_CFG_10BIT_MODE_ALIGN                   0
#define BSL_BSL_MODE_CONFIG_CFG_10BIT_MODE_BITS                    1
#define BSL_BSL_MODE_CONFIG_CFG_10BIT_MODE_SHIFT                   1
#define BSL_BSL_MODE_CONFIG_CFG_10BIT_MODE_DEFAULT                 0

/* BSL :: BSL_MODE_CONFIG :: CFG_ALLOW_HS_MODE [00:00] */
#define BSL_BSL_MODE_CONFIG_CFG_ALLOW_HS_MODE_MASK                 0x00000001
#define BSL_BSL_MODE_CONFIG_CFG_ALLOW_HS_MODE_ALIGN                0
#define BSL_BSL_MODE_CONFIG_CFG_ALLOW_HS_MODE_BITS                 1
#define BSL_BSL_MODE_CONFIG_CFG_ALLOW_HS_MODE_SHIFT                0
#define BSL_BSL_MODE_CONFIG_CFG_ALLOW_HS_MODE_DEFAULT              0

/***************************************************************************
 *BSL_PARAM - BSL Parameter Register
 ***************************************************************************/
/* BSL :: BSL_PARAM :: reserved_for_eco0 [31:00] */
#define BSL_BSL_PARAM_reserved_for_eco0_MASK                       0xffffffff
#define BSL_BSL_PARAM_reserved_for_eco0_ALIGN                      0
#define BSL_BSL_PARAM_reserved_for_eco0_BITS                       32
#define BSL_BSL_PARAM_reserved_for_eco0_SHIFT                      0
#define BSL_BSL_PARAM_reserved_for_eco0_DEFAULT                    0

/***************************************************************************
 *BSL_ATE_MODE_CONFIG - BSL High speed configuration for ATE
 ***************************************************************************/
/* BSL :: BSL_ATE_MODE_CONFIG :: reserved0 [31:01] */
#define BSL_BSL_ATE_MODE_CONFIG_reserved0_MASK                     0xfffffffe
#define BSL_BSL_ATE_MODE_CONFIG_reserved0_ALIGN                    0
#define BSL_BSL_ATE_MODE_CONFIG_reserved0_BITS                     31
#define BSL_BSL_ATE_MODE_CONFIG_reserved0_SHIFT                    1

/* BSL :: BSL_ATE_MODE_CONFIG :: CFG_FORCE_HS_DET [00:00] */
#define BSL_BSL_ATE_MODE_CONFIG_CFG_FORCE_HS_DET_MASK              0x00000001
#define BSL_BSL_ATE_MODE_CONFIG_CFG_FORCE_HS_DET_ALIGN             0
#define BSL_BSL_ATE_MODE_CONFIG_CFG_FORCE_HS_DET_BITS              1
#define BSL_BSL_ATE_MODE_CONFIG_CFG_FORCE_HS_DET_SHIFT             0
#define BSL_BSL_ATE_MODE_CONFIG_CFG_FORCE_HS_DET_DEFAULT           0

/***************************************************************************
 *BSL_I2C_IO_CONFIG - BSL I2C IO drive strength configuration
 ***************************************************************************/
/* BSL :: BSL_I2C_IO_CONFIG :: reserved0 [31:11] */
#define BSL_BSL_I2C_IO_CONFIG_reserved0_MASK                       0xfffff800
#define BSL_BSL_I2C_IO_CONFIG_reserved0_ALIGN                      0
#define BSL_BSL_I2C_IO_CONFIG_reserved0_BITS                       21
#define BSL_BSL_I2C_IO_CONFIG_reserved0_SHIFT                      11

/* BSL :: BSL_I2C_IO_CONFIG :: CFG_SCL_OUTPUT_DRIVE [10:08] */
#define BSL_BSL_I2C_IO_CONFIG_CFG_SCL_OUTPUT_DRIVE_MASK            0x00000700
#define BSL_BSL_I2C_IO_CONFIG_CFG_SCL_OUTPUT_DRIVE_ALIGN           0
#define BSL_BSL_I2C_IO_CONFIG_CFG_SCL_OUTPUT_DRIVE_BITS            3
#define BSL_BSL_I2C_IO_CONFIG_CFG_SCL_OUTPUT_DRIVE_SHIFT           8
#define BSL_BSL_I2C_IO_CONFIG_CFG_SCL_OUTPUT_DRIVE_DEFAULT         3

/* BSL :: BSL_I2C_IO_CONFIG :: reserved1 [07:03] */
#define BSL_BSL_I2C_IO_CONFIG_reserved1_MASK                       0x000000f8
#define BSL_BSL_I2C_IO_CONFIG_reserved1_ALIGN                      0
#define BSL_BSL_I2C_IO_CONFIG_reserved1_BITS                       5
#define BSL_BSL_I2C_IO_CONFIG_reserved1_SHIFT                      3

/* BSL :: BSL_I2C_IO_CONFIG :: CFG_SDA_OUTPUT_DRIVE [02:00] */
#define BSL_BSL_I2C_IO_CONFIG_CFG_SDA_OUTPUT_DRIVE_MASK            0x00000007
#define BSL_BSL_I2C_IO_CONFIG_CFG_SDA_OUTPUT_DRIVE_ALIGN           0
#define BSL_BSL_I2C_IO_CONFIG_CFG_SDA_OUTPUT_DRIVE_BITS            3
#define BSL_BSL_I2C_IO_CONFIG_CFG_SDA_OUTPUT_DRIVE_SHIFT           0
#define BSL_BSL_I2C_IO_CONFIG_CFG_SDA_OUTPUT_DRIVE_DEFAULT         3

/***************************************************************************
 *BSL_ALIGN_EPU_CYC_DEC - BSL decode to align to EPU for BSL RMA accesses
 ***************************************************************************/
/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: reserved0 [31:12] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved0_MASK                   0xfffff000
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved0_ALIGN                  0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved0_BITS                   20
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved0_SHIFT                  12

/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: CFG_BSL_EPU_CYC_CNT_BSL_RD_DEC [11:10] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_BSL_RD_DEC_MASK 0x00000c00
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_BSL_RD_DEC_ALIGN 0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_BSL_RD_DEC_BITS 2
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_BSL_RD_DEC_SHIFT 10
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_BSL_RD_DEC_DEFAULT 3

/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: CFG_BSL_EPU_CYC_CNT_CPU_RD_DEC [09:08] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_CPU_RD_DEC_MASK 0x00000300
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_CPU_RD_DEC_ALIGN 0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_CPU_RD_DEC_BITS 2
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_CPU_RD_DEC_SHIFT 8
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_CPU_RD_DEC_DEFAULT 1

/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: CFG_BSL_EPU_CYC_CNT_RD_ACK_DEC [07:06] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_RD_ACK_DEC_MASK 0x000000c0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_RD_ACK_DEC_ALIGN 0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_RD_ACK_DEC_BITS 2
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_RD_ACK_DEC_SHIFT 6
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_RD_ACK_DEC_DEFAULT 3

/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: CFG_BSL_EPU_CYC_CNT_WR_ACK_DEC [05:04] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_WR_ACK_DEC_MASK 0x00000030
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_WR_ACK_DEC_ALIGN 0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_WR_ACK_DEC_BITS 2
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_WR_ACK_DEC_SHIFT 4
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_WR_ACK_DEC_DEFAULT 1

/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: CFG_BSL_EPU_CYC_CNT_START_DEC [03:02] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_START_DEC_MASK 0x0000000c
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_START_DEC_ALIGN 0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_START_DEC_BITS 2
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_START_DEC_SHIFT 2
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_START_DEC_DEFAULT 0

/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: reserved1 [01:01] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved1_MASK                   0x00000002
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved1_ALIGN                  0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved1_BITS                   1
#define BSL_BSL_ALIGN_EPU_CYC_DEC_reserved1_SHIFT                  1

/* BSL :: BSL_ALIGN_EPU_CYC_DEC :: CFG_BSL_EPU_CYC_CNT_DEC_EN [00:00] */
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_DEC_EN_MASK  0x00000001
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_DEC_EN_ALIGN 0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_DEC_EN_BITS  1
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_DEC_EN_SHIFT 0
#define BSL_BSL_ALIGN_EPU_CYC_DEC_CFG_BSL_EPU_CYC_CNT_DEC_EN_DEFAULT 0

/***************************************************************************
 *BSL_CRC32_RESULTS - BSL PMD IRAM WRITE CRC32 results
 ***************************************************************************/
/* BSL :: BSL_CRC32_RESULTS :: BSL_CRC32_RESULTS [31:00] */
#define BSL_BSL_CRC32_RESULTS_BSL_CRC32_RESULTS_MASK               0xffffffff
#define BSL_BSL_CRC32_RESULTS_BSL_CRC32_RESULTS_ALIGN              0
#define BSL_BSL_CRC32_RESULTS_BSL_CRC32_RESULTS_BITS               32
#define BSL_BSL_CRC32_RESULTS_BSL_CRC32_RESULTS_SHIFT              0
#define BSL_BSL_CRC32_RESULTS_BSL_CRC32_RESULTS_DEFAULT            4294967295

/***************************************************************************
 *BSL_CRC32_POSTD_RESULTS - BSL PMD POST DECRYPTION IRAM WRITE CRC32 results
 ***************************************************************************/
/* BSL :: BSL_CRC32_POSTD_RESULTS :: BSL_CRC32_POSTD_RESULTS [31:00] */
#define BSL_BSL_CRC32_POSTD_RESULTS_BSL_CRC32_POSTD_RESULTS_MASK   0xffffffff
#define BSL_BSL_CRC32_POSTD_RESULTS_BSL_CRC32_POSTD_RESULTS_ALIGN  0
#define BSL_BSL_CRC32_POSTD_RESULTS_BSL_CRC32_POSTD_RESULTS_BITS   32
#define BSL_BSL_CRC32_POSTD_RESULTS_BSL_CRC32_POSTD_RESULTS_SHIFT  0
#define BSL_BSL_CRC32_POSTD_RESULTS_BSL_CRC32_POSTD_RESULTS_DEFAULT 4294967295

#endif /* #ifndef BSL_H__ */

/* End of File */
