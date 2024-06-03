/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
 ------------------------------------------------------------------------- */
#ifndef PIN_MUX_H__
#define PIN_MUX_H__

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
#define PIN_MUX_PM_CONFIG                        0x00000000 /* Configuration for PM pin */
#define PIN_MUX_RX_SD_CONFIG                     0x00000004 /* Configuration for RX_SD pin */
#define PIN_MUX_BSC_SCL_CONFIG                   0x00000008 /* Configuration for BSC_SCL pin */
#define PIN_MUX_BSC_SDA_CONFIG                   0x0000000c /* Configuration for BSC_SDA pin */
#define PIN_MUX_PM_VALUE                         0x00000010 /* PM pin value when PM pin is configured as GPIO */
#define PIN_MUX_RXSD_VALUE                       0x00000014 /* RXSD pin value when RXSD pin is configured as GPIO */
#define PIN_MUX_UART_TX_CONFIG                   0x00000018 /* UART_TX pin configuration. Use to configure the UART_TX dedicated pad. */
#define PIN_MUX_UART_RX_CONFIG                   0x0000001c /* UART_RX pin configuration. Use to configure the UART_RX dedicated pad. */
#define PIN_MUX_RING_OSC_UL_CONFIG               0x00000020 /* Controls the upper-left ring oscillator IO cell */
#define PIN_MUX_RING_OSC_LR_CONFIG               0x00000024 /* Controls the lower-right ring oscillator IO cell */
#define PIN_MUX_DEBUG_CONFIG                     0x00000028 /* Configures the Debug Mux functionality */
#define PIN_MUX_PM_DEBUG_OUT                     0x0000002c /* DRS readback of pm_debug_out */
#define PIN_MUX_RXSD_DEBUG_OUT                   0x00000030 /* DRS readback of rxsd_debug_out */
#define PIN_MUX_RING_OSC_UL_CNT                  0x00000034 /* Frequency counter for RING OSC UL */
#define PIN_MUX_RING_OSC_LR_CNT                  0x00000038 /* Frequency counter for RING OSC LR */

/***************************************************************************
 *PM_CONFIG - Configuration for PM pin
 ***************************************************************************/
/* PIN_MUX :: PM_CONFIG :: reserved0 [31:04] */
#define PIN_MUX_PM_CONFIG_reserved0_MASK                           0xfffffff0
#define PIN_MUX_PM_CONFIG_reserved0_ALIGN                          0
#define PIN_MUX_PM_CONFIG_reserved0_BITS                           28
#define PIN_MUX_PM_CONFIG_reserved0_SHIFT                          4

/* PIN_MUX :: PM_CONFIG :: PM_MODE_CONFIG [03:00] */
#define PIN_MUX_PM_CONFIG_PM_MODE_CONFIG_MASK                      0x0000000f
#define PIN_MUX_PM_CONFIG_PM_MODE_CONFIG_ALIGN                     0
#define PIN_MUX_PM_CONFIG_PM_MODE_CONFIG_BITS                      4
#define PIN_MUX_PM_CONFIG_PM_MODE_CONFIG_SHIFT                     0
#define PIN_MUX_PM_CONFIG_PM_MODE_CONFIG_DEFAULT                   0

/***************************************************************************
 *RX_SD_CONFIG - Configuration for RX_SD pin
 ***************************************************************************/
/* PIN_MUX :: RX_SD_CONFIG :: reserved0 [31:04] */
#define PIN_MUX_RX_SD_CONFIG_reserved0_MASK                        0xfffffff0
#define PIN_MUX_RX_SD_CONFIG_reserved0_ALIGN                       0
#define PIN_MUX_RX_SD_CONFIG_reserved0_BITS                        28
#define PIN_MUX_RX_SD_CONFIG_reserved0_SHIFT                       4

/* PIN_MUX :: RX_SD_CONFIG :: RX_SD_CONFIG [03:00] */
#define PIN_MUX_RX_SD_CONFIG_RX_SD_CONFIG_MASK                     0x0000000f
#define PIN_MUX_RX_SD_CONFIG_RX_SD_CONFIG_ALIGN                    0
#define PIN_MUX_RX_SD_CONFIG_RX_SD_CONFIG_BITS                     4
#define PIN_MUX_RX_SD_CONFIG_RX_SD_CONFIG_SHIFT                    0
#define PIN_MUX_RX_SD_CONFIG_RX_SD_CONFIG_DEFAULT                  0

/***************************************************************************
 *BSC_SCL_CONFIG - Configuration for BSC_SCL pin
 ***************************************************************************/
/* PIN_MUX :: BSC_SCL_CONFIG :: reserved0 [31:03] */
#define PIN_MUX_BSC_SCL_CONFIG_reserved0_MASK                      0xfffffff8
#define PIN_MUX_BSC_SCL_CONFIG_reserved0_ALIGN                     0
#define PIN_MUX_BSC_SCL_CONFIG_reserved0_BITS                      29
#define PIN_MUX_BSC_SCL_CONFIG_reserved0_SHIFT                     3

/* PIN_MUX :: BSC_SCL_CONFIG :: BSC_SCL_CONFIG [02:00] */
#define PIN_MUX_BSC_SCL_CONFIG_BSC_SCL_CONFIG_MASK                 0x00000007
#define PIN_MUX_BSC_SCL_CONFIG_BSC_SCL_CONFIG_ALIGN                0
#define PIN_MUX_BSC_SCL_CONFIG_BSC_SCL_CONFIG_BITS                 3
#define PIN_MUX_BSC_SCL_CONFIG_BSC_SCL_CONFIG_SHIFT                0
#define PIN_MUX_BSC_SCL_CONFIG_BSC_SCL_CONFIG_DEFAULT              0

/***************************************************************************
 *BSC_SDA_CONFIG - Configuration for BSC_SDA pin
 ***************************************************************************/
/* PIN_MUX :: BSC_SDA_CONFIG :: reserved0 [31:03] */
#define PIN_MUX_BSC_SDA_CONFIG_reserved0_MASK                      0xfffffff8
#define PIN_MUX_BSC_SDA_CONFIG_reserved0_ALIGN                     0
#define PIN_MUX_BSC_SDA_CONFIG_reserved0_BITS                      29
#define PIN_MUX_BSC_SDA_CONFIG_reserved0_SHIFT                     3

/* PIN_MUX :: BSC_SDA_CONFIG :: BSC_SDA_CONFIG [02:00] */
#define PIN_MUX_BSC_SDA_CONFIG_BSC_SDA_CONFIG_MASK                 0x00000007
#define PIN_MUX_BSC_SDA_CONFIG_BSC_SDA_CONFIG_ALIGN                0
#define PIN_MUX_BSC_SDA_CONFIG_BSC_SDA_CONFIG_BITS                 3
#define PIN_MUX_BSC_SDA_CONFIG_BSC_SDA_CONFIG_SHIFT                0
#define PIN_MUX_BSC_SDA_CONFIG_BSC_SDA_CONFIG_DEFAULT              0

/***************************************************************************
 *PM_VALUE - PM pin value when PM pin is configured as GPIO
 ***************************************************************************/
/* PIN_MUX :: PM_VALUE :: reserved0 [31:01] */
#define PIN_MUX_PM_VALUE_reserved0_MASK                            0xfffffffe
#define PIN_MUX_PM_VALUE_reserved0_ALIGN                           0
#define PIN_MUX_PM_VALUE_reserved0_BITS                            31
#define PIN_MUX_PM_VALUE_reserved0_SHIFT                           1

/* PIN_MUX :: PM_VALUE :: PM_VALUE [00:00] */
#define PIN_MUX_PM_VALUE_PM_VALUE_MASK                             0x00000001
#define PIN_MUX_PM_VALUE_PM_VALUE_ALIGN                            0
#define PIN_MUX_PM_VALUE_PM_VALUE_BITS                             1
#define PIN_MUX_PM_VALUE_PM_VALUE_SHIFT                            0
#define PIN_MUX_PM_VALUE_PM_VALUE_DEFAULT                          0

/***************************************************************************
 *RXSD_VALUE - RXSD pin value when RXSD pin is configured as GPIO
 ***************************************************************************/
/* PIN_MUX :: RXSD_VALUE :: reserved0 [31:01] */
#define PIN_MUX_RXSD_VALUE_reserved0_MASK                          0xfffffffe
#define PIN_MUX_RXSD_VALUE_reserved0_ALIGN                         0
#define PIN_MUX_RXSD_VALUE_reserved0_BITS                          31
#define PIN_MUX_RXSD_VALUE_reserved0_SHIFT                         1

/* PIN_MUX :: RXSD_VALUE :: RXSD_VALUE [00:00] */
#define PIN_MUX_RXSD_VALUE_RXSD_VALUE_MASK                         0x00000001
#define PIN_MUX_RXSD_VALUE_RXSD_VALUE_ALIGN                        0
#define PIN_MUX_RXSD_VALUE_RXSD_VALUE_BITS                         1
#define PIN_MUX_RXSD_VALUE_RXSD_VALUE_SHIFT                        0
#define PIN_MUX_RXSD_VALUE_RXSD_VALUE_DEFAULT                      0

/***************************************************************************
 *UART_TX_CONFIG - UART_TX pin configuration. Use to configure the UART_TX dedicated pad.
 ***************************************************************************/
/* PIN_MUX :: UART_TX_CONFIG :: reserved0 [31:09] */
#define PIN_MUX_UART_TX_CONFIG_reserved0_MASK                      0xfffffe00
#define PIN_MUX_UART_TX_CONFIG_reserved0_ALIGN                     0
#define PIN_MUX_UART_TX_CONFIG_reserved0_BITS                      23
#define PIN_MUX_UART_TX_CONFIG_reserved0_SHIFT                     9

/* PIN_MUX :: UART_TX_CONFIG :: UART_TX_DIN [08:08] */
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DIN_MASK                    0x00000100
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DIN_ALIGN                   0
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DIN_BITS                    1
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DIN_SHIFT                   8
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DIN_DEFAULT                 0

/* PIN_MUX :: UART_TX_CONFIG :: reserved1 [07:06] */
#define PIN_MUX_UART_TX_CONFIG_reserved1_MASK                      0x000000c0
#define PIN_MUX_UART_TX_CONFIG_reserved1_ALIGN                     0
#define PIN_MUX_UART_TX_CONFIG_reserved1_BITS                      2
#define PIN_MUX_UART_TX_CONFIG_reserved1_SHIFT                     6

/* PIN_MUX :: UART_TX_CONFIG :: UART_TX_DOUT_EN [05:05] */
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_EN_MASK                0x00000020
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_EN_ALIGN               0
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_EN_BITS                1
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_EN_SHIFT               5
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_EN_DEFAULT             0

/* PIN_MUX :: UART_TX_CONFIG :: UART_TX_DOUT [04:04] */
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_MASK                   0x00000010
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_ALIGN                  0
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_BITS                   1
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_SHIFT                  4
#define PIN_MUX_UART_TX_CONFIG_UART_TX_DOUT_DEFAULT                0

/* PIN_MUX :: UART_TX_CONFIG :: reserved2 [03:01] */
#define PIN_MUX_UART_TX_CONFIG_reserved2_MASK                      0x0000000e
#define PIN_MUX_UART_TX_CONFIG_reserved2_ALIGN                     0
#define PIN_MUX_UART_TX_CONFIG_reserved2_BITS                      3
#define PIN_MUX_UART_TX_CONFIG_reserved2_SHIFT                     1

/* PIN_MUX :: UART_TX_CONFIG :: UART_TX_OEB [00:00] */
#define PIN_MUX_UART_TX_CONFIG_UART_TX_OEB_MASK                    0x00000001
#define PIN_MUX_UART_TX_CONFIG_UART_TX_OEB_ALIGN                   0
#define PIN_MUX_UART_TX_CONFIG_UART_TX_OEB_BITS                    1
#define PIN_MUX_UART_TX_CONFIG_UART_TX_OEB_SHIFT                   0
#define PIN_MUX_UART_TX_CONFIG_UART_TX_OEB_DEFAULT                 1

/***************************************************************************
 *UART_RX_CONFIG - UART_RX pin configuration. Use to configure the UART_RX dedicated pad.
 ***************************************************************************/
/* PIN_MUX :: UART_RX_CONFIG :: reserved0 [31:09] */
#define PIN_MUX_UART_RX_CONFIG_reserved0_MASK                      0xfffffe00
#define PIN_MUX_UART_RX_CONFIG_reserved0_ALIGN                     0
#define PIN_MUX_UART_RX_CONFIG_reserved0_BITS                      23
#define PIN_MUX_UART_RX_CONFIG_reserved0_SHIFT                     9

/* PIN_MUX :: UART_RX_CONFIG :: UART_RX_DIN [08:08] */
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DIN_MASK                    0x00000100
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DIN_ALIGN                   0
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DIN_BITS                    1
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DIN_SHIFT                   8
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DIN_DEFAULT                 0

/* PIN_MUX :: UART_RX_CONFIG :: reserved1 [07:05] */
#define PIN_MUX_UART_RX_CONFIG_reserved1_MASK                      0x000000e0
#define PIN_MUX_UART_RX_CONFIG_reserved1_ALIGN                     0
#define PIN_MUX_UART_RX_CONFIG_reserved1_BITS                      3
#define PIN_MUX_UART_RX_CONFIG_reserved1_SHIFT                     5

/* PIN_MUX :: UART_RX_CONFIG :: UART_RX_DOUT [04:04] */
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DOUT_MASK                   0x00000010
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DOUT_ALIGN                  0
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DOUT_BITS                   1
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DOUT_SHIFT                  4
#define PIN_MUX_UART_RX_CONFIG_UART_RX_DOUT_DEFAULT                0

/* PIN_MUX :: UART_RX_CONFIG :: reserved2 [03:01] */
#define PIN_MUX_UART_RX_CONFIG_reserved2_MASK                      0x0000000e
#define PIN_MUX_UART_RX_CONFIG_reserved2_ALIGN                     0
#define PIN_MUX_UART_RX_CONFIG_reserved2_BITS                      3
#define PIN_MUX_UART_RX_CONFIG_reserved2_SHIFT                     1

/* PIN_MUX :: UART_RX_CONFIG :: UART_RX_OEB [00:00] */
#define PIN_MUX_UART_RX_CONFIG_UART_RX_OEB_MASK                    0x00000001
#define PIN_MUX_UART_RX_CONFIG_UART_RX_OEB_ALIGN                   0
#define PIN_MUX_UART_RX_CONFIG_UART_RX_OEB_BITS                    1
#define PIN_MUX_UART_RX_CONFIG_UART_RX_OEB_SHIFT                   0
#define PIN_MUX_UART_RX_CONFIG_UART_RX_OEB_DEFAULT                 1

/***************************************************************************
 *RING_OSC_UL_CONFIG - Controls the upper-left ring oscillator IO cell
 ***************************************************************************/
/* PIN_MUX :: RING_OSC_UL_CONFIG :: reserved0 [31:06] */
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved0_MASK                  0xffffffc0
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved0_ALIGN                 0
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved0_BITS                  26
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved0_SHIFT                 6

/* PIN_MUX :: RING_OSC_UL_CONFIG :: RING_OSC_UL_SEL [05:04] */
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_SEL_MASK            0x00000030
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_SEL_ALIGN           0
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_SEL_BITS            2
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_SEL_SHIFT           4
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_SEL_DEFAULT         0

/* PIN_MUX :: RING_OSC_UL_CONFIG :: reserved1 [03:01] */
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved1_MASK                  0x0000000e
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved1_ALIGN                 0
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved1_BITS                  3
#define PIN_MUX_RING_OSC_UL_CONFIG_reserved1_SHIFT                 1

/* PIN_MUX :: RING_OSC_UL_CONFIG :: RING_OSC_UL_EN [00:00] */
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_EN_MASK             0x00000001
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_EN_ALIGN            0
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_EN_BITS             1
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_EN_SHIFT            0
#define PIN_MUX_RING_OSC_UL_CONFIG_RING_OSC_UL_EN_DEFAULT          0

/***************************************************************************
 *RING_OSC_LR_CONFIG - Controls the lower-right ring oscillator IO cell
 ***************************************************************************/
/* PIN_MUX :: RING_OSC_LR_CONFIG :: reserved0 [31:06] */
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved0_MASK                  0xffffffc0
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved0_ALIGN                 0
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved0_BITS                  26
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved0_SHIFT                 6

/* PIN_MUX :: RING_OSC_LR_CONFIG :: RING_OSC_LR_SEL [05:04] */
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_SEL_MASK            0x00000030
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_SEL_ALIGN           0
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_SEL_BITS            2
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_SEL_SHIFT           4
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_SEL_DEFAULT         0

/* PIN_MUX :: RING_OSC_LR_CONFIG :: reserved1 [03:01] */
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved1_MASK                  0x0000000e
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved1_ALIGN                 0
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved1_BITS                  3
#define PIN_MUX_RING_OSC_LR_CONFIG_reserved1_SHIFT                 1

/* PIN_MUX :: RING_OSC_LR_CONFIG :: RING_OSC_LR_EN [00:00] */
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_EN_MASK             0x00000001
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_EN_ALIGN            0
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_EN_BITS             1
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_EN_SHIFT            0
#define PIN_MUX_RING_OSC_LR_CONFIG_RING_OSC_LR_EN_DEFAULT          0

/***************************************************************************
 *DEBUG_CONFIG - Configures the Debug Mux functionality
 ***************************************************************************/
/* PIN_MUX :: DEBUG_CONFIG :: reserved0 [31:07] */
#define PIN_MUX_DEBUG_CONFIG_reserved0_MASK                        0xffffff80
#define PIN_MUX_DEBUG_CONFIG_reserved0_ALIGN                       0
#define PIN_MUX_DEBUG_CONFIG_reserved0_BITS                        25
#define PIN_MUX_DEBUG_CONFIG_reserved0_SHIFT                       7

/* PIN_MUX :: DEBUG_CONFIG :: DEBUG_MUX_RXSD_SELECT [06:04] */
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_RXSD_SELECT_MASK            0x00000070
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_RXSD_SELECT_ALIGN           0
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_RXSD_SELECT_BITS            3
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_RXSD_SELECT_SHIFT           4
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_RXSD_SELECT_DEFAULT         0

/* PIN_MUX :: DEBUG_CONFIG :: reserved1 [03:03] */
#define PIN_MUX_DEBUG_CONFIG_reserved1_MASK                        0x00000008
#define PIN_MUX_DEBUG_CONFIG_reserved1_ALIGN                       0
#define PIN_MUX_DEBUG_CONFIG_reserved1_BITS                        1
#define PIN_MUX_DEBUG_CONFIG_reserved1_SHIFT                       3

/* PIN_MUX :: DEBUG_CONFIG :: DEBUG_MUX_PM_SELECT [02:00] */
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_PM_SELECT_MASK              0x00000007
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_PM_SELECT_ALIGN             0
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_PM_SELECT_BITS              3
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_PM_SELECT_SHIFT             0
#define PIN_MUX_DEBUG_CONFIG_DEBUG_MUX_PM_SELECT_DEFAULT           0

/***************************************************************************
 *PM_DEBUG_OUT - DRS readback of pm_debug_out
 ***************************************************************************/
/* PIN_MUX :: PM_DEBUG_OUT :: reserved0 [31:01] */
#define PIN_MUX_PM_DEBUG_OUT_reserved0_MASK                        0xfffffffe
#define PIN_MUX_PM_DEBUG_OUT_reserved0_ALIGN                       0
#define PIN_MUX_PM_DEBUG_OUT_reserved0_BITS                        31
#define PIN_MUX_PM_DEBUG_OUT_reserved0_SHIFT                       1

/* PIN_MUX :: PM_DEBUG_OUT :: PM_DEBUG_OUT [00:00] */
#define PIN_MUX_PM_DEBUG_OUT_PM_DEBUG_OUT_MASK                     0x00000001
#define PIN_MUX_PM_DEBUG_OUT_PM_DEBUG_OUT_ALIGN                    0
#define PIN_MUX_PM_DEBUG_OUT_PM_DEBUG_OUT_BITS                     1
#define PIN_MUX_PM_DEBUG_OUT_PM_DEBUG_OUT_SHIFT                    0

/***************************************************************************
 *RXSD_DEBUG_OUT - DRS readback of rxsd_debug_out
 ***************************************************************************/
/* PIN_MUX :: RXSD_DEBUG_OUT :: reserved0 [31:01] */
#define PIN_MUX_RXSD_DEBUG_OUT_reserved0_MASK                      0xfffffffe
#define PIN_MUX_RXSD_DEBUG_OUT_reserved0_ALIGN                     0
#define PIN_MUX_RXSD_DEBUG_OUT_reserved0_BITS                      31
#define PIN_MUX_RXSD_DEBUG_OUT_reserved0_SHIFT                     1

/* PIN_MUX :: RXSD_DEBUG_OUT :: RXSD_DEBUG_OUT [00:00] */
#define PIN_MUX_RXSD_DEBUG_OUT_RXSD_DEBUG_OUT_MASK                 0x00000001
#define PIN_MUX_RXSD_DEBUG_OUT_RXSD_DEBUG_OUT_ALIGN                0
#define PIN_MUX_RXSD_DEBUG_OUT_RXSD_DEBUG_OUT_BITS                 1
#define PIN_MUX_RXSD_DEBUG_OUT_RXSD_DEBUG_OUT_SHIFT                0

/***************************************************************************
 *RING_OSC_UL_CNT - Frequency counter for RING OSC UL
 ***************************************************************************/
/* PIN_MUX :: RING_OSC_UL_CNT :: reserved0 [31:14] */
#define PIN_MUX_RING_OSC_UL_CNT_reserved0_MASK                     0xffffc000
#define PIN_MUX_RING_OSC_UL_CNT_reserved0_ALIGN                    0
#define PIN_MUX_RING_OSC_UL_CNT_reserved0_BITS                     18
#define PIN_MUX_RING_OSC_UL_CNT_reserved0_SHIFT                    14

/* PIN_MUX :: RING_OSC_UL_CNT :: RING_OSC_UL_CNT [13:00] */
#define PIN_MUX_RING_OSC_UL_CNT_RING_OSC_UL_CNT_MASK               0x00003fff
#define PIN_MUX_RING_OSC_UL_CNT_RING_OSC_UL_CNT_ALIGN              0
#define PIN_MUX_RING_OSC_UL_CNT_RING_OSC_UL_CNT_BITS               14
#define PIN_MUX_RING_OSC_UL_CNT_RING_OSC_UL_CNT_SHIFT              0

/***************************************************************************
 *RING_OSC_LR_CNT - Frequency counter for RING OSC LR
 ***************************************************************************/
/* PIN_MUX :: RING_OSC_LR_CNT :: reserved0 [31:14] */
#define PIN_MUX_RING_OSC_LR_CNT_reserved0_MASK                     0xffffc000
#define PIN_MUX_RING_OSC_LR_CNT_reserved0_ALIGN                    0
#define PIN_MUX_RING_OSC_LR_CNT_reserved0_BITS                     18
#define PIN_MUX_RING_OSC_LR_CNT_reserved0_SHIFT                    14

/* PIN_MUX :: RING_OSC_LR_CNT :: RING_OSC_LR_CNT [13:00] */
#define PIN_MUX_RING_OSC_LR_CNT_RING_OSC_LR_CNT_MASK               0x00003fff
#define PIN_MUX_RING_OSC_LR_CNT_RING_OSC_LR_CNT_ALIGN              0
#define PIN_MUX_RING_OSC_LR_CNT_RING_OSC_LR_CNT_BITS               14
#define PIN_MUX_RING_OSC_LR_CNT_RING_OSC_LR_CNT_SHIFT              0

#endif /* #ifndef PIN_MUX_H__ */

/* End of File */
