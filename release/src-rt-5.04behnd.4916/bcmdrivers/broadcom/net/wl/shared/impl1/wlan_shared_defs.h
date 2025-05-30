/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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

#ifndef __WLAN_SHARED_DEFS_H__
#define __WLAN_SHARED_DEFS_H__

#if IS_ENABLED(CONFIG_BCM_ARCHER)
#include "bcm_wlan_defs.h"
#endif

/*
 * netdev_path hw_port related defines for wlan
 */
#if !defined(WLAN_NETDEVPATH_HWPORT)
/* re-defined for compatibility to old releases */

#if !defined(WL_NUM_OF_SSID_PER_UNIT)
#if defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)
#define WL_NUM_OF_SSID_PER_UNIT                   16
#else
#define WL_NUM_OF_SSID_PER_UNIT                   8
#endif /* CONFIG_BCM_PON_XRDP || CONFIG_BCM963158 */
#endif /* !WL_NUM_OF_SSID_PER_UNIT */


#if defined(WL_NUM_OF_SSID_PER_UNIT)
/* wlan hw_port is unique to the radio. hw_port = subunit */
#define WLAN_NETDEVPATH_HWPORT(unit, ssid)  ((ssid) + (unit) * WL_NUM_OF_SSID_PER_UNIT)
#define WLAN_NETDEVPATH_SSID(hw_port)       ((hw_port) % WL_NUM_OF_SSID_PER_UNIT)

#else /* !WL_NUM_OF_SSID_PER_UNIT  */

/*
 * hw_port is unique within a radio.
 * hw_port = ssid
 */
#define WLAN_NETDEVPATH_HWPORT(unit, ssid)  (ssid)
#define WLAN_NETDEVPATH_SSID(hw_port)       (hw_port)

#endif /* !WL_NUM_OF_SSID_PER_UNIT */

#endif /* !WLAN_NETDEVPATH_HWPORT */

#define WLAN_WFD_INVALID_IDX                -1
#define WLAN_WFD_DISABLE_IDX                -2
#define WLAN_WFD_ENABLED(idx)               ((idx) > WLAN_WFD_INVALID_IDX)
#define WLAN_WFD_DISABLED(idx)              ((idx) == WLAN_WFD_DISABLE_IDX)

/*
 * Maximum number of radios (DHD + NIC) supported by the system.  Also maximum
 * number of PCIe slots (external and internal) possible.
 *
 * Note:
 *  Maximum number of radios accelerated and/or offloaded may be less than
 *  this number
 */
#define WLAN_RADIOS_MAX                     4

#endif /* !__WLAN_SHARED_DEFS_H__ */
