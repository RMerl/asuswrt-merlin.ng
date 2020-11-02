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

/*
 *  Created on: Jul 2017
 *      Author: ido.brezel@broadcom.com
 */

#ifndef __CROSSBAR_DEV_H__
#define __CROSSBAR_DEV_H__

#include "phy_drv_crossbar.h"
#include "crossbar_dev_plat.h"

static inline phy_dev_t *crossbar_phy_dev_get(int unit, int port)
{
    const sw_port_t *sp;
    int id, endpoint;
    phy_dev_t *phy_dev_crossbar;

    /* loop thru crossbars */
    for (id = 0, sp = crossbar_plat_int_endpoints[id]; id < MAX_CROSSBARS && sp->unit != -1; id++)
    {   /* loop thru crossbar internal end points */
        for (endpoint = 0; endpoint < MAX_CROSSBAR_INT_ENDPOINTS && sp->unit != -1; endpoint++, sp++)
        {
            if (sp->unit == unit && sp->port == port)
            {
                if ((phy_dev_crossbar = crossbar_group(id, endpoint)))
                    return phy_dev_crossbar;

                return phy_drv_crossbar_group_alloc(id, endpoint, &crossbar_plat_select);
            }
        }
    }

    return NULL;
}

static inline int crossbar_finalize(void)
{
    return crossbar_plat_finalize();
}

static inline int crossbar_phy_add(phy_dev_t *phy_dev_crossbar, phy_dev_t *phy_dev, int external_endpoint)
{
    return phy_drv_crossbar_group_phy_add(phy_dev_crossbar, phy_dev, external_endpoint);
}

static inline int crossbar_phy_del(phy_dev_t *phy_dev_crossbar, phy_dev_t *phy_dev)
{
    return phy_drv_crossbar_group_phy_del(phy_dev_crossbar, phy_dev);
}

phy_dev_t *crossbar_phy_dev_active(phy_dev_t *phy_dev_crossbar);
int crossbar_current_status(phy_dev_t *phy_crossbar, int *internal_endpoint, int *external_endpoint);

#define phy_is_crossbar(phy)    ((phy) && (phy)->phy_drv->phy_type == PHY_TYPE_CROSSBAR)
#define get_active_phy(phy)     (phy_is_crossbar(phy)? crossbar_phy_dev_active(phy):(phy)? cascade_phy_get_last_active(phy):phy)
 
#endif

