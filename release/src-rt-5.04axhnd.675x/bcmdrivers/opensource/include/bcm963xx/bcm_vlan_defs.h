/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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


#ifndef _BCM_VLAN_DEFS_H_
#define _BCM_VLAN_DEFS_H_

/*
 * Macros, type definitions
 */

#define BCM_VLAN_FILTER_FLAGS_IS_UNICAST   0x0001
#define BCM_VLAN_FILTER_FLAGS_IS_MULTICAST 0x0002
#define BCM_VLAN_FILTER_FLAGS_IS_BROADCAST 0x0004

#define BCM_VLAN_FILTER_FLAGS_ALL               \
    ( BCM_VLAN_FILTER_FLAGS_IS_UNICAST   |      \
      BCM_VLAN_FILTER_FLAGS_IS_MULTICAST |      \
      BCM_VLAN_FILTER_FLAGS_IS_BROADCAST )

#define BCM_VLAN_PBITS_MASK  0xE000
#define BCM_VLAN_PBITS_SHIFT 13
#define BCM_VLAN_CFI_MASK    0x1000
#define BCM_VLAN_CFI_SHIFT   12
#define BCM_VLAN_VID_MASK    0x0FFF
#define BCM_VLAN_VID_SHIFT   0

#endif /* _BCM_VLAN_DEFS_H_ */

