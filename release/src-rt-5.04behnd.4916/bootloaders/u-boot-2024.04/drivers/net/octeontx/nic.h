/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef NIC_H
#define	NIC_H

#include <linux/netdevice.h>
#include "bgx.h"

#define	PCI_DEVICE_ID_CAVIUM_NICVF_1	0x0011

/* Subsystem device IDs */
#define PCI_SUBSYS_DEVID_88XX_NIC_PF		0xA11E
#define PCI_SUBSYS_DEVID_81XX_NIC_PF		0xA21E
#define PCI_SUBSYS_DEVID_83XX_NIC_PF		0xA31E

#define PCI_SUBSYS_DEVID_88XX_PASS1_NIC_VF	0xA11E
#define PCI_SUBSYS_DEVID_88XX_NIC_VF		0xA134
#define PCI_SUBSYS_DEVID_81XX_NIC_VF		0xA234
#define PCI_SUBSYS_DEVID_83XX_NIC_VF		0xA334

#define	NIC_INTF_COUNT			2  /* Interfaces btw VNIC and TNS/BGX */
#define	NIC_CHANS_PER_INF		128
#define	NIC_MAX_CHANS			(NIC_INTF_COUNT * NIC_CHANS_PER_INF)

/* PCI BAR nos */
#define	PCI_CFG_REG_BAR_NUM		0
#define	PCI_MSIX_REG_BAR_NUM		4

/* NIC SRIOV VF count */
#define	MAX_NUM_VFS_SUPPORTED		128
#define	DEFAULT_NUM_VF_ENABLED		8

#define	NIC_TNS_BYPASS_MODE		0
#define	NIC_TNS_MODE			1

/* NIC priv flags */
#define	NIC_SRIOV_ENABLED		BIT(0)
#define	NIC_TNS_ENABLED			BIT(1)

/* VNIC HW optimiation features */
#define	VNIC_RX_CSUM_OFFLOAD_SUPPORT
#undef	VNIC_TX_CSUM_OFFLOAD_SUPPORT
#undef	VNIC_SG_SUPPORT
#undef	VNIC_TSO_SUPPORT
#undef	VNIC_LRO_SUPPORT
#undef  VNIC_RSS_SUPPORT

/* TSO not supported in Thunder pass1 */
#ifdef	VNIC_TSO_SUPPORT
#define	VNIC_SW_TSO_SUPPORT
#undef	VNIC_HW_TSO_SUPPORT
#endif

/* ETHTOOL enable or disable, undef this to disable */
#define	NICVF_ETHTOOL_ENABLE

/* Min/Max packet size */
#define	NIC_HW_MIN_FRS			64
#define	NIC_HW_MAX_FRS			9200 /* 9216 max packet including FCS */

/* Max pkinds */
#define	NIC_MAX_PKIND			16

/* Max when CPI_ALG is IP diffserv */
#define	NIC_MAX_CPI_PER_LMAC		64

/* NIC VF Interrupts */
#define	NICVF_INTR_CQ			0
#define	NICVF_INTR_SQ			1
#define	NICVF_INTR_RBDR			2
#define	NICVF_INTR_PKT_DROP		3
#define	NICVF_INTR_TCP_TIMER	4
#define	NICVF_INTR_MBOX			5
#define	NICVF_INTR_QS_ERR		6

#define	NICVF_INTR_CQ_SHIFT			0
#define	NICVF_INTR_SQ_SHIFT			8
#define	NICVF_INTR_RBDR_SHIFT		16
#define	NICVF_INTR_PKT_DROP_SHIFT	20
#define	NICVF_INTR_TCP_TIMER_SHIFT	21
#define	NICVF_INTR_MBOX_SHIFT		22
#define	NICVF_INTR_QS_ERR_SHIFT		23

#define	NICVF_INTR_CQ_MASK		(0xFF << NICVF_INTR_CQ_SHIFT)
#define	NICVF_INTR_SQ_MASK		(0xFF << NICVF_INTR_SQ_SHIFT)
#define	NICVF_INTR_RBDR_MASK		(0x03 << NICVF_INTR_RBDR_SHIFT)
#define	NICVF_INTR_PKT_DROP_MASK	BIT(NICVF_INTR_PKT_DROP_SHIFT)
#define	NICVF_INTR_TCP_TIMER_MASK	BIT(NICVF_INTR_TCP_TIMER_SHIFT)
#define	NICVF_INTR_MBOX_MASK		BIT(NICVF_INTR_MBOX_SHIFT)
#define	NICVF_INTR_QS_ERR_MASK		BIT(NICVF_INTR_QS_ERR_SHIFT)

/* MSI-X interrupts */
#define	NIC_PF_MSIX_VECTORS		10
#define	NIC_VF_MSIX_VECTORS		20

#define NIC_PF_INTR_ID_ECC0_SBE		0
#define NIC_PF_INTR_ID_ECC0_DBE		1
#define NIC_PF_INTR_ID_ECC1_SBE		2
#define NIC_PF_INTR_ID_ECC1_DBE		3
#define NIC_PF_INTR_ID_ECC2_SBE		4
#define NIC_PF_INTR_ID_ECC2_DBE		5
#define NIC_PF_INTR_ID_ECC3_SBE		6
#define NIC_PF_INTR_ID_ECC3_DBE		7
#define NIC_PF_INTR_ID_MBOX0		8
#define NIC_PF_INTR_ID_MBOX1		9

/* Global timer for CQ timer thresh interrupts
 * Calculated for SCLK of 700Mhz
 * value written should be a 1/16thof what is expected
 *
 * 1 tick per ms
 */
#define NICPF_CLK_PER_INT_TICK		43750

struct nicvf_cq_poll {
	u8	cq_idx;		/* Completion queue index */
};

#define NIC_MAX_RSS_HASH_BITS		8
#define NIC_MAX_RSS_IDR_TBL_SIZE	BIT(NIC_MAX_RSS_HASH_BITS)
#define RSS_HASH_KEY_SIZE		5 /* 320 bit key */

#ifdef VNIC_RSS_SUPPORT
struct nicvf_rss_info {
	bool enable;
#define	RSS_L2_EXTENDED_HASH_ENA	BIT(0)
#define	RSS_IP_HASH_ENA			BIT(1)
#define	RSS_TCP_HASH_ENA		BIT(2)
#define	RSS_TCP_SYN_DIS			BIT(3)
#define	RSS_UDP_HASH_ENA		BIT(4)
#define RSS_L4_EXTENDED_HASH_ENA	BIT(5)
#define	RSS_ROCE_ENA			BIT(6)
#define	RSS_L3_BI_DIRECTION_ENA		BIT(7)
#define	RSS_L4_BI_DIRECTION_ENA		BIT(8)
	u64 cfg;
	u8  hash_bits;
	u16 rss_size;
	u8  ind_tbl[NIC_MAX_RSS_IDR_TBL_SIZE];
	u64 key[RSS_HASH_KEY_SIZE];
};
#endif

enum rx_stats_reg_offset {
	RX_OCTS = 0x0,
	RX_UCAST = 0x1,
	RX_BCAST = 0x2,
	RX_MCAST = 0x3,
	RX_RED = 0x4,
	RX_RED_OCTS = 0x5,
	RX_ORUN = 0x6,
	RX_ORUN_OCTS = 0x7,
	RX_FCS = 0x8,
	RX_L2ERR = 0x9,
	RX_DRP_BCAST = 0xa,
	RX_DRP_MCAST = 0xb,
	RX_DRP_L3BCAST = 0xc,
	RX_DRP_L3MCAST = 0xd,
	RX_STATS_ENUM_LAST,
};

enum tx_stats_reg_offset {
	TX_OCTS = 0x0,
	TX_UCAST = 0x1,
	TX_BCAST = 0x2,
	TX_MCAST = 0x3,
	TX_DROP = 0x4,
	TX_STATS_ENUM_LAST,
};

struct nicvf_hw_stats {
	u64 rx_bytes_ok;
	u64 rx_ucast_frames_ok;
	u64 rx_bcast_frames_ok;
	u64 rx_mcast_frames_ok;
	u64 rx_fcs_errors;
	u64 rx_l2_errors;
	u64 rx_drop_red;
	u64 rx_drop_red_bytes;
	u64 rx_drop_overrun;
	u64 rx_drop_overrun_bytes;
	u64 rx_drop_bcast;
	u64 rx_drop_mcast;
	u64 rx_drop_l3_bcast;
	u64 rx_drop_l3_mcast;
	u64 tx_bytes_ok;
	u64 tx_ucast_frames_ok;
	u64 tx_bcast_frames_ok;
	u64 tx_mcast_frames_ok;
	u64 tx_drops;
};

struct nicvf_drv_stats {
	/* Rx */
	u64 rx_frames_ok;
	u64 rx_frames_64;
	u64 rx_frames_127;
	u64 rx_frames_255;
	u64 rx_frames_511;
	u64 rx_frames_1023;
	u64 rx_frames_1518;
	u64 rx_frames_jumbo;
	u64 rx_drops;
	/* Tx */
	u64 tx_frames_ok;
	u64 tx_drops;
	u64 tx_busy;
	u64 tx_tso;
};

struct hw_info {
	u8		bgx_cnt;
	u8		chans_per_lmac;
	u8		chans_per_bgx; /* Rx/Tx chans */
	u8		chans_per_rgx;
	u8		chans_per_lbk;
	u16		cpi_cnt;
	u16		rssi_cnt;
	u16		rss_ind_tbl_size;
	u16		tl4_cnt;
	u16		tl3_cnt;
	u8		tl2_cnt;
	u8		tl1_cnt;
	bool		tl1_per_bgx; /* TL1 per BGX or per LMAC */
	u8		model_id;
};

struct nicvf {
	struct udevice		*dev;
	u8			vf_id;
	bool			sqs_mode:1;
	bool			loopback_supported:1;
	u8			tns_mode;
	u8			node;
	u16		mtu;
	struct queue_set	*qs;
#define		MAX_SQS_PER_VF_SINGLE_NODE	5
#define		MAX_SQS_PER_VF			11
	u8			num_qs;
	void			*addnl_qs;
	u16		vf_mtu;
	void __iomem		*reg_base;
#define	MAX_QUEUES_PER_QSET			8
	struct nicvf_cq_poll	*napi[8];

	u8			cpi_alg;

	struct nicvf_hw_stats	stats;
	struct nicvf_drv_stats	drv_stats;

	struct nicpf		*nicpf;

	/* VF <-> PF mailbox communication */
	bool			pf_acked;
	bool			pf_nacked;
	bool			set_mac_pending;

	bool			link_up;
	u8			duplex;
	u32		speed;
	u8			rev_id;
	u8			rx_queues;
	u8			tx_queues;

	bool			open;
	bool			rb_alloc_fail;
	void			*rcv_buf;
	bool			hw_tso;
};

static inline int node_id(void *addr)
{
	return ((uintptr_t)addr >> 44) & 0x3;
}

struct nicpf {
	struct udevice		*udev;
	struct hw_info		*hw;
	u8			node;
	unsigned int		flags;
	u16			total_vf_cnt;	/* Total num of VF supported */
	u16			num_vf_en;	/* No of VF enabled */
	void __iomem		*reg_base;	/* Register start address */
	u16			rss_ind_tbl_size;
	u8			num_sqs_en;	/* Secondary qsets enabled */
	u64			nicvf[MAX_NUM_VFS_SUPPORTED];
	u8			vf_sqs[MAX_NUM_VFS_SUPPORTED][MAX_SQS_PER_VF];
	u8			pqs_vf[MAX_NUM_VFS_SUPPORTED];
	bool			sqs_used[MAX_NUM_VFS_SUPPORTED];
	struct pkind_cfg	pkind;
	u8			bgx_cnt;
	u8			rev_id;
#define	NIC_SET_VF_LMAC_MAP(bgx, lmac)	((((bgx) & 0xF) << 4) | ((lmac) & 0xF))
#define	NIC_GET_BGX_FROM_VF_LMAC_MAP(map)	(((map) >> 4) & 0xF)
#define	NIC_GET_LMAC_FROM_VF_LMAC_MAP(map)	((map) & 0xF)
	u8			vf_lmac_map[MAX_LMAC];
	u16			cpi_base[MAX_NUM_VFS_SUPPORTED];
	u64			mac[MAX_NUM_VFS_SUPPORTED];
	bool			mbx_lock[MAX_NUM_VFS_SUPPORTED];
	u8			link[MAX_LMAC];
	u8			duplex[MAX_LMAC];
	u32			speed[MAX_LMAC];
	bool			vf_enabled[MAX_NUM_VFS_SUPPORTED];
	u16			rssi_base[MAX_NUM_VFS_SUPPORTED];
	u8			lmac_cnt;
};

/* PF <--> VF Mailbox communication
 * Eight 64bit registers are shared between PF and VF.
 * Separate set for each VF.
 * Writing '1' into last register mbx7 means end of message.
 */

/* PF <--> VF mailbox communication */
#define	NIC_PF_VF_MAILBOX_SIZE		2
#define	NIC_PF_VF_MBX_TIMEOUT		2000 /* ms */

/* Mailbox message types */
#define	NIC_MBOX_MSG_READY		0x01	/* Is PF ready to rcv msgs */
#define	NIC_MBOX_MSG_ACK		0x02	/* ACK the message received */
#define	NIC_MBOX_MSG_NACK		0x03	/* NACK the message received */
#define	NIC_MBOX_MSG_QS_CFG		0x04	/* Configure Qset */
#define	NIC_MBOX_MSG_RQ_CFG		0x05	/* Configure receive queue */
#define	NIC_MBOX_MSG_SQ_CFG		0x06	/* Configure Send queue */
#define	NIC_MBOX_MSG_RQ_DROP_CFG	0x07	/* Configure receive queue */
#define	NIC_MBOX_MSG_SET_MAC		0x08	/* Add MAC ID to DMAC filter */
#define	NIC_MBOX_MSG_SET_MAX_FRS	0x09	/* Set max frame size */
#define	NIC_MBOX_MSG_CPI_CFG		0x0A	/* Config CPI, RSSI */
#define	NIC_MBOX_MSG_RSS_SIZE		0x0B	/* Get RSS indir_tbl size */
#define	NIC_MBOX_MSG_RSS_CFG		0x0C	/* Config RSS table */
#define	NIC_MBOX_MSG_RSS_CFG_CONT	0x0D	/* RSS config continuation */
#define	NIC_MBOX_MSG_RQ_BP_CFG		0x0E	/* RQ backpressure config */
#define	NIC_MBOX_MSG_RQ_SW_SYNC		0x0F	/* Flush inflight pkts to RQ */
#define	NIC_MBOX_MSG_BGX_STATS		0x10	/* Get stats from BGX */
#define	NIC_MBOX_MSG_BGX_LINK_CHANGE	0x11	/* BGX:LMAC link status */
#define	NIC_MBOX_MSG_ALLOC_SQS		0x12	/* Allocate secondary Qset */
#define	NIC_MBOX_MSG_NICVF_PTR		0x13	/* Send nicvf ptr to PF */
#define	NIC_MBOX_MSG_PNICVF_PTR		0x14	/* Get primary qset nicvf ptr */
#define	NIC_MBOX_MSG_SNICVF_PTR		0x15	/* Send sqet nicvf ptr to PVF */
#define	NIC_MBOX_MSG_LOOPBACK		0x16	/* Set interface in loopback */
#define	NIC_MBOX_MSG_CFG_DONE		0xF0	/* VF configuration done */
#define	NIC_MBOX_MSG_SHUTDOWN		0xF1	/* VF is being shutdown */

struct nic_cfg_msg {
	u8    msg;
	u8    vf_id;
	u8    node_id;
	bool  tns_mode:1;
	bool  sqs_mode:1;
	bool  loopback_supported:1;
	u8    mac_addr[6];
};

/* Qset configuration */
struct qs_cfg_msg {
	u8    msg;
	u8    num;
	u8    sqs_count;
	u64   cfg;
};

/* Receive queue configuration */
struct rq_cfg_msg {
	u8    msg;
	u8    qs_num;
	u8    rq_num;
	u64   cfg;
};

/* Send queue configuration */
struct sq_cfg_msg {
	u8    msg;
	u8    qs_num;
	u8    sq_num;
	bool  sqs_mode;
	u64   cfg;
};

/* Set VF's MAC address */
struct set_mac_msg {
	u8    msg;
	u8    vf_id;
	u8    mac_addr[6];
};

/* Set Maximum frame size */
struct set_frs_msg {
	u8    msg;
	u8    vf_id;
	u16   max_frs;
};

/* Set CPI algorithm type */
struct cpi_cfg_msg {
	u8    msg;
	u8    vf_id;
	u8    rq_cnt;
	u8    cpi_alg;
};

/* Get RSS table size */
struct rss_sz_msg {
	u8    msg;
	u8    vf_id;
	u16   ind_tbl_size;
};

/* Set RSS configuration */
struct rss_cfg_msg {
	u8    msg;
	u8    vf_id;
	u8    hash_bits;
	u8    tbl_len;
	u8    tbl_offset;
#define RSS_IND_TBL_LEN_PER_MBX_MSG	8
	u8    ind_tbl[RSS_IND_TBL_LEN_PER_MBX_MSG];
};

struct bgx_stats_msg {
	u8    msg;
	u8    vf_id;
	u8    rx;
	u8    idx;
	u64   stats;
};

/* Physical interface link status */
struct bgx_link_status {
	u8    msg;
	u8    link_up;
	u8    duplex;
	u32   speed;
};

#ifdef VNIC_MULTI_QSET_SUPPORT
/* Get Extra Qset IDs */
struct sqs_alloc {
	u8    msg;
	u8    vf_id;
	u8    qs_count;
};

struct nicvf_ptr {
	u8    msg;
	u8    vf_id;
	bool  sqs_mode;
	u8    sqs_id;
	u64   nicvf;
};
#endif

/* Set interface in loopback mode */
struct set_loopback {
	u8    msg;
	u8    vf_id;
	bool  enable;
};

/* 128 bit shared memory between PF and each VF */
union nic_mbx {
	struct { u8 msg; }	msg;
	struct nic_cfg_msg	nic_cfg;
	struct qs_cfg_msg	qs;
	struct rq_cfg_msg	rq;
	struct sq_cfg_msg	sq;
	struct set_mac_msg	mac;
	struct set_frs_msg	frs;
	struct cpi_cfg_msg	cpi_cfg;
	struct rss_sz_msg	rss_size;
	struct rss_cfg_msg	rss_cfg;
	struct bgx_stats_msg    bgx_stats;
	struct bgx_link_status  link_status;
#ifdef VNIC_MULTI_QSET_SUPPORT
	struct sqs_alloc        sqs_alloc;
	struct nicvf_ptr	nicvf;
#endif
	struct set_loopback	lbk;
};

int nicvf_set_real_num_queues(struct udevice *dev,
			      int tx_queues, int rx_queues);
int nicvf_open(struct udevice *dev);
void nicvf_stop(struct udevice *dev);
int nicvf_send_msg_to_pf(struct nicvf *vf, union nic_mbx *mbx);
void nicvf_update_stats(struct nicvf *nic);

void nic_handle_mbx_intr(struct nicpf *nic, int vf);

int bgx_poll_for_link(int node, int bgx_idx, int lmacid);
const u8 *bgx_get_lmac_mac(int node, int bgx_idx, int lmacid);
void bgx_set_lmac_mac(int node, int bgx_idx, int lmacid, const u8 *mac);
void bgx_lmac_rx_tx_enable(int node, int bgx_idx, int lmacid, bool enable);
void bgx_lmac_internal_loopback(int node, int bgx_idx,
				int lmac_idx, bool enable);

static inline bool pass1_silicon(unsigned int revision, int model_id)
{
	return ((revision < 8) && (model_id == 0x88));
}

static inline bool pass2_silicon(unsigned int revision, int model_id)
{
	return ((revision >= 8) && (model_id == 0x88));
}

#endif /* NIC_H */
