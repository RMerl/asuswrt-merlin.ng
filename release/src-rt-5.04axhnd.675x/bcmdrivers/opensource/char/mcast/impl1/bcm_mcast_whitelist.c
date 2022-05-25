/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

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

#include "bcm_mcast_priv.h"
#include "bcm_mcast_whitelist.h"

#if defined(CC_MCAST_WHITELIST_SUPPORT)
#include <rdpa_api.h>

bcm_mcast_whitelist_add_hook_t bcm_mcast_whitelist_add_fn = bcm_mcast_whitelist_add;
bcm_mcast_whitelist_delete_hook_t bcm_mcast_whitelist_delete_fn = bcm_mcast_whitelist_delete;
static bdmf_object_handle mcast_whitelist_class = NULL;

static void bcm_mcast_build_whitelist_key(bcm_mcast_whitelist_info_t *pWhitelistInfo, 
                                          rdpa_mcast_whitelist_t *mcast_wlist)
{
    uint16_t vlan_id;

    if ( pWhitelistInfo->grp.is_ipv4 )
    {
        mcast_wlist->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
        mcast_wlist->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
        if (pWhitelistInfo->is_ssm) 
        {
            /* Use source to lookup only for SSM flows */
            mcast_wlist->mcast_group.l3.src_ip.addr.ipv4 = ntohl(pWhitelistInfo->src.ipv4_addr);
        }
        else
        {
            mcast_wlist->mcast_group.l3.src_ip.addr.ipv4 = 0;
        }
        mcast_wlist->mcast_group.l3.gr_ip.addr.ipv4 = ntohl(pWhitelistInfo->grp.ipv4_addr);
    }
    else
    {
        mcast_wlist->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
        mcast_wlist->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
        if (pWhitelistInfo->is_ssm) 
        {
            /* Use source to lookup only for SSM flows */
            memcpy(mcast_wlist->mcast_group.l3.src_ip.addr.ipv6.data, pWhitelistInfo->src.ipv6_addr.p8, 16);
        }
        else
        {
            memset(mcast_wlist->mcast_group.l3.src_ip.addr.ipv6.data, 0, 16); 
        }
        memcpy(mcast_wlist->mcast_group.l3.gr_ip.addr.ipv6.data, pWhitelistInfo->grp.ipv6_addr.p8, 16);
    }
    vlan_id = pWhitelistInfo->outer_vlanid;
    mcast_wlist->vid = vlan_id & 0xFFF;
}

int bcm_mcast_whitelist_add(bcm_mcast_whitelist_info_t *pWhitelistInfo,
                            whitelist_key_t            *whitelist_hdl)
{
    rdpa_mcast_whitelist_t mcast_wlist;
    int rc = 0;
    bdmf_index rdpa_index = 0;

    if ( pWhitelistInfo->grp.is_ipv4 )
    {
        __logDebug("is_ssm %d grp %pI4 src %pI4 vid %u",
                   pWhitelistInfo->is_ssm,
                   &pWhitelistInfo->grp.ipv4_addr, 
                   &pWhitelistInfo->src.ipv4_addr,
                   pWhitelistInfo->outer_vlanid );
    }
    else
    {
        __logDebug("is_ssm %d grp %pI6 src %pI6 vid %u",
                   pWhitelistInfo->is_ssm,
                   &pWhitelistInfo->grp.ipv6_addr,
                   &pWhitelistInfo->src.ipv6_addr,
                   pWhitelistInfo->outer_vlanid );
    }
    if (!whitelist_hdl) 
    {
        __logError("whitelist_hdl arg NULL");
        return -1;
    }

    if (mcast_whitelist_class == NULL)
        return -1;

    memset(&mcast_wlist, 0, sizeof(mcast_wlist));
    bcm_mcast_build_whitelist_key(pWhitelistInfo, &mcast_wlist);

    bdmf_lock();

    rc = rdpa_mcast_whitelist_entry_add(mcast_whitelist_class, &rdpa_index, &mcast_wlist);
    if (rc)
    {
        bdmf_unlock();
        __logError("Could not rdpa_mcast_whitelist_entry_add, rc = %d", rc);
        return -1;
    }

    bdmf_unlock();
    *whitelist_hdl = (whitelist_key_t)rdpa_index;

    return 0;
}

int bcm_mcast_whitelist_delete(whitelist_key_t whitelist_hdl)
{

    int rc = 0;
    bdmf_index rdpa_index = (bdmf_index)whitelist_hdl;

    if (mcast_whitelist_class == NULL)
        return -1;

    __logDebug("whitelist hdl = %u", whitelist_hdl);
    bdmf_lock();

    rc = rdpa_mcast_whitelist_entry_delete(mcast_whitelist_class, rdpa_index);
    if (rc)
    {
        __logError("Cannot rdpa_mcast_whitelist_entry_delete, rc = %d\n", rc);
    }

    bdmf_unlock();

    return rc;
}

int bcm_mcast_whitelist_init()
{
   int rc;
   BDMF_MATTR(mcast_whitelist_attrs, rdpa_mcast_whitelist_drv());

   rc = rdpa_mcast_whitelist_get(&mcast_whitelist_class);
   if (rc)
   {
       rc = bdmf_new_and_set(rdpa_mcast_whitelist_drv(), NULL, mcast_whitelist_attrs, &mcast_whitelist_class);
       if (rc)
       {
           __logError("rdpa mcast_whitelist class object does not exist and can't be created.\n");
           return rc;
       }
   }

   return rc;
}

void bcm_mcast_whitelist_exit()
{
   if (mcast_whitelist_class)
   {
       bdmf_destroy(mcast_whitelist_class);
   }

}

#else

bcm_mcast_whitelist_add_hook_t bcm_mcast_whitelist_add_fn = NULL;
bcm_mcast_whitelist_delete_hook_t bcm_mcast_whitelist_delete_fn = NULL;

int bcm_mcast_whitelist_add(bcm_mcast_whitelist_info_t *pWhitelistInfo,
                            whitelist_key_t            *whitelist_hdl)
{
    return 0;
}

int bcm_mcast_whitelist_delete(whitelist_key_t whitelist_hdl)
{
    return 0;
}

int bcm_mcast_whitelist_init()
{
   return 0;
}

void bcm_mcast_whitelist_exit()
{
}
#endif
