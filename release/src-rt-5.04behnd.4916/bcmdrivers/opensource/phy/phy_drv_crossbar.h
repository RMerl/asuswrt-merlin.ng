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

