/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

#define phy_is_crossbar(phy)    ((phy) && (phy)->phy_drv->phy_type == PHY_TYPE_CROSSBAR)
#define get_active_phy(phy)     (phy_is_crossbar(phy)? crossbar_phy_dev_active(phy):(phy)? cascade_phy_get_last_active(phy):phy)
 
#endif

