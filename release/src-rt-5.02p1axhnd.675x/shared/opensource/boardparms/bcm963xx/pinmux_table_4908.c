/*
    Copyright 2000-2018 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
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

#include "bp_defs.h"
#include "boardparms.h"




bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_xMII | 1, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 0, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 9, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 11, 48 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_xMII | 11, 49 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_xMII | 11, 68 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 69 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 70 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 71 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 72 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 73 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 74 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 75 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 76 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 77 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 78 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 79 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 81 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 82 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 83 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 35 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 36 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 45 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_EMMC, 46 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_EMMC, 47 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 27 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 28 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 29 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 30 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 63 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 64 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 66 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 67 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_PCM, 14 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 15 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 16 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 17 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_HS_SPI, 52 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 54 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 53 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 56 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 23 | BP_PINMUX_VAL_2 },
};

bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usSpeedLed100, 0, 0, 0 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed1000, 0, 1, 1 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 1, 2, 2 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed1000, 1, 3, 3 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 2, 4, 4 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed1000, 2, 5, 5 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 3, 6, 6 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed1000, 3, 7, 7 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 4, 8, 8 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed1000, 4, 9, 9 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 3, 10, 10 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, //WAN port
  { bp_usSpeedLed100, 3, 8, 8 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10) },//WAN port
  { bp_usSpeedLed1000, 3, 11, 11 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },//WAN port
  { bp_usSpeedLed1000, 3, 9, 9 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11) },//WAN port
  { bp_usSpeedLed100, 3, 22, 22 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },//WAN port
  { bp_usSpeedLed1000, 3, 23, 23 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },//WAN port
  { bp_usLinkLed, 0, 16, 16|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 0, 26, 26|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 1, 17, 17|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 1, 27, 27|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 2, 18, 18|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 2, 28, 28|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 19, 19|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 29, 29|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 4, 20, 20|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 4, 30, 30|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 21, 21|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },//WAN port
  { bp_usLinkLed, 3, 31, 31|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },//WAN port
  { bp_usLinkLed, 3, 20, 20|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21) },//WAN port
  { bp_usLinkLed, 3, 30, 30|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(31) },//WAN port
  { bp_usGpioLedAggregateLnk, -1, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usGpioLedAggregateLnk, -1, 0, 0|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_usGpioLedAggregateAct, -1, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usGpioLedAggregateAct, -1, 1, 1|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_usGpioLedAggregateLnk, -1, 24, 24|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usGpioLedAggregateAct, -1, 25, 25|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usMiiMdc, -1, 48, 48 | BP_PINMUX_VAL_0},
  { bp_usMiiMdio, -1, 49, 49 | BP_PINMUX_VAL_0},
  { bp_usGpioLedPwmReserved, -1, 14, 14 | BP_PINMUX_VAL_3 | BP_PINMUX_PWMLED },
  { bp_usGpioLedPwmReserved, -1, 15, 15 | BP_PINMUX_VAL_3 | BP_PINMUX_PWMLED },
  { bp_ReservedAnyGpio, -1, -1, BP_PINMUX_VAL_4 },
  { bp_ReservedAnyLed, -1, -1, BP_PINMUX_VAL_3 },
  { bp_usSerialLedData, -1, 0, 0 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 1, 1 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedMask, -1, 2, 2 | BP_PINMUX_VAL_1 },
  { bp_usGpioUart2Cts, -1, 10, 10 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Rts, -1, 11, 11 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdin, -1, 12, 12 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdout, -1, 13, 13 | BP_PINMUX_VAL_0},
  { bp_usUsbPwrFlt0, -1, 63, 63 | BP_PINMUX_VAL_0 },
  { bp_usUsbPwrOn0, -1, 64, 64 | BP_PINMUX_VAL_0 },
  { bp_usUsbPwrFlt1, -1, 66, 66 | BP_PINMUX_VAL_0 },
  { bp_usUsbPwrOn1, -1, 67, 67 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda, -1, 18, 18 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, -1, 19, 19 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda, -1, 22, 22 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, -1, 23, 23 | BP_PINMUX_VAL_0 },
  { bp_usSpiSlaveSelectNum, 1, 56, 56 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 2, 23, 23 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 3, 22, 22 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 4, 21, 21 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 5, 20, 20 | BP_PINMUX_VAL_2},
  { bp_last, -1, -1, 0 },
};

bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
