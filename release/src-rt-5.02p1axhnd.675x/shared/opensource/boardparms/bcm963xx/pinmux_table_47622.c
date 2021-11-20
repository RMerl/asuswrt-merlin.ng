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

	{ BP_PINMUX_FNTYPE_HS_SPI, 72 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_HS_SPI, 73 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_HS_SPI, 74 | BP_PINMUX_VAL_1 },

	{ BP_PINMUX_FNTYPE_xMII | 10, 56 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 57 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 58 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 59 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 60 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 61 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 62 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 63 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 64 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 65 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 66 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 67 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 10, 68 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_xMII | 10, 69 | BP_PINMUX_VAL_1 },

	{ BP_PINMUX_FNTYPE_xMII | 11, 56 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 57 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 58 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 59 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 60 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 61 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 62 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 63 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 64 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 65 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 66 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 67 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 11, 68 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_xMII | 11, 69 | BP_PINMUX_VAL_1 },

	{ BP_PINMUX_FNTYPE_xMII | 0, 56 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 57 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 58 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 59 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 60 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 61 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 62 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 63 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 64 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 65 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 66 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 67 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 0, 68 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_xMII | 0, 69 | BP_PINMUX_VAL_1 },

	{ BP_PINMUX_FNTYPE_xMII | 1, 56 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 57 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 58 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 59 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 60 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 61 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 62 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 63 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 64 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 65 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 66 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 67 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
	{ BP_PINMUX_FNTYPE_xMII | 1, 68 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_xMII | 1, 69 | BP_PINMUX_VAL_1 },

	{ BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
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
	{ BP_PINMUX_FNTYPE_NAND, 51 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 52 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_NAND, 53 | BP_PINMUX_VAL_1 },

	{ BP_PINMUX_FNTYPE_EMMC, 54 | BP_PINMUX_VAL_1 },
	{ BP_PINMUX_FNTYPE_EMMC, 55 | BP_PINMUX_VAL_1 },
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {

	{ bp_usNetLed0, 0,      0,  0 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 0,      9,  9 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 0,      17, 17| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 0,      25, 25| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 0,      40, 40| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(0) },
	{ bp_usNetLed0, 0,      49, 49| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(9) },
	{ bp_usNetLed0, 0,      57, 57| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(17) },
	{ bp_usNetLed0, 0,      65, 65| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(25) },
	{ bp_usNetLed1, 0,      1,  1 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 0,      10, 10| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 0,      18, 18| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 0,      26, 26| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 0,      41, 41| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(1) },
	{ bp_usNetLed1, 0,      50, 50| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(10) },
	{ bp_usNetLed1, 0,      58, 58| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(18) },
	{ bp_usNetLed1, 0,      66, 66| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(26) },
	{ bp_usNetLed2, 0,      2,  2 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 0,      11, 11| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 0,      19, 19| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 0,      27, 27| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 0,      42, 42| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(2) },
	{ bp_usNetLed2, 0,      51, 51| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(11) },
	{ bp_usNetLed2, 0,      59, 59| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(19) },
	{ bp_usNetLed2, 0,      67, 67| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(27) },
	{ bp_usNetLed3, 0,      3,  3 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed3, 0,      12, 12| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed3, 0,      20, 20| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed3, 0,      28, 28| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed3, 0,      43, 43| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(3) },
	{ bp_usNetLed3, 0,      52, 52| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(12) },
	{ bp_usNetLed3, 0,      60, 60| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(20) },
	{ bp_usNetLed3, 0,      68, 68| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(28) },
	{ bp_usNetLed0, 1,      4,  4 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 1,      13, 13| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 1,      21, 21| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 1,      29, 29| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed0, 1,      44, 44| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(4) },
	{ bp_usNetLed0, 1,      53, 53| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(13) },
	{ bp_usNetLed0, 1,      61, 61| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(21) },
	{ bp_usNetLed0, 1,      69, 69| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(29) },
	{ bp_usNetLed1, 1,      5,  5 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 1,      14, 14| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 1,      22, 22| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 1,      33, 33| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed1, 1,      45, 45| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(5) },
	{ bp_usNetLed1, 1,      54, 54| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(14) },
	{ bp_usNetLed1, 1,      62, 62| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(22) },
	{ bp_usNetLed2, 1,      6,  6 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 1,      15, 15| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 1,      23, 23| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 1,      34, 34| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed2, 1,      46, 46| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(6) },
	{ bp_usNetLed2, 1,      55, 55| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(15) },
	{ bp_usNetLed2, 1,      63, 63| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(23) },
	{ bp_usNetLed3, 1,      7,  7 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed3, 1,      16, 16| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed3, 1,      24, 24| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
	{ bp_usNetLed3, 1,      47, 47| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(7) },
	{ bp_usNetLed3, 1,      56, 56| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(16) },
	{ bp_usNetLed3, 1,      64, 64| BP_PINMUX_VAL_2 | BP_PINMUX_HWLED| BP_PINMUX_OPTLED_NUM(24) },
	
  
	{ bp_usSerialLedData, -1, 0,  0 | BP_PINMUX_VAL_1},
	{ bp_usSerialLedClk, -1, 1,  1 | BP_PINMUX_VAL_1 },
	{ bp_usSerialLedMask, -1, 2,  2 | BP_PINMUX_VAL_1 },

	{ bp_usUartSdin, 0, 70, 70 | BP_PINMUX_VAL_1},
	{ bp_usUartSdout, 0, 71, 71 | BP_PINMUX_VAL_1},
	{ bp_usUartSdin, 1, 5, 5 | BP_PINMUX_VAL_1},    /* HS UART */
	{ bp_usUartSdout, 1, 6, 6 | BP_PINMUX_VAL_1},
	{ bp_usUartRts, 1, 4, 4 | BP_PINMUX_VAL_1},
	{ bp_usUartCts, 1, 3, 3 | BP_PINMUX_VAL_1},
	{ bp_usUartSdin, 2, 26, 26 | BP_PINMUX_VAL_3},    /* DDR UART */
	{ bp_usUartSdout, 2, 25, 25 | BP_PINMUX_VAL_3},

	{ bp_usI2sSclk,	-1, 11, 11|BP_PINMUX_VAL_1 },
	{ bp_usI2sLrck,	-1, 12, 12|BP_PINMUX_VAL_1 },
	{ bp_usI2sRxSdata, -1, 13, 13|BP_PINMUX_VAL_1 },
	{ bp_usI2sTxSdata, -1, 15, 15|BP_PINMUX_VAL_1 },
	{ bp_usI2sMclk,	-1, 14, 14|BP_PINMUX_VAL_1 },

	{ bp_usGpioI2cSda, 0, 16, 16 | BP_PINMUX_VAL_1 },
	{ bp_usGpioI2cScl, 0, 17, 17 | BP_PINMUX_VAL_1 },

	{ bp_usPcmSdin,-1, 22, 22 | BP_PINMUX_VAL_1},
	{ bp_usPcmSdout,-1, 23, 23 | BP_PINMUX_VAL_1},
	{ bp_usPcmClk, -1, 24, 24 | BP_PINMUX_VAL_1},
	{ bp_usPcmFs, -1, 25, 25 | BP_PINMUX_VAL_1},
	
	{ bp_usPcmSdin,-1, 11, 11 | BP_PINMUX_VAL_1},
	{ bp_usPcmSdout,-1, 12, 12 | BP_PINMUX_VAL_1},
	{ bp_usPcmClk, -1, 13, 13 | BP_PINMUX_VAL_1},
	{ bp_usPcmFs, -1, 14, 14 | BP_PINMUX_VAL_1},

	{ bp_usSfpSigDetect, -1, 26, 26 | BP_PINMUX_VAL_1 },

	{ bp_usUsbPwrFlt0,	-1, 79, 79 | BP_PINMUX_VAL_1 },
	{ bp_usUsbPwrOn0,	-1, 80, 80 | BP_PINMUX_VAL_1 },
	{ bp_usUsbPwrFlt1,	-1, 81, 81 | BP_PINMUX_VAL_1 },
	{ bp_usUsbPwrOn1,	-1, 82, 82 | BP_PINMUX_VAL_1 },

	{ bp_usMiiMdc,	-1, 68, 68 | BP_PINMUX_VAL_1},
	{ bp_usMiiMdio,	-1, 69, 69 | BP_PINMUX_VAL_1},

	{ bp_usSpiSlaveSelectNum, 0, 75, 75 | BP_PINMUX_VAL_1},
	{ bp_usSpiSlaveSelectNum, 1, 76, 76 | BP_PINMUX_VAL_1},
	{ bp_usSpiSlaveSelectNum, 2, 10, 10 | BP_PINMUX_VAL_1},
	{ bp_usSpiSlaveSelectNum, 3, 9, 9 | BP_PINMUX_VAL_1},
	{ bp_usSpiSlaveSelectNum, 4, 8, 8 | BP_PINMUX_VAL_1},
	{ bp_usSpiSlaveSelectNum, 5, 7, 7 | BP_PINMUX_VAL_1},

	{ bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_4 }, // ALL SW GPIOs use pinmux 5
	{ bp_ReservedAnyLed,  -1, -1,  BP_PINMUX_VAL_2 }, // ALL LEDs use pinmux 2
	{ bp_ReservedAnyLed,  -1, 40, 40 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(0) },
	{ bp_ReservedAnyLed,  -1, 41, 41 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(1) },
	{ bp_ReservedAnyLed,  -1, 42, 42 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(2) },
	{ bp_ReservedAnyLed,  -1, 43, 43 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(3) },
	{ bp_ReservedAnyLed,  -1, 44, 44 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(4) },
	{ bp_ReservedAnyLed,  -1, 45, 45 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(5) },
	{ bp_ReservedAnyLed,  -1, 46, 46 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(6) },
	{ bp_ReservedAnyLed,  -1, 47, 47 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(7) },
	{ bp_ReservedAnyLed,  -1, 48, 48 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(8) },
	{ bp_ReservedAnyLed,  -1, 49, 49 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(9) },
	{ bp_ReservedAnyLed,  -1, 50, 50 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(10) },
	{ bp_ReservedAnyLed,  -1, 51, 51 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(11) },
	{ bp_ReservedAnyLed,  -1, 52, 52 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(12) },
	{ bp_ReservedAnyLed,  -1, 53, 53 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(13) },
	{ bp_ReservedAnyLed,  -1, 54, 54 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(14) },
	{ bp_ReservedAnyLed,  -1, 55, 55 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(15) },
	{ bp_ReservedAnyLed,  -1, 56, 56 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(16) },
	{ bp_ReservedAnyLed,  -1, 57, 57 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(17) },
	{ bp_ReservedAnyLed,  -1, 58, 58 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(18) },
	{ bp_ReservedAnyLed,  -1, 59, 59 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(19) },
	{ bp_ReservedAnyLed,  -1, 60, 60 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(20) },
	{ bp_ReservedAnyLed,  -1, 61, 61 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(21) },
	{ bp_ReservedAnyLed,  -1, 62, 62 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(22) },
	{ bp_ReservedAnyLed,  -1, 63, 63 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(23) },
	{ bp_ReservedAnyLed,  -1, 64, 64 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(24) },
	{ bp_ReservedAnyLed,  -1, 65, 65 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(25) },
	{ bp_ReservedAnyLed,  -1, 66, 66 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(26) },
	{ bp_ReservedAnyLed,  -1, 67, 67 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(27) },
	{ bp_ReservedAnyLed,  -1, 68, 68 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(28) },
	{ bp_ReservedAnyLed,  -1, 69, 69 | BP_PINMUX_VAL_2|BP_PINMUX_OPTLED_NUM(29) },

	{ bp_usGpioWlanReserved,  -1, 0, 0 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 1, 1 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 2, 2 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 3, 3 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 4, 4 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 5, 5 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 6, 6 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 7, 7 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 8, 8 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,  -1, 9, 9 | BP_PINMUX_VAL_0 },

	{ bp_usGpioWlanReserved,   -1, 10, 10 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 11, 11 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 12, 12 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 13, 13 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 14, 14 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 15, 15 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 16, 16 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 17, 17 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 22, 22 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 23, 23 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 24, 24 | BP_PINMUX_VAL_3 },
	{ bp_usGpioWlanReserved,   -1, 30, 30 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved1,  -1, 30, 30 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,   -1, 31, 31 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved1,  -1, 31, 31 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,   -1, 32, 32 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved1,  -1, 32, 32 | BP_PINMUX_VAL_0 },
	{ bp_usGpioWlanReserved,   -1, 33, 33 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,   -1, 34, 34 | BP_PINMUX_VAL_1 },

	/* wlan fem_ctrl */
	{ bp_usGpioWlanReserved,  -1, 84, 84 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 85, 85 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 86, 86 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 87, 87 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 88, 88 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 89, 89 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 90, 90 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 91, 91 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 92, 92 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 93, 93 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 94, 94 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 95, 95 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 96, 96 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 97, 97 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 98, 98 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 99, 99 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 100, 100 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 101, 101 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 102, 102 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 103, 103 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 104, 104 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 105, 105 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 106, 106 | BP_PINMUX_VAL_1 },
	{ bp_usGpioWlanReserved,  -1, 107, 107 | BP_PINMUX_VAL_1 },
	{ bp_last, -1, -1,  0 }
};


bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

int g_pinmux_fn_defs_size = sizeof(g_pinmux_fn_defs);
