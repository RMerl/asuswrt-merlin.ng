/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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

