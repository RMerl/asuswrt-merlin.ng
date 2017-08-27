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

#ifndef _RDPA_DHD_HELPER_BASIC_H_
#define _RDPA_DHD_HELPER_BASIC_H_

#include "rdpa_types.h"

/** \addtogroup dhd_helper DHD Helper Interface
 *
 * @{
 */

/* RDP Feature capability defines for this release */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
#define RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT
#define RDPA_DHD_HELPER_FEATURE_LLCSNAPHDR_SUPPORT
#endif

#ifndef BDMF_SYSTEM_SIM
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define RDPA_DHD_DOORBELL_IRQ (INTERRUPT_ID_RUNNER_0 + 2) /* Comply to definition in RDD */
#else
#define RDPA_DHD_DOORBELL_IRQ (INTERRUPT_ID_RDP_RUNNER + 2) /* Comply to definition in RDD */
#endif
#else
#define RDPA_DHD_DOORBELL_IRQ 2 /* Comply to definition in RDD */
#endif

#define RDPA_DHD_HELPER_CPU_QUEUE_SIZE 128 

#define RDPA_DHD_HELPER_NUM_OF_FLOW_RINGS (4 * 136)
#define RDPA_MAX_RADIOS 3


#define RDPA_DHD_TX_POST_SKB_BUFFER_VALUE    0 /* 00: possible value in tx complete only */
#define RDPA_DHD_TX_POST_HOST_BUFFER_VALUE   1 /* 01: possible value in tx post and tx complete */
#define RDPA_DHD_TX_POST_BPM_BUFFER_VALUE    2 /* 10: possible value in tx post and tx complete */
#define RDPA_DHD_TX_POST_FKB_BUFFER_VALUE    3 /* 11: possible value in tx complete only */

/** DHD init configuration */
typedef struct
{
    /* FlowRings base addresses */
    void *rx_post_flow_ring_base_addr;
    void *tx_post_flow_ring_base_addr; /**< Fake base, (first 2 indexes are not in use) */
    void *rx_complete_flow_ring_base_addr;
    void *tx_complete_flow_ring_base_addr;

    /* RD/WR indexes arrays base addresses */
    void *r2d_wr_arr_base_addr;
    void *d2r_rd_arr_base_addr;
    void *r2d_rd_arr_base_addr;
    void *d2r_wr_arr_base_addr;

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    uint32_t r2d_wr_arr_base_phys_addr;
    uint32_t d2r_rd_arr_base_phys_addr;
    uint32_t r2d_rd_arr_base_phys_addr;
    uint32_t d2r_wr_arr_base_phys_addr;
#endif
    void *tx_post_mgmt_arr_base_addr;
    uint32_t tx_post_mgmt_arr_base_phys_addr;

    int (*doorbell_isr)(int irq, void *priv);
    void *doorbell_ctx;

    uint32_t dongle_wakeup_register;
    uint8_t  add_llcsnap_header;
} rdpa_dhd_init_cfg_t;

/** Extra data that can be passed along with the packet to be posted for transmission */
typedef struct
{
    uint32_t radio_idx;

    bdmf_boolean is_bpm;  /**< 1 for BPM, 0 for host buffer */
    uint32_t flow_ring_id; /**< Destination Flow-Ring */
    uint32_t ssid_if_idx; /**< SSID index */
    int is_spdsvc_setup_packet; /**<when set, indicates that a Speed Service Setup packet is being transmitted */
} rdpa_dhd_tx_post_info_t;

/** Runner wakeup information returned to DHD by RDD */
typedef struct
{
    uint32_t radio_idx;

    uint32_t tx_complete_wakeup_register;
    uint32_t tx_complete_wakeup_value;
    uint32_t rx_complete_wakeup_register;
    uint32_t rx_complete_wakeup_value;
} rdpa_dhd_wakeup_info_t;

/* Description of TxPost ring for caching */
typedef struct rdpa_dhd_flring_cache
{
    uint32_t base_addr_low; /* XXX: Add dedicated struct for addr_64 */
    uint32_t base_addr_high;
    uint16_t items; /* Number of descriptors in flow ring */
#define FLOW_RING_FLAG_DISABLED (1 << 1)
    uint16_t flags;
    uint32_t reserved;
} rdpa_dhd_flring_cache_t;


/** Tx Complete descriptor data for host DHD driver */
typedef struct rdpa_dhd_complete_data
{
    uint32_t radio_idx;

    uint32_t request_id;
    uint8_t  buf_type;          /* RDPA_DHD_TX_POST_SKB_BUFFER_VALUE, RDPA_DHD_TX_POST_FKB_BUFFER_VALUE, RDPA_DHD_TX_POST_HOST_BUFFER_VALUE  */
    void     *txp;    
    uint16_t status;
    uint16_t flow_ring_id;
} rdpa_dhd_complete_data_t;

/** @} end of dhd_heler Doxygen group */


#endif /* _RDPA_DHD_HELPER_H_ */
