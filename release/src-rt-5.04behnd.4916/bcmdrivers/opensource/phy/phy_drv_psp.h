/*
   Copyright (c) 2022 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2022:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
