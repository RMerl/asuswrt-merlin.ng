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

  { BP_PINMUX_FNTYPE_PCM, 13 | BP_PINMUX_VAL_6 },
  { BP_PINMUX_FNTYPE_PCM, 14 | BP_PINMUX_VAL_6 },
  { BP_PINMUX_FNTYPE_PCM, 15 | BP_PINMUX_VAL_6 },
  { BP_PINMUX_FNTYPE_PCM, 16 | BP_PINMUX_VAL_6 },

  { BP_PINMUX_FNTYPE_HS_SPI, 76 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 77 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 78 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 79 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 80 | BP_PINMUX_VAL_0 },

  { BP_PINMUX_FNTYPE_xMII | 5, 60 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 61 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 62 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 63 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 64 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 66 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 67 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 68 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 69 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 70 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 71 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 72 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 73 | BP_PINMUX_VAL_1 },

  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_4 },
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
  { BP_PINMUX_FNTYPE_NAND, 57 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 58 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 59 | BP_PINMUX_VAL_1 },

};

bp_pinmux_defs_t g_pinmux_defs_0[] =
{
  { bp_usNetLed0, 0,  0,  0 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, //QGPHY0
  { bp_usNetLed0, 0,  55,  55 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(0) }, //QGPHY0
  { bp_usNetLed0, 0,  48,  48 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(0) }, //QGPHY0
  { bp_usNetLed0, 0,  71,  71 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(0) }, //QGPHY0

  { bp_usNetLed0, 1,  57,  57 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(2) }, //QGPHY1
  { bp_usNetLed0, 1,  7,  7 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(2) }, //QGPHY1
  { bp_usNetLed0, 1,  66,  66 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(2) }, //QGPHY1

  { bp_usNetLed0, 2,  46,  46 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(4) }, //QGPHY2
  { bp_usNetLed0, 2,  5, 5  | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(4) }, //QGPHY2
  { bp_usNetLed0, 2,  62, 62  | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(4) }, //QGPHY2

  { bp_usNetLed0, 3,  58,  58 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(6) }, //QGPHY3
  { bp_usNetLed0, 3,  1,  1 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED |BP_PINMUX_OPTLED_NUM(6) }, //QGPHY3
  { bp_usNetLed0, 3,  56,  56 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(6) }, //QGPHY3
  { bp_usNetLed0, 3,  65,  65 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(6) }, //QGPHY3

  { bp_usNetLed0, 4,  3,   3 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED |BP_PINMUX_OPTLED_NUM(8) }, //QGPHY4
  { bp_usNetLed0, 4,  59,  59 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(8) }, //QGPHY4
  { bp_usNetLed0, 4,  61,  61 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(8) }, //QGPHY4

  { bp_usNetLed1, 0,  70, 70| BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_usNetLed1, 0,  22, 22| BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_usNetLed1, 1,  6, 6 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_usNetLed1, 1,  69, 69 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 2,  68,  68 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(5) },
  { bp_usNetLed1, 2,  9, 9 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_usNetLed1, 3,  67,  67 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed1, 3,  2,  2 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(7) },
  { bp_usNetLed1, 3,  20,  20 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(7) },

  { bp_usNetLed1, 4,  4,  4 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(9) },

  { bp_usNetLed3, 0, 4, 4 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(15) },
  { bp_usNetLed3, 0, 24, 24 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(15) },
  { bp_usNetLed3, 0, 47, 47 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(15)},
  { bp_usNetLed3, 0, 72, 72 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(15) },

  { bp_usNetLed3, 1, 8, 8 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_usNetLed3, 1, 51, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_usNetLed3, 1, 25, 25 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_usNetLed3, 1, 60, 60 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED |BP_PINMUX_OPTLED_NUM(16) },

  { bp_usNetLed3, 2, 11, 11 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(17) },
  { bp_usNetLed3, 2, 29, 29 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(17) },
  { bp_usNetLed3, 2, 50, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(17) },
  { bp_usNetLed3, 2, 63, 63 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(17) },

  { bp_usNetLed3, 3,  10,  10 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(18) },
  { bp_usNetLed3, 3,  49,  49 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(18) },
  { bp_usNetLed3, 3,  55,  55 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(18) },
  { bp_usNetLed3, 3,  64,  64 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(18) },

  { bp_usNetLed3, 4, 16, 16 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(19) },
  { bp_usNetLed3, 4, 21, 21 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(19) },
  { bp_usNetLed3, 4, 44, 44 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(19) },
  { bp_usNetLed3, 4, 45, 45 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(19) },
  { bp_usNetLed3, 4, 56, 56 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(19) },
  { bp_usNetLed3, 4, 73, 73 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED|BP_PINMUX_OPTLED_NUM(19) },

  { bp_usGpioLedAggregateAct, -1, 20, 20 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21) }, //Aggregate activity LED
  { bp_usGpioLedAggregateLnk, -1, 21, 21 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(22) }, //Aggregate link LED
  { bp_usGpioLedWanAct, 0, 8, 8 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(23)}, //DSL WAN activity
  { bp_usGpioLedWanAct, 0, 49, 49 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(23)}, //DSL WAN activity

  { bp_usVregSync, -1, 2, 2 | BP_PINMUX_VAL_6 },

  { bp_usPcmSdin,       -1, 13,  13  | BP_PINMUX_VAL_6},
  { bp_usPcmSdout,      -1, 14,  14  | BP_PINMUX_VAL_6},
  { bp_usPcmClk,        -1, 15,  15  | BP_PINMUX_VAL_6},
  { bp_usPcmFs,         -1, 16,  16  | BP_PINMUX_VAL_6},

  { bp_usSerialLedData, -1, 23,  23 | BP_PINMUX_VAL_4}, 
  { bp_usSerialLedClk,  -1, 24,  24 | BP_PINMUX_VAL_4 }, 
  { bp_usSerialLedMask, -1, 25,  25 | BP_PINMUX_VAL_4 }, 
  { bp_usSerialLedData, -1, 26,  26 | BP_PINMUX_VAL_6 }, 
  { bp_usSerialLedClk,  -1, 27,  27 | BP_PINMUX_VAL_6 }, 
  { bp_usSerialLedMask, -1, 28,  28 | BP_PINMUX_VAL_6 },
  { bp_usUartSdin,  0, 74, 74 | BP_PINMUX_VAL_1},
  { bp_usUartSdout, 0, 75, 75 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,  1,  25,  25 | BP_PINMUX_VAL_2},
  { bp_usUartSdout, 1,  24,  24 | BP_PINMUX_VAL_2},
  { bp_usUartRts,    1,  12, 12 | BP_PINMUX_VAL_6},
  { bp_usUartSdout,  1,  20, 20 | BP_PINMUX_VAL_6},
  { bp_usUartSdin,   1,  21, 21 | BP_PINMUX_VAL_6},
  { bp_usUartCts,    1,  22, 22 | BP_PINMUX_VAL_6},
  { bp_usUartSdout,  0,  69, 69 | BP_PINMUX_VAL_3},
  { bp_usUartSdin,   0,  70, 70 | BP_PINMUX_VAL_3},
  { bp_usUartCts,    0,  71, 71 | BP_PINMUX_VAL_3},
  { bp_usUartRts,   0,  72, 72 | BP_PINMUX_VAL_3},
  { bp_ReservedDslCtl,  -1, 17, 17 | BP_PINMUX_VAL_6 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl,  -1, 18, 18 | BP_PINMUX_VAL_6 | BP_VDSLCTL_1 },
  { bp_ReservedDslCtl,  -1, 19, 19 | BP_PINMUX_VAL_6 | BP_VDSLCTL_2 },
  { bp_usI2sSclk,             -1, 6, 6|BP_PINMUX_VAL_3 },
  { bp_usI2sLrck,             -1, 7, 7|BP_PINMUX_VAL_3 },
  { bp_usI2sRxSdata,          -1, 8, 8|BP_PINMUX_VAL_3 },
  { bp_usI2sTxSdata,          -1, 9, 9|BP_PINMUX_VAL_3 },
  { bp_usI2sMclk,             -1, 10, 10|BP_PINMUX_VAL_3 },
  { bp_usI2sSclk,             -1, 13, 13|BP_PINMUX_VAL_4 },
  { bp_usI2sLrck,             -1, 14, 14|BP_PINMUX_VAL_4 },
  { bp_usI2sRxSdata,          -1, 15, 15|BP_PINMUX_VAL_4 },
  { bp_usI2sTxSdata,          -1, 16, 16|BP_PINMUX_VAL_4 },
  { bp_usI2sMclk,             -1, 17, 17|BP_PINMUX_VAL_4 },
  { bp_usI2sSclk,             -1, 29, 29|BP_PINMUX_VAL_6 },
  { bp_usI2sLrck,             -1, 30, 30|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSdata,          -1, 31, 31|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSdata,          -1, 32, 32|BP_PINMUX_VAL_6 },
  { bp_usI2sMclk,             -1, 33, 33|BP_PINMUX_VAL_6 },
  { bp_usI2sSclk,             -1, 52, 52|BP_PINMUX_VAL_2 },
  { bp_usI2sLrck,             -1, 53, 53|BP_PINMUX_VAL_2 },
  { bp_usI2sRxSdata,          -1, 54, 54|BP_PINMUX_VAL_2 },
  { bp_usI2sTxSdata,          -1, 55, 55|BP_PINMUX_VAL_2 },
  { bp_usI2sMclk,             -1, 56, 56|BP_PINMUX_VAL_2 },
  { bp_usI2sSclk,             -1, 67, 67|BP_PINMUX_VAL_4 },
  { bp_usI2sLrck,             -1, 68, 68|BP_PINMUX_VAL_4 },
  { bp_usI2sRxSdata,          -1, 69, 69|BP_PINMUX_VAL_4 },
  { bp_usI2sTxSdata,          -1, 70, 70|BP_PINMUX_VAL_4 },
  { bp_usI2sMclk,             -1, 71, 71|BP_PINMUX_VAL_4 },
  { bp_usSpiSlaveSelectNum, 0, 79, 79 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 1, 80, 80 | BP_PINMUX_VAL_0},
  { bp_usUsbPwrFlt0, -1, 85, 85 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrOn0,  -1, 86, 86 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrFlt1, -1, 83, 83 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrOn1,  -1, 84, 84 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrFlt0, -1, 83, 83 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0,  -1, 84, 84 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 85, 85 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1,  -1, 86, 86 | BP_PINMUX_VAL_1 },
  { bp_usMiiMdc,     -1, 72, 72 | BP_PINMUX_VAL_1},
  { bp_usMiiMdio,    -1, 73, 73 | BP_PINMUX_VAL_1},
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_5 }, // ALL SW GPIOs use pinmux 5
  { bp_ReservedAnyLed,  -1, 0, 0 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(0) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 1, 1 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(6) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 2, 2 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(7) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 3, 3 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(8) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 4, 4 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(9) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 4, 4 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(15) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 5, 5 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(4) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 6, 6 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(3) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 7, 7 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(2) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 8, 8 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(23) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 8, 8 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(16) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 9, 9 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(31) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 9, 9 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(5) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 10, 10 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(27) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 10, 10 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(18) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 11, 11 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(10) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 11, 11 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(17) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 12, 12 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(11) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 13, 13 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(12) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 14, 14 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(13) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 15, 15 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(14) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 16, 16 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(19) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 17, 17 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(20) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 18, 18 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(24) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 19, 19 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(25) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 20, 20 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(21) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 20, 20 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(7) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 21, 21 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(22) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 21, 21 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(19) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 22, 22 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(1) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 23, 23 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(26) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 24, 24 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(15) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 24, 24 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(25) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 25, 25 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(16) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 25, 25 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(24) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 26, 26 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(28) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 27, 27 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(29) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 28, 28 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(30) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 29, 29 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(17) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 30, 30 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(27) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 31, 31 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(31) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 44, 44 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(19) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 45, 45 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(19) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 46, 46 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(4) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 47, 47 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(15) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 48, 48 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(0) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 49, 49 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(18) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 49, 49 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(23) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 50, 50 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(17) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 50, 50 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(31) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 51, 51 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(16) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 51, 51 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(27) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 52, 52 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(28) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 53, 53 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(29) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 54, 54 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(30) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 55, 55 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(18) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 55, 55 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(0) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 56, 56 | BP_PINMUX_VAL_3|BP_PINMUX_OPTLED_NUM(19) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 56, 56 | BP_PINMUX_VAL_4|BP_PINMUX_OPTLED_NUM(6) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 57, 57 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(2) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 58, 58 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(6) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 59, 59 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(8) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 60, 60 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(16) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 61, 61 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(8) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 62, 62 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(4) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 63, 63 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(17) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 64, 64 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(18) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 65, 65 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(6) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 66, 66 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(2) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 67, 67 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(7) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 68, 68 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(5) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 69, 69 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(3) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 70, 70 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(1) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 71, 71 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(0) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 72, 72 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(15) }, //  SW LED
  { bp_ReservedAnyLed,  -1, 73, 73 | BP_PINMUX_VAL_6|BP_PINMUX_OPTLED_NUM(19) }, //  SW LED

  { bp_usGpioWlanReserved,  -1, 0, 0| BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 1, 1| BP_PINMUX_VAL_4 },
  { bp_usGpioWlanReserved,  -1, 3, 3| BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 4, 4| BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 5, 5| BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 11, 11| BP_PINMUX_VAL_4 },
  { bp_usGpioWlanReserved,  -1, 12, 12| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 13, 13| BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 17, 17| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 18, 18| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 19, 19| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 20, 20| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 21, 21| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 26, 26| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 27, 27| BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 28, 28| BP_PINMUX_VAL_1 },

  /*These are wlan bluetooth co-existance */
  { bp_usGpioWlanReserved,  -1, 2, 2 | BP_PINMUX_VAL_4 },
  { bp_usGpioWlanReserved,  -1, 7, 7 | BP_PINMUX_VAL_6 },
  { bp_usGpioWlanReserved,  -1, 8, 8 | BP_PINMUX_VAL_6 },
  { bp_usGpioWlanReserved,  -1, 9, 9 | BP_PINMUX_VAL_6 },
  { bp_usGpioWlanReserved,  -1, 10,10| BP_PINMUX_VAL_6 },
  { bp_usGpioWlanReserved,  -1, 28, 28 | BP_PINMUX_VAL_2 },
  { bp_usGpioWlanReserved,  -1, 30, 30 | BP_PINMUX_VAL_2 },
  { bp_usGpioWlanReserved,  -1, 31, 31 | BP_PINMUX_VAL_2 },
  { bp_usGpioWlanReserved,  -1, 32, 32 | BP_PINMUX_VAL_2 },
  { bp_usGpioWlanReserved,  -1, 33, 33 | BP_PINMUX_VAL_2 },
  { bp_usGpioWlanReserved,  -1, 61, 61 | BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 62, 62 | BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 63, 63 | BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 64, 64 | BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 68, 68 | BP_PINMUX_VAL_3 },
  { bp_usGpioWlanReserved,  -1, 80, 80 | BP_PINMUX_VAL_6 },

  /* wlan fem_ctrl */
  { bp_usGpioWlanReserved,  -1, 34, 34 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 35, 35 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 36, 36 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 37, 37 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 38, 38 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 39, 39 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 40, 40 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 41, 41 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 42, 42 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 43, 43 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 44, 44 | BP_PINMUX_VAL_1 },
  { bp_usGpioWlanReserved,  -1, 45, 45 | BP_PINMUX_VAL_1 },

  { bp_last, -1, -1,  0 },
};
bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
