// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHY_DRV_LPORT_H__
#define __PHY_DRV_LPORT_H__

#include "phy_drv.h"

int lport_link_change_register(phy_dev_t *phy_dev);
int lport_link_change_unregister(phy_dev_t *phy_dev);

#endif
