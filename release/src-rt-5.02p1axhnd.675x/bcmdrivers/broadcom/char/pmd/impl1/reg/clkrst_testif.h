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

#ifndef CLKRST_TESTIF_H__
#define CLKRST_TESTIF_H__

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
#define CRT_SW_RESET_CONFIG                      0x00000000 /* Software controlled reset and reset configuration bits */
#define CRT_CLOCK_SOURCE_CONFIG                  0x00000004 /* Clock source configuration */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG           0x00000008 /* Digital block resets configuration */
#define CRT_ADC_CLOCK_COUNTER_CONFIG             0x0000000c /* ADC clock frequency counter configuration */
#define CRT_ADC_CLOCK_COUNTER_STATUS             0x00000010 /* ADC clock frequency counter status */
#define CRT_CPU_STATUS                           0x00000014 /* Contains CPU state indicators */
#define CRT_CPU_EGRESS_MAIL_BOX                  0x00000018 /* CPU-to-host mailbox register */
#define CRT_CPU_INGRESS_MAIL_BOX                 0x0000001c /* Host-to-CPU mailbox register */
#define CRT_REF_CLOCK_MONITOR_CONFIG             0x00000020 /* 25 MHz reference clock activity monitor configuation */
#define CRT_REF_CLOCK_MONITOR_STATUS             0x00000024 /* 25 MHz reference clock activity monitor status */
#define CRT_CRT_PARAM                            0x00000028 /* CRT Parameter Register */

/***************************************************************************
 *SW_RESET_CONFIG - Software controlled reset and reset configuration bits
 ***************************************************************************/
/* CRT :: SW_RESET_CONFIG :: reserved0 [31:08] */
#define CRT_SW_RESET_CONFIG_reserved0_MASK                         0xffffff00
#define CRT_SW_RESET_CONFIG_reserved0_ALIGN                        0
#define CRT_SW_RESET_CONFIG_reserved0_BITS                         24
#define CRT_SW_RESET_CONFIG_reserved0_SHIFT                        8

/* CRT :: SW_RESET_CONFIG :: CFG_CORE_JTAG_MODE_RST_N [07:07] */
#define CRT_SW_RESET_CONFIG_CFG_CORE_JTAG_MODE_RST_N_MASK          0x00000080
#define CRT_SW_RESET_CONFIG_CFG_CORE_JTAG_MODE_RST_N_ALIGN         0
#define CRT_SW_RESET_CONFIG_CFG_CORE_JTAG_MODE_RST_N_BITS          1
#define CRT_SW_RESET_CONFIG_CFG_CORE_JTAG_MODE_RST_N_SHIFT         7
#define CRT_SW_RESET_CONFIG_CFG_CORE_JTAG_MODE_RST_N_DEFAULT       0

/* CRT :: SW_RESET_CONFIG :: CFG_RST_ON_PLL_LOL [06:06] */
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_PLL_LOL_MASK                0x00000040
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_PLL_LOL_ALIGN               0
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_PLL_LOL_BITS                1
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_PLL_LOL_SHIFT               6
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_PLL_LOL_DEFAULT             0

/* CRT :: SW_RESET_CONFIG :: CFG_RST_ON_NO_REF [05:05] */
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_NO_REF_MASK                 0x00000020
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_NO_REF_ALIGN                0
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_NO_REF_BITS                 1
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_NO_REF_SHIFT                5
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_NO_REF_DEFAULT              0

/* CRT :: SW_RESET_CONFIG :: CFG_RST_ON_CPU_WD [04:04] */
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_CPU_WD_MASK                 0x00000010
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_CPU_WD_ALIGN                0
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_CPU_WD_BITS                 1
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_CPU_WD_SHIFT                4
#define CRT_SW_RESET_CONFIG_CFG_RST_ON_CPU_WD_DEFAULT              0

/* CRT :: SW_RESET_CONFIG :: reserved1 [03:01] */
#define CRT_SW_RESET_CONFIG_reserved1_MASK                         0x0000000e
#define CRT_SW_RESET_CONFIG_reserved1_ALIGN                        0
#define CRT_SW_RESET_CONFIG_reserved1_BITS                         3
#define CRT_SW_RESET_CONFIG_reserved1_SHIFT                        1

/* CRT :: SW_RESET_CONFIG :: CFG_SW_RESET [00:00] */
#define CRT_SW_RESET_CONFIG_CFG_SW_RESET_MASK                      0x00000001
#define CRT_SW_RESET_CONFIG_CFG_SW_RESET_ALIGN                     0
#define CRT_SW_RESET_CONFIG_CFG_SW_RESET_BITS                      1
#define CRT_SW_RESET_CONFIG_CFG_SW_RESET_SHIFT                     0
#define CRT_SW_RESET_CONFIG_CFG_SW_RESET_DEFAULT                   0

/***************************************************************************
 *CLOCK_SOURCE_CONFIG - Clock source configuration
 ***************************************************************************/
/* CRT :: CLOCK_SOURCE_CONFIG :: reserved0 [31:02] */
#define CRT_CLOCK_SOURCE_CONFIG_reserved0_MASK                     0xfffffffc
#define CRT_CLOCK_SOURCE_CONFIG_reserved0_ALIGN                    0
#define CRT_CLOCK_SOURCE_CONFIG_reserved0_BITS                     30
#define CRT_CLOCK_SOURCE_CONFIG_reserved0_SHIFT                    2

/* CRT :: CLOCK_SOURCE_CONFIG :: CFG_BSL_DB_CLOCK_SOURCE [01:01] */
#define CRT_CLOCK_SOURCE_CONFIG_CFG_BSL_DB_CLOCK_SOURCE_MASK       0x00000002
#define CRT_CLOCK_SOURCE_CONFIG_CFG_BSL_DB_CLOCK_SOURCE_ALIGN      0
#define CRT_CLOCK_SOURCE_CONFIG_CFG_BSL_DB_CLOCK_SOURCE_BITS       1
#define CRT_CLOCK_SOURCE_CONFIG_CFG_BSL_DB_CLOCK_SOURCE_SHIFT      1
#define CRT_CLOCK_SOURCE_CONFIG_CFG_BSL_DB_CLOCK_SOURCE_DEFAULT    0

/* CRT :: CLOCK_SOURCE_CONFIG :: CFG_CORE_CLOCK_SOURCE [00:00] */
#define CRT_CLOCK_SOURCE_CONFIG_CFG_CORE_CLOCK_SOURCE_MASK         0x00000001
#define CRT_CLOCK_SOURCE_CONFIG_CFG_CORE_CLOCK_SOURCE_ALIGN        0
#define CRT_CLOCK_SOURCE_CONFIG_CFG_CORE_CLOCK_SOURCE_BITS         1
#define CRT_CLOCK_SOURCE_CONFIG_CFG_CORE_CLOCK_SOURCE_SHIFT        0
#define CRT_CLOCK_SOURCE_CONFIG_CFG_CORE_CLOCK_SOURCE_DEFAULT      0

/***************************************************************************
 *DIGITAL_BLOCK_RESET_CONFIG - Digital block resets configuration
 ***************************************************************************/
/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: reserved0 [31:07] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_reserved0_MASK              0xffffff80
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_reserved0_ALIGN             0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_reserved0_BITS              25
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_reserved0_SHIFT             7

/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: CFG_DYN_MDIV_RST [06:06] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_DYN_MDIV_RST_MASK       0x00000040
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_DYN_MDIV_RST_ALIGN      0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_DYN_MDIV_RST_BITS       1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_DYN_MDIV_RST_SHIFT      6
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_DYN_MDIV_RST_DEFAULT    1

/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: CFG_LRL_RST [05:05] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LRL_RST_MASK            0x00000020
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LRL_RST_ALIGN           0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LRL_RST_BITS            1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LRL_RST_SHIFT           5
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LRL_RST_DEFAULT         1

/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: CFG_LDC_RST [04:04] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LDC_RST_MASK            0x00000010
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LDC_RST_ALIGN           0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LDC_RST_BITS            1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LDC_RST_SHIFT           4
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_LDC_RST_DEFAULT         1

/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: CFG_ESC_RST [03:03] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ESC_RST_MASK            0x00000008
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ESC_RST_ALIGN           0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ESC_RST_BITS            1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ESC_RST_SHIFT           3
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ESC_RST_DEFAULT         1

/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: CFG_ADF_RST [02:02] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ADF_RST_MASK            0x00000004
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ADF_RST_ALIGN           0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ADF_RST_BITS            1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ADF_RST_SHIFT           2
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_ADF_RST_DEFAULT         1

/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: CFG_CPU_RST [01:01] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_CPU_RST_MASK            0x00000002
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_CPU_RST_ALIGN           0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_CPU_RST_BITS            1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_CPU_RST_SHIFT           1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_CPU_RST_DEFAULT         1

/* CRT :: DIGITAL_BLOCK_RESET_CONFIG :: CFG_GEN_CLOCKS_RST [00:00] */
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_GEN_CLOCKS_RST_MASK     0x00000001
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_GEN_CLOCKS_RST_ALIGN    0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_GEN_CLOCKS_RST_BITS     1
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_GEN_CLOCKS_RST_SHIFT    0
#define CRT_DIGITAL_BLOCK_RESET_CONFIG_CFG_GEN_CLOCKS_RST_DEFAULT  1

/***************************************************************************
 *ADC_CLOCK_COUNTER_CONFIG - ADC clock frequency counter configuration
 ***************************************************************************/
/* CRT :: ADC_CLOCK_COUNTER_CONFIG :: CFG_ADC_CLKCNT_EN [31:31] */
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_EN_MASK        0x80000000
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_EN_ALIGN       0
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_EN_BITS        1
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_EN_SHIFT       31
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_EN_DEFAULT     0

/* CRT :: ADC_CLOCK_COUNTER_CONFIG :: reserved0 [30:12] */
#define CRT_ADC_CLOCK_COUNTER_CONFIG_reserved0_MASK                0x7ffff000
#define CRT_ADC_CLOCK_COUNTER_CONFIG_reserved0_ALIGN               0
#define CRT_ADC_CLOCK_COUNTER_CONFIG_reserved0_BITS                19
#define CRT_ADC_CLOCK_COUNTER_CONFIG_reserved0_SHIFT               12

/* CRT :: ADC_CLOCK_COUNTER_CONFIG :: CFG_ADC_CLKCNT_INTERVAL [11:00] */
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_INTERVAL_MASK  0x00000fff
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_INTERVAL_ALIGN 0
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_INTERVAL_BITS  12
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_INTERVAL_SHIFT 0
#define CRT_ADC_CLOCK_COUNTER_CONFIG_CFG_ADC_CLKCNT_INTERVAL_DEFAULT 4000

/***************************************************************************
 *ADC_CLOCK_COUNTER_STATUS - ADC clock frequency counter status
 ***************************************************************************/
/* CRT :: ADC_CLOCK_COUNTER_STATUS :: STAT_ADC_CLKCNT_DONE [31:31] */
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_DONE_MASK     0x80000000
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_DONE_ALIGN    0
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_DONE_BITS     1
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_DONE_SHIFT    31

/* CRT :: ADC_CLOCK_COUNTER_STATUS :: reserved0 [30:12] */
#define CRT_ADC_CLOCK_COUNTER_STATUS_reserved0_MASK                0x7ffff000
#define CRT_ADC_CLOCK_COUNTER_STATUS_reserved0_ALIGN               0
#define CRT_ADC_CLOCK_COUNTER_STATUS_reserved0_BITS                19
#define CRT_ADC_CLOCK_COUNTER_STATUS_reserved0_SHIFT               12

/* CRT :: ADC_CLOCK_COUNTER_STATUS :: STAT_ADC_CLKCNT_RESULT [11:00] */
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_RESULT_MASK   0x00000fff
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_RESULT_ALIGN  0
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_RESULT_BITS   12
#define CRT_ADC_CLOCK_COUNTER_STATUS_STAT_ADC_CLKCNT_RESULT_SHIFT  0

/***************************************************************************
 *CPU_STATUS - Contains CPU state indicators
 ***************************************************************************/
/* CRT :: CPU_STATUS :: reserved0 [31:02] */
#define CRT_CPU_STATUS_reserved0_MASK                              0xfffffffc
#define CRT_CPU_STATUS_reserved0_ALIGN                             0
#define CRT_CPU_STATUS_reserved0_BITS                              30
#define CRT_CPU_STATUS_reserved0_SHIFT                             2

/* CRT :: CPU_STATUS :: STAT_CPU_IDLE_MODE [01:01] */
#define CRT_CPU_STATUS_STAT_CPU_IDLE_MODE_MASK                     0x00000002
#define CRT_CPU_STATUS_STAT_CPU_IDLE_MODE_ALIGN                    0
#define CRT_CPU_STATUS_STAT_CPU_IDLE_MODE_BITS                     1
#define CRT_CPU_STATUS_STAT_CPU_IDLE_MODE_SHIFT                    1

/* CRT :: CPU_STATUS :: STAT_CPU_STOP_MODE [00:00] */
#define CRT_CPU_STATUS_STAT_CPU_STOP_MODE_MASK                     0x00000001
#define CRT_CPU_STATUS_STAT_CPU_STOP_MODE_ALIGN                    0
#define CRT_CPU_STATUS_STAT_CPU_STOP_MODE_BITS                     1
#define CRT_CPU_STATUS_STAT_CPU_STOP_MODE_SHIFT                    0

/***************************************************************************
 *CPU_EGRESS_MAIL_BOX - CPU-to-host mailbox register
 ***************************************************************************/
/* CRT :: CPU_EGRESS_MAIL_BOX :: CPU_HOST_MAIL_BOX [31:00] */
#define CRT_CPU_EGRESS_MAIL_BOX_CPU_HOST_MAIL_BOX_MASK             0xffffffff
#define CRT_CPU_EGRESS_MAIL_BOX_CPU_HOST_MAIL_BOX_ALIGN            0
#define CRT_CPU_EGRESS_MAIL_BOX_CPU_HOST_MAIL_BOX_BITS             32
#define CRT_CPU_EGRESS_MAIL_BOX_CPU_HOST_MAIL_BOX_SHIFT            0
#define CRT_CPU_EGRESS_MAIL_BOX_CPU_HOST_MAIL_BOX_DEFAULT          0

/***************************************************************************
 *CPU_INGRESS_MAIL_BOX - Host-to-CPU mailbox register
 ***************************************************************************/
/* CRT :: CPU_INGRESS_MAIL_BOX :: HOST_CPU_MAIL_BOX [31:00] */
#define CRT_CPU_INGRESS_MAIL_BOX_HOST_CPU_MAIL_BOX_MASK            0xffffffff
#define CRT_CPU_INGRESS_MAIL_BOX_HOST_CPU_MAIL_BOX_ALIGN           0
#define CRT_CPU_INGRESS_MAIL_BOX_HOST_CPU_MAIL_BOX_BITS            32
#define CRT_CPU_INGRESS_MAIL_BOX_HOST_CPU_MAIL_BOX_SHIFT           0
#define CRT_CPU_INGRESS_MAIL_BOX_HOST_CPU_MAIL_BOX_DEFAULT         0

/***************************************************************************
 *REF_CLOCK_MONITOR_CONFIG - 25 MHz reference clock activity monitor configuation
 ***************************************************************************/
/* CRT :: REF_CLOCK_MONITOR_CONFIG :: reserved0 [31:15] */
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved0_MASK                0xffff8000
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved0_ALIGN               0
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved0_BITS                17
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved0_SHIFT               15

/* CRT :: REF_CLOCK_MONITOR_CONFIG :: CFG_REFCLK_MON_NO_CLK_THRESH [14:08] */
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_NO_CLK_THRESH_MASK 0x00007f00
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_NO_CLK_THRESH_ALIGN 0
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_NO_CLK_THRESH_BITS 7
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_NO_CLK_THRESH_SHIFT 8
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_NO_CLK_THRESH_DEFAULT 3

/* CRT :: REF_CLOCK_MONITOR_CONFIG :: reserved1 [07:01] */
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved1_MASK                0x000000fe
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved1_ALIGN               0
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved1_BITS                7
#define CRT_REF_CLOCK_MONITOR_CONFIG_reserved1_SHIFT               1

/* CRT :: REF_CLOCK_MONITOR_CONFIG :: CFG_REFCLK_MON_EN [00:00] */
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_EN_MASK        0x00000001
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_EN_ALIGN       0
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_EN_BITS        1
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_EN_SHIFT       0
#define CRT_REF_CLOCK_MONITOR_CONFIG_CFG_REFCLK_MON_EN_DEFAULT     0

/***************************************************************************
 *REF_CLOCK_MONITOR_STATUS - 25 MHz reference clock activity monitor status
 ***************************************************************************/
/* CRT :: REF_CLOCK_MONITOR_STATUS :: reserved0 [31:07] */
#define CRT_REF_CLOCK_MONITOR_STATUS_reserved0_MASK                0xffffff80
#define CRT_REF_CLOCK_MONITOR_STATUS_reserved0_ALIGN               0
#define CRT_REF_CLOCK_MONITOR_STATUS_reserved0_BITS                25
#define CRT_REF_CLOCK_MONITOR_STATUS_reserved0_SHIFT               7

/* CRT :: REF_CLOCK_MONITOR_STATUS :: STAT_REFCLK_MON_RESULT [06:00] */
#define CRT_REF_CLOCK_MONITOR_STATUS_STAT_REFCLK_MON_RESULT_MASK   0x0000007f
#define CRT_REF_CLOCK_MONITOR_STATUS_STAT_REFCLK_MON_RESULT_ALIGN  0
#define CRT_REF_CLOCK_MONITOR_STATUS_STAT_REFCLK_MON_RESULT_BITS   7
#define CRT_REF_CLOCK_MONITOR_STATUS_STAT_REFCLK_MON_RESULT_SHIFT  0

/***************************************************************************
 *CRT_PARAM - CRT Parameter Register
 ***************************************************************************/
/* CRT :: CRT_PARAM :: reserved_for_eco0 [31:00] */
#define CRT_CRT_PARAM_reserved_for_eco0_MASK                       0xffffffff
#define CRT_CRT_PARAM_reserved_for_eco0_ALIGN                      0
#define CRT_CRT_PARAM_reserved_for_eco0_BITS                       32
#define CRT_CRT_PARAM_reserved_for_eco0_SHIFT                      0
#define CRT_CRT_PARAM_reserved_for_eco0_DEFAULT                    0

#endif /* #ifndef CLKRST_TESTIF_H__ */

/* End of File */
