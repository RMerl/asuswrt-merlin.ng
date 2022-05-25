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


  { BP_PINMUX_FNTYPE_PCM, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 81 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 82 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 85 | BP_PINMUX_VAL_1 },

  { BP_PINMUX_FNTYPE_HS_SPI, 23 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 83 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 26 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 36 | BP_PINMUX_VAL_0 },
  
  { BP_PINMUX_FNTYPE_xMII | 6, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 7, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 5, 53 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 54 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 55 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 56 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 57 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 58 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 59 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 60 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 61 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 62 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 63 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 64 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 5, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 66 | BP_PINMUX_VAL_1 },

  { BP_PINMUX_FNTYPE_NAND, 27 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 48 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 49 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 50 | BP_PINMUX_VAL_1 },


};

bp_pinmux_defs_t g_pinmux_defs_0[] =
{

  { bp_usVregSync, -1, 10, 10 | BP_PINMUX_VAL_0 },

  { bp_usPcmSdin,       -1, 80,  80  | BP_PINMUX_VAL_1},
  { bp_usPcmSdout,      -1, 81,  81  | BP_PINMUX_VAL_1},
  { bp_usPcmClk,        -1, 85,  85  | BP_PINMUX_VAL_1},
  { bp_usPcmFs,         -1, 82,  82  | BP_PINMUX_VAL_1},

  { bp_usSerialLedData, -1, 8,  8 | BP_PINMUX_VAL_0}, 
  { bp_usSerialLedClk,  -1, 6,  6 | BP_PINMUX_VAL_0 }, 
  { bp_usSerialLedMask, -1, 11,  11 | BP_PINMUX_VAL_0 }, 

  { bp_usUartCts,      0,  0, 0 | BP_PINMUX_VAL_1},
  { bp_usUartRts,      0,  4, 4 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,     0,  7, 7 | BP_PINMUX_VAL_1},
  { bp_usUartSdout,    0,  9, 9 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,     2,  13, 13 | BP_PINMUX_VAL_2},
  { bp_usUartSdout,    2,  14, 14 | BP_PINMUX_VAL_2},
  { bp_usUartSdout,    3,  18, 18 | BP_PINMUX_VAL_2},
  { bp_usUartSdin,     1,  22, 22 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,     3,  24, 24 | BP_PINMUX_VAL_2},
  { bp_usUartSdout,    1,  25, 25 | BP_PINMUX_VAL_1},
  { bp_usUartSdout,    6,  28, 28 | BP_PINMUX_VAL_0},
  { bp_usUartRts,      1,  30, 30 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,     6,  31, 31 | BP_PINMUX_VAL_0},
  { bp_usUartCts,      1,  35, 35 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,     2,  37, 37 | BP_PINMUX_VAL_0},
  { bp_usUartSdout,    2,  38, 38 | BP_PINMUX_VAL_0},
  { bp_usUartRts,      2,  39, 39 | BP_PINMUX_VAL_0},
  { bp_usUartCts,      2,  40, 40 | BP_PINMUX_VAL_0},
  { bp_usUartSdout,    1,  65, 65 | BP_PINMUX_VAL_0},
  { bp_usUartSdin,     1,  66, 66 | BP_PINMUX_VAL_0},
  { bp_usUartSdin,     5,  67, 67 | BP_PINMUX_VAL_0},
  { bp_usUartSdout,    5,  68, 68 | BP_PINMUX_VAL_0},

  /* dpfe ddr uart */
  { bp_usUartSdout,    0,  17, 17 | BP_PINMUX_VAL_3},
  { bp_usUartSdout,    2,  18, 18 | BP_PINMUX_VAL_3},
  { bp_usUartSdin,     2,  24, 24 | BP_PINMUX_VAL_3},
  { bp_usUartSdout,    1,  28, 28 | BP_PINMUX_VAL_6},
  { bp_usUartSdin,     1,  31, 31 | BP_PINMUX_VAL_6},
  { bp_usUartSdin,     0,  34, 34 | BP_PINMUX_VAL_3},



  { bp_ReservedDslCtl,  -1, 87, 87 | BP_PINMUX_VAL_0 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl,  -1, 17, 17 | BP_PINMUX_VAL_0 | BP_VDSLCTL_1 },
  { bp_ReservedDslCtl,  -1, 34, 34 | BP_PINMUX_VAL_0 | BP_VDSLCTL_2 },
  { bp_ReservedDslCtl,  -1, 33, 33 | BP_PINMUX_VAL_0 | BP_VDSLCTL_3 },
  { bp_ReservedDslCtl,  -1, 86, 86 | BP_PINMUX_VAL_0 | BP_VDSLCTL_4 },
  { bp_ReservedDslCtl,  -1, 84, 84 | BP_PINMUX_VAL_0 | BP_VDSLCTL_5 },

  { bp_usUsbPwrFlt0,  -1, 69, 69|BP_PINMUX_VAL_0 },
  { bp_usUsbPwrOn0,  -1, 70, 70|BP_PINMUX_VAL_0 },
  { bp_usUsbPwrFlt1,  -1, 71, 71|BP_PINMUX_VAL_0 },
  { bp_usUsbPwrOn1,  -1, 72, 72|BP_PINMUX_VAL_0 },

  { bp_usPcmSdin,  -1, 80, 80|BP_PINMUX_VAL_1 },
  { bp_usPcmSdout,  -1, 81, 81|BP_PINMUX_VAL_1 },
  { bp_usPcmFs,  -1, 82, 82|BP_PINMUX_VAL_1 },
  { bp_usPcmClk,  -1, 85, 85|BP_PINMUX_VAL_1 },

  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_4 }, // ALL SW GPIOs use pinmux 5

  { bp_usMiiMdc,  -1, 15, 15|BP_PINMUX_VAL_6 },
  { bp_usMiiMdio,  -1, 16, 16|BP_PINMUX_VAL_6 },
  { bp_usMiiMdc,  -1, 65, 65|BP_PINMUX_VAL_1 },
  { bp_usMiiMdio,  -1, 66, 66|BP_PINMUX_VAL_1 },

  { bp_usI2sTxSdata,          -1, 0, 0|BP_PINMUX_VAL_6 },
  { bp_usI2sRxMclk,             -1, 1, 1|BP_PINMUX_VAL_6 },
  { bp_usI2sTxMclk,             -1, 2, 2|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSdata,          -1, 4, 4|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSclk,             -1, 7, 7|BP_PINMUX_VAL_6 },
  { bp_usI2sTxLrck,             -1, 9, 9|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSclk,             -1, 13, 13|BP_PINMUX_VAL_6 },
  { bp_usI2sRxLrck,             -1, 14, 14|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSclk,             -1, 18, 18|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSclk,             -1, 22, 22|BP_PINMUX_VAL_6 },
  { bp_usI2sTxMclk,             -1, 24, 24|BP_PINMUX_VAL_6 },
  { bp_usI2sTxLrck,             -1, 25, 25|BP_PINMUX_VAL_6 },
  { bp_usI2sRxLrck,             -1, 29, 29|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSdata,          -1, 30, 30|BP_PINMUX_VAL_6 },
  { bp_usI2sRxMclk,             -1, 32, 32|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSdata,          -1, 35, 35|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSclk,             -1, 41, 41|BP_PINMUX_VAL_6 },
  { bp_usI2sTxLrck,             -1, 42, 42|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSdata,          -1, 43, 43|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSdata,          -1, 44, 44|BP_PINMUX_VAL_6 },
  { bp_usI2sTxMclk,             -1, 45, 45|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSclk,             -1, 46, 46|BP_PINMUX_VAL_6 },
  { bp_usI2sRxLrck,             -1, 47, 47|BP_PINMUX_VAL_6 },
  { bp_usI2sRxMclk,             -1, 48, 48|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSclk,             -1, 57, 57|BP_PINMUX_VAL_6 },
  { bp_usI2sTxLrck,             -1, 58, 58|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSdata,          -1, 59, 59|BP_PINMUX_VAL_6 },
  { bp_usI2sTxSdata,          -1, 60, 60|BP_PINMUX_VAL_6 },
  { bp_usI2sTxMclk,             -1, 61, 61|BP_PINMUX_VAL_6 },
  { bp_usI2sRxSclk,             -1, 62, 62|BP_PINMUX_VAL_6 },
  { bp_usI2sRxLrck,             -1, 63, 63|BP_PINMUX_VAL_6 },
  { bp_usI2sRxMclk,             -1, 64, 64|BP_PINMUX_VAL_6 },

  { bp_usGpioI2cScl, 0,3, 3|BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, 1,7, 7|BP_PINMUX_VAL_2 },
  { bp_usGpioI2cSda, 1,9, 9|BP_PINMUX_VAL_2 },
  { bp_usGpioI2cSda, 0,12, 12|BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, 1,22, 22|BP_PINMUX_VAL_2 },
  { bp_usGpioI2cSda, 1,25, 25|BP_PINMUX_VAL_2 },
  { bp_usGpioI2cScl, 1,53, 53|BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda, 1,54, 54|BP_PINMUX_VAL_0 },


  { bp_usSpiSlaveSelectNum, 1,  20, 20|BP_PINMUX_VAL_1 },
  { bp_usSpiSlaveSelectNum, 2,  21, 21|BP_PINMUX_VAL_1 },
  { bp_usSpiSlaveSelectNum, 5,  28, 28|BP_PINMUX_VAL_1 },
  { bp_usSpiSlaveSelectNum, 4,  31, 31|BP_PINMUX_VAL_1 },
  { bp_usSpiSlaveSelectNum, 3,  33, 33|BP_PINMUX_VAL_1 },
  { bp_usSpiSlaveSelectNum, 0,  36, 36|BP_PINMUX_VAL_0 },

  { bp_usNtrRefIn, -1,  19, 19|BP_PINMUX_VAL_3 },
  { bp_usNtrRefIn, -1,  1, 1|BP_PINMUX_VAL_3 },

  { bp_usSfpSigDetect, 6, 4, 4 | BP_PINMUX_VAL_2 },
  { bp_usSfpSigDetect, 6, 16, 16 | BP_PINMUX_VAL_2 },
  { bp_usSfpSigDetect, 6, 35, 35 | BP_PINMUX_VAL_2 },
  { bp_usSfpSigDetect, 6, 56, 56 | BP_PINMUX_VAL_2 },
  { bp_usSfpSigDetect, 7, 0, 0 | BP_PINMUX_VAL_3 },
  { bp_usSfpSigDetect, 7, 5, 5 | BP_PINMUX_VAL_3 },
  { bp_usSfpSigDetect, 7, 56, 56 | BP_PINMUX_VAL_3 },


  { bp_last, -1, -1,  0 },
};
bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0} ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
