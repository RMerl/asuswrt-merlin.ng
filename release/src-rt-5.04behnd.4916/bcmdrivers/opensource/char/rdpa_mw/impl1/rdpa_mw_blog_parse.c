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

#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_mw_arch.h"
#include "rdpa_mw_blog_parse.h"
#include "rdpa_mw_vlan.h"
#include "rdpa_mw_qos.h"
#include "bcm_util_func.h"


#if defined(CONFIG_BLOG)
int rdpa_mw_set_mcast_dscp_remark = -1;
EXPORT_SYMBOL(rdpa_mw_set_mcast_dscp_remark);

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
            enable ? rdpa_qos_method_pbit : rdpa_qos_method_flow;
    }
    
    mcast_result->queue_id = SKBMARK_GET_Q(blog->mark);
    
    if (rdpa_mw_set_mcast_dscp_remark != -1)
    {
        mcast_result->dscp_val = rdpa_mw_set_mcast_dscp_remark;
        mcast_result->dscp_remark = 1;
    }

    return 0; /* Temporary for testing */
}

static rdpa_forward_action blog_command_parse_mcast_action(Blog_t *blog)
{
#if !defined(CONFIG_BRCM_QEMU)
    bdmf_object_handle cpu;

    /* If WFD supported action forward, else trap. */
    if (__is_enet_wlan_port(blog, 0))
    {
        if (rdpa_cpu_get(rdpa_cpu_wlan0, &cpu))
            return rdpa_forward_action_host;
        bdmf_put(cpu);
    }
#endif    
    return rdpa_forward_action_forward;
}

bdmf_object_handle blog_parse_ingress_port_get(Blog_t *blog)
{
    return rdpa_mw_get_port_object_by_dev_dir(blog->rx_dev_p, RDPA_DIR_RX);
}
EXPORT_SYMBOL(blog_parse_ingress_port_get);

bdmf_object_handle blog_parse_egress_port_get(Blog_t *blog)
{
    return rdpa_mw_get_port_object_by_dev_dir(blog->tx_dev_p, RDPA_DIR_TX);
}
EXPORT_SYMBOL(blog_parse_egress_port_get);

int blog_parse_mcast_result_get(Blog_t *blog, rdpa_ic_result_t *mcast_result)
{
    struct net_device *root_dev;
    blogRuleAction_t *blog_rule_action_p;
    int cmd_index;
    blogRule_t *blog_rule_p;

    if (blog)
       blog_rule_p = blog->blogRule_p;
    else
       return -1;

    memset(mcast_result, 0, sizeof(rdpa_ic_result_t));

    BCM_LOG_INFO(BCM_LOG_ID_RDPA, "Parsing Multicast blog commands\n");
    root_dev = netdev_path_get_root((struct net_device *)blog->tx_dev_p);

    mcast_result->port_egress_obj = blog_parse_egress_port_get(blog);
    if (mcast_result->port_egress_obj == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_RDPA, "Egress port NULL blog->tx_dev_p %s\n", 
                      dev_name_or_null(blog->tx_dev_p));
        return -1;
    }
#ifdef XRDP
    mcast_result->ssid = rdpa_mw_root_dev2rdpa_ssid(root_dev);
#endif

    if (blog_rule_to_vlan_action(blog->blogRule_p, rdpa_dir_ds, &mcast_result->vlan_action))
        return -1;
    mcast_result->action = blog_command_parse_mcast_action(blog);

    for (cmd_index = 0; cmd_index < blog_rule_p->actionCount; cmd_index++)
    {
        blog_rule_action_p = &blog_rule_p->action[cmd_index];
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

#if defined(POLICER_SUPPORT)
void blog_parse_policer_get(Blog_t *blog_p, bdmf_object_handle *policer)
{
    bdmf_object_handle m_policer = NULL;
    bdmf_number index = 0;
    int rc;

    /* For PON, we use skb->mark[10:5] to store policer index, -1 for Runner index */
    index = SKBMARK_GET_TC_ID(blog_p->mark) - 1;

    rc = rdpa_policer_get(index, &m_policer);
    if (rc < 0)
      m_policer = NULL;
    
    *policer = m_policer;
    return;
}
EXPORT_SYMBOL(blog_parse_policer_get);
#endif
#endif
