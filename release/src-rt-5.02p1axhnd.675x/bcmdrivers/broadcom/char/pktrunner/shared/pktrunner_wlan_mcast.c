/*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/*
*******************************************************************************
*
* File Name  : pktrunner_wlan_mcast.c
*
* Description: Runner WLAN Multicast Flows
*
*******************************************************************************
*/

#if defined(RDP_SIM)
#include "pktrunner_rdpa_sim.h"
#else
#include <linux/module.h>

#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/blog_rule.h>

#include "linux/bcm_skb_defines.h"
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <net/ipv6.h>
#include "fcachehw.h"

#include "bcmtypes.h"
#include "bcm_vlan.h"
#endif

#include <rdpa_api.h>

#include "pktrunner_wlan_mcast.h"

#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)

static int wlan_mcast_class_created_here = 0;
static bdmf_object_handle wlan_mcast_class = NULL;

static blogRuleAction_t *__find_blog_rule_command(blogRule_t *blogRule_p,
                                                  blogRuleCommand_t blogRuleCommand,
                                                  uint32 *cmdIndex_p)
{
    blogRuleAction_t *action_p;
    int i;

    for(i=*cmdIndex_p; i<blogRule_p->actionCount; ++i)
    {
        action_p = &blogRule_p->action[i];

        if(action_p->cmd == blogRuleCommand)
        {
            *cmdIndex_p = i;

            return action_p;
        }
    }

    return NULL;
}

static int __ssid_add(uint16_t *ssid_vector_p, uint8_t ssid)
{
    uint16_t ssid_mask = 1 << ssid;

    if(*ssid_vector_p & ssid_mask)
    {
        __logError("SSID %d already joined the group (0x%04X)", ssid, *ssid_vector_p);

        return BDMF_ERR_PARM;
    }

    *ssid_vector_p |= ssid_mask;

    return BDMF_ERR_OK;
}

static int __ssid_delete(uint16_t *ssid_vector_p, uint8_t ssid)
{
    uint16_t ssid_mask = 1 << ssid;

    if(!(*ssid_vector_p & ssid_mask))
    {
        __logError("SSID %d did not join the group (0x%04X)", ssid, *ssid_vector_p);

        return BDMF_ERR_PARM;
    }

    *ssid_vector_p &= ~ssid_mask;

    return BDMF_ERR_OK;
}

static int __wlan_mcast_blog_xlate(Blog_t *blog_p, rdpa_wlan_mcast_fwd_table_t *fwd_table_p,
                                   rdpa_wlan_mcast_dhd_station_t *dhd_station_p, int add,
                                   rdpa_wlan_mcast_ssid_mac_address_t *ssid_mac_address_p)
{
    int ret = BDMF_ERR_OK;

    if(blog_p->wfd.mcast.is_wfd)
    {
        __logDebug("WLAN MCAST ADD: WFD");

        fwd_table_p->wfd_tx_priority = SKBMARK_GET_Q_PRIO(blog_p->mark);

        switch(blog_p->wfd.mcast.wfd_idx)
        {
            case 0:
                if(add)
                {
                    fwd_table_p->wfd_0_priority = blog_p->wfd.mcast.wfd_prio;
                    ret = __ssid_add(&fwd_table_p->wfd_0_ssid_vector, blog_p->wfd.mcast.ssid);
                }
                else
                {
                    ret = __ssid_delete(&fwd_table_p->wfd_0_ssid_vector, blog_p->wfd.mcast.ssid);
                }
                break;

            case 1:
                if(add)
                {
                    fwd_table_p->wfd_1_priority = blog_p->wfd.mcast.wfd_prio;
                    ret = __ssid_add(&fwd_table_p->wfd_1_ssid_vector, blog_p->wfd.mcast.ssid);
                }
                else
                {
                    ret = __ssid_delete(&fwd_table_p->wfd_1_ssid_vector, blog_p->wfd.mcast.ssid);
                }
                break;

            case 2:
                if(add)
                {
                    fwd_table_p->wfd_2_priority = blog_p->wfd.mcast.wfd_prio;
                    ret = __ssid_add(&fwd_table_p->wfd_2_ssid_vector, blog_p->wfd.mcast.ssid);
                }
                else
                {
                    ret = __ssid_delete(&fwd_table_p->wfd_2_ssid_vector, blog_p->wfd.mcast.ssid);
                }
                break;

            default:
                return BDMF_ERR_PARM;
        }
    }
    else
    {
        blogRuleAction_t *blogRuleAction_p;
        blogRule_t *blogRule_p;
        uint32 cmdIndex = 0;

        __logDebug("WLAN MCAST ADD: RNR");

        blogRule_p = blog_p->blogRule_p;
        if(blogRule_p == NULL)
        {
            __logError("Blog Rule is NULL");

            return BDMF_ERR_PARM;
        }

        blogRuleAction_p = __find_blog_rule_command(blogRule_p, BLOG_RULE_CMD_SET_STA_MAC_ADDRESS, &cmdIndex);
        if(blogRuleAction_p == NULL)
        {
            __logError("Missing STA MAC Address");

            return BDMF_ERR_PARM;
        }

        memcpy(dhd_station_p->mac_address.b, blogRuleAction_p->macAddr, ETH_ALEN);

        dhd_station_p->radio_index = blog_p->rnr.radio_idx;
        dhd_station_p->ssid = blog_p->rnr.ssid;
        dhd_station_p->flowring_index = blog_p->rnr.flowring_idx;
        dhd_station_p->tx_priority = blog_p->rnr.priority;

        blogRuleAction_p = __find_blog_rule_command(blogRule_p, BLOG_RULE_CMD_SET_MAC_SA, &cmdIndex);
        if(blogRuleAction_p)
        {
            fwd_table_p->is_proxy_enabled = 1;

            ssid_mac_address_p->radio_index = blog_p->rnr.radio_idx;
            ssid_mac_address_p->ssid = blog_p->rnr.ssid;
            memcpy(ssid_mac_address_p->mac_address.b, blogRuleAction_p->macAddr, ETH_ALEN);
        }
    }

    return ret;
}

int pktrunner_wlan_mcast_add(Blog_t *blog_p, bdmf_index *fwd_table_index_p)
{
    rdpa_wlan_mcast_fwd_table_t fwd_table;
    rdpa_wlan_mcast_dhd_station_t dhd_station;
    rdpa_wlan_mcast_ssid_mac_address_t ssid_mac_address;
    bdmf_index ssid_mac_address_index;
    int ret;

    if(*fwd_table_index_p == RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID)
    {
        /* New FWD table entry */

        memset(&fwd_table, 0, sizeof(rdpa_wlan_mcast_fwd_table_t));

        ret = __wlan_mcast_blog_xlate(blog_p, &fwd_table, &dhd_station, 1, &ssid_mac_address);
        if(ret)
        {
            return ret;
        }

        if(!blog_p->wfd.mcast.is_wfd)
        {
            ret = rdpa_wlan_mcast_dhd_station_add(wlan_mcast_class, &fwd_table.dhd_station_index, &dhd_station);
            if (ret)
            {
                __logError("Could not rdpa_wlan_mcast_dhd_station_add");
                
                return ret;
            }

            if(fwd_table.is_proxy_enabled)
            {
                ret = rdpa_wlan_mcast_ssid_mac_address_add(wlan_mcast_class, &ssid_mac_address_index, &ssid_mac_address);
                if (ret)
                {
                    __logError("Could not rdpa_wlan_mcast_ssid_mac_address_add");
                
                    return ret;
                }
            }
        }
        else
        {
            fwd_table.dhd_station_index = RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID;
        }

        ret = rdpa_wlan_mcast_fwd_table_add(wlan_mcast_class, fwd_table_index_p, &fwd_table);
        if (ret)
        {
            __logError("Could not rdpa_wlan_mcast_fwd_table_add");

            if(fwd_table.dhd_station_index != RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID)
            {
                rdpa_wlan_mcast_dhd_station_delete(wlan_mcast_class, fwd_table.dhd_station_index);

                if(!blog_p->wfd.mcast.is_wfd && fwd_table.is_proxy_enabled)
                {
                    rdpa_wlan_mcast_ssid_mac_address_delete(wlan_mcast_class, ssid_mac_address_index);
                }
            }
        }

        return ret;
    }
    else
    {
        /* Existing FWD table entry */

        ret = rdpa_wlan_mcast_fwd_table_get(wlan_mcast_class, *fwd_table_index_p, &fwd_table);
        if(ret)
        {
            __logError("Could not rdpa_wlan_mcast_fwd_table_get");
            return ret;
        }

        ret = __wlan_mcast_blog_xlate(blog_p, &fwd_table, &dhd_station, 1, &ssid_mac_address);
        if(ret)
        {
            return ret;
        }

        if(!blog_p->wfd.mcast.is_wfd)
        {
            /* test if the requested dhd_station already exists in requested fwd_table. if yes, ignore add request(double add).
                    ** rdpa_wlan_mcast_fwd_table_set with old "fwd_table.dhd_station_count", indicate just test the existence, but never change fwd_table.
                    */
            if ((rdpa_wlan_mcast_dhd_station_find(wlan_mcast_class, &fwd_table.dhd_station_index, &dhd_station) == BDMF_ERR_OK) &&
               (rdpa_wlan_mcast_fwd_table_set(wlan_mcast_class, *fwd_table_index_p, &fwd_table) == BDMF_ERR_ALREADY))
            {               
                __logInfo("Ignore add request. dhd_station[%ld] already exists in fwd_table[%ld]", fwd_table.dhd_station_index, *fwd_table_index_p);
                return BDMF_ERR_OK;
            }
        
            ret = rdpa_wlan_mcast_dhd_station_add(wlan_mcast_class, &fwd_table.dhd_station_index, &dhd_station);
            if (ret)
            {
                __logError("Could not rdpa_wlan_mcast_dhd_station_add");
                
                return ret;
            }

            fwd_table.dhd_station_count++;

            if(fwd_table.is_proxy_enabled)
            {
                ret = rdpa_wlan_mcast_ssid_mac_address_add(wlan_mcast_class, &ssid_mac_address_index, &ssid_mac_address);
                if (ret)
                {
                    __logError("Could not rdpa_wlan_mcast_ssid_mac_address_add");
                
                    return ret;
                }
            }
        }
        else
        {
            fwd_table.dhd_station_index = RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID;
        }

        ret = rdpa_wlan_mcast_fwd_table_set(wlan_mcast_class, *fwd_table_index_p, &fwd_table);
        if (ret)
        {
            __logError("Could not rdpa_wlan_mcast_fwd_table_set");

            if(fwd_table.dhd_station_index != RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID)
            {
                rdpa_wlan_mcast_dhd_station_delete(wlan_mcast_class, fwd_table.dhd_station_index);

                if(!blog_p->wfd.mcast.is_wfd && fwd_table.is_proxy_enabled)
                {
                    rdpa_wlan_mcast_ssid_mac_address_delete(wlan_mcast_class, ssid_mac_address_index);
                }
            }
        }

        return ret;
    }
}

int pktrunner_wlan_mcast_delete(Blog_t *blog_p, bdmf_index *fwd_table_index_p)
{
    rdpa_wlan_mcast_fwd_table_t fwd_table;
    rdpa_wlan_mcast_dhd_station_t dhd_station;
    rdpa_wlan_mcast_ssid_mac_address_t ssid_mac_address;
    int ret;

    ret = rdpa_wlan_mcast_fwd_table_get(wlan_mcast_class, *fwd_table_index_p, &fwd_table);
    if(ret)
    {
        __logError("Could not rdpa_wlan_mcast_fwd_table_get");
                
        return ret;
    }

    ret = __wlan_mcast_blog_xlate(blog_p, &fwd_table, &dhd_station, 0, &ssid_mac_address);
    if(ret)
    {
        __logError("Could not __wlan_mcast_blog_xlate");
 
        return ret;
    }

    if(!blog_p->wfd.mcast.is_wfd)
    {
        if(!fwd_table.dhd_station_count)
        {
            __logError("No DHD Stations to be deleted from FWD Table index %lu", *fwd_table_index_p);
                
            return ret;
        }

        ret = rdpa_wlan_mcast_dhd_station_find(wlan_mcast_class, &fwd_table.dhd_station_index, &dhd_station);
        if (ret)
        {
            if (ret == BDMF_ERR_NOENT)
            {
                __logNotice("dhd_station[%ld] not exist!", fwd_table.dhd_station_index);
            }
            else
            {
                __logError("Could not rdpa_wlan_mcast_dhd_station_find");
            }
                
            return ret;
        }

        /* test if the requested dhd_station remains in requested fwd_table, if not, ignore del request(double delete).
             ** rdpa_wlan_mcast_fwd_table_set with old "fwd_table.dhd_station_count", indicate just test the existence, but never change fwd_table.
             */
        if (rdpa_wlan_mcast_fwd_table_set(wlan_mcast_class, *fwd_table_index_p, &fwd_table) == BDMF_ERR_NOENT)
        {               
            __logInfo("Ignore del request. dhd_station[%ld] doesn't exist in fwd_table[%ld]", fwd_table.dhd_station_index, *fwd_table_index_p);
            return BDMF_ERR_OK;
        }

        ret = rdpa_wlan_mcast_dhd_station_delete(wlan_mcast_class, fwd_table.dhd_station_index);
        if (ret)
        {
            __logError("Could not rdpa_wlan_mcast_dhd_station_delete");
                
            return ret;
        }

        if(fwd_table.is_proxy_enabled)
        {
            bdmf_index ssid_mac_address_index =
                RDPA_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_INDEX(ssid_mac_address.radio_index,
                                                             ssid_mac_address.ssid);

            rdpa_wlan_mcast_ssid_mac_address_delete(wlan_mcast_class, ssid_mac_address_index);
        }

        fwd_table.dhd_station_count--;
    }
    else
    {
        fwd_table.dhd_station_index = RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID;
    }

    if(!fwd_table.wfd_0_ssid_vector &&
       !fwd_table.wfd_1_ssid_vector &&
       !fwd_table.wfd_2_ssid_vector &&
       !fwd_table.dhd_station_count)
    {
        ret = rdpa_wlan_mcast_fwd_table_delete(wlan_mcast_class, *fwd_table_index_p);

        *fwd_table_index_p = RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID;

        return ret;
    }
    else
    {
        return rdpa_wlan_mcast_fwd_table_set(wlan_mcast_class, *fwd_table_index_p, &fwd_table);
    }
}

/*
*******************************************************************************
* Function   : pktrunner_wlan_mcast_construct
* Description: Constructs the Runner WLAN Multicast layer
*******************************************************************************
*/
int __init pktrunner_wlan_mcast_construct(void)
{
    int ret;

    BDMF_MATTR(wlan_mcast_attrs, rdpa_wlan_mcast_drv());

    ret = rdpa_wlan_mcast_get(&wlan_mcast_class);
    if (ret)
    {
        ret = bdmf_new_and_set(rdpa_wlan_mcast_drv(), NULL, wlan_mcast_attrs, &wlan_mcast_class);
        if (ret)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "rdpa wlan_mcast_class object does not exist and can't be created.\n");

            return ret;
        }

        wlan_mcast_class_created_here = 1;
    }

    return 0;
}

/*
*******************************************************************************
* Function   : pktrunner_wlan_mcast_destruct
* Description: Destructs the Runner WLAN Multicast layer
*******************************************************************************
*/
void __exit pktrunner_wlan_mcast_destruct(void)
{
    if(wlan_mcast_class)
    {
        if(wlan_mcast_class_created_here)
        {
            bdmf_destroy(wlan_mcast_class);

            wlan_mcast_class_created_here = 0;
        }
        else
        {
            bdmf_put(wlan_mcast_class);
        }
    }
}
