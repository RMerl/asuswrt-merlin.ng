/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

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
