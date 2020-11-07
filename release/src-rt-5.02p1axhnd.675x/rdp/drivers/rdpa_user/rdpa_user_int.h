// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// Unless you and Broadcom execute a separate written software license
// agreement governing use of this software, this software is licensed
// to you under the terms of the GNU General Public License version 2
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give
//    you permission to link this software with independent modules, and
//    to copy and distribute the resulting executable under terms of your
//    choice, provided that you also meet, for each linked independent
//    module, the terms and conditions of the license of that module.
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications
//    of the software.
// 
// Not withstanding the above, under no circumstances may you combine
// this software in any way with any other Broadcom software provided
// under a license other than the GPL, without Broadcom's express prior
// written consent.
// 
// :>

#ifndef _RDPA_USER_INT_H_
#define _RDPA_USER_INT_H_

#include "rdpa_user.h"

int bdmf_user_new_and_set(bdmf_ioctl_t *ba);
int bdmf_user_destroy(bdmf_ioctl_t *ba);
int bdmf_user_mattr_alloc(bdmf_ioctl_t *ba);
int bdmf_user_mattr_free(bdmf_ioctl_t *ba);
int bdmf_user_get(bdmf_ioctl_t *ba);
int bdmf_user_put(bdmf_ioctl_t *ba);
int bdmf_user_get_next(bdmf_ioctl_t *ba);
int bdmf_user_link(bdmf_ioctl_t *ba);
int bdmf_user_unlink(bdmf_ioctl_t *ba);
int bdmf_user_get_next_us_link(bdmf_ioctl_t *ba);
int bdmf_user_get_next_ds_link(bdmf_ioctl_t *ba);
int bdmf_user_us_link_to_object(bdmf_ioctl_t *ba);
int bdmf_user_ds_link_to_object(bdmf_ioctl_t *ba);
int bdmf_user_get_owner(bdmf_ioctl_t *ba);

#ifdef PORT_OBJECT
long rdpa_port_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* PORT_OBJECT */
#ifdef EGRESS_TM_OBJECT 
long rdpa_egress_tm_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* EGRESS_TM_OBJECT */
#ifdef VLAN_OBJECT
long rdpa_vlan_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* VLAN_OBJECT */
#ifdef VLAN_ACTION_OBJECT
long rdpa_vlan_action_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* VLAN_ACTION_OBJECT */
#ifdef TCONT_OBJECT
long rdpa_tcont_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pai);
#endif /* TCONT_OBJECT */
#ifdef SYSTEM_OBJECT
long rdpa_system_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* SYSTEM_OBJECT */
#ifdef IPTV_OBJECT
long rdpa_iptv_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* IPTV_OBJECT */
#ifdef TC_TO_QUEUE_OBJECT
long rdpa_tc_to_queue_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* TC_TO_QUEUE_OBJECT */
#ifdef LLID_OBJECT
long rdpa_llid_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* LLID_OBJECT */
#ifdef INGRESS_CLASS_OBJECT
long rdpa_ingress_class_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* INGRESS_CLASS_OBJECT */
#ifdef POLICER_OBJECT
long rdpa_policer_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* POLICER_OBJECT */
#ifdef PBIT_TO_QUEUE_OBJECT
long rdpa_pbit_to_queue_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* PBIT_TO_QUEUE_OBJECT */
#ifdef PBIT_TO_GEM_OBJECT
long rdpa_pbit_to_gem_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* PBIT_TO_GEM_OBJECT */
#ifdef DSCP_TO_PBIT_OBJECT
long rdpa_dscp_to_pbit_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* DSCP_TO_PBIT_OBJECT */
#ifdef UDPSPDTEST_OBJECT
long rdpa_udpspdtest_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* UDPSPDTEST_OBJECT */
#ifdef CAPWAP_OBJECT
long rdpa_capwap_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
long rdpa_capwap_reassembly_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
long rdpa_capwap_fragmentation_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif
#ifdef MLLID_OBJECT
long rdpa_mllid_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa);
#endif /* MLLID_OBJECT */


#define __BDMF_LOG__

#endif /* _RDPA_USER_INT_H_ */
