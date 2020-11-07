/*
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
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
* File Name  : ptkrunner_host.c
*
* Description: Management of Host MAC Addresses
*
*******************************************************************************
*/

#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#if defined(CONFIG_IPV6)
#include <net/ipv6.h>
#endif
#include <net/addrconf.h>
#include "fcachehw.h"

#include <rdpa_api.h>

#include "pktrunner_proto.h"

/*******************************************************************************
 *
 * Forward Declarations
 *
 *******************************************************************************/

#if defined(CONFIG_BCM_CSO)
#if defined(CONFIG_IPV6)
static int runnerUcast_inet6addr_event(struct notifier_block *this, unsigned long event, void *ptr);
#endif
static int runnerUcast_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr);
#endif

/*******************************************************************************
 *
 * Global Variables and Definitions
 *
 *******************************************************************************/

typedef enum {
    PKTRUNNER_OK                                        = 0,
    PKTRUNNER_ERROR_NOENT                               = 1,
    PKTRUNNER_ERROR_NORES                               = 2,
    PKTRUNNER_ERROR_ENTRY_EXISTS                        = 3,
    PKTRUNNER_ERROR_HOST_MAC_INVALID                    = 4,
    PKTRUNNER_ERROR_MAX
} PKTRUNNER_ERROR_t;

extern bdmf_object_handle ucast_class;
#if defined(CONFIG_BCM_CSO)
bdmf_object_handle system_obj;

#if defined(CONFIG_IPV6)
static struct notifier_block runnerUcast_inet6addr_notifier = {
    .notifier_call = runnerUcast_inet6addr_event,
};
#endif

static struct notifier_block runnerUcast_inetaddr_notifier = {
    .notifier_call = runnerUcast_inetaddr_event,
};
#endif

/*******************************************************************************
 *
 * Local Functions
 *
 *******************************************************************************/

#if defined(CONFIG_BCM_CSO)
#if defined(CONFIG_IPV6)
static int runnerUcast_inet6addr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct inet6_ifaddr *ifa = (struct inet6_ifaddr *)ptr;
    int ret;
    bdmf_index index=0;
    bdmf_ipv6_t ipv6_host_address;

    memcpy(&ipv6_host_address, &ifa->addr, 16);

    switch (event) {
    case NETDEV_UP:
        __logDebug("Adding IPv6 host address %pI6", &ifa->addr);
        ret = rdpa_system_ipv6_host_address_table_add(system_obj, &index, &ipv6_host_address);
        if(ret != 0)
        {
            __logError("Could not rdpa_system_ipv6_host_address_table_add ret=%d", ret);
        }

        break;
    case NETDEV_DOWN:
        __logInfo("Removing IPv6 host address %pI6", &ifa->addr);
        ret = rdpa_system_ipv6_host_address_table_find(system_obj, &index, &ipv6_host_address);
        if(ret != 0)
        {
            __logError("Could not rdpa_system_ipv6_host_address_table_find ret=%d", ret);
        }
        else
        {
            ret = rdpa_system_ipv6_host_address_table_delete(system_obj, index);
            if(ret != 0)
            {
                __logError("Could not rdpa_system_ipv6_host_address_table_delete ret=%d", ret);
            }
        }

        break;
    }

    return NOTIFY_DONE;
}
#endif

static int runnerUcast_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;
    int ret;
    bdmf_index index=0;
    bdmf_ipv4 ipv4_host_address;

    ipv4_host_address = (bdmf_ipv4)be32_to_cpu(ifa->ifa_local);

    switch (event) {
    case NETDEV_UP:
        __logDebug("Adding IPv4 host address %pI4", &ifa->ifa_local);
        ret = rdpa_system_ipv4_host_address_table_add(system_obj, &index, ipv4_host_address);
        if(ret != 0)
        {
            __logError("Could not rdpa_system_ipv4_host_address_table_add ret=%d", ret);
        }
        break;

    case NETDEV_DOWN:
        __logDebug("Removing IPv4 host address %pI4", &ifa->ifa_local);
        ret = rdpa_system_ipv4_host_address_table_find(system_obj, &index, &ipv4_host_address);
        if(ret != 0)
        {
            __logError("Could not rdpa_system_ipv4_host_address_table_find ret=%d", ret);
        }
        else
        {
            ret = rdpa_system_ipv4_host_address_table_delete(system_obj, index);
            if(ret != 0)
            {
                __logError("Could not rdpa_system_ipv4_host_address_table_delete ret=%d", ret);
            }
        }
        break;
    }
    
    return NOTIFY_DONE;
}
#endif

int runnerUcast_add_host_mac(char *mac_p)
{
    int ret;
    bdmf_index index=0;
    bdmf_mac_t zero_mac = {};
    rdpa_host_mac_address_table_t host_mac_entry;

    __logDebug("fhw add host MAC <%pM>\n", mac_p);

    memcpy(&host_mac_entry.host_mac_address.b[0], mac_p, sizeof(bdmf_mac_t));
    host_mac_entry.reference_count = 0; /*Reference Count is managed by RDPA layer*/

    if (!memcmp(&host_mac_entry.host_mac_address.b[0], &zero_mac.b[0], sizeof(bdmf_mac_t)))
        return PKTRUNNER_ERROR_HOST_MAC_INVALID;

    __logInfo("Adding host MAC <%pM>\n", host_mac_entry.host_mac_address.b);

    ret = rdpa_ucast_host_mac_address_table_add(ucast_class, &index, &host_mac_entry);
    if (ret != 0)
        __logError("Could not rdpa_ucast_host_mac_address_table_add\n");

    return ret;
}

int runnerUcast_delete_host_mac(char *mac_p)
{
    int ret;
    bdmf_index index=0;
    rdpa_host_mac_address_table_t host_mac_entry;

    __logDebug("fhw delete host MAC <%pM>\n", mac_p);

    memcpy(&host_mac_entry.host_mac_address.b[0], mac_p, sizeof(bdmf_mac_t));
    host_mac_entry.reference_count = 0; /*Reference Count is managed by RDPA layer*/

    __logInfo("Removing host MAC <%pM>\n", host_mac_entry.host_mac_address.b);

    ret = rdpa_ucast_host_mac_address_table_find(ucast_class, &index, &host_mac_entry);
    if (ret != 0)
    {
        __logError("Could not rdpa_ucast_host_mac_address_table_find\n");
    }
    else
    {
        ret = rdpa_ucast_host_mac_address_table_delete(ucast_class, index);
        if (ret != 0)
            __logError("Could not rdpa_ucast_host_mac_address_table_delete\n");
    }

    return ret;
}

static int runnerUcast_fc_accel_mode_set( uint32_t accel_mode )
{
    int ret;
    rdpa_fc_global_cfg_t    rdpa_fc_global_cfg;

    ret = rdpa_ucast_fc_global_cfg_get(ucast_class, &rdpa_fc_global_cfg);

    if (ret == 0)
    {
        rdpa_fc_global_cfg.fc_accel_mode = (uint8_t) accel_mode;
        ret = rdpa_ucast_fc_global_cfg_set(ucast_class, &rdpa_fc_global_cfg);
    }

    return ret;
}

static int runnerUcast_fc_tcp_ack_mflows_set( uint32_t enable )
{
    int ret;
    rdpa_fc_global_cfg_t    rdpa_fc_global_cfg;

    ret = rdpa_ucast_fc_global_cfg_get(ucast_class, &rdpa_fc_global_cfg);

    if (ret == 0)
    {
        rdpa_fc_global_cfg.fc_tcp_ack_mflows = (uint8_t) enable;
        ret = rdpa_ucast_fc_global_cfg_set(ucast_class, &rdpa_fc_global_cfg);
    }

    return ret;
}

int __init runnerHost_construct(void)
{
#if defined(CONFIG_BCM_CSO)
    rdpa_system_get(&system_obj);

    register_inetaddr_notifier(&runnerUcast_inetaddr_notifier);
#if defined(CONFIG_IPV6)
    register_inet6addr_notifier(&runnerUcast_inet6addr_notifier);
#endif
#endif

    /* bind to acceleration mode function hook used by blog/flow_cache */
    blog_accel_mode_set_fn = (blog_accel_mode_set_t) runnerUcast_fc_accel_mode_set;
    blog_tcp_ack_mflows_set_fn = (blog_tcp_ack_mflows_set_t) runnerUcast_fc_tcp_ack_mflows_set;

    /* Set the Runner acceleration mode to be in sync with blog/flow cache */
    runnerUcast_fc_accel_mode_set( blog_support_get_accel_mode() );

    /* Set the Runner TCP ack priority flow to be in sync with blog/flow cache */
    runnerUcast_fc_tcp_ack_mflows_set( blog_support_get_tcp_ack_mflows() );

    bcm_print("Initialized Runner Host Layer\n");

    return 0;
}

void __exit runnerHost_destruct(void)
{
#if defined(CONFIG_BCM_CSO)
    unregister_inetaddr_notifier(&runnerUcast_inetaddr_notifier);
#if defined(CONFIG_IPV6)
    unregister_inet6addr_notifier(&runnerUcast_inet6addr_notifier);
#endif

    bdmf_put(system_obj);
#endif
    blog_accel_mode_set_fn = (blog_accel_mode_set_t) NULL;
    blog_tcp_ack_mflows_set_fn = (blog_tcp_ack_mflows_set_t) NULL;
}
