/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

#ifndef __WLAN_SHARED_DEFS_H__
#define __WLAN_SHARED_DEFS_H__

/* Enable after svn addition
#include "bcm_wlan_defs.h"
*/
/*
 * netdev_path hw_port related defines for wlan
 */
#if !defined(WLAN_NETDEVPATH_HWPORT)
/* re-defined for compatibility to old releases */
#if !defined(WL_NUM_OF_SSID_PER_UNIT) && defined(CONFIG_BCM_PON)
#define WL_NUM_OF_SSID_PER_UNIT                   8
#endif /* !WL_NUM_OF_SSID_PER_UNIT && CONFIG_BCM_PON */

#if defined(WL_NUM_OF_SSID_PER_UNIT)
/* wlan hw_port is unique to the radio. hw_port = subunit */
#define WLAN_NETDEVPATH_HWPORT(unit, ssid)  ((ssid) + (unit) * WL_NUM_OF_SSID_PER_UNIT)
#define WLAN_NETDEVPATH_SSID(hw_port)       ((hw_port) % WL_NUM_OF_SSID_PER_UNIT )

#else /* !WL_NUM_OF_SSID_PER_UNIT  */

/*
 * hw_port is unique within a radio.
 * hw_port = ssid
 */
#define WLAN_NETDEVPATH_HWPORT(unit, ssid)  (ssid)
#define WLAN_NETDEVPATH_SSID(hw_port)       (hw_port)

#endif /* !WL_NUM_OF_SSID_PER_UNIT */

#endif /* !WLAN_NETDEVPATH_HWPORT */

#endif /* !__WLAN_SHARED_DEFS_H__ */
