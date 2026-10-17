/*
   Copyright (c) 2023 Broadcom 
   All Rights Reserved
   <:label-BRCM:2023:DUAL/GPL:standard
   
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

#ifndef _dhd_offload_helper_h_
#define _dhd_offload_helper_h_

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#include <archer_dhd_helper.h>
#include <linux/bcm_log.h>
#include <bcm_pktfwd.h>
#include <bcm_archer.h>
#undef ROUNDUP
#undef ROUNDDN
#else
#include <rdpa_api.h>
#include <rdpa_dhd_helper.h>
#include <rdpa_ag_dhd_helper.h>
#include <rdpa_mw_cpu_queue_ids.h>
#include <rdd.h>
#endif

/**
 * +----------------------------------------------------------------------------
 *           Section: Defines and Macros
 * +----------------------------------------------------------------------------
 */
#if !IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#define DOL_HLPR "Runner"
#elif IS_ENABLED(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)
#define DOL_HLPR "CrossBow"
#else
#define DOL_HLPR "Archer"
#endif

/* DHD Offload helper supported features */
#if defined(RDPA_DHD_HELPER_FEATURE_NPLUSM)
#define DOL_HLPR_CAP_NPLUSM
#endif /* RDPA_DHD_HELPER_FEATURE_NPLUSM */
#if defined(RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT)
#define DOL_HLPR_CAP_TXCMPL2HOST
#endif /* RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_LLCSNAPHDR_SUPPORT)
#define DOL_HLPR_CAP_LLCSNAPHDR
#endif /* DOL_HLPR_CAP_LLCSNAPHDR */

#if defined(RDPA_DHD_HELPER_FEATURE_LBRAGGR_SUPPORT)
#define DOL_HLPR_CAP_LBRAGGR
#endif /* RDPA_DHD_HELPER_FEATURE_LBRAGGR_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT)
#define DOL_HLPR_CAP_MSGRINGFRMT
#endif /* RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT)
#define DOL_HLPR_CAP_FFRD
#endif /*  RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_IDMA_GROUP_CFG_SUPPORT)
#define DOL_HLPR_CAP_IDMAGRPCFG
#endif /*  RDPA_DHD_HELPER_FEATURE_IDMA_GROUP_CFG_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_IDMA64_SUPPORT)
#define DOL_HLPR_CAP_IDMA64
#endif /*  RDPA_DHD_HELPER_FEATURE_IDMA64_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT)
#define DOL_HLPR_CAP_BKUPQUEUE
#endif /* RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT)
#define DOL_HLPR_CAP_HWA_WAKEUP
#endif /* RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT)
#define DOL_HLPR_CAP_CODEL
#endif /* RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_HBQD_SUPPORT)
#define DOL_HLPR_CAP_HBQD
#endif /* RDPA_DHD_HELPER_FEATURE_HBQD_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_FLOWRING_UPDATE_SUPPORT)
#define DOL_HLPR_CAP_DYNBKUPQ
#endif /* RDPA_DHD_HELPER_FEATURE_FLOWRING_UPDATE_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_COMMON_RING_SIZES_CFG_SUPPORT)
#define DOL_HLPR_CAP_CMNRNGSZ
#endif /* RDPA_DHD_HELPER_FEATURE_COMMON_RING_SIZES_CFG_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_MSCS_SUPPORT)
#define DOL_HLPR_CAP_RXCMPLV2
#endif /*  RDPA_DHD_HELPER_FEATURE_MSCS_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_RCH_SUPPORT)
#define DOL_HLPR_CAP_RXCMPLRCH
#endif /*  RDPA_DHD_HELPER_FEATURE_RCH_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_BA256_CFG_SUPPORT)
#define DOL_HLPR_CAP_BA256CFG
#endif /*  RDPA_DHD_HELPER_FEATURE_BA256_CFG_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_PCAP_SUPPORT)
#define DOL_HLPR_CAP_PCAP
#endif /*  RDPA_DHD_HELPER_FEATURE_PCAP_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_DUMMY_TXCOMPL_SUPPORT)
#define DOL_HLPR_CAP_DUMMYTXC
#endif /* RDPA_DHD_HELPER_FEATURE_DUMMY_TXCOMPL_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_TXCPL_EWI_SUPPORT)
#define DOL_HLPR_CAP_TXCPL_EWI
#endif /* RDPA_DHD_HELPER_FEATURE_TXCPL_EWI_SUPPORT */

/* Max concurrent radios */
#define DOL_RADIO_OFFL_MAX                 RDPA_MAX_DHD_OFFL_RADIOS
#if defined(RDPA_MAX_CONCUR_OFFL_RADIOS)
#define DOL_RADIO_CONCUR_OFFL_MAX          RDPA_MAX_CONCUR_OFFL_RADIOS
#else /* !RDPA_MAX_CONCUR_OFFL_RADIOS */
#define DOL_RADIO_CONCUR_OFFL_MAX          RDPA_MAX_DHD_OFFL_RADIOS
#endif /* !RDPA_MAX_CONCUR_OFFL_RADIOS */

/* Logging */
#define ERROR(fmt, args...)                printk(fmt, ##args)

/* Radio Attributes */
#define DOL_HLPR_ATTR_ALLOC(attr)          \
	BDMF_MATTR_ALLOC(attr, rdpa_dhd_helper_drv())
#define DOL_HLPR_ATTR_FREE(attr)           if (attr) { BDMF_MATTR_FREE(attr); }

/* Return values */
#define DOL_HLPR_OK                        BDMF_ERR_OK
#define DOL_HLPR_ERR                       -1  /* BCME_ERROR */
#define DOL_HLPR_NOOBJ                     -25 /* BCME_NOTREADY No radio_obj */

/* CPU Queue sizes for RxC and TxC */
#define DOL_HLPR_CPUQ_RXC_SIZE             RDPA_DHD_HELPER_CPU_QUEUE_SIZE
#define DOL_HLPR_CPUQ_TXC_SIZE             1024 /* D2HRING_TXCMPLT_MAX_ITEM */

/* msgring info definitions */
#define DOL_HLPR_TXP_HOST_BUFF             RDPA_DHD_TX_POST_HOST_BUFFER_VALUE
#define DOL_HLPR_WL_METADATA_TID_SHIFT    RDPA_CPU_RX_INFO_WL_METADATA_TID_SHIFT
#define DOL_HLPR_WL_METADATA_TID_MASK     RDPA_CPU_RX_INFO_WL_METADATA_TID_MASK

/* Locking definitions */
#define hlpr_set_nonblocking_log           bdmf_set_nonblocking_log
#define hlpr_clear_nonblocking_log         bdmf_clear_nonblocking_log

/**
 * +----------------------------------------------------------------------------
 *           Section: Structures
 * +----------------------------------------------------------------------------
 */
typedef rdpa_dhd_flring_cache_t  hlpr_fr_cache_t;
typedef rdpa_dhd_init_cfg_t      hlpr_init_cfg_t;
typedef rdpa_dhd_complete_data_t hlpr_txc_data_t;
typedef rdpa_dhd_pcap_data_t     hlpr_pcap_data_t;
typedef rdpa_cpu_rx_info_t       hlpr_rxc_info_t;
typedef rdpa_cpu_rxq_cfg_t       hlpr_cpuq_cfg_t;
typedef rdpa_dhd_tx_post_info_t  hlpr_txp_info_t;
typedef rdpa_dhd_wakeup_info_t   hlpr_wakeup_info_t;
typedef rdpa_dhd_msg_data_t      hlpr_msgring_data_t;
typedef bdmf_object_handle       hlpr_obj_handle;


/* Offload Helper object structure */
typedef struct dol_helper {
	struct dhd_offload_obj *dhdol;      /* Reference to offload object */
	uint32_t               radio_idx;   /* radio index or dhd unit */
	hlpr_init_cfg_t        init_cfg;    /* init config object */
	hlpr_obj_handle        radio_obj;   /* radio object in Helper */
	hlpr_obj_handle        cpu_obj;     /* cpu object in Helper */
	int                    cpuq_id;     /* Helper -> CPU Queue id */
	int                    cpu_port;    /* CPU port */
#ifndef XRDP
	rdpa_cpu_reason        trap_reason; /* Helper flow miss reason */
#endif
} dol_hlpr_t;


/**
 * +----------------------------------------------------------------------------
 *           Section: Global variables
 * +----------------------------------------------------------------------------
 */
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
static int bdmf_global_trace_level = 0;
#else
extern int bdmf_global_trace_level;
#endif

/**
 * +----------------------------------------------------------------------------
 *           Section: External Function declerations
 * +----------------------------------------------------------------------------
 */

/**
 * +----------------------------------------------------------------------------
 *           Section: Inline wrapper functions
 * +----------------------------------------------------------------------------
 */
/* Allocate SKB for Rx Complete pakcet */
#ifdef XRDP
static inline struct sk_buff* dol_hlpr_alloc_skb(rdpa_cpu_rx_info_t *info)
{
	struct sk_buff *skb;

	skb = skb_header_alloc();
	if (unlikely(!skb))
	    return NULL;

	skb_headerinit(BCM_PKT_HEADROOM + info->data_offset,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION) || defined(CONFIG_CPU_RX_FROM_XPM)
	    SKB_DATA_ALIGN(info->size + BCM_SKB_TAILROOM + info->data_offset),
#else /* CC_NBUFF_FLUSH_OPTIMIZATION */
	    BCM_MAX_PKT_LEN - info->data_offset,
#endif /* CC_NBUFF_FLUSH_OPTIMIZATION */
	    skb, info->data + info->data_offset, bdmf_sysb_recycle, 0, NULL);

	skb_trim(skb, info->size);
	/* no skb recycle,just do data recyle */
	skb->recycle_flags &= SKB_NO_RECYCLE;

	return (bdmf_sysb)SKBUFF_2_PNBUFF(skb);
}
#endif /* XRDP */

/* Get SKB for Rx Complete pakcet */
static inline int dol_hlpr_get_rxc_skb(dol_hlpr_t *hlpr, struct sk_buff **pskb,
	hlpr_rxc_info_t *info)
{
	bdmf_sysb *sysb = (bdmf_sysb*)pskb;
	int rc;

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) && defined(XRDP) && !defined(CONFIG_BCM963158)
	uint32_t flow_key = 0;
#endif

	rc = rdpa_cpu_packet_get(hlpr->cpu_port, hlpr->cpuq_id, info);
	if (rc)
	    return rc;

#ifdef XRDP
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) && !defined(CONFIG_BCM963158)
	if (!info->is_exception)
	{
	    flow_key = *(uint32_t*)((uint8_t*)info->data + info->data_offset);
	    info->data_offset += sizeof(flow_key);
	    info->size -= sizeof(flow_key);
	}
#endif

	*sysb = (bdmf_sysb*)dol_hlpr_alloc_skb(info);
	info->reason_data = info->dest_ssid;
#else /* XRDP */
	/* Extract the ssid ifidx from mapped hw_port */
	info->reason_data = WLAN_NETDEVPATH_SSID(info->reason_data);

	/* create a sysb and initialize it with packet data & len */
	*sysb = bdmf_sysb_header_alloc(bdmf_sysb_skb, (uint8_t *)info->data,
	    info->data_offset, info->size, 0,
	    info->rx_csum_verified?BDMF_RX_CSUM_VERIFIED_MASK:0);
#endif /* XRDP */

	if (!*sysb) {
	    ERROR("%s: Failed to allocate skb header\n", __FUNCTION__);
	    bdmf_sysb_databuf_free((uint8_t *)info->data, 0);
	    return -1;
	}

	*sysb = bdmf_sysb_2_fkb_or_skb(*sysb);

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) && defined(XRDP) && !defined(CONFIG_BCM963158)
	if (flow_key)
	    skbuff_bcm_ext_fc_ctxt_set(((struct sk_buff *)(*sysb)), flow_key);
#endif

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	    (*pskb)->ip_summed = info->csum_verified;
#endif

	return 0;
}

/*
 * Wrapper for rdpa_cpu_int_enable
 * Get Tx Complete pakcet data
 */
static inline int dol_hlpr_get_txc_data(dol_hlpr_t *hlpr,
	hlpr_txc_data_t *txc_data)
{
	txc_data->radio_idx = hlpr->radio_idx;

	/* Fetch each tx complete packet from Helper queue */
	return rdpa_dhd_helper_dhd_complete_message_get(txc_data);
}

/*
 * Wrapper for rdpa_cpu_int_enable
 * Get PCAP pakcet data
 */
static inline int dol_hlpr_get_pcap_data(dol_hlpr_t *hlpr,
	hlpr_pcap_data_t *pcap_data)
{
	/* Fetch each PCAP packet from Helper queue */
	return rdpa_dhd_helper_dhd_pcap_packet_get(pcap_data);
}

/*
 * Wrapper for rdpa_cpu_int_enable
 * Send host packet to dongle on TxPost rings
 */
static inline int dol_hlpr_send_txp(dol_hlpr_t *hlpr, void *pkt,
	uint32_t pktlen, hlpr_txp_info_t *info)
{
	info->radio_idx = hlpr->radio_idx;

	return rdpa_dhd_helper_send_packet_to_dongle(pkt, pktlen, info);
}

/* Wrapper for rdpa_cpu_rxq_empty */
static inline int dol_hlpr_empty_cpuq(dol_hlpr_t *hlpr) {
#if IS_ENABLED(CONFIG_BCM_DHD_CROSSBOW)
	return rdpa_cpu_rxq_empty(hlpr->cpu_port, hlpr->cpuq_id);
#else
	return 1;
#endif
}

/* Wrapper for rdpa_cpu_int_enable */
static inline void dol_hlpr_enable_cpuq_int(dol_hlpr_t *hlpr) {
	return rdpa_cpu_int_enable(hlpr->cpu_port, hlpr->cpuq_id);
}

/* Wrapper for rdpa_cpu_int_disable */
static inline void dol_hlpr_disable_cpuq_int(dol_hlpr_t *hlpr) {
	return rdpa_cpu_int_disable(hlpr->cpu_port, hlpr->cpuq_id);
}

/* Wrapper for rdpa_cpu_int_clear */
static inline void dol_hlpr_clear_cpuq_int(dol_hlpr_t *hlpr) {
	return rdpa_cpu_int_clear(hlpr->cpu_port, hlpr->cpuq_id);
}

/* Wrapper for rdpa_dhd_helper_doorbell_interrupt_clear */
static inline void dol_hlpr_clear_db_int(dol_hlpr_t *hlpr) {
	return rdpa_dhd_helper_doorbell_interrupt_clear(hlpr->radio_idx);
}

/* Get CPU Queue ID */
static inline int dol_hlpr_get_cpuq_id(dol_hlpr_t *hlpr)
{
#ifdef XRDP
	bdmf_number num_queues;
	int rc;

	/* In XRDP, we use dedicated cpu objects per radio, as allocated by WFD.
	 * When create cpu object, WFD will allow to configure 1 extra queue for DHD
	 * exception traffic, which will be the last queue in it's range. If WFD is
	 * not * configured, use same logic as in RDP.
	 */
	rc = rdpa_cpu_num_queues_get(hlpr->cpu_obj, &num_queues);

	if (rc != 0) {
	    ERROR("dor%d rdpa_cpu_num_queues_get(0x%px, 0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, &num_queues, rc);
	    num_queues = 0;
	}
	return (int)num_queues - 1;
#else
	int cpu_queues[] = {RDPA_DHD_HELPER_1_CPU_QUEUE,
	    RDPA_DHD_HELPER_2_CPU_QUEUE,
	    RDPA_DHD_HELPER_3_CPU_QUEUE};

	return cpu_queues[hlpr->radio_idx];
#endif /* XRDP */
}

/* Configure the Offload Helper to DHD cpu queue */
static inline int dol_hlpr_config_cpuq(dol_hlpr_t *hlpr, hlpr_cpuq_cfg_t *cfg)
{
	int rc;
#ifndef XRDP
	int i;
	int cpu_reasons[] = {rdpa_cpu_rx_reason_pci_ip_flow_miss_1,
	    rdpa_cpu_rx_reason_pci_ip_flow_miss_2,
	    rdpa_cpu_rx_reason_pci_ip_flow_miss_3};
#endif
	rdpa_cpu_rxq_cfg_t rxq_cfg = {};

#ifndef XRDP
	hlpr->trap_reason = cpu_reasons[hlpr->radio_idx];
#endif

	if (hlpr->cpuq_id < 0) {
	    ERROR("dol%d: Error - Invalid cpuq_id [%d]\n", hlpr->radio_idx,
	        hlpr->cpuq_id);
	    rc = BDMF_ERR_PARM;
	    goto exit;
	}

	/* Get the RxC ring configuration */
	rc = rdpa_cpu_rxq_cfg_get(hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg);
	if (rc != 0) {
	    ERROR("dol%d: rdpa_cpu_rxq_cfg_get(0x%px, %d, 0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg, rc);
	}

	rxq_cfg.size = DOL_HLPR_CPUQ_RXC_SIZE;
	rxq_cfg.isr_priv = cfg->isr_priv;
	rxq_cfg.rx_isr = cfg->rx_isr;

	rc = rdpa_cpu_rxq_cfg_set(hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg);
	if (rc < 0) {
	    ERROR("dol%d: rdpa_cpu_rxq_cfg_set(0x%px, %d, 0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg, rc);
	    goto exit;
	}

	rc = rdpa_dhd_helper_dhd_complete_ring_create(hlpr->radio_idx,
	        DOL_HLPR_CPUQ_TXC_SIZE);
	if (rc < 0) {
	    ERROR("rdpa_dhd_helper_dhd_complete_ring_create(%d, %d) error %d\n",
	        hlpr->radio_idx, DOL_HLPR_CPUQ_TXC_SIZE, rc);
	    goto exit;
	}

#if !IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#ifdef XRDP
	{
	    rdpa_dhd_cpu_data_t cpu_data;

	    cpu_data.cpu_port = hlpr->cpu_port;
	    cpu_data.exception_rxq = hlpr->cpuq_id;

	    if (cfg->size)
	    {
	        /* Rx Offload Enabled */
	        rc = rdpa_cpu_tc_to_rxq_set(hlpr->cpu_obj, hlpr->cpuq_id,
	            hlpr->cpuq_id);
	        if (rc != 0) {
	            ERROR("dol%d: rdpa_cpu_tc_to_rxq_set error %d\n",
	            hlpr->radio_idx, rc);
	        } else {
	            cpu_data.set_exception_tc_to_rxq = true;
	            rc = rdpa_dhd_helper_cpu_data_set(hlpr->radio_obj, &cpu_data);
	        }
	    } else if (rc == 0) {
	        /*
	         * do not touch tc to queue configuration,
	         * just setup interrupt for TxComplete
	         */
	        cpu_data.set_exception_tc_to_rxq = false;
	        rc = rdpa_dhd_helper_cpu_data_set(hlpr->radio_obj, &cpu_data);
	    }

	    if (rc < 0) {
	        ERROR("dol%d: rdpa_dhd_helper_cpu_data_set error %d\n",
	             hlpr->radio_idx, rc);
	        return rc;
	    }
	}
#else /* RDP */
	for (i = 0; i < 2; i++) {
	    rdpa_cpu_reason_cfg_t reason_cfg = {};
	    rdpa_cpu_reason_index_t cpu_reason = {};

	    cpu_reason.reason = hlpr->trap_reason;
	    cpu_reason.dir = i ? rdpa_dir_us : rdpa_dir_ds;
	    reason_cfg.queue = hlpr->cpuq_id;
	    reason_cfg.meter = BDMF_INDEX_UNASSIGNED;
	    rc = rdpa_cpu_reason_cfg_set(hlpr->cpu_obj, &cpu_reason, &reason_cfg);
	    if (rc < 0) {
	        ERROR("dol%d: rdpa_cpu_reason_cfg_set error %d\n",
	            hlpr->radio_idx, rc);
	        goto exit;
	    }
	}
#endif /* RDP */
#endif /* !CONFIG_BCM_DHD_ARCHER */

exit:
	return rc;
}

/* Un-Configure the Offload Helper to DHD cpu queue */
static inline int dol_hlpr_unconfig_cpuq(dol_hlpr_t *hlpr, hlpr_cpuq_cfg_t *cfg)
{
	int rc;
#ifndef XRDP
	int i;
#endif
	rdpa_cpu_rxq_cfg_t rxq_cfg = {};
	rdpa_cpu_port cpu_port;


	/* Cpu object is not configured yet, nothing to do */
	if (!hlpr->cpu_obj)
	    return 0;

	rc = rdpa_cpu_index_get(hlpr->cpu_obj, &cpu_port);
	if (rc != 0) {
	    ERROR("dol%d: rdpa_cpu_index_get(0x%px, 0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, &cpu_port, rc);
	}

	dol_hlpr_disable_cpuq_int(hlpr);
	dol_hlpr_clear_cpuq_int(hlpr);

	rc = rdpa_cpu_rxq_flush_set(hlpr->cpu_obj, hlpr->cpuq_id, 1);
	if (rc != 0) {
	    ERROR("dol%d: rdpa_cpu_rxq_flush_set(0x%px, %d, 1) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, hlpr->cpuq_id, rc);
	}

	/* Get the RxC ring configuration */
	rc = rdpa_cpu_rxq_cfg_get(hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg);
	if (rc != 0) {
	    ERROR("dol%d: rdpa_cpu_rxq_cfg_get(0x%px, %d, 0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg, rc);
	}

	/* Destroy the RxC ring */
	rxq_cfg.size = 0;
	rxq_cfg.isr_priv = (long)NULL;
	rxq_cfg.rx_isr = NULL;
	rxq_cfg.ring_head = NULL;

	rc = rdpa_cpu_rxq_cfg_set(hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg);
	if (rc < 0) {
	    ERROR("dol%d: rdpa_cpu_rxq_cfg_set(0x%px, %d, 0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, hlpr->cpuq_id, &rxq_cfg, rc);
	    goto exit;
	}

	/* Destroy the TxC ring */
	rc = rdpa_dhd_helper_dhd_complete_ring_destroy(hlpr->radio_idx,
	        DOL_HLPR_CPUQ_TXC_SIZE);
	if (rc < 0) {
	    ERROR("rdpa_dhd_helper_dhd_complete_ring_destroy(%d, %d) error %d\n",
	        hlpr->radio_idx, DOL_HLPR_CPUQ_TXC_SIZE, rc);
	    goto exit;
	}

#if !IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#ifdef XRDP
	rc = rdpa_dhd_helper_cpu_data_set(hlpr->radio_obj, NULL);
	if (rc != 0) {
	    ERROR("dol%d: rdpa_dhd_helper_cpu_data_set(0x%px, NULL) error %d\n",
	        hlpr->radio_idx, hlpr->cpu_obj, rc);
	}
#endif
#endif /* !CONFIG_BCM_DHD_ARCHER */

exit:
	return rc;
}

/* initialize the cpuq object */
static inline int dol_hlpr_init_cpuq(dol_hlpr_t *hlpr)
{
	int rc;
	rdpa_cpu_port cpu_port;

#ifdef XRDP
	cpu_port = rdpa_cpu_wlan0 + hlpr->radio_idx;
#else
	cpu_port = rdpa_cpu_host;
	hlpr->trap_reason = cpu_reasons[hlpr->radio_idx];
#endif

	rc = rdpa_cpu_get(cpu_port, &hlpr->cpu_obj);
	if (rc) {
	    ERROR("dol%d: rdpa_cpu_get(%d, 0x%px) error %d\n",
	        hlpr->radio_idx, cpu_port, &hlpr->cpu_obj, rc);
	    return rc;
	}

	hlpr->cpu_port = cpu_port;
	hlpr->cpuq_id = dol_hlpr_get_cpuq_id(hlpr);
	if (hlpr->cpuq_id < 0)
	    rc = DOL_HLPR_ERR;

	return rc;
}

/* De-initialize the cpuq object */
static inline void dol_hlpr_uninit_cpuq(dol_hlpr_t *hlpr)
{
	if (hlpr->cpu_obj) {
	    bdmf_put(hlpr->cpu_obj);
	    hlpr->cpu_obj = NULL;
	}

	return;
}

/* Init Connect radio in the helper */
static inline int dol_hlpr_set_init_connect(dol_hlpr_t *hlpr,
	hlpr_obj_handle hlpr_attr)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_int_connect_set(hlpr->radio_obj, true);
	if (rc) {
	    ERROR("dol%d: Failed rdpa_dhd_helper_int_connect_set, error %d\n",
	        hlpr->radio_idx, rc);
	    rc = DOL_HLPR_ERR;
	}

	return rc;
}

/* initlaize Offload object in the helper */
static inline int dol_hlpr_init_radio(dol_hlpr_t *hlpr,
	hlpr_obj_handle hlpr_attr)
{
	int rc;

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	if (hlpr->init_cfg.rx_complete_flow_ring_base_addr) {
	    hlpr->init_cfg.rxoffl = 1;
	} else {
	    hlpr->init_cfg.rxoffl = 0;
	}

	if (hlpr->init_cfg.tx_complete_flow_ring_base_addr) {
	    hlpr->init_cfg.txoffl = 1;
	} else {
	    hlpr->init_cfg.txoffl = 0;
	}
#endif

	rc = rdpa_dhd_helper_radio_idx_set(hlpr_attr, hlpr->radio_idx);
	if (rc != 0) {
	    ERROR("dol%d: rdpa_dhd_helper_radio_idx_set(0x%px, %d) error %d\n",
	        hlpr->radio_idx, &hlpr_attr, hlpr->radio_idx, rc);
	    return DOL_HLPR_ERR;
	}

	rc = rdpa_dhd_helper_init_cfg_set(hlpr_attr, &hlpr->init_cfg);
	if (rc != 0) {
	    ERROR("dol%d: rdpa_dhd_helper_init_cfg_set(0x%px, 0x%px) error %d\n",
	        hlpr->radio_idx, &hlpr_attr, &hlpr->init_cfg, rc);
	    return DOL_HLPR_ERR;
	}

	rc = bdmf_new_and_set(rdpa_dhd_helper_drv(), NULL, hlpr_attr,
	        &hlpr->radio_obj);
	if (rc) {
	    ERROR("dol%d: bdmf_new_and_set error %d\n", hlpr->radio_idx, rc);
	    return DOL_HLPR_ERR;
	}

	return rc;
}

/* De-initlaize Offload object in the helper */
static inline void dol_hlpr_uninit_radio(dol_hlpr_t *hlpr)
{
	if (hlpr->radio_obj)
	    bdmf_destroy(hlpr->radio_obj);

	hlpr->radio_obj = NULL;

	return;
}

/* Wrapper for rdpa_dhd_helper_tx_complete_send2host_set */
static inline int dol_hlpr_set_tx_complete_send2host(dol_hlpr_t *hlpr,
	bdmf_object_handle mo, bool enable)
{
	int rc;

	rc = rdpa_dhd_helper_tx_complete_send2host_set(mo, enable);
	if (rc < 0)
	    ERROR("dol%d: set tx_complete_send2host error %d\n",
	        hlpr->radio_idx, rc);

	return rc;
}

#if defined(DOL_HLPR_CAP_CODEL)
/* Wrapper for rdpa_dhd_helper_codel_enable_set */
static inline int dol_hlpr_set_codel_enable(dol_hlpr_t *hlpr, bool enable)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_codel_enable_set(hlpr->radio_obj, enable);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_codel_enable_set error rc: %d\n",
	        hlpr->radio_idx, rc);

	return rc;
}

/* Wrapper for rdpa_dhd_helper_codel_enable_get */
static inline int dol_hlpr_get_codel_enable(dol_hlpr_t *hlpr, bool *enable)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_codel_enable_get(hlpr->radio_obj,
	        (bdmf_boolean *)enable);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_codel_enable_get error rc: %d\n",
	        hlpr->radio_idx, rc);

	return rc;
}
#endif /* DOL_HLPR_CAP_CODEL */

/* Wrapper for rdpa_dhd_helper_flow_ring_enable_set */
static inline int dol_hlpr_set_flow_ring_enable(dol_hlpr_t *hlpr,
	uint16_t ringid, bool enable)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_flow_ring_enable_set(hlpr->radio_obj,
	        (bdmf_index)ringid, enable);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_flow_ring_enable_set(0x%px, %d, %d)"
	        " error %d\n", hlpr->radio_idx, hlpr->radio_obj, ringid,
	        enable, rc);

	return rc;
}

#if defined(DOL_HLPR_CAP_LBRAGGR)
/* Wrapper for dol_hlpr_set_aggregation_size */
static inline int dol_hlpr_set_aggregation_size(dol_hlpr_t *hlpr,
	uint32_t wme_ac, uint32_t size)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_aggregation_size_set(hlpr->radio_obj,
	        (bdmf_index)wme_ac, (bdmf_number)size);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_aggregation_size_set error %d\n",
	        hlpr->radio_idx, rc);

	return rc;
}

/* Wrapper for rdpa_dhd_helper_aggregation_timer_set */
static inline int dol_hlpr_set_aggregation_timer(dol_hlpr_t *hlpr,
	uint32_t timer)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_aggregation_timer_set(hlpr->radio_obj,
	        (bdmf_number)timer);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_aggregation_timer_set(%d) error %d\n",
	        hlpr->radio_idx, timer, rc);

	return rc;
}
#endif /* DOL_HLPR_CAP_LBRAGGR */

/* Wrapper for rdpa_dhd_helper_flow_ring_update_set */
static inline int dol_hlpr_set_flow_ring_update(dol_hlpr_t *hlpr,
	uint32_t param)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_flow_ring_update_set(hlpr->radio_obj,
	        (bdmf_number)param);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_flow_ring_update_set(0x%px, 0x%x) "
	        "error %d\n", hlpr->radio_idx, hlpr->radio_obj, param, rc);

	return rc;
}

/* Wrapper for rdpa_dhd_helper_ssid_tx_dropped_packets_get */
static inline int dol_hlpr_get_ssid_tx_dropped_pkts(dol_hlpr_t *hlpr,
	uint32_t ssid, uint32_t *pkts)
{
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_ssid_tx_dropped_packets_get(hlpr->radio_obj,
	        (bdmf_index)ssid, (bdmf_number *)pkts);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_ssid_tx_dropped_packets_get Error %d\n",
	        hlpr->radio_idx, rc);

	return rc;
}

/* Wrapper for rdpa_dhd_helper_wakeup_information_get */
static inline void dol_hlpr_get_wakeup_info(dol_hlpr_t *hlpr,
	hlpr_wakeup_info_t *wakeup_info)
{
	return rdpa_dhd_helper_wakeup_information_get(wakeup_info);
}

/* Wrapper for rdpa_dhd_helper_rx_post_init */
static inline int dol_hlpr_init_rx_post(dol_hlpr_t *hlpr) {
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	bdmf_set_nonblocking_log();
	rc = rdpa_dhd_helper_rx_post_init(hlpr->radio_obj);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_rx_post_init(0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->radio_obj, rc);
	bdmf_clear_nonblocking_log();

	return rc;
}

/* Wrapper for rdpa_dhd_helper_rx_post_uninit */
static inline int dol_hlpr_uninit_rx_post(dol_hlpr_t *hlpr) {
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	bdmf_set_nonblocking_log();
	rc = rdpa_dhd_helper_rx_post_uninit(hlpr->radio_obj);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_rx_post_uninit(0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->radio_obj, rc);
	bdmf_clear_nonblocking_log();

	return rc;
}

/* Wrapper for rdpa_dhd_helper_rx_post_reinit */
static inline int dol_hlpr_reinit_rx_post(dol_hlpr_t *hlpr) {
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_rx_post_reinit(hlpr->radio_obj);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_rx_post_reinit(0x%px) error %d\n",
	        hlpr->radio_idx, hlpr->radio_obj, rc);

	return rc;
}

/* Wrapper for rdpa_dhd_helper_complete_wakeup */
static inline void dol_hlpr_wakeup_complete(dol_hlpr_t *hlpr, bool is_txc) {
	return rdpa_dhd_helper_complete_wakeup(hlpr->radio_idx, is_txc);
}

/* Wrapper for rdpa_dhd_helper_flush_set */
static inline int dol_hlpr_set_flush(dol_hlpr_t *hlpr, uint32_t param) {
	int rc;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	rc = rdpa_dhd_helper_flush_set(hlpr->radio_obj, (bdmf_number)param);
	if (rc < 0)
	    ERROR("dol%d: rdpa_dhd_helper_flush_set(0x%px, %d) error %d\n",
	        hlpr->radio_idx, hlpr->radio_obj, param, rc);

	return rc;
}

/* Wrapper for archer_wlan_get_int_radio_index */
static inline int dol_hlpr_get_radio_idx(int dhd_unit) {
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	int radio_idx;

	radio_idx = archer_wlan_get_int_radio_index(dhd_unit);

	if (radio_idx >= DOL_RADIO_CONCUR_OFFL_MAX) {
	   ERROR("dol%d: radio_index %d exceeded max %d\n", dhd_unit,
	       radio_idx, DOL_RADIO_CONCUR_OFFL_MAX);
	}

	return radio_idx;
#else /* !CONFIG_BCM_DHD_CROSSBOW */
	return dhd_unit;
#endif /* !CONFIG_BCM_DHD_CROSSBOW */
}

#if defined(DOL_HLPR_CAP_PCAP)
/* Wrapper for rdpa_dhd_helper_pcap_cfg_set */
static inline int dol_hlpr_pcap_cfg_set(dol_hlpr_t *hlpr, uint8 rx_en, uint8 tx_en, uint16 maxlen)
{
	int rc;
	rdpa_dhd_pcap_cfg_t pcap_cfg;

	if (hlpr->radio_obj == NULL) {
	    ERROR("dol%d: %s error: %s helper radio object not initialized",
	        hlpr->radio_idx, __FUNCTION__, DOL_HLPR);
	    return DOL_HLPR_NOOBJ;
	}

	pcap_cfg.rx_copy_en = rx_en;
	pcap_cfg.tx_copy_en = tx_en;
	pcap_cfg.pcap_max_len = maxlen;
	memset(&pcap_cfg.reserved, 0, sizeof(pcap_cfg.reserved));

	rc = rdpa_dhd_helper_pcap_cfg_set(hlpr->radio_obj, &pcap_cfg);
	if (rc < 0) {
	    ERROR("dol%d: rdpa_dhd_helper_pcap_cfg_set(%d, %d, %d) error: %d", hlpr->radio_idx,
	         rx_en, tx_en, maxlen, rc);
	}
	return rc;
}
#endif /* DOL_HLPR_CAP_PCAP */

#endif /* _dhd_offload_helper_h_ */
