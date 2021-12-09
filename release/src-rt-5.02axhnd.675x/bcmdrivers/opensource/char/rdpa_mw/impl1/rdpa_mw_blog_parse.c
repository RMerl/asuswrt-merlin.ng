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

#if defined(CONFIG_BLOG)

#include "bcm_OS_Deps.h"
#include "rdpa_mw_blog_parse.h"
#include "bcmenet_common.h"
#include "rdpa_mw_vlan.h"
#include "rdpa_mw_qos.h"
#include "bcm_wlan_defs.h"

int rdpa_mw_set_mcast_dscp_remark = -1;
EXPORT_SYMBOL(rdpa_mw_set_mcast_dscp_remark);

rdpa_if rdpa_mw_root_dev2rdpa_if(struct net_device *root_dev)
{
    uint32_t hw_port, hw_port_type, physical_hw_port;

    hw_port = netdev_path_get_hw_port(root_dev);
    /* In case of external switch netdev_path_get_hw_port return logical port and not HW port.  
       It is assumed that hw port cvalues are 0-7 and logical port values are 8-15*/
    physical_hw_port = LOGICAL_PORT_TO_PHYSICAL_PORT(hw_port);

    hw_port_type = netdev_path_get_hw_port_type(root_dev);

    switch (hw_port_type)
    {
    case BLOG_SIDPHY:
        return rdpa_if_lan0 + hw_port;
    case BLOG_ENETPHY:
        return rdpa_port_map_from_hw_port(physical_hw_port, 1);
    case BLOG_WLANPHY:
#ifdef XRDP
        return rdpa_if_wlan0 + WLAN_RADIO_GET(hw_port);
#else
        return rdpa_if_ssid0 + hw_port;
#endif
    case BLOG_GPONPHY:
        return rdpa_wan_type_to_if(rdpa_wan_gpon);
    case BLOG_EPONPHY:        
        return rdpa_wan_type_to_if(rdpa_wan_epon);
    case BLOG_NETXLPHY:
        return rdpa_if_wlan0 + (hw_port & 0xff);
    case BLOG_NOPHY:
        break;
    default:
        BCM_LOG_ERROR(BCM_LOG_ID_RDPA, "Unknown HW port type %u\n", hw_port_type);
        break;
    }
    return rdpa_if_none;
}
EXPORT_SYMBOL(rdpa_mw_root_dev2rdpa_if);

uint8_t rdpa_mw_root_dev2rdpa_ssid(struct net_device *root_dev)
{
    uint32_t hw_port, hw_port_type;

    hw_port = netdev_path_get_hw_port(root_dev);
    hw_port_type = netdev_path_get_hw_port_type(root_dev);

    switch (hw_port_type)
    {
    case BLOG_WLANPHY:
#ifdef XRDP
        return WLAN_SSID_GET(hw_port);
#else
        return rdpa_if_none;
#endif
    case BLOG_NETXLPHY:
        return (hw_port >> 16) & 0xff;
    default:
        break;
    }

    return (uint8_t)-1;
}
EXPORT_SYMBOL(rdpa_mw_root_dev2rdpa_ssid);

static int blog_commands_parse_qos(Blog_t *blog, rdpa_ic_result_t *mcast_result)
{
    /* TODO: Implement me. */
    BOOL enable;
    
    if (0 != 
        rdpa_mw_pkt_based_qos_get(rdpa_dir_ds, RDPA_MW_QOS_TYPE_MCAST, &enable))
    {
        mcast_result->qos_method = rdpa_qos_method_flow;
    }
    else
    {
        mcast_result->qos_method = 
            enable? rdpa_qos_method_pbit : rdpa_qos_method_flow;
    }
    
    mcast_result->queue_id = SKBMARK_GET_Q(blog->mark);
    
    if (rdpa_mw_set_mcast_dscp_remark != -1)
    {
        mcast_result->dscp_val = rdpa_mw_set_mcast_dscp_remark;
        mcast_result->dscp_remark = 1;
    }

    return 0; /* Temporary for testing */
}

static inline rdpa_if _blog_parse_port_get(struct net_device *dev)
{
    return rdpa_mw_root_dev2rdpa_if(dev);
}

static rdpa_forward_action blog_command_parse_mcast_action(rdpa_if port)
{
    bdmf_object_handle cpu;

    /* If WFD supported action forward, else trap. */
    if (rdpa_if_is_wifi(port))
    {
        if (rdpa_cpu_get(rdpa_cpu_wlan0, &cpu))
            return rdpa_forward_action_host;
        bdmf_put(cpu);
    }
    return rdpa_forward_action_forward;
}

rdpa_if blog_parse_ingress_port_get(Blog_t *blog)
{
    return _blog_parse_port_get(netdev_path_get_root((struct net_device *)blog->rx_dev_p));
}
EXPORT_SYMBOL(blog_parse_ingress_port_get);

rdpa_if blog_parse_egress_port_get(Blog_t *blog)
{
    return _blog_parse_port_get(netdev_path_get_root((struct net_device *)blog->tx_dev_p));
}
EXPORT_SYMBOL(blog_parse_egress_port_get);

int blog_parse_mcast_result_get(Blog_t *blog, rdpa_ic_result_t *mcast_result)
{
    struct net_device *root_dev;
    blogRuleAction_t *blog_rule_action_p;
    int cmdIndex;
    blogRule_t *blog_rule_p;

    if (blog)
       blog_rule_p = blog->blogRule_p;
    else
       return -1;

    memset(mcast_result, 0, sizeof(rdpa_ic_result_t));

    BCM_LOG_INFO(BCM_LOG_ID_RDPA, "Parsing Multicast blog commands\n");
    root_dev = netdev_path_get_root((struct net_device *)blog->tx_dev_p);

    mcast_result->egress_port = rdpa_mw_root_dev2rdpa_if(root_dev);
#ifdef XRDP
    mcast_result->ssid = rdpa_mw_root_dev2rdpa_ssid(root_dev);
#endif

    if (blog_rule_to_vlan_action(blog->blogRule_p, rdpa_dir_ds, &mcast_result->vlan_action))
        return -1;
    mcast_result->action = blog_command_parse_mcast_action(mcast_result->egress_port);

    for (cmdIndex = 0; cmdIndex < blog_rule_p->actionCount; cmdIndex++)
    {
        blog_rule_action_p = &blog_rule_p->action[cmdIndex];
        if (blog_rule_action_p->cmd == BLOG_RULE_CMD_DECR_TTL)
        {
            mcast_result->action_vec |= rdpa_ic_action_ttl;
            break;
        }
    }

    /* Parse DSCP, Opbit/Ipbit, and other QoS parameters */
    return blog_commands_parse_qos(blog, mcast_result); 
}
EXPORT_SYMBOL(blog_parse_mcast_result_get);

void blog_parse_mcast_result_put(rdpa_ic_result_t *mcast_result)
{
    if (mcast_result->policer)
        bdmf_put(mcast_result->policer);
    if (mcast_result->vlan_action)
        bdmf_put(mcast_result->vlan_action);
}
EXPORT_SYMBOL(blog_parse_mcast_result_put);

#if defined(CONFIG_BCM_PON)
void blog_parse_policer_get(Blog_t *blog_p, bdmf_object_handle *policer)
{
    rdpa_policer_key_t key = { rdpa_dir_ds, 0 };
    bdmf_object_handle m_policer = NULL;
    int rc;
    
    key.dir = (((struct net_device *)blog_p->tx_dev_p)->priv_flags & IFF_WANDEV) ?
            rdpa_dir_us : rdpa_dir_ds;
    /* For PON, we use skb->mark[10:5] to store policer index, -1 for Runner index */
    key.index = SKBMARK_GET_TC_ID(blog_p->mark) - 1;
	
    rc = rdpa_policer_get(&key, &m_policer);
    if(rc < 0)
      m_policer = NULL;
    
    *policer = m_policer;
    return;
}
EXPORT_SYMBOL(blog_parse_policer_get);
#endif
#endif
