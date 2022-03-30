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

#ifndef __PHY_DRV_CROSSBAR_H__
#define __PHY_DRV_CROSSBAR_H__

#define MAX_PHYS_PER_CROSSBAR_GROUP 5
#define MAX_CROSSBAR_GROUPS 5

typedef int (*select_cb_t)(int crossbar_id, int internal_endpoint, int external_endpoint, phy_dev_t *phy_dev);
phy_dev_t *phy_drv_crossbar_group_alloc(int crossbar_id, int internal_endpoint, select_cb_t select);
int phy_drv_crossbar_group_phy_add(phy_dev_t *phy_dev_crossbar, phy_dev_t *phy_dev, int external_endpoint);
int phy_drv_crossbar_group_phy_del(phy_dev_t *phy_dev_crossbar, phy_dev_t *phy_dev);
int crossbar_group_external_endpoint_count(phy_dev_t *phy_dev_crossbar, int *external_map);
phy_dev_t *crossbar_group_phy_get(phy_dev_t *phy_dev_crossbar, int external_endpoint);
int crossbar_set_active_external_endpoint(int crossbar_id, int internal_endpoint, int external_endpoint);

phy_dev_t *crossbar_group(int crossbar_id, int internal_endpoint);
//phy_dev_t *crossbar_group_phy_by_indices(int crossbar_id, int internal_endpoint, int external_endpoint);
int crossbar_external_endpoint(phy_dev_t *phy_dev);
int crossbar_info_by_phy(phy_dev_t *phy_dev, int *crossbar_id, int *internal_endpoint, int *external_endpoint);

phy_dev_t *crossbar_phy_dev_first(phy_dev_t *phy_dev_crossbar);
phy_dev_t *crossbar_phy_dev_next(phy_dev_t *phy_dev);

#endif

