/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
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

#ifndef _RDPA_GEM_H_
#define _RDPA_GEM_H_

/**
 * \defgroup gem GEM Flow Management
 * \ingroup xgpon
 * Objects in this group control GEM-related configuration
 * @{
 */

/**< Max number of GEM flows */
#if (defined(G9991) || defined(BRCM_FTTDP)) && defined(__OREN__)
#define G9991_SID_PORTS_DS 25
#define RDPA_MAX_GEM_FLOW (256 - G9991_SID_PORTS_DS) 
#else
#if defined(XRDP) || defined(BCM_PON_XRDP)
#if !defined(BCM6878)
#define RDPA_MAX_GEM_FLOW 128 
#else
#define RDPA_MAX_GEM_FLOW 64
#endif
#else
#define RDPA_MAX_GEM_FLOW 256 
#endif
#endif

/** GEM Flow Type */
typedef enum {
    rdpa_gem_flow_ethernet, /**< Ethernet flow. MAC calculates Ethernet FCS */
    rdpa_gem_flow_omci /**< OMCI Flow */
} rdpa_gem_flow_type;

/** GEM Flow Upstream Configuration
 * Underlying type for gem_us_cfg aggregate
 */
typedef struct 
{
    bdmf_object_handle tcont; /**< TCONT ID */
    bdmf_boolean enc; /**< Encryption - used for XGPON */
} rdpa_gem_flow_us_cfg_t;

/** GEM Flow Downstream Configuration
 * Underlying type for gem_flow_ds_cfg_t aggregate
 */
typedef struct 
{
    rdpa_discard_prty discard_prty; /**< Discard priority */
    rdpa_flow_destination destination; /**< Flow destination */
} rdpa_gem_flow_ds_cfg_t;

/** GEM Flow Statistics 
 * Underlying type for gem_stat aggregate
 */
typedef struct 
{
    uint32_t rx_packets; /**< Rx Packets */
    uint32_t rx_bytes; /**< Rx Bytes */
    uint32_t tx_packets; /**< Tx Packets */
    uint32_t tx_bytes; /**< Tx bytes */
    uint32_t rx_packets_discard; /**< Rx Packet discard */  
    uint32_t tx_packets_discard; /**< Tx Packet discard */ 
} rdpa_gem_stat_t;

/** GEM def flow per port action */
typedef struct
{
    bdmf_object_handle vlan_action; /**< VLAN action object */
    bdmf_boolean drop; /**< Drop action - true/false */
} rdpa_gem_port_action_t;

/** @} end of gem Doxygen group */

#endif /* _RDPA_GEM_H_ */
