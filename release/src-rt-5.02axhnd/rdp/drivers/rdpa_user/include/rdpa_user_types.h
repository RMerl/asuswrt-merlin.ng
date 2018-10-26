#ifndef _RDPA_USER_TYPES_H_
#define _RDPA_USER_TYPES_H_

#include <bdmf_interface.h>
#include <bdmf_system.h>
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
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908) && !defined(BCM63158)
#include <rdpa_ip_class.h>
#endif

#include <rdpa_bridge.h>
#include <rdpa_vlan.h>
#include <rdpa_qos_mapper.h>
#include <rdpa_tm.h>
#include <rdpa_wlan_mcast.h>
#if defined(CONFIG_BCM_SPDSVC_SUPPORT) && (!defined(G9991) || defined(XRDP))
#include <rdpa_spdsvc.h>
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

#endif /* _RDPA_USER_TYPES_H_ */
