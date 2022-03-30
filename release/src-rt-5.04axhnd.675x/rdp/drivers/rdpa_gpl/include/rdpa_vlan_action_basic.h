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


#ifndef _RDPA_VLAN_ACTION_BASIC_H_
#define _RDPA_VLAN_ACTION_BASIC_H_

#ifdef XRDP
#if !defined(BCM6878)
#define RDPA_MAX_VLAN_ACTION        129  /**< Max number of VLAN actions per direction */
#else
#define RDPA_MAX_VLAN_ACTION        33  /**< Max number of VLAN actions per direction */
#endif
#define RDPA_DROP_ACTION RDPA_MAX_VLAN_ACTION
#else
#define RDPA_MAX_VLAN_ACTION        128  /**< Max number of VLAN actions per direction */
#define RDPA_DROP_ACTION RDPA_MAX_VLAN_ACTION
#endif

/** Max number of tags supported by vlan_action */
#define RDPA_VLAN_MAX_TAGS          2

/** Outer tag index */
#define RDPA_VLAN_TAG_OUT   0 
/** Inner tag index */
#define RDPA_VLAN_TAG_IN    1 

#define RDPA_VLAN_ACTION_TPID_DONT_CARE (0xffff)

/* default transparent vlan action entrance */
#define RDPA_DS_TRANSPARENT_VLAN_ACTION (RDPA_MAX_VLAN_ACTION - 1)
#define RDPA_US_TRANSPARENT_VLAN_ACTION (RDPA_MAX_VLAN_ACTION - 1)

#endif /* _RDPA_VLAN_ACTION_BASIC_H_ */

