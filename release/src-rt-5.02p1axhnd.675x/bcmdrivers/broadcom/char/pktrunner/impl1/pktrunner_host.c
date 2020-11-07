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

#include <rdpa_api.h>
#include "fcachehw.h"
#include "pktrunner_proto.h"


/*******************************************************************************
 *
 * Forward Declarations
 *
 *******************************************************************************/

#if defined(CONFIG_BCM_CSO)
#if defined(CONFIG_IPV6)
static int runner_ip_class_inet6addr_event(struct notifier_block *this, unsigned long event, void *ptr);
#endif
static int runner_ip_class_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr);
#endif

/*******************************************************************************
 *
 * Global Variables and Definitions
 *
 *******************************************************************************/


extern bdmf_object_handle system_obj;
extern bdmf_object_handle ip_class;
#if defined(CONFIG_BCM_CSO)

#if defined(CONFIG_IPV6)
static struct notifier_block runner_ip_class_inet6addr_notifier = {
    .notifier_call = runner_ip_class_inet6addr_event,
};
#endif

static struct notifier_block runner_ip_class_inetaddr_notifier = {
    .notifier_call = runner_ip_class_inetaddr_event,
};
#endif


/*******************************************************************************
 *
 * Local Functions
 *
 *******************************************************************************/


#if defined(CONFIG_BCM_CSO)
#if defined(CONFIG_IPV6)
static int runner_ip_class_inet6addr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct inet6_ifaddr *ifa = (struct inet6_ifaddr *)ptr;
    int ret;
    bdmf_index index=0;
    bdmf_ipv6_t ipv6_host_address;

    memcpy(&ipv6_host_address, &ifa->addr, 16);

    switch (event) {
    case NETDEV_UP:
        protoDebug("Adding IPv6 host address %pI6", &ifa->addr);
        ret = rdpa_system_ipv6_host_address_table_add(system_obj, &index, &ipv6_host_address);
        if(ret != 0)
        {
            protoError("Could not rdpa_system_ipv6_host_address_table_add ret=%d", ret);
        }

        break;
    case NETDEV_DOWN:
        protoDebug("Removing IPv6 host address %pI6", &ifa->addr);
        ret = rdpa_system_ipv6_host_address_table_find(system_obj, &index, &ipv6_host_address);
        if(ret != 0)
        {
            protoError("Could not rdpa_system_ipv6_host_address_table_find ret=%d", ret);
        }
        else
        {
            ret = rdpa_system_ipv6_host_address_table_delete(system_obj, index);
            if(ret != 0)
            {
                protoError("Could not rdpa_system_ipv6_host_address_table_delete ret=%d", ret);
            }
        }

        break;
    }

    return NOTIFY_DONE;
}
#endif

static int runner_ip_class_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;
    int ret;
    bdmf_index index=0;
    bdmf_ipv4 ipv4_host_address;

    ipv4_host_address = (bdmf_ipv4)be32_to_cpu(ifa->ifa_local);

    switch (event) {
    case NETDEV_UP:
        protoDebug("Adding IPv4 host address %pI4", &ifa->ifa_local);
        ret = rdpa_system_ipv4_host_address_table_add(system_obj, &index, ipv4_host_address);
        if(ret != 0)
        {
            protoError("Could not rdpa_system_ipv4_host_address_table_add ret=%d", ret);
        }
        break;

    case NETDEV_DOWN:
        protoDebug("Removing IPv4 host address %pI4", &ifa->ifa_local);
        ret = rdpa_system_ipv4_host_address_table_find(system_obj, &index, &ipv4_host_address);
        if(ret != 0)
        {
            protoError("Could not rdpa_system_ipv4_host_address_table_find ret=%d", ret);
        }
        else
        {
            ret = rdpa_system_ipv4_host_address_table_delete(system_obj, index);
            if(ret != 0)
            {
                protoError("Could not rdpa_system_ipv4_host_address_table_delete ret=%d", ret);
            }
        }
        break;
    }
    
    return NOTIFY_DONE;
}
#endif

static int runner_ip_class_tcp_ack_prio_set(uint32_t enable)
{
    return rdpa_ip_class_tcp_ack_prio_set(ip_class, enable);
}

int __init runnerHost_construct(void)
{
#if defined(CONFIG_BCM_CSO)
    register_inetaddr_notifier(&runner_ip_class_inetaddr_notifier);
#if defined(CONFIG_IPV6)
    register_inet6addr_notifier(&runner_ip_class_inet6addr_notifier);
#endif
#endif

    blog_tcp_ack_mflows_set_fn = (blog_tcp_ack_mflows_set_t)runner_ip_class_tcp_ack_prio_set;
    runner_ip_class_tcp_ack_prio_set(blog_support_get_tcp_ack_mflows());

    return 0;
}

void __exit runnerHost_destruct(void)
{
    blog_tcp_ack_mflows_set_fn = (blog_tcp_ack_mflows_set_t)NULL;

#if defined(CONFIG_BCM_CSO)
    unregister_inetaddr_notifier(&runner_ip_class_inetaddr_notifier);
#if defined(CONFIG_IPV6)
    unregister_inet6addr_notifier(&runner_ip_class_inet6addr_notifier);
#endif
#endif
}
