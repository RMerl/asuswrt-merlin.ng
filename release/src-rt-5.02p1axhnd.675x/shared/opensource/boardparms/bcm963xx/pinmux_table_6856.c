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
  { BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 35 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 36 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 66 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 67 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 75 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 48 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 49 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 50 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 51 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 52 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 53 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 54 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 55 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 56 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 57 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 58 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 59 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
};

bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_ReservedAnyGpio, -1, -1, BP_PINMUX_VAL_4 },
  { bp_usNetLed0, 0, 48, 48 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 0, 49, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed0, 1, 50, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed1, 1, 51, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed0, 2, 52, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed1, 2, 53, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed0, 3, 54, 54 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 55, 55 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed0, 4, 56, 56 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed1, 4, 57, 57 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_usNetLed0, 0, 11, 11 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 0, 12, 12 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed0, 1, 13, 13 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed1, 1, 14, 14 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed0, 2, 15, 15 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed1, 2, 16, 16 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed0, 3, 17, 17 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 18, 18 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed0, 4, 0, 0 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed1, 4, 1, 1 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_ReservedAnyLed, -1, 26, 26|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 27, 27|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 28, 28|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 48, 48|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 49, 49|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 50, 50|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 53, 53|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 54, 54|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 55, 55|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 56, 56|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 57, 57|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 58, 58|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 59, 59|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 9, 9|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 82, 82|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 83, 83|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyLed, -1, 50, 50|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 11, 11|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 14, 14|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 15, 15|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 16, 16|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 17, 17|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 18, 18|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 0, 0|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 1, 1|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 2, 2|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 56, 56|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 57, 57|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 58, 58|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 59, 59|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_usUart1Sdin, -1, 24, 24 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout, -1, 25, 25 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdin, -1, 35, 35 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout, -1, 36, 36 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdin, -1, 55, 55 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout, -1, 54, 54 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdin, -1, 3, 3 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout, -1, 6, 6 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout, -1, 23, 23 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1, 2, 2 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdout,-1, 25, 25 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Cts, -1, 0, 0 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Rts, -1, 1, 1 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdin, -1, 35, 35 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdout,-1, 36, 36 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Cts, -1, 4, 4 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Rts, -1, 5, 5 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1, 82, 82 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdout,-1, 83, 83 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdin, -1, 7, 7 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdout,-1, 6, 6 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1, 3, 3 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 0, 68, 68 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 1, 81, 81 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2, 11, 11 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 2, 25, 25 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 3, 12, 12 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 3, 26, 26 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 4, 13, 14 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 4, 27, 27 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 5, 14, 14 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 5, 28, 28 | BP_PINMUX_VAL_0},
  { bp_usSerialLedData, -1, 26, 26 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedClk, -1, 27, 27 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedMask, -1, 28, 28 | BP_PINMUX_VAL_1 },
  { bp_usTsync1pps, -1, 9, 9 | BP_PINMUX_VAL_3 },
  { bp_usTsync1pps, -1, 24, 24 | BP_PINMUX_VAL_2 },
  { bp_usSimDat, -1, 52, 52 | BP_PINMUX_VAL_0 },
  { bp_usSimClk, -1, 53, 53 | BP_PINMUX_VAL_0 },
  { bp_usSimPresence, -1, 54, 54 | BP_PINMUX_VAL_0 },
  { bp_usSimVccEn, -1, 48, 48 | BP_PINMUX_VAL_0 },
  { bp_usSimVccVolSel, -1, 49, 49 | BP_PINMUX_VAL_0 },
  { bp_usSimRst, -1, 50, 50 | BP_PINMUX_VAL_0 },
  { bp_usSimVppEn, -1, 51, 51 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda, -1, 19, 19 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cScl, -1, 20, 20 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cSda, -1, 83, 83 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cScl, -1, 82, 82 | BP_PINMUX_VAL_1 },
  { bp_usI2sSdata, -1, 0, 0 | BP_PINMUX_VAL_1 },
  { bp_usI2sSclk, -1, 1, 1 | BP_PINMUX_VAL_1 },
  { bp_usI2sLrck, -1, 2, 2 | BP_PINMUX_VAL_1 },
  { bp_usRogueOnuEn, -1, 10, 10 | BP_PINMUX_VAL_2 },
  { bp_usRogueOnuEn, -1, 62, 62 | BP_PINMUX_VAL_0 },
  { bp_usPmdMACEwakeEn, -1, 24, 24 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0, -1, 77, 77 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt0, -1, 76, 76 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1, -1, 79, 79 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 78, 78 | BP_PINMUX_VAL_1 },
  { bp_usMiiMdc, -1, 60, 60 | BP_PINMUX_VAL_3 },
  { bp_usMiiMdio, -1, 61, 61 | BP_PINMUX_VAL_3 },
  { bp_usPonLbe, -1, 22, 22 | BP_PINMUX_VAL_1},
  { bp_last, -1, -1, 0 },
};

bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
