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
  { BP_PINMUX_FNTYPE_PCM, 0 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 1 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 2 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 3 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 19 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 20 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 21 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 22 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 23 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 24 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 25 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 26 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 27 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 28 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_IRQ | 0, 49 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 1, 50 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 2, 67 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 3, 68 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 4, 71 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 5, 72 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 6, 33 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 7, 34 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 57 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 58 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 59 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 60 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 0, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 1, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 2, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 3, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 5, BP_PINMUX_VAL_DUMMY },
};

bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usTsync1pps, -1, 6, 6 | BP_PINMUX_VAL_4 },
  { bp_usTsync1pps, -1, 52, 52 | BP_PINMUX_VAL_7 },
  { bp_usSerialLedClk, -1, 10, 10 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedData, -1, 11, 11 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 33, 33 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedData, -1, 34, 34 | BP_PINMUX_VAL_3},
  { bp_usSerialLedClk, -1, 65, 65 | BP_PINMUX_VAL_4 },
  { bp_usSerialLedData, -1, 67, 67 | BP_PINMUX_VAL_4},
  { bp_usSerialLedMask, -1, 17, 17 | BP_PINMUX_VAL_1 },
  { bp_usGpioUart2Cts, -1, 13, 13 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Rts, -1, 16, 16 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1, 14, 14 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout, -1, 15, 15 | BP_PINMUX_VAL_1},
  { bp_usLinkLed, 1, 9, 9|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usLinkLed, 0, 5, 5|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usLinkLed, 3, 52, 52|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usLinkLed, 2, 54, 54|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usLinkLed, 1, 10, 10|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usLinkLed, 1, 53, 53|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usLinkLed, 1, 11, 11|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usLinkLed, 1, 54, 54|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usLinkLed, 0, 33, 33|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usLinkLed, 0, 51, 51|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usLinkLed, 0, 34, 34|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usLinkLed, 0, 52, 52|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usLinkLed, 3, 17, 17|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usLinkLed, 3, 18, 18|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usLinkLed, 2, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usLinkLed, 2, 14, 14|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usLinkLed, 2, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usLinkLed, 2, 15, 15|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},


  { bp_usGpioI2cSda, -1, 64, 64 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, -1, 63, 63 | BP_PINMUX_VAL_0 },
  { bp_usSpiSlaveSelectNum, 1, 61, 61 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 3, 8, 8 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 4, 9, 9 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 6, 12, 12 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 7, 67, 67 | BP_PINMUX_VAL_1},
  { bp_usSimDat, -1, 6, 6 | BP_PINMUX_VAL_3},
  { bp_usSimClk, -1, 7, 7 | BP_PINMUX_VAL_3},
  { bp_usSimPresence, -1, 8, 8 | BP_PINMUX_VAL_3},
  { bp_usPmdMACEwakeEn, -1, 10, 10 | BP_PINMUX_VAL_3},
  { bp_usSimVccEn, -1, 14, 14 | BP_PINMUX_VAL_3},
  { bp_usSimVccVolSel, -1, 15, 15 | BP_PINMUX_VAL_3},
  { bp_usSimRst, -1, 16, 16 | BP_PINMUX_VAL_3},
  { bp_usWanNco10MClk, -1, 16, 16 | BP_PINMUX_VAL_7},
  { bp_usSimVppEn, -1, 17, 17 | BP_PINMUX_VAL_3},
  { bp_usSimDat, -1, 43, 43 | BP_PINMUX_VAL_3},
  { bp_usSimClk, -1, 44, 44 | BP_PINMUX_VAL_3},
  { bp_usSimPresence, -1, 45, 45 | BP_PINMUX_VAL_3},
  { bp_usSimVccEn, -1, 35, 35 | BP_PINMUX_VAL_3},
  { bp_usSimVccVolSel, -1, 36, 36 | BP_PINMUX_VAL_3},
  { bp_usSimRst, -1, 37, 37 | BP_PINMUX_VAL_3},
  { bp_usSimVppEn, -1, 39, 39 | BP_PINMUX_VAL_3},
  { bp_usMiiMdc, -1, 47, 47 | BP_PINMUX_VAL_0},
  { bp_usMiiMdio, -1, 48, 48 | BP_PINMUX_VAL_0},
  { bp_usRogueOnuEn, -1, 51, 51 | BP_PINMUX_VAL_7},
  { bp_usTrxSignalDetect, -1, 53, 53 | BP_PINMUX_VAL_7},
  { bp_usTxLaserOnOutN, -1, 62, 62 | BP_PINMUX_VAL_1},
  { bp_usMs1588TodAlarm, -1, 65, 65 | BP_PINMUX_VAL_7},
  { bp_usSgmiiDetect, -1, 66, 66 | BP_PINMUX_VAL_7},
  { bp_usWanNcoProgMClk, -1, 67, 67 | BP_PINMUX_VAL_3},
  { bp_usProbeClk, -1, 74, 74 | BP_PINMUX_VAL_4},
  { bp_ReservedAnyLed, -1, 33, 33|BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 34, 34|BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 10, 10|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 11, 11|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 14, 14|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 15, 15|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 17, 17|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 18, 18|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 5, 5|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 6, 6|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 7, 7|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 8, 8|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 9, 9|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 69, 69|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 53, 53|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 54, 54|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyGpio, -1, -1, BP_PINMUX_VAL_5 },
  { bp_last, -1, -1, 0 },
};

bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
