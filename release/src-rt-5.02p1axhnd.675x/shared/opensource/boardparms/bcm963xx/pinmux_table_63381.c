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
  { BP_PINMUX_FNTYPE_PCM, 70 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 71 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 72 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 73 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 48 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 49 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 50 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 51 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 52 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 53 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 54 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 55 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 56 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 86 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 87 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_IRQ | 0, 63 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_IRQ | 1, 64 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_IRQ | 2, 61 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_IRQ | 3, 62 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_IRQ | 4, 54 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_IRQ | 5, 44 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_IRQ | 6, 46 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_IRQ | 7, 94 | BP_PINMUX_VAL_4 },
  { BP_PINMUX_FNTYPE_HS_SPI, 90 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_HS_SPI, 91 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_HS_SPI, 92 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_HS_SPI, 93 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_DEFAULT, 96 | BP_PINMUX_VAL_1},
  { BP_PINMUX_FNTYPE_DEFAULT, 97 | BP_PINMUX_VAL_1},
};


bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usSerialLedData, -1, 17, 60 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 16, 59 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedMask, -1, 24, 67 | BP_PINMUX_VAL_1 },
  { bp_usGpioUart2Sdin, -1, 23, 66 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout, -1, 22, 65 | BP_PINMUX_VAL_1 },

  { bp_usSpeedLed100, 0, 2, 45|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 0, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2) },

  { bp_usSpeedLed100, 0, 16, 59|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 0, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) },

  { bp_usSpeedLed100, 1, 3, 46|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3) },

  { bp_usSpeedLed100, 1, 17, 60|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 1, 43, 86 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(17) },

  { bp_usSpeedLed100, 2, 4, 47|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 2, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4) },

  { bp_usSpeedLed100, 2, 22, 65|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },

  { bp_usSpeedLed100, 3, 6, 49|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 3, 42, 85 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6) },

  { bp_usSpeedLed100, 3, 24, 67|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },


  { bp_usLinkLed, 0, 5, 48|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 0, 35, 78 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5) },

  { bp_usLinkLed, 0, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(27) },

  { bp_usLinkLed, 1, 8, 51|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 1, 33, 76 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8) },

  { bp_usLinkLed, 1, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(28) },

  { bp_usLinkLed, 2, 9, 52|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 2, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9) },

  { bp_usLinkLed, 2, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(29) },

  { bp_usLinkLed, 3, 10, 53|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 41, 84 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10) },

  { bp_usLinkLed, 3, 31, 74|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 49, 94 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(31) },

  { bp_usGpioLedWanData, -1, 13, 56|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED},
  { bp_usGpioLedWanData, -1, 40, 83 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13) },

  { bp_usGpioLedWanData, -1, 19, 62|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED},
  { bp_ReservedAnyGpio, -1, 0, 43 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 1, 44 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 2, 45 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 3, 46 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 4, 47 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 5, 48 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 6, 49 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 7, 50 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 8, 51 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 9, 52 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 10, 53 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 11, 54 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 12, 55 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 13, 56 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 14, 57 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 15, 58 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 16, 59 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 17, 60 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 18, 61 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 19, 62 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 20, 63 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 21, 64 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 22, 65 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 23, 66 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 24, 67 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 25, 68 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 26, 69 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 27, 70 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 28, 71 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 29, 72 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 30, 73 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 31, 74 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 32, 75 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 33, 76 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 34, 77 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 35, 78 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 36, 79 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 37, 80 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 38, 81 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 39, 82 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 40, 83 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 41, 84 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 42, 85 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 43, 86 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 44, 87 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 45, 90 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 46, 91 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 47, 92 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 48, 93 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 49, 94 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 50, 96 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 51, 97 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyLed, -1, 0, 43 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 1, 44 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 2, 45 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 3, 46 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 4, 47 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 5, 48 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 6, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 7, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 8, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 9, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 10, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 11, 54 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 12, 55 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 13, 56 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 14, 57 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 15, 58 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 16, 59 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 17, 60 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 18, 61 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 19, 62 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 20, 63 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 21, 64 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 22, 65 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 23, 66 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 24, 67 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 25, 68 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 26, 69 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 27, 70 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 28, 71 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 29, 72 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 33, 76 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 35, 78 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 37, 80 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 39, 82 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) },
  { bp_ReservedAnyLed, -1, 40, 83 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 41, 84 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 42, 85 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 43, 86 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyLed, -1, 44, 87 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) },
  { bp_ReservedAnyLed, -1, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(27) },
  { bp_ReservedAnyLed, -1, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(28) },
  { bp_ReservedAnyLed, -1, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(29) },
  { bp_ReservedAnyLed, -1, 48, 93 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(30) },
  { bp_ReservedAnyLed, -1, 49, 94 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(31) },
  { bp_ReservedDslCtl, -1, 15, 58|BP_PINMUX_VAL_1 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl, -1, 26, 69|BP_PINMUX_VAL_1 | BP_VDSLCTL_4 },
  { bp_ReservedDslCtl, -1, 25, 68|BP_PINMUX_VAL_1 | BP_VDSLCTL_5 },
  { bp_usSpiSlaveSelectNum, 1, 49, 94 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 2, 18, 61 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 3, 19, 62 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 4, 6, 49 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 5, 7, 50 | BP_PINMUX_VAL_3},
  { bp_usUsbPwrFlt0, -1, 50, 96 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0, -1, 51, 97 | BP_PINMUX_VAL_1 },
  { bp_last, -1, -1, 0 },
};


bp_pinmux_defs_t g_pinmux_defs_1[] = {
  { bp_usSerialLedData, -1, 17, 60 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 16, 59 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedMask, -1, 24, 67 | BP_PINMUX_VAL_1 },
  { bp_usGpioUart2Sdin, -1, 23, 66 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout, -1, 22, 65 | BP_PINMUX_VAL_1 },

  { bp_usSpeedLed100, 0, 16, 59 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 0, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) },

  { bp_usSpeedLed100, 1, 15, 58 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 1, 37, 80 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(15) },

  { bp_usSpeedLed100, 1, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(29) },

  { bp_usSpeedLed100, 2, 9, 52 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 2, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_usSpeedLed100, 2, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(28) },


  { bp_usSpeedLed100, 3, 26, 69 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 3, 50, 96 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(26)},


  { bp_usLinkLed, 0, 4, 47 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 0, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_usLinkLed, 0, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(27) },


  { bp_usLinkLed, 1, 3, 46 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3) },

  { bp_usLinkLed, 1, 24, 67 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },


  { bp_usLinkLed, 2, 2, 45 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 2, 23, 66 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 2, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED |BP_PINMUX_OPTLED_NUM(3)},
  { bp_usLinkLed, 2, 51, 97 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED |BP_PINMUX_OPTLED_NUM(23)},


  { bp_usLinkLed, 3, 0, 43 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 22, 65 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 27, 70 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},


  { bp_usGpioLedWanData, -1, 19, 62 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED},
  { bp_usGpioLedWanData, -1, 29, 72 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},

  { bp_ReservedAnyGpio, -1, 0, 43 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 1, 44 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 2, 45 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 3, 46 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 4, 47 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 5, 48 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 6, 49 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 7, 50 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 8, 51 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 9, 52 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 10, 53 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 11, 54 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 12, 55 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 13, 56 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 14, 57 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 15, 58 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 16, 59 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 17, 60 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 18, 61 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 19, 62 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 20, 63 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 21, 64 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 22, 65 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 23, 66 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 24, 67 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 25, 68 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyGpio, -1, 26, 69 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO },
  { bp_ReservedAnyLed, -1, 0, 43 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 1, 44 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 2, 45 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 3, 46 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 4, 47 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 5, 48 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 6, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 7, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 8, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 9, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 10, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 11, 54 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 12, 55 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 13, 56 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 14, 57 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 15, 58 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 16, 59 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 17, 60 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 18, 61 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 19, 62 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 20, 63 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 21, 64 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 22, 65 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 23, 66 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 24, 67 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 25, 68 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 26, 69 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED },
  { bp_ReservedAnyLed, -1, 27, 70 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 28, 71 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 29, 72 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 33, 76 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 35, 78 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 37, 80 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 39, 82 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) },
  { bp_ReservedAnyLed, -1, 40, 83 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 41, 84 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 42, 85 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 43, 86 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyLed, -1, 44, 87 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) },
  { bp_ReservedAnyLed, -1, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(27) },
  { bp_ReservedAnyLed, -1, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(28) },
  { bp_ReservedAnyLed, -1, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(29) },
  { bp_ReservedAnyLed, -1, 48, 93 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(30) },
  { bp_ReservedAnyLed, -1, 49, 94 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(31) },
  { bp_ReservedDslCtl, -1, 15, 58|BP_PINMUX_VAL_1 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl, -1, 26, 69|BP_PINMUX_VAL_1 | BP_VDSLCTL_4 },
  { bp_ReservedDslCtl, -1, 25, 68|BP_PINMUX_VAL_1 | BP_VDSLCTL_5 },
  { bp_usSpiSlaveSelectNum, 1, 49, 94 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 2, 18, 61 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 3, 19, 62 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 4, 6, 49 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 5, 7, 50 | BP_PINMUX_VAL_3},
  { bp_usUsbPwrFlt0, -1, 50, 96 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0, -1, 51, 97 | BP_PINMUX_VAL_1 },
  { bp_last, -1, -1, 0 },
};

bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0, g_pinmux_defs_1 } ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
