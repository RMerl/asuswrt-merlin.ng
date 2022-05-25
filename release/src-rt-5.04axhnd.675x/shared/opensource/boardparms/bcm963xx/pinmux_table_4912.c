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

	{ BP_PINMUX_FNTYPE_PCM, 8 | BP_PINMUX_VAL_0 },
	{ BP_PINMUX_FNTYPE_PCM, 9 | BP_PINMUX_VAL_0 },
	{ BP_PINMUX_FNTYPE_PCM, 10 | BP_PINMUX_VAL_0 },
	{ BP_PINMUX_FNTYPE_PCM, 11 | BP_PINMUX_VAL_0 },
	
        { BP_PINMUX_FNTYPE_HS_SPI, 20 | BP_PINMUX_VAL_0 },
	{ BP_PINMUX_FNTYPE_HS_SPI, 21 | BP_PINMUX_VAL_0 },
	{ BP_PINMUX_FNTYPE_HS_SPI, 22 | BP_PINMUX_VAL_0 },
	{ BP_PINMUX_FNTYPE_HS_SPI, 23 | BP_PINMUX_VAL_0 },

	{ BP_PINMUX_FNTYPE_xMII | 5, BP_PINMUX_VAL_DUMMY },
	{ BP_PINMUX_FNTYPE_xMII | 6, BP_PINMUX_VAL_DUMMY },
	{ BP_PINMUX_FNTYPE_xMII | 7, BP_PINMUX_VAL_DUMMY },
	{ BP_PINMUX_FNTYPE_xMII | 4, 42 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 43 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 44 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 45 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 46 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 47 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 48 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 49 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 50 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 51 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 52 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4, 53 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 4,  0 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_xMII | 4,  3 | BP_PINMUX_VAL_1 },

	{ BP_PINMUX_FNTYPE_NAND, 25 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 26 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 27 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 28 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 30 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 31 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 35 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 36 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_1 },

};

bp_pinmux_defs_t g_pinmux_defs_0[] =
{
	{ bp_usSerialLedData, -1, 16,  16 | BP_PINMUX_VAL_0}, 
	{ bp_usSerialLedClk,  -1, 17,  17 | BP_PINMUX_VAL_0 }, 
	{ bp_usSerialLedMask, -1, 18,  18 | BP_PINMUX_VAL_0 }, 
	{ bp_usMiiMdc,  -1, 0, 0|BP_PINMUX_VAL_1 },
	{ bp_usMiiMdio,  -1, 3, 3|BP_PINMUX_VAL_1 },
	{ bp_usI2sTxLrck, -1, 6, 6|BP_PINMUX_VAL_0 },
	{ bp_usI2sTxMclk, -1, 7, 7|BP_PINMUX_VAL_0 },
	{ bp_usPcmSdin,  -1, 8, 8|BP_PINMUX_VAL_0 },
	{ bp_usPcmSdout,  -1, 9, 9|BP_PINMUX_VAL_0 },
	{ bp_usPcmClk,  -1, 10, 10|BP_PINMUX_VAL_0 },
	{ bp_usPcmFs,  -1, 11, 11|BP_PINMUX_VAL_0 },
	{ bp_usGpioI2cScl, 0,12, 12|BP_PINMUX_VAL_0 },
	{ bp_usGpioI2cSda, 0,13, 13|BP_PINMUX_VAL_0 },
	{ bp_usGpioI2cScl, 1,14, 14|BP_PINMUX_VAL_0 },
	{ bp_usGpioI2cSda, 1,15, 15|BP_PINMUX_VAL_0 },
	{ bp_usSpiSlaveSelectNum, 1,  20, 20|BP_PINMUX_VAL_0 },
	{ bp_usGpioSpiClk, -1,  21, 21|BP_PINMUX_VAL_0}, 
	{ bp_usGpioSpiMosi, -1,  22, 22|BP_PINMUX_VAL_0}, 
	{ bp_usGpioSpiMiso, -1,  23, 23|BP_PINMUX_VAL_0}, 
	{ bp_usUartSdout, -1,  21, 21 | BP_PINMUX_VAL_3}, //DDR UART
	{ bp_usUartSdin,  -1,  22, 22 | BP_PINMUX_VAL_3}, //DDR UART
	{ bp_usSpiSlaveSelectNum, 0,  24, 24|BP_PINMUX_VAL_0 },
	{ bp_usUartSdout, 7,  42, 42 | BP_PINMUX_VAL_0},
	{ bp_usUartRts,   7,  43, 43 | BP_PINMUX_VAL_0},
	{ bp_usUartCts,   7,  44, 44 | BP_PINMUX_VAL_0},
	{ bp_usI2sRxSclk, -1, 45, 45|BP_PINMUX_VAL_0 },
	{ bp_usI2sRxLrck, -1, 46, 46|BP_PINMUX_VAL_0 },
	{ bp_usI2sRxSdata,-1, 47, 47|BP_PINMUX_VAL_0 },
	{ bp_usI2sRxMclk, -1, 48, 48|BP_PINMUX_VAL_0 },
	{ bp_usI2sTxSdata,-1, 49, 49|BP_PINMUX_VAL_0 },
	{ bp_usI2sTxSclk, -1, 50, 50|BP_PINMUX_VAL_0 },
	{ bp_usUartSdin,  7,  51, 51 | BP_PINMUX_VAL_0},
	{ bp_usSpiSlaveSelectNum, 2,  52, 52|BP_PINMUX_VAL_0 },
	{ bp_usSpiSlaveSelectNum, 3,  53, 53|BP_PINMUX_VAL_0 },
	{ bp_usSpiSlaveSelectNum, 4,  54, 54|BP_PINMUX_VAL_0 },
	{ bp_usMiiMdc,  -1, 54, 54|BP_PINMUX_VAL_1 },
	{ bp_usSpiSlaveSelectNum, 5,  55, 55|BP_PINMUX_VAL_0 },
	{ bp_usMiiMdio,  -1, 55, 55|BP_PINMUX_VAL_1 },
	{ bp_usUartSdin,  20,  56, 56 | BP_PINMUX_VAL_0},
	{ bp_usUartSdout, 20,  57, 57 | BP_PINMUX_VAL_0},
	{ bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_4 },
	{ bp_usSfpSigDetect, 6, 1, 1 | BP_PINMUX_VAL_0 },
	{ bp_usSfpSigDetect, 7, 2, 2 | BP_PINMUX_VAL_0 },
	{ bp_usVregSync, -1, 19, 19 | BP_PINMUX_VAL_1 },
	{ bp_usVregSync, -1, 69, 69 | BP_PINMUX_VAL_0 },
	{ bp_last, -1, -1,  0 },
};
bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0} ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
