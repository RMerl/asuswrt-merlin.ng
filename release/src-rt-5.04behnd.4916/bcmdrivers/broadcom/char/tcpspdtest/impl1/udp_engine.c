/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
*/

#include <bdmf_system.h>
#include <bdmf_system_common.h>
#include <rdpa_api.h>
#include "tcpspdtest.h"

static bdmf_object_handle bdmf_udpspdtest_obj_h = NULL;

int udpspd_engine_init(void)
{
    int rc = 0;

#ifdef CONFIG_BCM_XRDP
    if (bdmf_udpspdtest_obj_h)
        return -1;

    rc = rdpa_udpspdtest_get(&bdmf_udpspdtest_obj_h);
    if (rc)
    {
        rc = bdmf_new_and_set(rdpa_udpspdtest_drv(), NULL, NULL, &bdmf_udpspdtest_obj_h);
        if (rc)
        {
            tc_err("Failed to create bdmf udpspdtest\n");
            return rc;
        }
    }

#endif    
    return rc;
}

int udpspd_engine_shutdown(void)
{
#ifdef CONFIG_BCM_XRDP
    if (!bdmf_udpspdtest_obj_h)
        return -1;

    bdmf_destroy(bdmf_udpspdtest_obj_h);
    bdmf_udpspdtest_obj_h = NULL;
#endif
    return 0;
}

