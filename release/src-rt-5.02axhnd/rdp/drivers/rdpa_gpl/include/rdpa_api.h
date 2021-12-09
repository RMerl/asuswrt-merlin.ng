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


#ifndef _RDPA_API_H_
#define _RDPA_API_H_

/*
 * Forward declarations of commonly-used types
 */

#include <bdmf_interface.h>
#include <rdpa_types.h>
#include <rdpa_config.h>

#include <rdpa_system.h>
#include <rdpa_port.h>
#include <rdpa_cpu_basic.h>
#include <rdpa_cpu.h>
#include <rdpa_ingress_class_basic.h>
#include <rdpa_ingress_class.h>
#include <rdpa_tcont.h>
#include <rdpa_llid.h>
#include <rdpa_egress_tm.h>
#include <rdpa_vlan_action.h>
#include <rdpa_gem.h>
#include <rdpa_filter.h>

#include <rdpa_ag_system.h>
#include <rdpa_ag_port.h>
#include <rdpa_ag_cpu.h>
#include <rdpa_ag_ingress_class.h>
#include <rdpa_ag_tcont.h>
#include <rdpa_ag_llid.h>
#include <rdpa_ag_egress_tm.h>
#include <rdpa_ag_vlan_action.h>
#include <rdpa_ag_gem.h>

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908) && !defined(BCM63158)
#include <rdpa_ip_class.h>
#include <rdpa_ag_ip_class.h>
#ifdef XRDP
#include <rdpa_l2_common.h>
#include <rdpa_l2_class.h>
#include <rdpa_ag_l2_class.h>
#endif
#endif

#include <rdpa_bridge.h>
#include <rdpa_vlan.h>
#include <rdpa_qos_mapper.h>
#include <rdpa_tm.h>
#include <rdpa_wlan_mcast.h>
#include <rdpa_ag_wlan_mcast.h>
#if defined(CONFIG_BCM_SPDSVC_SUPPORT) && (!defined(G9991) || defined(XRDP))
#include <rdpa_spdsvc.h>
#include <rdpa_ag_spdsvc.h>
#endif
#if defined(CONFIG_BCM_TCPSPDTEST_SUPPORT) && defined(XRDP)
#include <rdpa_tcpspdtest.h>
#include <rdpa_ag_tcpspdtest.h>
#endif
#if defined(DSL_63138) || defined(DSL_63148)
#include <rdpa_ipsec.h>
#endif
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908) || defined(BCM63158)
#include <rdpa_ucast.h>
#include <rdpa_mcast.h>
#include <rdpa_xtm.h>
#include <rdpa_l2_ucast.h>
#else
#include <rdpa_iptv.h>
#endif /* DSL_138 */

#ifdef CONFIG_DHD_RUNNER
#include <rdpa_dhd_helper.h>
#endif

#include <rdpa_ag_bridge.h>
#if defined(DSL_63138) || defined(DSL_63148)
#include <rdpa_ag_ipsec.h>
#endif
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908) || defined(BCM63158)
#include <rdpa_ag_ucast.h>
#include <rdpa_ag_mcast.h>
#include <rdpa_ag_xtm.h>
#include <rdpa_ag_l2_ucast.h>
#else
#include <rdpa_ag_iptv.h>
#include <rdpa_ag_pbit_to_gem.h>
#include <rdpa_ag_pbit_to_queue.h>
#include <rdpa_ag_tc_to_queue.h>
#endif /* DSL_138 */

#include <rdpa_ag_policer.h>
#include <rdpa_ag_dscp_to_pbit.h>
#include <rdpa_ag_vlan.h>
#include <rdpa_ag_filter.h>

#ifdef CONFIG_DHD_RUNNER
#include <rdpa_ag_dhd_helper.h>
#endif

#endif /* _RDPA_API_H_ */
