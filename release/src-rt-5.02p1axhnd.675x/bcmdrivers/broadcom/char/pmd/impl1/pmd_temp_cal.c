/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:proprietary:standard 

   Copyright (c) 2013 Broadcom 
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
 ------------------------------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <bcm_OS_Deps.h>
#include <linux/bcm_log.h>
#include "pmd.h"
#include "pmd_temp_cal.h"

/* This file holds the resistant to temperature  converting table
 * for Vishay thermistor part number - NTHS0603N01N1003FE.
 * To use different thermistor change the below defines (TEMP_TABLE_SIZE and TEMP_TABLE_LOWEST_TEMP)
 * and the pmd_res_temp_conv table according to the new part used.
 */

/* temperature converting function for thermistor  manufactured by Vishay part number is - NTHS0603N01N1003FE */
const uint32_t default_pmd_res_temp_conv[TEMP_TABLE_SIZE] = {3372700, 3156000, 2954700, 2767500, 2593300, 2431200,2280200, 2139500,
        2008400, 1886100, 1772100,1665600, 1566200, 1473300, 1386600, 1305400, 1229500, 1158500, 1092000, 1029800,
        971400, 916700, 865500, 817400, 772200, 729900, 690100, 652700, 617600, 584500, 553500, 524200, 496700, 470800,
        446400, 423400, 401700, 381300, 362000, 343800.0, 326600.0, 310400.0, 295100, 280600, 266900, 254000,241800, 230200,
        219200, 208900, 199000, 189700, 180900, 172600, 164700, 157100, 150000, 143200,136800, 130700, 124900, 119400,
        114200, 109200, 104500, 100000, 95720, 91650, 87770, 84080, 80550, 77210, 74020, 70980, 68080, 65310, 62670, 60150,
        57740, 55450, 53250, 51160, 49160, 47250, 45430, 43680, 42010, 40410, 38880, 37420, 36020, 34680, 33400, 32170, 30990,
        29870, 28780, 27750, 26750, 25800, 24880, 24010, 23170, 22360, 21580, 20840, 20120, 19440, 18780, 18140, 17530, 16950,
        16380, 15840, 15320,  14820, 14330, 13870, 13420, 12990, 12570, 12180, 11790, 11420, 11060, 10720, 10390, 10070, 9759,
        9461, 9174, 8897, 8630, 8372, 8123, 7882, 7650, 7426, 7209, 7000, 6798, 6602, 6413, 6231, 6054, 5884, 5719, 5559, 5404,
        5255, 5110, 4970, 4835, 4704, 4577, 4454, 4335, 4219, 4108, 3999, 3894, 3793, 3694, 3598, 3506, 3416};

const uint16_t default_pmd_temp_apd_conv[APD_TEMP_TABLE_SIZE] = {0};
