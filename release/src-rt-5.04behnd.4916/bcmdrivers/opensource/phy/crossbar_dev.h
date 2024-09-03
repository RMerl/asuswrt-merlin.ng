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

static inline phy_dev_t * get_active_phy(phy_dev_t *phy)
{
    if (phy_is_crossbar(phy))
        phy = crossbar_phy_dev_active(phy);
    return (phy)? cascade_phy_get_last_active(phy):phy;
}

static inline int phy_is_mac_to_mac(phy_dev_t *phy_dev)
{
    if (phy_is_crossbar(phy_dev))
    {
        phy_dev = crossbar_phy_dev_first(phy_dev);
        if (phy_dev == NULL)    /* Empty PHY under a crossbar port by phy-crossbar move in run time */
            return 0;
    }
 
    phy_dev = get_active_phy(phy_dev);
    if(phy_dev && phy_dev->phy_drv && phy_dev->phy_drv->phy_type == PHY_TYPE_MAC2MAC)
        return 1;
    return 0;
}

#endif

