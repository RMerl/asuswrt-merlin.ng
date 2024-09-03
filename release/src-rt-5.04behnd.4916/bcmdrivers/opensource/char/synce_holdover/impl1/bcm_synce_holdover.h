/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#ifndef _BCM_SYNCE_HOLDOVER_H_
#define _BCM_SYNCE_HOLDOVER_H_

#include <phy_drv.h>

#if defined(CONFIG_BCM_SYNCE_HOLDOVER) || defined(CONFIG_BCM_SYNCE_HOLDOVER_MODULE)
extern void synce_holdover_wan_linkup_atomic_handler(void);
extern void synce_holdover_wan_linkdown_atomic_handler(void);
extern void synce_holdover_ae_linkup_atomic_handler(void);
#else
static inline void synce_holdover_wan_linkup_atomic_handler(void) {}
static inline void synce_holdover_wan_linkdown_atomic_handler(void) {}
static inline void synce_holdover_ae_linkup_atomic_handler(void) {}
#endif

#endif
