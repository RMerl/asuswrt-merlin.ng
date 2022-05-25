/*
    Copyright (c) 2021 Broadcom
    All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
*/

/*
*******************************************************************************
*
* File Name  : archer_dhd_helper.h
*
* Description: Archer DHD API
*
*******************************************************************************
*/

#ifndef __ARCHER_DHD_HELPER_H__
#define __ARCHER_DHD_HELPER_H__

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)

#include <linux/nbuff.h>
#include <linux/gbpm.h>
#include "bpm.h"

#define XRDP

/** Generic error codes
 */
typedef enum { 
    BDMF_ERR_OK               =  0,   /**< OK */
    BDMF_ERR_PARM             = -1,   /**< Error in parameters */
    BDMF_ERR_NOMEM            = -2,   /**< No memory */
    BDMF_ERR_NORES            = -3,   /**< No resources */
    BDMF_ERR_INTERNAL         = -4,   /**< Internal error */
    BDMF_ERR_NOENT            = -5,   /**< Entry doesn't exist */
    BDMF_ERR_NODEV            = -6,   /**< Device doesn't exist */
    BDMF_ERR_ALREADY          = -7,   /**< Entry already exists */
    BDMF_ERR_RANGE            = -8,   /**< Out of range */
    BDMF_ERR_PERM             = -9,   /**< No permission to perform an operation */
    BDMF_ERR_NOT_SUPPORTED    = -10,  /**< Operation is not supported */
    BDMF_ERR_PARSE            = -11,  /**< Parsing error */
    BDMF_ERR_INVALID_OP       = -12,  /**< Invalid operation */
    BDMF_ERR_IO               = -13,  /**< I/O error */
    BDMF_ERR_STATE            = -14,  /**< Object is in bad state */
    BDMF_ERR_DELETED          = -15,  /**< Object is deleted */
    BDMF_ERR_TOO_MANY         = -16,  /**< Too many objects */
    BDMF_ERR_NOT_LINKED       = -17,  /**< Objects are not linked */
    BDMF_ERR_NO_MORE          = -18,  /**< No more entries */
    BDMF_ERR_OVERFLOW         = -19,  /**< Buffer overflow */
    BDMF_ERR_COMM_FAIL        = -20,  /**< Communication failure */
    BDMF_ERR_NOT_CONNECTED    = -21,  /**< No connection with the target system */
    BDMF_ERR_SYSCALL_ERR      = -22,  /**< System call returned error */
    BDMF_ERR_MSG_ERROR        = -23,  /**< Received message is insane */
    BDMF_ERR_TOO_MANY_REQS    = -24,  /**< Too many outstanding requests */
    BDMF_ERR_NO_MSG_SERVER    = -25,  /**< Remote delivery error. No message server. */
    BDMF_ERR_NO_LOCAL_SUBS    = -26,  /**< Local subsystem is not set */
    BDMF_ERR_NO_SUBS          = -27,  /**< Subsystem is not recognised */
    BDMF_ERR_INTR             = -28,  /**< Operation interrupted */
    BDMF_ERR_HIST_RES_MISMATCH= -29,  /**< History result mismatch */
    BDMF_ERR_MORE             = -30,  /**< More work to do */
    BDMF_ERR_IGNORE           = -31,  /**< Ignore the error */
    BDMF_ERR_LAST         = -100,    /**< Last generic error */
} bdmf_error_t;

/* RDP Feature capability defines for this release */
#define RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT
#define RDPA_DHD_HELPER_FEATURE_LLCSNAPHDR_SUPPORT
// FIXME
//#define RDPA_DHD_HELPER_FEATURE_LBRAGGR_SUPPORT
#define RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT
#define RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT
/*#define RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT*/
#define RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT
#define RDPA_DHD_TX_POST_PHY_RING_SIZE 512
#define RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT


/* #define RDPA_DHD_HELPER_FEATURE_HBQD_SUPPORT */
/* #define RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT */
/* #define DHD_MAX_SSID_NUM 16 */
/* typedef uint32_t rdpa_dhd_ssid_tx_dropped_t[DHD_MAX_SSID_NUM]; */
/* Not supported any more
#define RDPA_DHD_HELPER_FEATURE_NPLUSM
*/

#define RDPA_DHD_HELPER_CPU_QUEUE_SIZE 128 

#define RDPA_MAX_RADIOS 3

#define RDPA_DHD_TX_POST_SKB_BUFFER_VALUE    0 /* 00: possible value in tx complete only */
#define RDPA_DHD_TX_POST_HOST_BUFFER_VALUE   1 /* 01: possible value in tx post and tx complete */
#define RDPA_DHD_TX_POST_BPM_BUFFER_VALUE    2 /* 10: possible value in tx post and tx complete */
#define RDPA_DHD_TX_POST_FKB_BUFFER_VALUE    3 /* 11: possible value in tx complete only */

#include "bcm_rsvmem.h"
#define RDD_RSV_VIRT_TO_PHYS(_vbase, _pbase, _addr) BcmMemReserveVirtToPhys(_vbase, _pbase, _addr)

#define BDMF_MATTR(_var, _unused)                                       \
    archer_dhd_helper_attr_t __var; bdmf_object_handle _var = (bdmf_object_handle)(&__var)

#define BDMF_IRQ_NONE       IRQ_NONE        /**< IRQ is not from this device */
#define BDMF_IRQ_HANDLED    IRQ_HANDLED     /**< IRQ has been handled */

#define rdpa_cpu_wlan0  0

typedef uint32_t rdpa_cpu_port;

typedef void * bdmf_sysb;

typedef int bdmf_number;

typedef uint32_t bdmf_index;

typedef void * bdmf_object_handle;

typedef struct {
    uint32_t radio_idx;
} archer_dhd_helper_attr_t;

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

    uintptr_t dongle_wakeup_register;
    uint8_t  add_llcsnap_header;
    uint8_t  flow_ring_format;               /* 0-legacy, 1- CWI32 */
    uintptr_t dongle_wakeup_register_2;
    uint8_t  dongle_wakeup_hwa;              /* 0: disabled Use wakeup_register for all rings */
                                             /* 1: enabled  Use wakeup_register for txpost, and
                                                wakeup_register_2 for hwa rxpost, rxcpl, txcpl rings */
    void *dongle_wakeup_register_virt;       /* Virtual address of dongle_wakeup_register for rdd access */
    void *dongle_wakeup_register_2_virt;     /* Virtual address of dongle_wakeup_register_2 for rdd access */

    uint8_t  hbqd_mode;                      /* Host Backup Queue Depth (HBQD) mode
                                                0 - disabled, 32 FRs in idma group. 1 - enabled, 16 FRs in idma group */ 
    uint32_t reserved[4];                    /* reserved for next features to keep structure backward compatibility */
} rdpa_dhd_init_cfg_t;

typedef struct
{
    uint32_t radio_idx;
    uint32_t flow_ring_id;
    uint32_t ssid_if_idx;
} rdpa_dhd_tx_post_info_t;

typedef struct
{
    uint32_t dhd_rx_drop; /**< DHD RX drop packets */
    uint32_t dhd_tx_fpm_used; /**< DHD TX FPM used */
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
    uint32_t dhd_tx_drop_packets; /**< DHD RX total dropped packets packets */
} rdpa_dhd_data_stat_t;

typedef struct archer_dhd_backup_queue_entry {
    uint8_t *data_ptr;
    uint32_t request_id;
    uint16_t data_len;
    uint16_t priority;
    struct archer_dhd_backup_queue_entry *next_ptr;
} archer_dhd_backup_queue_entry_t;

typedef struct {
    archer_dhd_backup_queue_entry_t *head_ptr;
    archer_dhd_backup_queue_entry_t *tail_ptr;
    int entries;
} archer_dhd_backup_queue_ctrl_t;

/* Description of TxPost ring for caching */
typedef struct rdpa_dhd_flring_cache
{
    void *base_ptr;
    uint32_t base_addr_low;
    uint32_t base_addr_high;
    uint16_t items; /* Number of descriptors in flow ring (including backup queue if exist) */
#define FLOW_RING_FLAG_DISABLED_HTONS  __constant_htons((uint16_t)(FLOW_RING_FLAG_DISABLED))
#define FLOW_RING_FLAG_DISABLED   (1 << 1)
#define FLOW_RING_FLAG_SSID_SHIFT (8)
#define FLOW_RING_FLAG_SSID_MASK  (0xF << FLOW_RING_FLAG_SSID_SHIFT)
    uint16_t flags;
#ifdef RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT
    uint16_t phy_ring_size;  /* Number of descriptors in physical flow ring */
    uint16_t backup_queue_max;  /* Maximux backup queue items */
#else
    uint32_t reserved;       /* Maintain the structure for platforms without backup queues feature */
#endif
#ifdef RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT
    uint16_t window_ts;     /* Sliding window endpoint timestamp */
    uint16_t drop_interval; /* Number of consequitive drops */
    uint16_t codel_flags;   /* Codel state flags */
    uint16_t codel_drop_counter;
#endif
    uint16_t wr_idx;
    uint16_t rd_idx;
    archer_dhd_backup_queue_ctrl_t backup_queue;
} rdpa_dhd_flring_cache_t;

typedef struct rdpa_dhd_complete_data
{
    uint32_t radio_idx;

    uint32_t request_id;
    uint8_t  buf_type;
    void     *txp;    
    uint16_t status;
    uint16_t flow_ring_id;
} rdpa_dhd_complete_data_t;

typedef struct {
    void *data;
    uint16_t data_offset;
    uint16_t size;
    union {
        uint32_t reason_data;
        uint32_t dest_ssid;
    };
} rdpa_cpu_rx_info_t;

typedef union
{
    uint32_t u32;
    struct {
        uint32_t flowring_idx   :16;
        uint32_t read_idx       :10;
        uint32_t read_idx_valid :1;
        uint32_t reserved       :5;
    };
} rdpa_dhd_ffd_data_t;

typedef void (*rdpa_cpu_rxq_rx_isr_cb_t)(long isr_priv);

typedef struct {
    rdpa_cpu_rxq_rx_isr_cb_t rx_isr;
    long isr_priv;
    uint32_t size;
    volatile uint16_t irq_status;
    volatile uint16_t irq_enable;
    // FIXME: remove this
    void *ring_head;
} rdpa_cpu_rxq_cfg_t;

typedef struct
{
    uint32_t radio_idx;
    uint32_t tx_complete_wakeup_register;
    uint32_t tx_complete_wakeup_value;
    uint32_t rx_complete_wakeup_register;
    uint32_t rx_complete_wakeup_value;
} rdpa_dhd_wakeup_info_t;

#define rdpa_dhd_helper_drv()  0

int bdmf_new_and_set(int unused, void *unused_ptr, bdmf_object_handle mo_, bdmf_object_handle *radio_handle);
int bdmf_destroy(bdmf_object_handle mo);
#define bdmf_put(mo_)

int rdpa_dhd_helper_init_cfg_set(bdmf_object_handle mo_, rdpa_dhd_init_cfg_t * init_cfg);

static inline int rdpa_dhd_helper_radio_idx_set(bdmf_object_handle mo_, uint32_t radio_idx)
{
    archer_dhd_helper_attr_t *attr_p = (archer_dhd_helper_attr_t *)mo_;

    attr_p->radio_idx = radio_idx;

    return 0;
}

int rdp_drv_dhd_cpu_tx(const rdpa_dhd_tx_post_info_t *info, void *buffer, uint32_t  pkt_length);

static inline int rdpa_dhd_helper_send_packet_to_dongle(void *buffer, uint32_t length, const rdpa_dhd_tx_post_info_t *info)
{
    return rdp_drv_dhd_cpu_tx(info, buffer, length);
}

int rdp_drv_dhd_cpu_rx(uint32_t radio_idx, rdpa_cpu_rx_info_t *info);

static inline int rdpa_cpu_packet_get(uint32_t radio_idx, int queue, rdpa_cpu_rx_info_t *info)
{
    return rdp_drv_dhd_cpu_rx(radio_idx, info);
}

int rdp_drv_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info);

static inline int rdpa_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info)
{
    return rdp_drv_dhd_helper_dhd_complete_message_get(dhd_complete_info);
}

void rdp_drv_dhd_complete_wakeup(uint32_t radio_idx, int is_tx_complete);

static inline void rdpa_dhd_helper_complete_wakeup(uint32_t radio_idx, int is_tx_complete)
{
    rdp_drv_dhd_complete_wakeup(radio_idx, is_tx_complete);
}

void archer_dhd_recycle(pNBuff_t pNBuff, unsigned long context, uint32_t flags);

#define bdmf_sysb_recycle archer_dhd_recycle

static inline void *bdmf_sysb_2_fkb_or_skb(void *sysb)
{
    return PNBUFF_2_PBUF(sysb);
}

static inline void rdpa_dhd_helper_doorbell_interrupt_clear(uint32_t radio_idx)
{
}

static inline int rdpa_cpu_rxq_cfg_get(bdmf_object_handle mo_, uint32_t ai_, rdpa_cpu_rxq_cfg_t *rxq_cfg)
{
    memset(rxq_cfg, 0, sizeof(rdpa_cpu_rxq_cfg_t));

    return 0;
}

int rdpa_cpu_rxq_cfg_set(bdmf_object_handle mo_, uint32_t ai_, const rdpa_cpu_rxq_cfg_t *rxq_cfg);

int rdpa_dhd_helper_flush_set(bdmf_object_handle mo_, uint32_t flush);
int rdpa_dhd_helper_flow_ring_enable_set(bdmf_object_handle mo_, uint32_t flow_ring_idx, int enable);
int rdpa_dhd_helper_rx_post_init(bdmf_object_handle mo_);
int rdpa_dhd_helper_rx_post_uninit(bdmf_object_handle mo_);
int rdpa_dhd_helper_rx_post_reinit(bdmf_object_handle mo_);
int rdpa_dhd_helper_tx_complete_send2host_set(bdmf_object_handle mo_, int tx_complete_send2host);
int rdpa_dhd_helper_ssid_tx_dropped_packets_get(bdmf_object_handle mo_, uint32_t ai_, uint32_t *ssid_tx_dropped_packets);
int rdpa_dhd_helper_int_connect_set(bdmf_object_handle mo_, int int_connect);
int rdpa_dhd_helper_aggregation_size_set(bdmf_object_handle mo_, int access_category, int aggregation_size);
int rdpa_dhd_helper_aggregation_timer_set(bdmf_object_handle mo_, int aggregation_timer);
int rdpa_cpu_get(uint32_t radio_idx, bdmf_object_handle *radio_handle);
int rdpa_cpu_num_queues_get(bdmf_object_handle mo_, bdmf_number *num_queues);
int rdpa_cpu_index_get(bdmf_object_handle mo_, rdpa_cpu_port *index_ptr);
void rdpa_cpu_int_enable(uint32_t radio_idx, int queue);
void rdpa_cpu_int_disable(uint32_t radio_idx, int queue);
void rdpa_cpu_int_clear(uint32_t radio_idx, int queue);
int rdpa_cpu_rxq_flush_set(bdmf_object_handle mo_, uint32_t ai_, int rxq_flush);
void rdpa_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info);
int rdpa_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size);
int rdpa_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size);
int rdpa_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info);

static inline void *archer_dhd_buffer_alloc(void)
{
    return gbpm_alloc_buf();
}

static inline void archer_dhd_buffer_free(uint8_t *data_ptr)
{
    gbpm_free_buf(data_ptr);
}

static inline void bdmf_sysb_databuf_free(void *datap, unsigned long context)
{
    archer_dhd_buffer_free(datap);
}

#endif /* CONFIG_BCM_DHD_ARCHER */

#endif /* __ARCHER_DHD_HELPER_H__ */
