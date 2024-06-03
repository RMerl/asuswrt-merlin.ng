/*
   Copyright (c) 2022 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2022:DUAL/GPL:standard

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

/*
 *  Created on: Apr 2022
 *      Author: senyan.huang@broadcom.com
 */

#ifndef __PHY_DRV_PSP_H__
#define __PHY_DRV_PSP_H__
#include "board.h"

#define PSP_KEY_SILENT_START            "silent-start"
#define PSP_KEY_SILENT_START_ENABLE     "enable"

static inline int phy_drv_psp_silent_start_enable(void)
{
    char buf[16];

    if ((kerSysScratchPadGet(PSP_KEY_SILENT_START, buf, sizeof(buf)) > 0) && !strncmp(buf, PSP_KEY_SILENT_START_ENABLE, sizeof(buf)))
        return 1;
    else
        return 0;
}
#endif
