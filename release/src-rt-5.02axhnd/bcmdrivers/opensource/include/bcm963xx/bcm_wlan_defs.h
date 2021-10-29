/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom 
   All Rights Reserved

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
#ifndef _BCM_WLAN_DEFS_H_
#define _BCM_WLAN_DEFS_H_
#include "rdpa_types.h"

#define WLAN_INV_RADIO_UNIT                 0xFFFFFFFF

#if defined(WL_NUM_OF_SSID_PER_UNIT)

/* wlan hw_port is unique to the radio. hw_port = subunit */
#define WLAN_RADIO_GET(hw_port)             (uint32_t)((hw_port) / WL_NUM_OF_SSID_PER_UNIT)
#define WLAN_SSID_GET(hw_port)              ((hw_port) % WL_NUM_OF_SSID_PER_UNIT)
#define WLAN_NETDEVPATH_HWPORT(unit, ssid)  ((ssid) + (unit) * WL_NUM_OF_SSID_PER_UNIT)
#define WLAN_NETDEVPATH_SSID(hw_port)       WLAN_SSID_GET((hw_port))

#else /* !WL_NUM_OF_SSID_PER_UNIT */

#define WLAN_RADIO_GET(hw_port)             (uint32_t)(WLAN_INV_RADIO_UNIT)
#define WLAN_SSID_GET(hw_port)              (hw_port)
#define WLAN_NETDEVPATH_HWPORT(unit, ssid)  (ssid)
#define WLAN_NETDEVPATH_SSID(hw_port)       WLAN_SSID_GET((hw_port))
#endif /* !WL_NUM_OF_SSID_PER_UNIT */

#endif

