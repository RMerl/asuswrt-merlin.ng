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

/* send Txcomplete indications to host for all host originated packets */
#define RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT                 /* txcmpl2host */

/* Insert LLCSNAP header support  */
#define RDPA_DHD_HELPER_FEATURE_LLCSNAPHDR_SUPPORT              /* dhdhdr */

/* Supports CWI32 WI format with address of 64 bits (for TxPost rings) */
#define RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT     /* msgringformat */

/* Supports CWI32 WI format */
#define RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT               /* msgringformat */

/* Supports fast flowring delete procedure (flushing FR from runner) */
#define RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT    /* ffrd */

/* BQ support */
#define RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT            /* bkupq */

/* HWA support (dongle iDMA) */
#define RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT              /* hwawkup */

/* Backup Queue depth reporting support */
#define RDPA_DHD_HELPER_FEATURE_HBQD_SUPPORT                    /* hbqd */

/* Configurable number of FRs in idma group support */
#define RDPA_DHD_HELPER_FEATURE_IDMA_GROUP_CFG_SUPPORT

#if defined(BCM63146) || defined(BCM6855) || defined(BCM6888) || defined(BCM6837) || defined(BCM6813) || defined(BCM4912)

/* 64 iDMA groups support */
#define RDPA_DHD_HELPER_FEATURE_IDMA64_SUPPORT                  /* idma64 */
#endif

#if defined(XRDP)
/* RxCompletion via headroom support */
#define RDPA_DHD_HELPER_FEATURE_RCH_SUPPORT                     /* rxcmplrch */

/* support of RxPost buffers beyond ring size */
#define RDPA_DHD_HELPER_FEATURE_BA256_CFG_SUPPORT               /* ba256cfg */

/* WLAN codel support */
#define RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT                   /* codel */

/* Common rings (RxPost/Rxcomplete/Txcomplete) size configuration support */
#define RDPA_DHD_HELPER_FEATURE_COMMON_RING_SIZES_CFG_SUPPORT   /* cmnrngsz */

/* TID from dongle support */
#define RDPA_DHD_HELPER_FEATURE_MSCS_SUPPORT                    /* rxcmplv2 */

/* dynamic BQ size support via FR update message */
#define RDPA_DHD_HELPER_FEATURE_FLOWRING_UPDATE_SUPPORT         /* bqupd */

#define DHD_MAX_SSID_NUM 16
typedef uint32_t rdpa_dhd_ssid_tx_dropped_t[DHD_MAX_SSID_NUM];

#else
#define DHD_MAX_SSID_NUM 8
#endif
/* Not supported any more
#define RDPA_DHD_HELPER_FEATURE_NPLUSM
*/

#define RDPA_DHD_DOORBELL_IRQ 2 /* Comply to definition in RDD */

#define RDPA_DHD_HELPER_CPU_QUEUE_SIZE 128

#define RDPA_DHD_HELPER_NUM_OF_FLOW_RINGS (1024)

#if defined(BCM6888)
#define RDPA_MAX_RADIOS 4 /* NIC and dongle */
#define RDPA_MAX_DHD_OFFL_RADIOS 4 /* defines max allocated resourses */
#define RDPA_MAX_CONCUR_OFFL_RADIOS 3 /* max number of offloaded radios concurrently */
#elif defined(BCM6855)
#define RDPA_MAX_RADIOS 3 /* NIC and dongle */
#define RDPA_MAX_DHD_OFFL_RADIOS 3 /* defines max allocated resourses */
#define RDPA_MAX_CONCUR_OFFL_RADIOS 2 /* max number of offloaded radios concurrently */
#elif defined(BCM6846)
#define RDPA_MAX_RADIOS 2 /* NIC and dongle */
#define RDPA_MAX_DHD_OFFL_RADIOS 2 /* defines max allocated resourses */
#define RDPA_MAX_CONCUR_OFFL_RADIOS 1
#elif defined(BCM6878)
#define RDPA_MAX_RADIOS 2 /* NIC and dongle */
#define RDPA_MAX_DHD_OFFL_RADIOS 1 /* defines max allocated resourses */
#define RDPA_MAX_CONCUR_OFFL_RADIOS 1
#else
#define RDPA_MAX_RADIOS 3 /* NIC and dongle */
#define RDPA_MAX_DHD_OFFL_RADIOS 3 /* defines max allocated resourses */
#define RDPA_MAX_CONCUR_OFFL_RADIOS 3
#endif

#define RDPA_MAX_AC 4 /* exclude BC/MC */

#define RC_PKT_SENT_TO_SQ                    1 /* return code when CPU DHD packet is set to service queues */

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


    uint32_t r2d_wr_arr_base_phys_addr;
    uint32_t d2r_rd_arr_base_phys_addr;
    uint32_t r2d_rd_arr_base_phys_addr;
    uint32_t d2r_wr_arr_base_phys_addr;

    void *tx_post_mgmt_arr_base_addr;
    uint32_t tx_post_mgmt_arr_base_phys_addr;
    uint32_t tx_post_mgmt_arr_entry_count;

    int (*doorbell_isr)(int irq, void *priv);
    void *doorbell_ctx;

    uint32_t dongle_wakeup_register;
    uint8_t  add_llcsnap_header;
    uint8_t  flow_ring_format;               /* 0-legacy, 1- CWI32 */
    uint32_t dongle_wakeup_register_2;
    uint8_t  dongle_wakeup_hwa;              /* 0: disabled Use wakeup_register for all rings */
                                             /* 1: enabled  Use wakeup_register for txpost, and
                                                wakeup_register_2 for hwa rxpost, rxcpl, txcpl rings */
    void *dongle_wakeup_register_virt;       /* Virtual address of dongle_wakeup_register for rdd access */
    void *dongle_wakeup_register_2_virt;     /* Virtual address of dongle_wakeup_register_2 for rdd access */

    uint8_t  hbqd_mode;                      /* Host Backup Queue Depth (HBQD) mode
                                                0 - disabled, 32 FRs in idma group. 1 - enabled, 16 FRs in idma group */
    uint16_t rx_post_ring_size;              /* 0 - default (1K), N - ring size in entries */
    uint16_t rx_cmpl_ring_size;              /* 0 - default (1K), N - ring size in entries */
    uint16_t tx_cmpl_ring_size;              /* 0 - default (1K), N - ring size in entries */
    uint16_t rx_cmpl_rch_max_seg;            /* max amount of segments in RCH */
    uint16_t idma_group_shift;               /* 2^idma_group_shift = num of FR in the group, if 0 - set by runner(backward compatibility) */
    uint16_t idma_num_of_groups;             /* number of idma groups, if 0 - set to 16(backward compatibility) */
    uint16_t reserved[2];                    /* reserved for next features to keep structure backward compatibility */
} rdpa_dhd_init_cfg_t;

/** Extra data that can be passed along with the packet to be posted for transmission */
typedef struct
{
    uint32_t radio_idx;
    uint32_t flow_ring_id; /**< Destination Flow-Ring */
    uint32_t ssid_if_idx; /**< SSID index */
    int is_spdsvc_setup_packet; /**<when set, indicates that a Speed Service Setup packet is being transmitted */
    int is_bpm;  /**< 1 for BPM, 0 for host buffer */
    uint32_t spdt_so_mark; /**< in use for Speed Service packet setup */
    uint32_t seqnum; /**< in use for BC/MC rings  */
} rdpa_dhd_tx_post_info_t;

/* no autogenerated rdpa_dhd_data_stat_t to api */

typedef struct
{
    uint32_t dhd_rx_alloc_fail; /**< DHD RX FPM allocation failure packets */
    uint32_t dhd_tx_fpm_used; /**< DHD TX FPM used */
    uint32_t dhd_rx_fpm_used; /**< DHD RX FPM used */
    uint32_t dhd_tx_high_prio_fpm_used; /**< DHD TX High priority FPM used */
    uint32_t dhd_tx_total_fpm_used; /**< DHD TX FPM used */
    uint32_t dhd_tx_fpm_drop; /**< DHD TX FPM drop */
    uint32_t dhd_tx_high_prio_fpm_drop; /**< DHD TX high priority and mcast FPM drop */
    uint32_t dhd_mcast_sbpm_drop;  /**< DHD MCAST SBPM drop \RDP limited */
    uint32_t dhd_tx_fr_ac_bk_full; /**< DHD TX feeder ring AC BK is full*/
    uint32_t dhd_tx_fr_ac_be_full; /**< DHD TX feeder ring AC BE is full*/
    uint32_t dhd_tx_fr_ac_vi_full; /**< DHD TX feeder ring AC VI is full */
    uint32_t dhd_tx_fr_ac_vo_full; /**< DHD TX feeder ring AC VO is full */
    uint32_t dhd_tx_fr_ac_bc_mc_full; /**< DHD TX feeder ring AC BC/MC is full  */
    uint32_t dhd_tx_post_packets; /**< DHD TX post packets */
    uint32_t dhd_tx_complete_packets; /**< DHD TX completed packets */
    uint32_t dhd_rx_complete_packets; /**< DHD RX completed packets */
    uint32_t dhd_tx_drop_packets; /**< DHD TX total dropped packets  */
    uint32_t dhd_tx_host_drop_packets; /**< DHD TX host dropped packets */
    uint32_t dhd_bq_tx_drop_packets; /**< DHD BQ dropped packets */
} rdpa_dhd_data_stat_t;

/** Runner wakeup information returned to DHD by RDD */
typedef struct
{
    uint32_t radio_idx;
    bdmf_phys_addr_t tx_complete_wakeup_register;
    uint32_t tx_complete_wakeup_value;
    bdmf_phys_addr_t rx_complete_wakeup_register;
    uint32_t rx_complete_wakeup_value;
} rdpa_dhd_wakeup_info_t;

/* Description of TxPost ring for caching */
typedef struct rdpa_dhd_flring_cache
{
    uint32_t base_addr_low; /* XXX: Add dedicated struct for addr_64 */
    uint32_t base_addr_high;
    uint16_t items; /* Number of descriptors in flow ring (including backup queue if exist) */
#define FLOW_RING_FLAG_DISABLED   (1 << 1)
#define FLOW_RING_FLAG_SSID_SHIFT (8)
#define FLOW_RING_FLAG_SSID_MASK  (0xF << FLOW_RING_FLAG_SSID_SHIFT)
    uint16_t flags;
#ifdef RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT
    uint16_t backup_first_index;
    uint16_t backup_last_index;
    uint16_t backup_num_entries;
    uint16_t phy_ring_size;  /* Number of descriptors in physical flow ring */
#else
    uint32_t reserved;       /* Maintain the structure for platforms without backup queues feature */
#endif
#ifdef RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT
    uint16_t window_ts;     /* Sliding window endpoint timestamp */
    uint16_t drop_interval; /* Number of consequitive drops */
    uint16_t codel_flags;   /* Codel state flags */
    uint16_t codel_drop_counter;
#endif
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

/** DHD CPU Data resources for exception traffic */
typedef struct
{
    uint32_t cpu_port;
    uint8_t exception_rxq;
    bdmf_boolean set_exception_tc_to_rxq;
} rdpa_dhd_cpu_data_t;

typedef union
{
    uint32_t u32;
    struct {
        uint32_t flowring_idx:16;
        uint32_t read_idx:10;
        uint32_t idx_valid:1;
        uint32_t reserved:5;
    };
} rdpa_dhd_msg_data_t;

/** @} end of dhd_heler Doxygen group */


#endif /* _RDPA_DHD_HELPER_H_ */
