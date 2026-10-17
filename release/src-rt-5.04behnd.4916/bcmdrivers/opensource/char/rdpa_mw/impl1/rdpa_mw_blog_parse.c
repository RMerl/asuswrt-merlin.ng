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
#include "rdpa_mw_qos.h"
#include "bcm_util_func.h"


#if defined(CONFIG_BLOG)
int rdpa_mw_set_mcast_dscp_remark = -1;
EXPORT_SYMBOL(rdpa_mw_set_mcast_dscp_remark);




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
