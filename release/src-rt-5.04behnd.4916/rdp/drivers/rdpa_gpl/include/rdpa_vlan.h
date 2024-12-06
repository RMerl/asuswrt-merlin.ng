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


#ifndef _RDPA_VLAN_H_
#define _RDPA_VLAN_H_

/** \defgroup vlan VLAN Container Object
 * VLAN container object contains a list of VIDs supported by
 * a given port in a given bridge.\n
 * The object supports multiple VIDs with individual vlan_action per VID.\n
 * \n
 * A VLAN object must have a port object parent.\n
 * Once created, VLAN object is attached to the appropriate bridge using \ref bdmf_link "link" operation.
 * @{
 */

#if !defined(XRDP)
/* Number of AGGR WAN VLANS is derived from number of bridges since we support single
 * wan vid per Aggregatating Bridge 
 */
#define RDPA_MAX_VLANS                  128     /**< Max number of VIDs */
#define RDPA_MAX_AGGR_WAN_VLANS         4       /**< Max number of aggregated WAN VIDs */
#endif

/** Mac lookup configuration for VLAN object.
 * Underlying type for mac_lookup_cfg aggregate
 */
typedef struct
{
    bdmf_boolean sal_enable; /**< Do SA lookup */
    bdmf_boolean dal_enable; /**< Do DA lookup */
    rdpa_forward_action sal_miss_action; /**< SA lookup miss action */
    rdpa_forward_action dal_miss_action; /**< DA lookup miss action */
} rdpa_mac_lookup_cfg_t;

/** @} end of vlan Doxygen group */

#endif /* _RDPA_VLAN_H_ */
