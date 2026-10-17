/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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

#ifndef _dhd_runner_h_
#define _dhd_runner_h_

#include <bcmmsgbuf.h>

#if defined(BCM_DHD_OFFLOAD)

#include <dhd_offload.h>

typedef struct dhd_offload              dhd_runner_hlp_t;

#define DHD_RNR_LBR_AGGR                DHD_OFFLOAD_LBR_AGGR
#define DHD_RNR_NO_NONACCPKT_TXSTSOFFL  DHD_OFFLOAD_NO_NONACCPKT_TXSTSOFFL

/* dhd_runner_dma_buf_t */
#define ATTACH_R2D_WR_BUF               ATTACH_O2D_WR_BUF
#define ATTACH_R2D_RD_BUF               ATTACH_O2D_RD_BUF
#define ATTACH_D2R_WR_BUF               ATTACH_D2O_WR_BUF
#define ATTACH_D2R_RD_BUF               ATTACH_D2O_RD_BUF
#define DETACH_R2D_WR_BUF               DETACH_O2D_WR_BUF
#define DETACH_R2D_RD_BUF               DETACH_O2D_RD_BUF
#define DETACH_D2R_WR_BUF               DETACH_D2O_WR_BUF
#define DETACH_D2R_RD_BUF               DETACH_D2O_RD_BUF

/* dhd_runner_ops_t */
/* Host DHD to Runner Notifications (H ---> R) */
#define H2R_DMA_BUF_NOTIF               D2O_DMA_BUF_NOTIF
#define H2R_INIT_NOTIF                  D2O_INIT_NOTIF
#define H2R_RXPOST_NOTIF                D2O_RXPOST_NOTIF
#define H2R_RXPOST_FREE_NOTIF           D2O_RXPOST_FREE_NOTIF
#define H2R_TXPOST_NOTIF                D2O_TXPOST_NOTIF
#define H2R_RX_COMPL_NOTIF              D2O_RX_COMPL_NOTIF
#define H2R_TX_COMPL_NOTIF              D2O_TX_COMPL_NOTIF
#define H2R_FLRING_INIT_NOTIF           D2O_FLRING_INIT_NOTIF

#define H2R_FLRING_ENAB_NOTIF           D2O_FLRING_ENAB_NOTIF
#define H2R_FLRING_DISAB_NOTIF          D2O_FLRING_DISAB_NOTIF
#define H2R_FLRING_FLUSH_NOTIF          D2O_FLRING_FLUSH_NOTIF
#define H2R_AGGR_CONFIG_NOTIF           D2O_AGGR_CONFIG_NOTIF
#define H2R_TXSTS_CONFIG_NOTIF          D2O_TXSTS_CONFIG_NOTIF
#define H2R_MSGRING_FORMAT_NOTIF        D2O_MSGRING_FORMAT_NOTIF
#define H2R_RXPOST_REINIT_NOTIF         D2O_RXPOST_REINIT_NOTIF
#define H2R_PCIE_IPC_CAP_NOTIF          D2O_PCIE_IPC_CAP_NOTIF
#define H2R_PCIE_IPC_CAP1_NOTIF         D2O_PCIE_IPC_CAP1_NOTIF
#define H2R_FLRING_BQSIZE_NOTIF         D2O_FLRING_BQSIZE_NOTIF
#define H2R_PCAP_CFG_NOTIF              D2O_PCAP_CFG_NOTIF

#define H2R_IDX_BUF_RD_REQUEST          D2O_IDX_BUF_RD_REQUEST
#define H2R_IDX_BUF_WR_REQUEST          D2O_IDX_BUF_WR_REQUEST

#define R2H_RX_COMPL_REQUEST            O2D_RX_COMPL_REQUEST
#define R2H_TX_COMPL_REQUEST            O2D_TX_COMPL_REQUEST
#define R2H_WAKE_DNGL_REQUEST           O2D_WAKE_DNGL_REQUEST
typedef uint32                          dhd_runner_ops_t;

/* configuration structure used for H2R_AGGR_CONFIG_NOTIF */
typedef dhd_offload_aggr_config_t dhd_runner_aggr_config_t;

/**
 * H2R_TXSTS_CONFIG_NOTIF bitmap values
 */
#define DHD_RNR_TXSTS_CFG_ACCPKT_OFFL   DHD_OFFLOAD_TXSTS_CFG_ACCPKT_OFFL
#define DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL DHD_OFFLOAD_TXSTS_CFG_NONACCPKT_OFFL
#define DHD_RNR_TXSTS_CFG_OFFL_MASK      DHD_OFFLOAD_TXSTS_CFG_OFFL_MASK

/**
 * H2R_PCIE_IPC_CAP_NOTIF bitmap values
 *
 * The values are not guaranteed to have same as PCIE_IPC_DCAPX_XXXX
 */
#define H2R_PCIE_IPC_CAP_HBQD        D2O_PCIE_IPC_CAP_HBQD
#define H2R_PCIE_IPC_CAP_RX_CMPL_V2  D2O_PCIE_IPC_CAP_RX_CMPL_V2
#define H2R_PCIE_IPC_CAP_RX_CMPL_RCH D2O_PCIE_IPC_CAP_RX_CMPL_RCH
#define H2R_PCIE_IPC_CAP_BA256CFG    D2O_PCIE_IPC_CAP_BA256CFG
#define H2R_PCIE_IPC_CAP_IDMA64      D2O_PCIE_IPC_CAP_IDMA64
#define H2R_PCIE_IPC_CAP_DUMMYTXC    D2O_PCIE_IPC_CAP_DUMMYTXC
#define H2R_PCIE_IPC_CAP_FFRD        D2O_PCIE_IPC_CAP_FFRD

/**
 * Configuration key id's
 */
#define DHD_RNR_IOVAR_PROFILE        DHD_OFFLOAD_IOV_PROFILE
#define DHD_RNR_IOVAR_POLICY         DHD_OFFLOAD_IOV_POLICY
#define DHD_RNR_IOVAR_RXOFFL         DHD_OFFLOAD_IOV_RXOFFL
#define DHD_RNR_IOVAR_RNR_STATS      DHD_OFFLOAD_IOV_STATS
#define DHD_RNR_IOVAR_DUMP           DHD_OFFLOAD_IOV_DUMP
#define DHD_RNR_IOVAR_STATUS         DHD_OFFLOAD_IOV_STATUS
#define DHD_RNR_MAX_IOVARS           DHD_OFFLOAD_IOV_MAX
typedef uint32                       dhd_runner_iovar_t;

/* buffer structure used for DHD_RNR_IOVAR_RNR_STATS */
typedef dhd_offload_stats_t          dhd_helper_rnr_stats_t;
typedef dhd_offload_feat_t           dhd_helper_feat_t;
/* buffer structure used for DHD_RNR_IOVAR_RNR_STATUS */
typedef dhd_offload_status_t         dhd_helper_status_t;

/* structure used for sending non-accelerated TXSTS messages to prot layer */
typedef dhd_offload_txsts_t          dhd_runner_txsts_t;

/* structure used for sending non-accelerated RXCMPL messages to prot layer */
typedef dhd_offload_rxcmplt_t        dhd_runner_rxcmplt_t;

/**
 * dhd_runner_attach():
 * Runner allocates a helper object and initializes it.
 * Invoked on insmod (pcie probe), corresponds to dhd_prot_attach().
 *
 * During dhd_prot_init(), a dhd_runner_notify(H2R_INIT_NOTIF) will be invoked
 * to stage the completion of runner initialization.
 *
 * At this point in time, the DHD does not know the number of TxPost flowrings
 * that are advertized by the dongle (dhd_prot_init phase)
 */
static inline dhd_runner_hlp_t *dhd_runner_attach(dhd_pub_t *dhd,
	bcmpcie_soft_doorbell_t *soft_doorbells)
{
	return (dhd_runner_hlp_t *)dhd_offload_attach(dhd, soft_doorbells);
}
/**
 * dhd_runner_detach():
 * Runner de-initializes itself.
 * Invoked as part of rmmod, dhd_dettach -> dhd_prot_dettach
 */
static inline void dhd_runner_detach(dhd_pub_t * dhd, dhd_runner_hlp_t *dhd_hlp)
{
	return dhd_offload_detach(dhd, (struct dhd_offload_obj *)dhd_hlp);
}
/**
 * dhd_runner_notify():
 * Host DHD notifies Runner to perform an operation.
 */
static inline int dhd_runner_notify(dhd_runner_hlp_t *dhd_hlp,
	dhd_runner_ops_t ops, unsigned long arg1, unsigned long arg2)
{
	return dhd_offload_notify((struct dhd_offload_obj *)dhd_hlp, ops, arg1, arg2);
}
/**
 * dhd_runner_request():
 * Runner requests Host DHD to handle an operation.
 */
static inline int dhd_runner_request(dhd_runner_hlp_t *dhd_hlp,
	dhd_runner_ops_t ops, unsigned long arg1, unsigned long arg2)
{
	return dhd_offload_request((struct dhd_offload_obj *)dhd_hlp, ops, arg1, arg2);
}
/**
 * dhd_runner_flowmgr_init():
 * Initializes a flowid allocator for Runner.
 */
static inline void *dhd_runner_flowmgr_init(dhd_runner_hlp_t *dhd_hlp,
	int max_h2d_rings, int max_bss)
{
	return dhd_offload_flowmgr_init((struct dhd_offload_obj *)dhd_hlp, max_h2d_rings,
	    max_bss);
}
/**
 * dhd_runner_flowmgr_fini():
 * Deallocates a flowid allocator for Runner.
 */
static inline void *dhd_runner_flowmgr_fini(dhd_runner_hlp_t *dhd_hlp,
	void *flowmgr)
{
	return dhd_offload_flowmgr_fini((struct dhd_offload_obj *)dhd_hlp, flowmgr);
}
/**
 * dhd_runner_flowmgr_alloc():
 * Allocate a flowid, given the sta, wme_ac
 */
static inline uint16 dhd_runner_flowmgr_alloc(void *flowmgr,
	int ifidx, int prio, uint8 *mac, bool d11ac, bool *is_hw_ring)
{
	return dhd_offload_flowmgr_alloc(flowmgr, ifidx, prio, mac, d11ac,
	    is_hw_ring);
}
/**
 * dhd_runner_flowmgr_free():
 * Free a previously allocated flowid
 */
static inline int dhd_runner_flowmgr_free(void *flowmgr, uint16 flow_id)
{
	return dhd_offload_flowmgr_free(flowmgr, flow_id);
}
/**
 * dhd_runner_do_iovar():
 * Fill the iovar buffer with the profiles information
 */
static inline int dhd_runner_do_iovar(dhd_runner_hlp_t *dhd_hlp,
	dhd_runner_iovar_t key, bool set, char *buf, int buflen)
{
	return dhd_offload_do_iov((struct dhd_offload_obj *)dhd_hlp, key, set, buf,
	    buflen);
}
/**
 * dhd_runner_process_cpu_queue():
 * Process the Runner-> Host CPU Queue for TxComplete and RxComplete messages
 */
static inline bool dhd_runner_process_cpu_queue(dhd_runner_hlp_t *dhd_hlp,
	bool resched)
{
	return dhd_offload_process_cpu_queue((struct dhd_offload_obj *)dhd_hlp,
	    resched);
}
#else

#error "BCM_DHD_RUNNER deprecated, use code that supports BCM_DHD_OFFLOAD"

#endif /* BCM_DHD_OFFLOAD */
#endif /* _dhd_runner_h_ */
