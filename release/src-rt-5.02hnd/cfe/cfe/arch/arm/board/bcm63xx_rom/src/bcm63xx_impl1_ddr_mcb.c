/*  *********************************************************************
    *
    <:copyright-BRCM:2012:proprietary:standard
    
       Copyright (c) 2012 Broadcom 
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
    ********************************************************************* */

#include "lib_types.h"
#include "bcm63xx_impl1_ddr_cinit.h"
#include "boardparms.h"

#include "mcb/mcb_63138Ax_400MHz_16b_dev4Gx16_DDR3-800-E.h"
#include "mcb/mcb_63138Ax_533MHz_16b_dev1Gx16_DDR3-1066G.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev1Gx16_DDR3-1600K.h"
#include "mcb/mcb_63138Ax_533MHz_16b_dev2Gx16_DDR3-1066G.h"
#include "mcb/mcb_63138Ax_533MHz_16b_dev1Gx8_DDR3-1066G.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev2Gx16_DDR3-1600K.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev1Gx8_DDR3-1600K.h"
#include "mcb/mcb_63138Ax_533MHz_16b_dev4Gx16_DDR3-1066G.h"
#include "mcb/mcb_63138Ax_533MHz_16b_dev2Gx8_DDR3-1066G.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev4Gx16_DDR3-1600K.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev2Gx8_DDR3-1600K.h"
#include "mcb/mcb_63138Ax_533MHz_16b_dev4Gx8_DDR3-1066G.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev4Gx8_DDR3-1600K.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev2Gx16_DDR3-1600K_HT_ASR.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev2Gx16_DDR3-1600K_HT_SRT.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev4Gx16_DDR3-1600K_HT_ASR.h"
#include "mcb/mcb_63138Ax_800MHz_16b_dev4Gx16_DDR3-1600K_HT_SRT.h"

#include "mcb/mcb_63138A_800MHz_16b_dev2Gx8_DDR3-1600K_ssc_p5per.h"
#include "mcb/mcb_63138A_800MHz_16b_dev2Gx16_DDR3-1600K_ssc_p5per.h"
#include "mcb/mcb_63138A_800MHz_16b_dev4Gx8_DDR3-1600K_ssc_p5per.h"
#include "mcb/mcb_63138A_800MHz_16b_dev4Gx16_DDR3-1600K_ssc_p5per.h"


/* define an array of supported MCBs. The first one must be the safe mode MCB. */
mcbindex MCB[] = {
    /* safe mode MCB, use largest supported size  */
    { 
        BP_DDR_SPEED_400_6_6_6 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_16, 
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_400MHz_16b_dev4Gx16_DDR3_800_E
    },
    { 
        BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_128MB | BP_DDR_DEVICE_WIDTH_16, 
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_533MHz_16b_dev1Gx16_DDR3_1066G
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_128MB| BP_DDR_DEVICE_WIDTH_16,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_800MHz_16b_dev1Gx16_DDR3_1600K
    },
    { 
        BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_256MB | BP_DDR_DEVICE_WIDTH_16, 
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_533MHz_16b_dev2Gx16_DDR3_1066G
    },
    { 
        BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_256MB | BP_DDR_DEVICE_WIDTH_8, 
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_533MHz_16b_dev1Gx8_DDR3_1066G
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_256MB| BP_DDR_DEVICE_WIDTH_16,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_800MHz_16b_dev2Gx16_DDR3_1600K
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_256MB| BP_DDR_DEVICE_WIDTH_8,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_800MHz_16b_dev1Gx8_DDR3_1600K
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_256MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TEMP_EXTENDED_SRT,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_TEMP_MASK,  
        mcb_63138Ax_800MHz_16b_dev2Gx16_DDR3_1600K_HT_SRT
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_256MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TEMP_EXTENDED_ASR,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_TEMP_MASK,  
        mcb_63138Ax_800MHz_16b_dev2Gx16_DDR3_1600K_HT_ASR
    },
    { 
        BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_16, 
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_533MHz_16b_dev4Gx16_DDR3_1066G
    },
    { 
        BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_8,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_533MHz_16b_dev2Gx8_DDR3_1066G
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_16,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_800MHz_16b_dev4Gx16_DDR3_1600K
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB| BP_DDR_DEVICE_WIDTH_8,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_800MHz_16b_dev2Gx8_DDR3_1600K
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TEMP_EXTENDED_SRT,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_TEMP_MASK,  
        mcb_63138Ax_800MHz_16b_dev4Gx16_DDR3_1600K_HT_SRT
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TEMP_EXTENDED_ASR,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_TEMP_MASK,  
        mcb_63138Ax_800MHz_16b_dev4Gx16_DDR3_1600K_HT_ASR
    },
    { 
        BP_DDR_SPEED_533_8_8_8 | BP_DDR_TOTAL_SIZE_1024MB | BP_DDR_DEVICE_WIDTH_8,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_533MHz_16b_dev4Gx8_DDR3_1066G
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_1024MB | BP_DDR_DEVICE_WIDTH_8,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK,  
        mcb_63138Ax_800MHz_16b_dev4Gx8_DDR3_1600K
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_256MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_SSC_CONFIG_2,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_SSC_CONFIG_MASK,
        mcb_63138A_800MHz_16b_dev2Gx16_DDR3_1600K_ssc_p5per
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_8 | BP_DDR_SSC_CONFIG_2,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_SSC_CONFIG_MASK,
        mcb_63138A_800MHz_16b_dev2Gx8_DDR3_1600K_ssc_p5per
    }, 
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_512MB | BP_DDR_DEVICE_WIDTH_16 | BP_DDR_SSC_CONFIG_2,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_SSC_CONFIG_MASK,
        mcb_63138A_800MHz_16b_dev4Gx16_DDR3_1600K_ssc_p5per 
    },
    { 
        BP_DDR_SPEED_800_11_11_11 | BP_DDR_TOTAL_SIZE_1024MB | BP_DDR_DEVICE_WIDTH_8 | BP_DDR_SSC_CONFIG_2,
        BP_DDR_SPEED_MASK | BP_DDR_TOTAL_SIZE_MASK | BP_DDR_DEVICE_WIDTH_MASK | BP_DDR_SSC_CONFIG_MASK,
        mcb_63138A_800MHz_16b_dev4Gx8_DDR3_1600K_ssc_p5per
    },
    {
        0,0,NULL
    }
};


