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
#ifndef OTP_H__
#define OTP_H__

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
#define OTP_OTP_CONFIG                           0x00000000 /* OTP Config */
#define OTP_OTP_CPU_CONTROL_LO                   0x00000004 /* OTP CPU interface Control Low */
#define OTP_OTP_CPU_CONTROL_HI                   0x00000008 /* OTP CPU interface Control High */
#define OTP_OTP_CPU_STATUS                       0x0000000c /* OTP CPU interface Status */
#define OTP_OTP_CPU_WORD_ADDR                    0x00000010 /* OTP CPU interface Word Address */
#define OTP_OTP_CPU_BITSEL                       0x00000014 /* OTP CPU interface Bitsel */
#define OTP_OTP_CPU_WR_DATA                      0x00000018 /* OTP CPU interface Write Data */
#define OTP_OTP_CPU_RD_DATA                      0x0000001c /* OTP CPU interface Read Data */
#define OTP_OTP_CONFIG_DIRECT_0                  0x00000020 /* OTP Config Data Direct Access 0 */
#define OTP_OTP_CONFIG_DIRECT_1                  0x00000024 /* OTP Config Data Direct Access 1 */
#define OTP_OTP_CONFIG_DIRECT_2                  0x00000028 /* OTP Config Data Direct Access 2 */
#define OTP_OTP_CONFIG_DIRECT_3                  0x0000002c /* OTP Config Data Direct Access 3 */
#define OTP_OTP_CONFIG_DIRECT_4                  0x00000030 /* OTP Config Data Direct Access 4 */
#define OTP_OTP_CONFIG_DIRECT_5                  0x00000034 /* OTP Config Data Direct Access 5 */
#define OTP_OTP_CONFIG_DIRECT_6                  0x00000038 /* OTP Config Data Direct Access 6 */
#define OTP_OTP_CONFIG_DIRECT_7                  0x0000003c /* OTP Config Data Direct Access 7 */
#define OTP_OTP_CONFIG_DIRECT_8                  0x00000040 /* OTP Config Data Direct Access 8 */
#define OTP_OTP_CONFIG_DIRECT_9                  0x00000044 /* OTP Config Data Direct Access 9 */
#define OTP_OTP_CONFIG_DIRECT_10                 0x00000048 /* OTP Config Data Direct Access 10 */
#define OTP_OTP_CONFIG_DIRECT_11                 0x0000004c /* OTP Config Data Direct Access 11 */
#define OTP_OTP_CONFIG_DIRECT_12                 0x00000050 /* OTP Config Data Direct Access 12 */
#define OTP_OTP_CONFIG_DIRECT_13                 0x00000054 /* OTP Config Data Direct Access 13 */
#define OTP_OTP_CONFIG_DIRECT_14                 0x00000058 /* OTP Config Data Direct Access 14 */
#define OTP_OTP_CONFIG_DIRECT_15                 0x0000005c /* OTP Config Data Direct Access 15 */

/***************************************************************************
 *OTP_CONFIG - OTP Config
 ***************************************************************************/
/* OTP :: OTP_CONFIG :: reserved0 [31:06] */
#define OTP_OTP_CONFIG_reserved0_MASK                              0xffffffc0
#define OTP_OTP_CONFIG_reserved0_ALIGN                             0
#define OTP_OTP_CONFIG_reserved0_BITS                              26
#define OTP_OTP_CONFIG_reserved0_SHIFT                             6

/* OTP :: OTP_CONFIG :: OTP_SOFT_RST [05:05] */
#define OTP_OTP_CONFIG_OTP_SOFT_RST_MASK                           0x00000020
#define OTP_OTP_CONFIG_OTP_SOFT_RST_ALIGN                          0
#define OTP_OTP_CONFIG_OTP_SOFT_RST_BITS                           1
#define OTP_OTP_CONFIG_OTP_SOFT_RST_SHIFT                          5
#define OTP_OTP_CONFIG_OTP_SOFT_RST_DEFAULT                        0

/* OTP :: OTP_CONFIG :: reserved1 [04:02] */
#define OTP_OTP_CONFIG_reserved1_MASK                              0x0000001c
#define OTP_OTP_CONFIG_reserved1_ALIGN                             0
#define OTP_OTP_CONFIG_reserved1_BITS                              3
#define OTP_OTP_CONFIG_reserved1_SHIFT                             2

/* OTP :: OTP_CONFIG :: OTP_CPU_MODE [01:01] */
#define OTP_OTP_CONFIG_OTP_CPU_MODE_MASK                           0x00000002
#define OTP_OTP_CONFIG_OTP_CPU_MODE_ALIGN                          0
#define OTP_OTP_CONFIG_OTP_CPU_MODE_BITS                           1
#define OTP_OTP_CONFIG_OTP_CPU_MODE_SHIFT                          1
#define OTP_OTP_CONFIG_OTP_CPU_MODE_DEFAULT                        0

/* OTP :: OTP_CONFIG :: reserved2 [00:00] */
#define OTP_OTP_CONFIG_reserved2_MASK                              0x00000001
#define OTP_OTP_CONFIG_reserved2_ALIGN                             0
#define OTP_OTP_CONFIG_reserved2_BITS                              1
#define OTP_OTP_CONFIG_reserved2_SHIFT                             0

/***************************************************************************
 *OTP_CPU_CONTROL_LO - OTP CPU interface Control Low
 ***************************************************************************/
/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_BYPASS_OTP_CLK [31:31] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BYPASS_OTP_CLK_MASK         0x80000000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BYPASS_OTP_CLK_ALIGN        0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BYPASS_OTP_CLK_BITS         1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BYPASS_OTP_CLK_SHIFT        31
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BYPASS_OTP_CLK_DEFAULT      0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_READ_FOUT [30:30] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_READ_FOUT_MASK              0x40000000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_READ_FOUT_ALIGN             0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_READ_FOUT_BITS              1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_READ_FOUT_SHIFT             30
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_READ_FOUT_DEFAULT           0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_TEST_COL [29:29] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_TEST_COL_MASK               0x20000000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_TEST_COL_ALIGN              0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_TEST_COL_BITS               1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_TEST_COL_SHIFT              29
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_TEST_COL_DEFAULT            0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_DEBUG_SEL [28:25] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_SEL_MASK              0x1e000000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_SEL_ALIGN             0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_SEL_BITS              4
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_SEL_SHIFT             25
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_SEL_DEFAULT           0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_BURST_STAT_SEL [24:24] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BURST_STAT_SEL_MASK         0x01000000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BURST_STAT_SEL_ALIGN        0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BURST_STAT_SEL_BITS         1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BURST_STAT_SEL_SHIFT        24
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_BURST_STAT_SEL_DEFAULT      0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_ACC_MODE [23:22] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_ACC_MODE_MASK               0x00c00000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_ACC_MODE_ALIGN              0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_ACC_MODE_BITS               2
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_ACC_MODE_SHIFT              22
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_ACC_MODE_DEFAULT            0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_PROG_EN [21:21] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PROG_EN_MASK                0x00200000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PROG_EN_ALIGN               0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PROG_EN_BITS                1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PROG_EN_SHIFT               21
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PROG_EN_DEFAULT             0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_DEBUG_MODE [20:20] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_MODE_MASK             0x00100000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_MODE_ALIGN            0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_MODE_BITS             1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_MODE_SHIFT            20
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_MODE_DEFAULT          0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_WRP_CONT_FAIL [19:19] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_CONT_FAIL_MASK          0x00080000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_CONT_FAIL_ALIGN         0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_CONT_FAIL_BITS          1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_CONT_FAIL_SHIFT         19
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_CONT_FAIL_DEFAULT       0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_PRG_VF_FLAG [18:18] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PRG_VF_FLAG_MASK            0x00040000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PRG_VF_FLAG_ALIGN           0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PRG_VF_FLAG_BITS            1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PRG_VF_FLAG_SHIFT           18
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_PRG_VF_FLAG_DEFAULT         0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_WRP_DW [17:17] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_DW_MASK                 0x00020000
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_DW_ALIGN                0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_DW_BITS                 1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_DW_SHIFT                17
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_WRP_DW_DEFAULT              0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_DEBUG [16:09] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_MASK                  0x0001fe00
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_ALIGN                 0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_BITS                  8
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_SHIFT                 9
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_DEBUG_DEFAULT               0

/* OTP :: OTP_CPU_CONTROL_LO :: reserved0 [08:06] */
#define OTP_OTP_CPU_CONTROL_LO_reserved0_MASK                      0x000001c0
#define OTP_OTP_CPU_CONTROL_LO_reserved0_ALIGN                     0
#define OTP_OTP_CPU_CONTROL_LO_reserved0_BITS                      3
#define OTP_OTP_CPU_CONTROL_LO_reserved0_SHIFT                     6

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_COMMAND [05:01] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_COMMAND_MASK                0x0000003e
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_COMMAND_ALIGN               0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_COMMAND_BITS                5
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_COMMAND_SHIFT               1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_COMMAND_DEFAULT             0

/* OTP :: OTP_CPU_CONTROL_LO :: OTP_CPU_START [00:00] */
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_START_MASK                  0x00000001
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_START_ALIGN                 0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_START_BITS                  1
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_START_SHIFT                 0
#define OTP_OTP_CPU_CONTROL_LO_OTP_CPU_START_DEFAULT               0

/***************************************************************************
 *OTP_CPU_CONTROL_HI - OTP CPU interface Control High
 ***************************************************************************/
/* OTP :: OTP_CPU_CONTROL_HI :: OTP_CPU_CTRL_HI [31:00] */
#define OTP_OTP_CPU_CONTROL_HI_OTP_CPU_CTRL_HI_MASK                0xffffffff
#define OTP_OTP_CPU_CONTROL_HI_OTP_CPU_CTRL_HI_ALIGN               0
#define OTP_OTP_CPU_CONTROL_HI_OTP_CPU_CTRL_HI_BITS                32
#define OTP_OTP_CPU_CONTROL_HI_OTP_CPU_CTRL_HI_SHIFT               0
#define OTP_OTP_CPU_CONTROL_HI_OTP_CPU_CTRL_HI_DEFAULT             0

/***************************************************************************
 *OTP_CPU_STATUS - OTP CPU interface Status
 ***************************************************************************/
/* OTP :: OTP_CPU_STATUS :: reserved0 [31:17] */
#define OTP_OTP_CPU_STATUS_reserved0_MASK                          0xfffe0000
#define OTP_OTP_CPU_STATUS_reserved0_ALIGN                         0
#define OTP_OTP_CPU_STATUS_reserved0_BITS                          15
#define OTP_OTP_CPU_STATUS_reserved0_SHIFT                         17

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_OTP_READY [16:16] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_OTP_READY_MASK                  0x00010000
#define OTP_OTP_CPU_STATUS_OTP_CPU_OTP_READY_ALIGN                 0
#define OTP_OTP_CPU_STATUS_OTP_CPU_OTP_READY_BITS                  1
#define OTP_OTP_CPU_STATUS_OTP_CPU_OTP_READY_SHIFT                 16

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_INVLD_SECACC [15:15] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_SECACC_MASK               0x00008000
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_SECACC_ALIGN              0
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_SECACC_BITS               1
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_SECACC_SHIFT              15
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_SECACC_DEFAULT            0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_INVLD_ACCMD [14:14] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ACCMD_MASK                0x00004000
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ACCMD_ALIGN               0
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ACCMD_BITS                1
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ACCMD_SHIFT               14
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ACCMD_DEFAULT             0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_INVLD_ADDRESS [13:13] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ADDRESS_MASK              0x00002000
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ADDRESS_ALIGN             0
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ADDRESS_BITS              1
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ADDRESS_SHIFT             13
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_ADDRESS_DEFAULT           0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_PROGOK [12:12] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROGOK_MASK                     0x00001000
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROGOK_ALIGN                    0
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROGOK_BITS                     1
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROGOK_SHIFT                    12
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROGOK_DEFAULT                  0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_REFOK [11:11] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_REFOK_MASK                      0x00000800
#define OTP_OTP_CPU_STATUS_OTP_CPU_REFOK_ALIGN                     0
#define OTP_OTP_CPU_STATUS_OTP_CPU_REFOK_BITS                      1
#define OTP_OTP_CPU_STATUS_OTP_CPU_REFOK_SHIFT                     11
#define OTP_OTP_CPU_STATUS_OTP_CPU_REFOK_DEFAULT                   0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_WRP_ERROR [10:10] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_WRP_ERROR_MASK                  0x00000400
#define OTP_OTP_CPU_STATUS_OTP_CPU_WRP_ERROR_ALIGN                 0
#define OTP_OTP_CPU_STATUS_OTP_CPU_WRP_ERROR_BITS                  1
#define OTP_OTP_CPU_STATUS_OTP_CPU_WRP_ERROR_SHIFT                 10
#define OTP_OTP_CPU_STATUS_OTP_CPU_WRP_ERROR_DEFAULT               0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_INVLD_COMMAND [09:09] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_COMMAND_MASK              0x00000200
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_COMMAND_ALIGN             0
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_COMMAND_BITS              1
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_COMMAND_SHIFT             9
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVLD_COMMAND_DEFAULT           0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_STANDBY [08:08] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_STANDBY_MASK                    0x00000100
#define OTP_OTP_CPU_STATUS_OTP_CPU_STANDBY_ALIGN                   0
#define OTP_OTP_CPU_STATUS_OTP_CPU_STANDBY_BITS                    1
#define OTP_OTP_CPU_STATUS_OTP_CPU_STANDBY_SHIFT                   8
#define OTP_OTP_CPU_STATUS_OTP_CPU_STANDBY_DEFAULT                 0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_FDONE [07:07] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_FDONE_MASK                      0x00000080
#define OTP_OTP_CPU_STATUS_OTP_CPU_FDONE_ALIGN                     0
#define OTP_OTP_CPU_STATUS_OTP_CPU_FDONE_BITS                      1
#define OTP_OTP_CPU_STATUS_OTP_CPU_FDONE_SHIFT                     7
#define OTP_OTP_CPU_STATUS_OTP_CPU_FDONE_DEFAULT                   0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_PROG_BLOCKED [06:06] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROG_BLOCKED_MASK               0x00000040
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROG_BLOCKED_ALIGN              0
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROG_BLOCKED_BITS               1
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROG_BLOCKED_SHIFT              6
#define OTP_OTP_CPU_STATUS_OTP_CPU_PROG_BLOCKED_DEFAULT            0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_INVALID_PROG_REQ [05:05] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVALID_PROG_REQ_MASK           0x00000020
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVALID_PROG_REQ_ALIGN          0
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVALID_PROG_REQ_BITS           1
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVALID_PROG_REQ_SHIFT          5
#define OTP_OTP_CPU_STATUS_OTP_CPU_INVALID_PROG_REQ_DEFAULT        0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_FAIL [04:04] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_FAIL_MASK                       0x00000010
#define OTP_OTP_CPU_STATUS_OTP_CPU_FAIL_ALIGN                      0
#define OTP_OTP_CPU_STATUS_OTP_CPU_FAIL_BITS                       1
#define OTP_OTP_CPU_STATUS_OTP_CPU_FAIL_SHIFT                      4
#define OTP_OTP_CPU_STATUS_OTP_CPU_FAIL_DEFAULT                    0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_BUSY [03:03] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_BUSY_MASK                       0x00000008
#define OTP_OTP_CPU_STATUS_OTP_CPU_BUSY_ALIGN                      0
#define OTP_OTP_CPU_STATUS_OTP_CPU_BUSY_BITS                       1
#define OTP_OTP_CPU_STATUS_OTP_CPU_BUSY_SHIFT                      3
#define OTP_OTP_CPU_STATUS_OTP_CPU_BUSY_DEFAULT                    0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_BIT_DOUT [02:02] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_BIT_DOUT_MASK                   0x00000004
#define OTP_OTP_CPU_STATUS_OTP_CPU_BIT_DOUT_ALIGN                  0
#define OTP_OTP_CPU_STATUS_OTP_CPU_BIT_DOUT_BITS                   1
#define OTP_OTP_CPU_STATUS_OTP_CPU_BIT_DOUT_SHIFT                  2
#define OTP_OTP_CPU_STATUS_OTP_CPU_BIT_DOUT_DEFAULT                0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_DATA_READY [01:01] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_DATA_READY_MASK                 0x00000002
#define OTP_OTP_CPU_STATUS_OTP_CPU_DATA_READY_ALIGN                0
#define OTP_OTP_CPU_STATUS_OTP_CPU_DATA_READY_BITS                 1
#define OTP_OTP_CPU_STATUS_OTP_CPU_DATA_READY_SHIFT                1
#define OTP_OTP_CPU_STATUS_OTP_CPU_DATA_READY_DEFAULT              0

/* OTP :: OTP_CPU_STATUS :: OTP_CPU_COMMAND_DONE [00:00] */
#define OTP_OTP_CPU_STATUS_OTP_CPU_COMMAND_DONE_MASK               0x00000001
#define OTP_OTP_CPU_STATUS_OTP_CPU_COMMAND_DONE_ALIGN              0
#define OTP_OTP_CPU_STATUS_OTP_CPU_COMMAND_DONE_BITS               1
#define OTP_OTP_CPU_STATUS_OTP_CPU_COMMAND_DONE_SHIFT              0
#define OTP_OTP_CPU_STATUS_OTP_CPU_COMMAND_DONE_DEFAULT            0

/***************************************************************************
 *OTP_CPU_WORD_ADDR - OTP CPU interface Word Address
 ***************************************************************************/
/* OTP :: OTP_CPU_WORD_ADDR :: reserved0 [31:09] */
#define OTP_OTP_CPU_WORD_ADDR_reserved0_MASK                       0xfffffe00
#define OTP_OTP_CPU_WORD_ADDR_reserved0_ALIGN                      0
#define OTP_OTP_CPU_WORD_ADDR_reserved0_BITS                       23
#define OTP_OTP_CPU_WORD_ADDR_reserved0_SHIFT                      9

/* OTP :: OTP_CPU_WORD_ADDR :: OTP_CPU_WORD_ADDR [08:00] */
#define OTP_OTP_CPU_WORD_ADDR_OTP_CPU_WORD_ADDR_MASK               0x000001ff
#define OTP_OTP_CPU_WORD_ADDR_OTP_CPU_WORD_ADDR_ALIGN              0
#define OTP_OTP_CPU_WORD_ADDR_OTP_CPU_WORD_ADDR_BITS               9
#define OTP_OTP_CPU_WORD_ADDR_OTP_CPU_WORD_ADDR_SHIFT              0
#define OTP_OTP_CPU_WORD_ADDR_OTP_CPU_WORD_ADDR_DEFAULT            0

/***************************************************************************
 *OTP_CPU_BITSEL - OTP CPU interface Bitsel
 ***************************************************************************/
/* OTP :: OTP_CPU_BITSEL :: reserved0 [31:05] */
#define OTP_OTP_CPU_BITSEL_reserved0_MASK                          0xffffffe0
#define OTP_OTP_CPU_BITSEL_reserved0_ALIGN                         0
#define OTP_OTP_CPU_BITSEL_reserved0_BITS                          27
#define OTP_OTP_CPU_BITSEL_reserved0_SHIFT                         5

/* OTP :: OTP_CPU_BITSEL :: OTP_CPU_BITSEL [04:00] */
#define OTP_OTP_CPU_BITSEL_OTP_CPU_BITSEL_MASK                     0x0000001f
#define OTP_OTP_CPU_BITSEL_OTP_CPU_BITSEL_ALIGN                    0
#define OTP_OTP_CPU_BITSEL_OTP_CPU_BITSEL_BITS                     5
#define OTP_OTP_CPU_BITSEL_OTP_CPU_BITSEL_SHIFT                    0
#define OTP_OTP_CPU_BITSEL_OTP_CPU_BITSEL_DEFAULT                  0

/***************************************************************************
 *OTP_CPU_WR_DATA - OTP CPU interface Write Data
 ***************************************************************************/
/* OTP :: OTP_CPU_WR_DATA :: OTP_CPU_WR_DATA [31:00] */
#define OTP_OTP_CPU_WR_DATA_OTP_CPU_WR_DATA_MASK                   0xffffffff
#define OTP_OTP_CPU_WR_DATA_OTP_CPU_WR_DATA_ALIGN                  0
#define OTP_OTP_CPU_WR_DATA_OTP_CPU_WR_DATA_BITS                   32
#define OTP_OTP_CPU_WR_DATA_OTP_CPU_WR_DATA_SHIFT                  0
#define OTP_OTP_CPU_WR_DATA_OTP_CPU_WR_DATA_DEFAULT                0

/***************************************************************************
 *OTP_CPU_RD_DATA - OTP CPU interface Read Data
 ***************************************************************************/
/* OTP :: OTP_CPU_RD_DATA :: OTP_CPU_RD_DATA [31:00] */
#define OTP_OTP_CPU_RD_DATA_OTP_CPU_RD_DATA_MASK                   0xffffffff
#define OTP_OTP_CPU_RD_DATA_OTP_CPU_RD_DATA_ALIGN                  0
#define OTP_OTP_CPU_RD_DATA_OTP_CPU_RD_DATA_BITS                   32
#define OTP_OTP_CPU_RD_DATA_OTP_CPU_RD_DATA_SHIFT                  0
#define OTP_OTP_CPU_RD_DATA_OTP_CPU_RD_DATA_DEFAULT                0

/***************************************************************************
 *OTP_CONFIG_DIRECT_0 - OTP Config Data Direct Access 0
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_0 :: OTP_CONFIG_DIRECT_0 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_0_OTP_CONFIG_DIRECT_0_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_0_OTP_CONFIG_DIRECT_0_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_0_OTP_CONFIG_DIRECT_0_BITS           32
#define OTP_OTP_CONFIG_DIRECT_0_OTP_CONFIG_DIRECT_0_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_1 - OTP Config Data Direct Access 1
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_1 :: OTP_CONFIG_DIRECT_1 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_1_OTP_CONFIG_DIRECT_1_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_1_OTP_CONFIG_DIRECT_1_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_1_OTP_CONFIG_DIRECT_1_BITS           32
#define OTP_OTP_CONFIG_DIRECT_1_OTP_CONFIG_DIRECT_1_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_2 - OTP Config Data Direct Access 2
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_2 :: OTP_CONFIG_DIRECT_2 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_2_OTP_CONFIG_DIRECT_2_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_2_OTP_CONFIG_DIRECT_2_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_2_OTP_CONFIG_DIRECT_2_BITS           32
#define OTP_OTP_CONFIG_DIRECT_2_OTP_CONFIG_DIRECT_2_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_3 - OTP Config Data Direct Access 3
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_3 :: OTP_CONFIG_DIRECT_3 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_3_OTP_CONFIG_DIRECT_3_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_3_OTP_CONFIG_DIRECT_3_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_3_OTP_CONFIG_DIRECT_3_BITS           32
#define OTP_OTP_CONFIG_DIRECT_3_OTP_CONFIG_DIRECT_3_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_4 - OTP Config Data Direct Access 4
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_4 :: OTP_CONFIG_DIRECT_4 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_4_OTP_CONFIG_DIRECT_4_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_4_OTP_CONFIG_DIRECT_4_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_4_OTP_CONFIG_DIRECT_4_BITS           32
#define OTP_OTP_CONFIG_DIRECT_4_OTP_CONFIG_DIRECT_4_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_5 - OTP Config Data Direct Access 5
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_5 :: OTP_CONFIG_DIRECT_5 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_5_OTP_CONFIG_DIRECT_5_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_5_OTP_CONFIG_DIRECT_5_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_5_OTP_CONFIG_DIRECT_5_BITS           32
#define OTP_OTP_CONFIG_DIRECT_5_OTP_CONFIG_DIRECT_5_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_6 - OTP Config Data Direct Access 6
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_6 :: OTP_CONFIG_DIRECT_6 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_6_OTP_CONFIG_DIRECT_6_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_6_OTP_CONFIG_DIRECT_6_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_6_OTP_CONFIG_DIRECT_6_BITS           32
#define OTP_OTP_CONFIG_DIRECT_6_OTP_CONFIG_DIRECT_6_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_7 - OTP Config Data Direct Access 7
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_7 :: OTP_CONFIG_DIRECT_7 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_7_OTP_CONFIG_DIRECT_7_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_7_OTP_CONFIG_DIRECT_7_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_7_OTP_CONFIG_DIRECT_7_BITS           32
#define OTP_OTP_CONFIG_DIRECT_7_OTP_CONFIG_DIRECT_7_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_8 - OTP Config Data Direct Access 8
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_8 :: OTP_CONFIG_DIRECT_8 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_8_OTP_CONFIG_DIRECT_8_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_8_OTP_CONFIG_DIRECT_8_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_8_OTP_CONFIG_DIRECT_8_BITS           32
#define OTP_OTP_CONFIG_DIRECT_8_OTP_CONFIG_DIRECT_8_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_9 - OTP Config Data Direct Access 9
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_9 :: OTP_CONFIG_DIRECT_9 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_9_OTP_CONFIG_DIRECT_9_MASK           0xffffffff
#define OTP_OTP_CONFIG_DIRECT_9_OTP_CONFIG_DIRECT_9_ALIGN          0
#define OTP_OTP_CONFIG_DIRECT_9_OTP_CONFIG_DIRECT_9_BITS           32
#define OTP_OTP_CONFIG_DIRECT_9_OTP_CONFIG_DIRECT_9_SHIFT          0

/***************************************************************************
 *OTP_CONFIG_DIRECT_10 - OTP Config Data Direct Access 10
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_10 :: OTP_CONFIG_DIRECT_10 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_10_OTP_CONFIG_DIRECT_10_MASK         0xffffffff
#define OTP_OTP_CONFIG_DIRECT_10_OTP_CONFIG_DIRECT_10_ALIGN        0
#define OTP_OTP_CONFIG_DIRECT_10_OTP_CONFIG_DIRECT_10_BITS         32
#define OTP_OTP_CONFIG_DIRECT_10_OTP_CONFIG_DIRECT_10_SHIFT        0

/***************************************************************************
 *OTP_CONFIG_DIRECT_11 - OTP Config Data Direct Access 11
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_11 :: OTP_CONFIG_DIRECT_11 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_11_OTP_CONFIG_DIRECT_11_MASK         0xffffffff
#define OTP_OTP_CONFIG_DIRECT_11_OTP_CONFIG_DIRECT_11_ALIGN        0
#define OTP_OTP_CONFIG_DIRECT_11_OTP_CONFIG_DIRECT_11_BITS         32
#define OTP_OTP_CONFIG_DIRECT_11_OTP_CONFIG_DIRECT_11_SHIFT        0

/***************************************************************************
 *OTP_CONFIG_DIRECT_12 - OTP Config Data Direct Access 12
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_12 :: OTP_CONFIG_DIRECT_12 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_12_OTP_CONFIG_DIRECT_12_MASK         0xffffffff
#define OTP_OTP_CONFIG_DIRECT_12_OTP_CONFIG_DIRECT_12_ALIGN        0
#define OTP_OTP_CONFIG_DIRECT_12_OTP_CONFIG_DIRECT_12_BITS         32
#define OTP_OTP_CONFIG_DIRECT_12_OTP_CONFIG_DIRECT_12_SHIFT        0

/***************************************************************************
 *OTP_CONFIG_DIRECT_13 - OTP Config Data Direct Access 13
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_13 :: OTP_CONFIG_DIRECT_13 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_13_OTP_CONFIG_DIRECT_13_MASK         0xffffffff
#define OTP_OTP_CONFIG_DIRECT_13_OTP_CONFIG_DIRECT_13_ALIGN        0
#define OTP_OTP_CONFIG_DIRECT_13_OTP_CONFIG_DIRECT_13_BITS         32
#define OTP_OTP_CONFIG_DIRECT_13_OTP_CONFIG_DIRECT_13_SHIFT        0

/***************************************************************************
 *OTP_CONFIG_DIRECT_14 - OTP Config Data Direct Access 14
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_14 :: OTP_CONFIG_DIRECT_14 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_14_OTP_CONFIG_DIRECT_14_MASK         0xffffffff
#define OTP_OTP_CONFIG_DIRECT_14_OTP_CONFIG_DIRECT_14_ALIGN        0
#define OTP_OTP_CONFIG_DIRECT_14_OTP_CONFIG_DIRECT_14_BITS         32
#define OTP_OTP_CONFIG_DIRECT_14_OTP_CONFIG_DIRECT_14_SHIFT        0

/***************************************************************************
 *OTP_CONFIG_DIRECT_15 - OTP Config Data Direct Access 15
 ***************************************************************************/
/* OTP :: OTP_CONFIG_DIRECT_15 :: OTP_CONFIG_DIRECT_15 [31:00] */
#define OTP_OTP_CONFIG_DIRECT_15_OTP_CONFIG_DIRECT_15_MASK         0xffffffff
#define OTP_OTP_CONFIG_DIRECT_15_OTP_CONFIG_DIRECT_15_ALIGN        0
#define OTP_OTP_CONFIG_DIRECT_15_OTP_CONFIG_DIRECT_15_BITS         32
#define OTP_OTP_CONFIG_DIRECT_15_OTP_CONFIG_DIRECT_15_SHIFT        0

#endif /* #ifndef OTP_H__ */

/* End of File */
