/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#include "drivers_common_ag.h"
#include "top_scratch_ag.h"
int ag_drv_top_scratch_scratch_set(uint32_t data)
{
    uint32_t reg_atch=0;

#ifdef VALIDATE_PARMS
#endif

    reg_atch = RU_FIELD_SET(0, TOP_SCRATCH, ATCH, DATA, reg_atch, data);

    RU_REG_WRITE(0, TOP_SCRATCH, ATCH, reg_atch);

    return 0;
}

int ag_drv_top_scratch_scratch_get(uint32_t *data)
{
    uint32_t reg_atch=0;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOP_SCRATCH, ATCH, reg_atch);

    *data = RU_FIELD_GET(0, TOP_SCRATCH, ATCH, DATA, reg_atch);

    return 0;
}

