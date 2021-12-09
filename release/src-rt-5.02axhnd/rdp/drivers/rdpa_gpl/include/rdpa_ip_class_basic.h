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

#ifndef _RDPA_IP_FLOW_BASIC_H_
#define _RDPA_IP_FLOW_BASIC_H_

#include <bdmf_data_types.h>

/** \addtogroup ip_class
 * @{
 */


/* Actions of the optional actions vector */
typedef enum
{
    rdpa_fc_act_forward, 
    rdpa_fc_act_policer, 
    rdpa_fc_act_ttl,
    rdpa_fc_act_pppoe_passthrough,
    rdpa_fc_act_dscp_remark,
    rdpa_fc_act_nat,
    rdpa_fc_act_gre_remark,
    rdpa_fc_act_opbit_remark,
    rdpa_fc_act_ipbit_remark,
    rdpa_fc_act_dslite_tunnel,
    rdpa_fc_act_pppoe,
    rdpa_fc_act_l2gre_tunnel,
    rdpa_fc_act_service_q,
    rdpa_fc_act_spdsvc,
    rdpa_fc_act_llc_snap_set_len,
}
rdpa_fc_action;

/** Bitmask of actions applied on 5-tuple based IP flow entry */ 
typedef enum
{
    /** Disables forwarding action if set */
    rdpa_fc_action_forward = (1 << rdpa_fc_act_forward),
    /** Enables flow based policer if set */
    rdpa_fc_action_policer = (1 << rdpa_fc_act_policer),
    /** Enables ttl decrement if set */
    rdpa_fc_action_ttl = (1 << rdpa_fc_act_ttl),
    /** PPPoE Passthrough (for L2 FC) */
    rdpa_fc_action_pppoe_passthrough = (1 << rdpa_fc_act_pppoe_passthrough),
    /** Enables DSCP remarking if set */
    rdpa_fc_action_dscp_remark = (1 << rdpa_fc_act_dscp_remark),
    /** Enables NAT operation if set */
    rdpa_fc_action_nat = (1 << rdpa_fc_act_nat),
    /** Enables GRE remarking if set */
    rdpa_fc_action_gre_remark = (1 << rdpa_fc_act_gre_remark),
    /** Enables Outer pbit remarking if set */
    rdpa_fc_action_opbit_remark = (1 << rdpa_fc_act_opbit_remark),
    /** Enables Inner pbit remarking if set */
    rdpa_fc_action_ipbit_remark = (1 << rdpa_fc_act_ipbit_remark),
    /** Enables DS Lite operation if set */
    rdpa_fc_action_dslite_tunnel = (1 << rdpa_fc_act_dslite_tunnel),
    /** Enables GRE tunnel operation if set */
    rdpa_fc_action_l2gre_tunnel = (1 << rdpa_fc_act_l2gre_tunnel),
    /** Enables pppoe operation if set */
    rdpa_fc_action_pppoe = (1 << rdpa_fc_act_pppoe),
    /** Forward to service queue */
    rdpa_fc_action_service_q = (1 << rdpa_fc_act_service_q),
    /** Forward to speed service  */
    rdpa_fc_action_spdsvc = (1 << rdpa_fc_act_spdsvc),
    /** LLC Snap length set */
    rdpa_fc_action_llc_snap_set_len = (1 << rdpa_fc_act_llc_snap_set_len)
}
rdpa_fc_action_vector;

/** Vector of \ref rdpa_fc_action_vector "actions". All configured actions are applied on the 5-tuple based IP flow
 * entry */
typedef uint16_t rdpa_fc_action_vec_t;

/** 5-tuple based IP flow key.\n
 * This key is used to classify traffic.\n
 */
typedef struct {
    bdmf_ip_t src_ip;    /**< Source IP address, in GRE mode should be 0 */
    bdmf_ip_t dst_ip;    /**< Destination IP address, in GRE mode should be call ID*/
    uint8_t prot;        /**< Protocol */
    uint16_t src_port;   /**< Source port */
    uint16_t dst_port;   /**< Destination port */
    rdpa_traffic_dir dir;/**< Traffic direction */
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908) || defined(BCM63158)
    rdpa_if ingress_if;  /**< Ingress interface */
    uint8_t lookup_port; /**< Ingress bridge port */
#else                    /* All other platforms */
    rdpa_if ingress_if;  /**< Ingress interface */
#endif
    uint8_t tcp_pure_ack;/**< TCP pure ack flow */
} rdpa_ip_flow_key_t;

/** @} end of ip_class Doxygen group. */

union rdpa_wfd_u {
    uint32_t    u32;
    struct {
           uint32_t is_wfd:1;        /* is_wfd=1  have to be the first one */
           uint32_t is_chain:1;      /* is_chain=1 have to be the second one */
           uint32_t wfd_prio:3;      /* 0=high, 1=low for RDP, TC for XRDP */
           uint32_t reserved1:11;    /* unused */
           uint32_t wfd_idx:2;       /* WFD idx */
           uint32_t reserved0:2;     /* unused */
           uint32_t priority:4;      /* Tx Priority */
           uint32_t chain_idx:8;     /* Tx chain index */
    } nic_ucast;

    struct {
           uint32_t is_wfd:1;        /* is_wfd=1 have to be the first one */
           uint32_t is_chain:1;      /* is_chain=0 have to be the second one */
           uint32_t wfd_prio:3;      /* 0=high, 1=low for RDP, TC for XRDP */
           uint32_t ssid:4;          /* SSID for WLAN */
           uint32_t reserved1:7;     /* unused */
           uint32_t wfd_idx:2;       /* WFD idx */
           uint32_t reserved0:1;     /* unused */
           uint32_t priority:3;      /* Tx Priority */
           uint32_t flowring_idx:10; /* Tx flowring index */
    } dhd_ucast;

    struct {
           uint32_t is_wfd:1;        /* is_wfd=1 have to be the first one */
           uint32_t is_chain:1;      /* is_chain=0 have to be the second one */
           uint32_t wfd_idx:2;       /* WFD idx */
           uint32_t wfd_prio:3;      /* 0=high, 1=low for RDP, TC for XRDP */
           uint32_t ssid:4;          /* SSID */
           uint32_t reserved0:21;    /* unused */
    } mcast;
};
typedef union rdpa_wfd_u rdpa_wfd_t;

struct rdpa_rnr_u {
       uint32_t is_wfd:1;            /* rnr (is_wfd=0) have to be the first one */
       uint32_t radio_idx:2;         /* Radio index */
       uint32_t llcsnap_flag:1;      /* Indicates llcsnap header insertion in processing Used only in XRDP */
       uint32_t priority:3;          /* Tx Priority */
       uint32_t ssid:4;              /* SSID */
       uint32_t reserved1:9 ;        /* unused */
       uint32_t flow_prio:2;         /* flow priority - used for packet buffer reservation */
       uint32_t flowring_idx:10;     /* Tx flowring index */
};

typedef struct rdpa_rnr_u rdpa_rnr_t;

#endif

