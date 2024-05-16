/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
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

/*
*******************************************************************************
* File Name  : rnr_flow.c
*
* Description: This file contains the Broadcom Tcp Speed Test Runner Flow Implementation.
*
*  Created on: Dec 6, 2016
*      Author: yonatani, ilanb
*******************************************************************************
*/

#include "rnr_flow.h"
#include "tcpspdtest.h"

static int find_netdev_mac(bdmf_mac_t *pmac, struct net_device *dev)
{
    while (dev)
    {
        if (!memcmp(dev->dev_addr, pmac, sizeof(bdmf_mac_t)))
            return 1;

        dev = netdev_path_next_dev(dev);
    }

    return 0;
}

/* if dev == NULL get the first system host mac address.
   if dev != NULL search for the netdev mac addr in all system host macs. if not found search
   in the parent netdev until root.
*/
int tcpspd_rnr_flow_get_host_mac(bdmf_mac_t *pmac, struct net_device *dev)
{
    int found = 0;
    bdmf_index index = BDMF_INDEX_UNASSIGNED;
    bdmf_object_handle system_object = NULL;

    if (rdpa_system_get(&system_object))
    {
        tc_err("get system_object object failed\n");
        return -1;
    }

    while (rdpa_system_host_mac_address_table_get_next(system_object, &index) == BDMF_ERR_OK)
    {
        if (rdpa_system_host_mac_address_table_get(system_object, index, pmac)) /* Sanity, cannot fail */
            continue; 
        if (dev && !find_netdev_mac(pmac, dev))
            continue;

        tc_debug("system_object: found routed_mac %d\n", (int)index);
        found = 1;
        break;
    }

    if (system_object)
        bdmf_put(system_object);

    return found ? 0 : -1;
}

