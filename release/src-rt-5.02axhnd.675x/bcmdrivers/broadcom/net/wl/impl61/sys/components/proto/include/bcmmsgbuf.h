/*
 * MSGBUF network driver ioctl/indication encoding
 * Broadcom 802.11abg Networking Device Driver
 *
 * Definitions subject to change without notice.
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcmmsgbuf.h 749158 2018-02-27 22:57:16Z $
 */
#ifndef _bcmmsgbuf_h_
#define	_bcmmsgbuf_h_

#include <ethernet.h>
#include <wlioctl.h>
#include <bcmpcie.h>

#define MSGBUF_MAX_MSG_SIZE   ETHER_MAX_LEN

#define D2H_EPOCH_MODULO		253 /* sequence number wrap */
#define D2H_EPOCH_INIT_VAL		(D2H_EPOCH_MODULO + 1)

#define H2D_EPOCH_MODULO		253 /* sequence number wrap */
#define H2D_EPOCH_INIT_VAL		(H2D_EPOCH_MODULO + 1)

#define H2DRING_TXPOST_ITEMSIZE		48
#define H2DRING_RXPOST_ITEMSIZE		32

#define H2DRING_CTRL_SUB_ITEMSIZE	40

#define D2HRING_TXCMPLT_ITEMSIZE	16
#define D2HRING_RXCMPLT_ITEMSIZE	32

#define D2HRING_CTRL_CMPLT_ITEMSIZE	24
#define H2DRING_INFO_BUFPOST_ITEMSIZE	H2DRING_CTRL_SUB_ITEMSIZE
#define D2HRING_INFO_BUFCMPLT_ITEMSIZE	D2HRING_CTRL_CMPLT_ITEMSIZE

#define H2DRING_TXPOST_MAX_ITEM			512
#define H2DRING_RXPOST_MAX_ITEM			512
#define H2DRING_CTRL_SUB_MAX_ITEM		64
#define D2HRING_TXCMPLT_MAX_ITEM		1024
#define D2HRING_RXCMPLT_MAX_ITEM		512

#define H2DRING_DYNAMIC_INFO_MAX_ITEM          32
#define D2HRING_DYNAMIC_INFO_MAX_ITEM          32

#ifdef BCM_ROUTER_DHD
#define D2HRING_CTRL_CMPLT_MAX_ITEM		256
#else
#define D2HRING_CTRL_CMPLT_MAX_ITEM		64
#endif // endif

enum {
	DNGL_TO_HOST_MSGBUF,
	HOST_TO_DNGL_MSGBUF
};

enum {
	HOST_TO_DNGL_TXP_DATA,
	HOST_TO_DNGL_RXP_DATA,
	HOST_TO_DNGL_CTRL,
	DNGL_TO_HOST_DATA,
	DNGL_TO_HOST_CTRL
};

#define MESSAGE_PAYLOAD(a) (a & MSG_TYPE_INTERNAL_USE_START) ? TRUE : FALSE
#define PCIEDEV_FIRMWARE_TSINFO 0x1
#define PCIEDEV_FIRMWARE_TSINFO_FIRST	0x1
#define PCIEDEV_FIRMWARE_TSINFO_MIDDLE	0x2
#define PCIEDEV_BTLOG_POST				0x3

#ifdef PCIE_API_REV1

#define BCMMSGBUF_DUMMY_REF(a, b)	do {BCM_REFERENCE((a));BCM_REFERENCE((b));}  while (0)

#define BCMMSGBUF_API_IFIDX(a)		0
#define BCMMSGBUF_API_SEQNUM(a)		0
#define BCMMSGBUF_IOCTL_XTID(a)		0
#define BCMMSGBUF_IOCTL_PKTID(a)	((a)->cmd_id)

#define BCMMSGBUF_SET_API_IFIDX(a, b)	BCMMSGBUF_DUMMY_REF(a, b)
#define BCMMSGBUF_SET_API_SEQNUM(a, b)	BCMMSGBUF_DUMMY_REF(a, b)
#define BCMMSGBUF_IOCTL_SET_PKTID(a, b)	(BCMMSGBUF_IOCTL_PKTID(a) = (b))
#define BCMMSGBUF_IOCTL_SET_XTID(a, b)	BCMMSGBUF_DUMMY_REF(a, b)

#else /* PCIE_API_REV1 */

#define BCMMSGBUF_API_IFIDX(a)		((a)->if_id)
#define BCMMSGBUF_IOCTL_PKTID(a)	((a)->pkt_id)
#define BCMMSGBUF_API_SEQNUM(a)		((a)->u.seq.seq_no)
#define BCMMSGBUF_IOCTL_XTID(a)		((a)->xt_id)

#define BCMMSGBUF_SET_API_IFIDX(a, b)	(BCMMSGBUF_API_IFIDX((a)) = (b))
#define BCMMSGBUF_SET_API_SEQNUM(a, b)	(BCMMSGBUF_API_SEQNUM((a)) = (b))
#define BCMMSGBUF_IOCTL_SET_PKTID(a, b)	(BCMMSGBUF_IOCTL_PKTID((a)) = (b))
#define BCMMSGBUF_IOCTL_SET_XTID(a, b)	(BCMMSGBUF_IOCTL_XTID((a)) = (b))

#endif /* PCIE_API_REV1 */

/* utility data structures */

/* IOCTL req Hdr */
/* cmn Msg Hdr */
typedef struct cmn_msg_hdr {
	/** message type */
	uint8 msg_type;
	/** interface index this is valid for */
	uint8 if_id;
	/* flags */
	uint8 flags;
	/** sequence number */
	uint8 epoch;
	/** packet Identifier for the associated host buffer */
	uint32 request_id;
} cmn_msg_hdr_t;

/** message type */
typedef enum bcmpcie_msgtype {
	MSG_TYPE_GEN_STATUS		= 0x1,
	MSG_TYPE_RING_STATUS		= 0x2,
	MSG_TYPE_FLOW_RING_CREATE	= 0x3,
	MSG_TYPE_FLOW_RING_CREATE_CMPLT	= 0x4,
	/* Enum value as copied from BISON 7.15: new generic message */
	MSG_TYPE_RING_CREATE_CMPLT	= 0x4,
	MSG_TYPE_FLOW_RING_DELETE	= 0x5,
	MSG_TYPE_FLOW_RING_DELETE_CMPLT	= 0x6,
	/* Enum value as copied from BISON 7.15: new generic message */
	MSG_TYPE_RING_DELETE_CMPLT	= 0x6,
	MSG_TYPE_FLOW_RING_FLUSH	= 0x7,
	MSG_TYPE_FLOW_RING_FLUSH_CMPLT	= 0x8,
	MSG_TYPE_IOCTLPTR_REQ		= 0x9,
	MSG_TYPE_IOCTLPTR_REQ_ACK	= 0xA,
	MSG_TYPE_IOCTLRESP_BUF_POST	= 0xB,
	MSG_TYPE_IOCTL_CMPLT		= 0xC,
	MSG_TYPE_EVENT_BUF_POST		= 0xD,
	MSG_TYPE_WL_EVENT		= 0xE,
	MSG_TYPE_TX_POST		= 0xF,
	MSG_TYPE_TX_STATUS		= 0x10,
	MSG_TYPE_RXBUF_POST		= 0x11,
	MSG_TYPE_RX_CMPLT		= 0x12,
	MSG_TYPE_LPBK_DMAXFER		= 0x13,
	MSG_TYPE_LPBK_DMAXFER_CMPLT	= 0x14,
	MSG_TYPE_FLOW_RING_RESUME	 = 0x15,
	MSG_TYPE_FLOW_RING_RESUME_CMPLT	= 0x16,
	MSG_TYPE_FLOW_RING_SUSPEND	= 0x17,
	MSG_TYPE_FLOW_RING_SUSPEND_CMPLT	= 0x18,
	MSG_TYPE_INFO_BUF_POST		= 0x19,
	MSG_TYPE_INFO_BUF_CMPLT		= 0x1A,
	MSG_TYPE_H2D_RING_CREATE	= 0x1B,
	MSG_TYPE_D2H_RING_CREATE	= 0x1C,
	MSG_TYPE_H2D_RING_CREATE_CMPLT	= 0x1D,
	MSG_TYPE_D2H_RING_CREATE_CMPLT	= 0x1E,
	MSG_TYPE_H2D_RING_CONFIG	= 0x1F,
	MSG_TYPE_D2H_RING_CONFIG	= 0x20,
	MSG_TYPE_H2D_RING_CONFIG_CMPLT	= 0x21,
	MSG_TYPE_D2H_RING_CONFIG_CMPLT	= 0x22,
	MSG_TYPE_H2D_MAILBOX_DATA	= 0x23,
	MSG_TYPE_D2H_MAILBOX_DATA	= 0x24,
	MSG_TYPE_TIMSTAMP_BUFPOST	= 0x25,
	MSG_TYPE_HOSTTIMSTAMP		= 0x26,
	MSG_TYPE_HOSTTIMSTAMP_CMPLT	= 0x27,
	MSG_TYPE_FIRMWARE_TIMESTAMP	= 0x28,
	MSG_TYPE_SNAPSHOT_UPLOAD	= 0x29,
	MSG_TYPE_SNAPSHOT_CMPLT		= 0x2A,
	MSG_TYPE_H2D_RING_DELETE	= 0x2B,
	MSG_TYPE_D2H_RING_DELETE	= 0x2C,
	MSG_TYPE_H2D_RING_DELETE_CMPLT	= 0x2D,
	MSG_TYPE_D2H_RING_DELETE_CMPLT	= 0x2E,
	MSG_TYPE_API_MAX_RSVD		= 0x3F
} bcmpcie_msg_type_t;

typedef enum bcmpcie_msgtype_int {
	MSG_TYPE_INTERNAL_USE_START	= 0x40,
	MSG_TYPE_EVENT_PYLD		= 0x41,
	MSG_TYPE_IOCT_PYLD		= 0x42,
	MSG_TYPE_RX_PYLD		= 0x43,
	MSG_TYPE_HOST_FETCH		= 0x44,
	MSG_TYPE_LPBK_DMAXFER_PYLD	= 0x45,
	MSG_TYPE_TXMETADATA_PYLD	= 0x46,
	MSG_TYPE_INDX_UPDATE		= 0x47,
	MSG_TYPE_INFO_PYLD		= 0x48,
	MSG_TYPE_TS_EVENT_PYLD		= 0x49,
	MSG_TYPE_PVT_BTLOG_CMPLT	= 0x4A,
	MSG_TYPE_BTLOG_PYLD		= 0x4B,
	MSG_TYPE_HMAPTEST_PYLD		= 0x4C,
	MSG_TYPE_BUZZZ_STREAM		= 0x4D
} bcmpcie_msgtype_int_t;

typedef enum bcmpcie_msgtype_u {
	MSG_TYPE_TX_BATCH_POST		= 0x80,
	MSG_TYPE_IOCTL_REQ		= 0x81,
	MSG_TYPE_HOST_EVNT		= 0x82, /* console related */
	MSG_TYPE_LOOPBACK		= 0x83
} bcmpcie_msgtype_u_t;

/**
 * D2H ring host wakeup soft doorbell, override the PCIE doorbell.
 * Host configures a <64bit address,value> tuple, and dongle uses SBTOPCIE
 * Transl0 to write specified value to host address.
 *
 * Only lower 32bit address is used in SBTOPCIE.
 *
 * Use case: 32bit Address mapped to HW Accelerator Core/Thread Wakeup Register
 * and value is Core/Thread context. Host will ensure routing the 32bit address
 * offerred to PCIE to the mapped register.
 *
 * D2H_RING_CONFIG_SUBTYPE_SOFT_DOORBELL
 * When a D2H TxCpln or RxCpln ring is not configured with soft doorbell, then
 * a ringer with haddr.low defaulting to PCIE DB Dev2Host 0, will be used. A
 * PCIE Legacy INT to host will be sent on posting to the completion ring.
 */
typedef struct bcmpcie_soft_doorbell {
	uint32	value;  /* host defined value to be written, eg HW threadid */
	uint32  rsvd;
	haddr64_t haddr; /* host address, eg thread wakeup register address */
	uint16	items;  /* interrupt coalescing: item count before wakeup */
	uint16	msecs;  /* interrupt coalescing: timeout in millisecs */
} bcmpcie_soft_doorbell_t;

/**
 * D2H interrupt using MSI instead of INTX
 * Host configures MSI vector offset for each D2H interrupt
 *
 * D2H_RING_CONFIG_SUBTYPE_MSI_DOORBELL
 */
typedef enum bcmpcie_msi_intr_idx {
	MSI_INTR_IDX_CTRL_CMPL_RING	= 0,
	MSI_INTR_IDX_TXP_CMPL_RING	= 1,
	MSI_INTR_IDX_RXP_CMPL_RING	= 2,
	MSI_INTR_IDX_INFO_CMPL_RING	= 3,
	MSI_INTR_IDX_MAILBOX		= 4,
	MSI_INTR_IDX_MAX		= 5
} bcmpcie_msi_intr_idx_t;

typedef enum bcmpcie_msi_offset_type {
	BCMPCIE_D2H_MSI_OFFSET_MB0	= 2,
	BCMPCIE_D2H_MSI_OFFSET_MB1	= 3,
	BCMPCIE_D2H_MSI_OFFSET_DB0	= 4,
	BCMPCIE_D2H_MSI_OFFSET_DB1	= 5,
	BCMPCIE_D2H_MSI_OFFSET_H1_DB0	= 6,
	BCMPCIE_D2H_MSI_OFFSET_MAX	= 7
} bcmpcie_msi_offset_type_t;

typedef struct bcmpcie_msi_offset {
	uint16	intr_idx;    /* interrupt index */
	uint16	msi_offset;  /* msi vector offset */
} bcmpcie_msi_offset_t;

typedef struct bcmpcie_msi_offset_config {
	uint32	len;
	bcmpcie_msi_offset_t	bcmpcie_msi_offset[MSI_INTR_IDX_MAX];
} bcmpcie_msi_offset_config_t;

#define BCMPCIE_D2H_MSI_OFFSET_DEFAULT	BCMPCIE_D2H_MSI_OFFSET_DB1

#define BCMPCIE_D2H_MSI_SINGLE		0xFFFE

/* if_id */
#define BCMPCIE_CMNHDR_IFIDX_PHYINTF_SHFT	5
#define BCMPCIE_CMNHDR_IFIDX_PHYINTF_MAX	0x7
#define BCMPCIE_CMNHDR_IFIDX_PHYINTF_MASK	\
	(BCMPCIE_CMNHDR_IFIDX_PHYINTF_MAX << BCMPCIE_CMNHDR_IFIDX_PHYINTF_SHFT)
#define BCMPCIE_CMNHDR_IFIDX_VIRTINTF_SHFT	0
#define BCMPCIE_CMNHDR_IFIDX_VIRTINTF_MAX	0x1F
#define BCMPCIE_CMNHDR_IFIDX_VIRTINTF_MASK	\
	(BCMPCIE_CMNHDR_IFIDX_PHYINTF_MAX << BCMPCIE_CMNHDR_IFIDX_PHYINTF_SHFT)

/* flags */
#define BCMPCIE_CMNHDR_FLAGS_DMA_R_IDX		0x1
#define BCMPCIE_CMNHDR_FLAGS_DMA_R_IDX_INTR	0x2
#define BCMPCIE_CMNHDR_FLAGS_TS_SEQNUM_INIT	0x4
#define BCMPCIE_CMNHDR_FLAGS_PHASE_BIT		0x80
#define BCMPCIE_CMNHDR_PHASE_BIT_INIT		0x80

/* IOCTL request message */
typedef struct ioctl_req_msg {
	/** common message header */
	cmn_msg_hdr_t	cmn_hdr;
	/** ioctl command type */
	uint32		cmd;
	/** ioctl transaction ID, to pair with a ioctl response */
	uint16		trans_id;
	/** input arguments buffer len */
	uint16		input_buf_len;
	/** expected output len */
	uint16		output_buf_len;
	/** to align the host address on 8 byte boundary */
	uint16		rsvd[3];
	/** always align on 8 byte boundary */
	haddr64_t	host_input_buf_addr;
	/* rsvd */
	uint32		rsvd1[2];
} ioctl_req_msg_t;

/** buffer post messages for device to use to return IOCTL responses, Events */
typedef struct ioctl_resp_evt_buf_post_msg {
	/** common message header */
	cmn_msg_hdr_t	cmn_hdr;
	/** length of the host buffer supplied */
	uint16		host_buf_len;
	/** to align the host address on 8 byte boundary */
	uint16		reserved[3];
	/** always align on 8 byte boundary */
	haddr64_t	host_buf_addr;
	uint32		rsvd[4];
} ioctl_resp_evt_buf_post_msg_t;

/* buffer post messages for device to use to return dbg buffers */
typedef ioctl_resp_evt_buf_post_msg_t info_buf_post_msg_t;

/* An infobuf host buffer starts with a 32 bit (LE) version. */
#define PCIE_INFOBUF_V1                1
/* Infobuf v1 type MSGTRACE's data is exactly the same as the MSGTRACE data that
 * is wrapped previously/also in a WLC_E_TRACE event.  See structure
 * msgrace_hdr_t in msgtrace.h.
*/
#define PCIE_INFOBUF_V1_TYPE_MSGTRACE  1

/* Infobuf v1 type LOGTRACE data is exactly the same as the LOGTRACE data that
 * is wrapped previously/also in a WLC_E_TRACE event.  See structure
 * msgrace_hdr_t in msgtrace.h.  (The only difference between a MSGTRACE
 * and a LOGTRACE is the "trace type" field.)
*/
#define PCIE_INFOBUF_V1_TYPE_LOGTRACE  2

/* An infobuf version 1 host buffer has a single TLV.  The information on the
 * version 1 types follow this structure definition. (int's LE)
*/
typedef struct info_buf_payload_hdr_s {
	uint16 type;
	uint16 length;
} info_buf_payload_hdr_t;

/* BT logging data to DMA directly from BT memory to host */
typedef struct info_buf_btlog_s {
	void (*status_cb)(void *ctx, void *p, int error);
	void *ctx;
	dma64addr_t src_addr;
	uint32 length;
} info_buf_btlog_t;

#define PCIE_DMA_XFER_FLG_D11_LPBK_MASK	0x00000001
#define PCIE_DMA_XFER_FLG_D11_LPBK_SHIFT	0

typedef struct pcie_dma_xfer_params {
	/** common message header */
	cmn_msg_hdr_t	cmn_hdr;

	/** always align on 8 byte boundary */
	haddr64_t	host_input_buf_addr;

	/** always align on 8 byte boundary */
	haddr64_t	host_ouput_buf_addr;

	/** length of transfer */
	uint32		xfer_len;
	/** delay before doing the src txfer */
	uint32		srcdelay;
	/** delay before doing the dest txfer */
	uint32		destdelay;
	uint8		rsvd[3];
	/* bit0: D11 DMA loopback flag */
	uint8		flags;
} pcie_dma_xfer_params_t;

/** Complete msgbuf hdr for flow ring update from host to dongle */
typedef struct tx_flowring_create_request {
	cmn_msg_hdr_t   msg;
	uint8	da[ETHER_ADDR_LEN];
	uint8	sa[ETHER_ADDR_LEN];
	uint8	tid;
	uint8	item_type;
	uint16	flow_ring_id;
	uint8	tc;
	/* priority_ifrmmask is to define core mask in ifrm mode.
	 * currently it is not used for priority. so uses solely for ifrm mask
	 */
	uint8	priority_ifrmmask;
	uint16	int_vector;
	uint16	max_items;
	uint16	len_item; /* item_size */
	haddr64_t haddr64; /* 8 byte aligned field, host address of flowring mem */
} tx_flowring_create_request_t;

typedef struct tx_flowring_delete_request {
	cmn_msg_hdr_t   msg;
	uint16	flow_ring_id;
	uint16	reason;
	uint32	rsvd[7];
} tx_flowring_delete_request_t;

typedef tx_flowring_delete_request_t d2h_ring_delete_req_t;
typedef tx_flowring_delete_request_t h2d_ring_delete_req_t;

typedef struct tx_flowring_flush_request {
	cmn_msg_hdr_t   msg;
	uint16	flow_ring_id;
	uint16	reason;
	uint32	rsvd[7];
} tx_flowring_flush_request_t;

/** Subtypes for ring_config_req control message */
typedef enum ring_config_subtype {
	/** Default D2H PCIE doorbell override using ring_config_req msg */
	D2H_RING_CONFIG_SUBTYPE_SOFT_DOORBELL = 1, /* Software doorbell */
	D2H_RING_CONFIG_SUBTYPE_MSI_DOORBELL = 2   /* MSI configuration */
} ring_config_subtype_t;

typedef struct ring_config_req {
	cmn_msg_hdr_t	msg;
	uint16	subtype;
	uint16	ring_id;
	uint32	rsvd;
	union {
		uint32  data[6];
		/** D2H_RING_CONFIG_SUBTYPE_SOFT_DOORBELL */
		bcmpcie_soft_doorbell_t soft_doorbell;
		/** D2H_RING_CONFIG_SUBTYPE_MSI_DOORBELL */
		bcmpcie_msi_offset_config_t msi_offset;
	};
} ring_config_req_t;

/* data structure to use to create on the fly d2h rings */
typedef struct d2h_ring_create_req {
	cmn_msg_hdr_t	msg;
	uint16	ring_id;
	uint16	ring_type;
	uint32	flags;
	haddr64_t	haddr64; /* 8 byte aligned field */
	uint16	max_items;
	uint16	len_item;
	uint32	rsvd[3];
} d2h_ring_create_req_t;

/* data structure to use to create on the fly h2d rings */
#define MAX_COMPLETION_RING_IDS_ASSOCIATED	4
typedef struct h2d_ring_create_req {
	cmn_msg_hdr_t	msg;
	uint16	ring_id;
	uint8	ring_type;
	uint8	n_completion_ids;
	uint32	flags;
	haddr64_t	haddr64; /* 8 byte aligned field */
	uint16	max_items;
	uint16	len_item;
	uint16	completion_ring_ids[MAX_COMPLETION_RING_IDS_ASSOCIATED];
	uint32	rsvd;
} h2d_ring_create_req_t;

typedef struct d2h_ring_config_req {
	cmn_msg_hdr_t   msg;
	uint16	d2h_ring_config_subtype;
	uint16	d2h_ring_id;
	uint32  d2h_ring_config_data[4];
	uint32  rsvd[3];
} d2h_ring_config_req_t;

typedef struct h2d_ring_config_req {
	cmn_msg_hdr_t   msg;
	uint16	h2d_ring_config_subtype;
	uint16	h2d_ring_id;
	uint32  h2d_ring_config_data;
	uint32  rsvd[6];
} h2d_ring_config_req_t;

typedef struct h2d_mailbox_data {
	cmn_msg_hdr_t   msg;
	uint32	mail_box_data;
	uint32  rsvd[7];
} h2d_mailbox_data_t;

typedef struct host_timestamp_msg {
	cmn_msg_hdr_t	msg;
	uint16		xt_id; /* transaction ID */
	uint16		input_data_len; /* data len at the host_buf_addr, data in TLVs */
	uint16		seqnum; /* number of times host captured the timestamp */
	uint16		rsvd;
	/* always align on 8 byte boundary */
	haddr64_t	host_buf_addr;
	/* rsvd */
	uint32      rsvd1[4];
} host_timestamp_msg_t;

/* buffer post message for timestamp events MSG_TYPE_TIMSTAMP_BUFPOST */
typedef ioctl_resp_evt_buf_post_msg_t ts_buf_post_msg_t;

typedef union ctrl_submit_item {
	ioctl_req_msg_t			ioctl_req;
	ioctl_resp_evt_buf_post_msg_t	resp_buf_post;
	pcie_dma_xfer_params_t		dma_xfer;
	tx_flowring_create_request_t	flow_create;
	tx_flowring_delete_request_t	flow_delete;
	tx_flowring_flush_request_t	flow_flush;
	ring_config_req_t		ring_config_req;
	d2h_ring_create_req_t		d2h_create;
	h2d_ring_create_req_t		h2d_create;
	d2h_ring_config_req_t		d2h_config;
	h2d_ring_config_req_t		h2d_config;
	h2d_mailbox_data_t		h2d_mailbox_data;
	host_timestamp_msg_t		host_ts;
	ts_buf_post_msg_t		ts_buf_post;
	d2h_ring_delete_req_t		d2h_delete;
	h2d_ring_delete_req_t		h2d_delete;
	unsigned char			check[H2DRING_CTRL_SUB_ITEMSIZE];
} ctrl_submit_item_t;

typedef struct info_ring_submit_item {
	info_buf_post_msg_t		info_buf_post;
	unsigned char			check[H2DRING_INFO_BUFPOST_ITEMSIZE];
} info_sumbit_item_t;

/** Control Completion messages (20 bytes) */
typedef struct compl_msg_hdr {
	/** status for the completion */
	int16	status;
	/** submisison flow ring id which generated this status */
	union {
	    uint16	ring_id;
	    uint16	flow_ring_id;
	};
} compl_msg_hdr_t;

/** XOR checksum or a magic number to audit DMA done */
typedef uint32 dma_done_t;

#define MAX_CLKSRC_ID	0xF

typedef struct ts_timestamp_srcid {
	union {
		uint32	ts_low; /* time stamp low 32 bits */
		uint32	reserved; /* If timestamp not used */
	};
	union {
		uint32  ts_high; /* time stamp high 28 bits */
		union {
			uint32  ts_high_ext :28; /* time stamp high 28 bits */
			uint32  clk_id_ext :3; /* clock ID source  */
			uint32  phase :1; /* Phase bit */
			dma_done_t	marker_ext;
		};
	};
} ts_timestamp_srcid_t;

typedef ts_timestamp_srcid_t ipc_timestamp_t;

typedef struct ts_timestamp {
	uint32	low;
	uint32	high;
} ts_timestamp_t;

typedef ts_timestamp_t tick_count_64_t;
typedef ts_timestamp_t ts_timestamp_ns_64_t;
typedef ts_timestamp_t ts_correction_m_t;
typedef ts_timestamp_t ts_correction_b_t;

/* completion header status codes */
#define	BCMPCIE_SUCCESS			0
#define BCMPCIE_NOTFOUND		1
#define BCMPCIE_NOMEM			2
#define BCMPCIE_BADOPTION		3
#define BCMPCIE_RING_IN_USE		4
#define BCMPCIE_RING_ID_INVALID		5
#define BCMPCIE_PKT_FLUSH		6
#define BCMPCIE_NO_EVENT_BUF		7
#define BCMPCIE_NO_RX_BUF		8
#define BCMPCIE_NO_IOCTLRESP_BUF	9
#define BCMPCIE_MAX_IOCTLRESP_BUF	10
#define BCMPCIE_MAX_EVENT_BUF		11
#define BCMPCIE_BAD_PHASE		12
#define BCMPCIE_INVALID_CPL_RINGID	13
#define BCMPCIE_RING_TYPE_INVALID	14
#define BCMPCIE_NO_TS_EVENT_BUF		15
#define BCMPCIE_MAX_TS_EVENT_BUF	16
#define BCMPCIE_PCIE_NO_BTLOG_BUF	17
#define BCMPCIE_BT_DMA_ERR		18
#define BCMPCIE_BT_DMA_DESCR_FETCH_ERR	19

/** IOCTL completion response */
typedef struct ioctl_compl_resp_msg {
	/** common message header */
	cmn_msg_hdr_t		cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t		compl_hdr;
	/** response buffer len where a host buffer is involved */
	uint16			resp_len;
	/** transaction id to pair with a request */
	uint16			trans_id;
	/** cmd id */
	uint32			cmd;
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} ioctl_comp_resp_msg_t;

/** IOCTL request acknowledgement */
typedef struct ioctl_req_ack_msg {
	/** common message header */
	cmn_msg_hdr_t		cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t		compl_hdr;
	/** cmd id */
	uint32			cmd;
	uint32			rsvd;
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} ioctl_req_ack_msg_t;

/** WL event message: send from device to host */
typedef struct wlevent_req_msg {
	/** common message header */
	cmn_msg_hdr_t		cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t		compl_hdr;
	/** event data len valid with the event buffer */
	uint16			event_data_len;
	/** sequence number */
	uint16			seqnum;
	/** rsvd	*/
	uint32			rsvd;
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} wlevent_req_msg_t;

/** dma xfer complete message */
typedef struct pcie_dmaxfer_cmplt {
	/** common message header */
	cmn_msg_hdr_t		cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t		compl_hdr;
	uint32			rsvd[2];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} pcie_dmaxfer_cmplt_t;

/** general status message */
typedef struct pcie_gen_status {
	/** common message header */
	cmn_msg_hdr_t		cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t		compl_hdr;
	uint32			rsvd[2];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} pcie_gen_status_t;

/** ring status message */
typedef struct pcie_ring_status {
	/** common message header */
	cmn_msg_hdr_t		cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t		compl_hdr;
	/** message which firmware couldn't decode */
	uint16			write_idx;
	uint16			rsvd[3];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} pcie_ring_status_t;

typedef struct ring_create_response {
	cmn_msg_hdr_t		cmn_hdr;
	compl_msg_hdr_t		cmplt;
	uint32			rsvd[2];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} ring_create_response_t;

typedef ring_create_response_t tx_flowring_create_response_t;
typedef ring_create_response_t h2d_ring_create_response_t;
typedef ring_create_response_t d2h_ring_create_response_t;

typedef struct tx_flowring_delete_response {
	cmn_msg_hdr_t		msg;
	compl_msg_hdr_t		cmplt;
	uint16			read_idx;
	uint16			rsvd[3];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} tx_flowring_delete_response_t;

typedef tx_flowring_delete_response_t	h2d_ring_delete_response_t;
typedef tx_flowring_delete_response_t	d2h_ring_delete_response_t;

typedef struct tx_flowring_flush_response {
	cmn_msg_hdr_t		msg;
	compl_msg_hdr_t		cmplt;
	uint32			rsvd[2];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} tx_flowring_flush_response_t;

/** Common layout of all d2h control messages */
typedef struct ctrl_compl_msg {
	/** common message header */
	cmn_msg_hdr_t       cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t     compl_hdr;
	uint32          rsvd[2];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t      marker;
} ctrl_compl_msg_t;

typedef struct ring_config_resp {
	/** common message header */
	cmn_msg_hdr_t       cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t     compl_hdr;
	uint32          rsvd[2];
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t      marker;
} ring_config_resp_t;

typedef struct d2h_mailbox_data {
	cmn_msg_hdr_t		msg;
	compl_msg_hdr_t		cmplt;
	uint32			d2h_mailbox_data;
	uint32			rsvd[1];
	/* XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} d2h_mailbox_data_t;

/* dbg buf completion msg: send from device to host */
typedef struct info_buf_resp {
	/* common message header */
	cmn_msg_hdr_t		cmn_hdr;
	/* completion message header */
	compl_msg_hdr_t		compl_hdr;
	/* event data len valid with the event buffer */
	uint16			info_data_len;
	/* sequence number */
	uint16			seqnum;
	/* rsvd	*/
	uint32			rsvd;
	/* XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} info_buf_resp_t;

typedef struct info_ring_cpl_item {
	info_buf_resp_t		info_buf_post;
	unsigned char		check[D2HRING_INFO_BUFCMPLT_ITEMSIZE];
} info_cpl_item_t;

typedef struct host_timestamp_msg_cpl {
	cmn_msg_hdr_t		msg;
	compl_msg_hdr_t cmplt;
	uint16			xt_id; /* transaction ID */
	uint16			rsvd;
	uint32			rsvd1;
	/* XOR checksum or a magic number to audit DMA done */
	dma_done_t      marker;
} host_timestamp_msg_cpl_t;

typedef struct fw_timestamp_event_msg {
	cmn_msg_hdr_t		msg;
	compl_msg_hdr_t cmplt;
	/* fw captures time stamp info and passed that to host in TLVs */
	uint16			buf_len; /* length of the time stamp data copied in host buf */
	uint16			seqnum; /* number of times fw captured time stamp */
	uint32			rsvd;
	/* XOR checksum or a magic number to audit DMA done */
	dma_done_t		marker;
} fw_timestamp_event_msg_t;

typedef union ctrl_completion_item {
	ioctl_comp_resp_msg_t		ioctl_resp;
	wlevent_req_msg_t		event;
	ioctl_req_ack_msg_t		ioct_ack;
	pcie_dmaxfer_cmplt_t		pcie_xfer_cmplt;
	pcie_gen_status_t		pcie_gen_status;
	pcie_ring_status_t		pcie_ring_status;
	tx_flowring_create_response_t	txfl_create_resp;
	tx_flowring_delete_response_t	txfl_delete_resp;
	tx_flowring_flush_response_t	txfl_flush_resp;
	ctrl_compl_msg_t		ctrl_compl;
	ring_config_resp_t		ring_config_resp;
	d2h_mailbox_data_t		d2h_mailbox_data;
	info_buf_resp_t			dbg_resp;
	h2d_ring_create_response_t	h2d_ring_create_resp;
	d2h_ring_create_response_t	d2h_ring_create_resp;
	host_timestamp_msg_cpl_t	host_ts_cpl;
	fw_timestamp_event_msg_t	fw_ts_event;
	h2d_ring_delete_response_t	h2d_ring_delete_resp;
	d2h_ring_delete_response_t	d2h_ring_delete_resp;
	unsigned char			ctrl_response[D2HRING_CTRL_CMPLT_ITEMSIZE];
} ctrl_completion_item_t;

/** H2D Rxpost ring work items */
typedef struct host_rxbuf_post {
	/** common message header */
	cmn_msg_hdr_t   cmn_hdr;
	/** provided meta data buffer len */
	uint16		metadata_buf_len;
	/** provided data buffer len to receive data */
	uint16		data_buf_len;
	/** alignment to make the host buffers start on 8 byte boundary */
	uint32		rsvd;
	/** provided meta data buffer */
	haddr64_t	metadata_buf_haddr64;
	/** provided data buffer to receive data */
	haddr64_t	data_buf_haddr64;
} host_rxbuf_post_t;

typedef union rxbuf_submit_item {
	host_rxbuf_post_t	rxpost;
	unsigned char		check[H2DRING_RXPOST_ITEMSIZE];
} rxbuf_submit_item_t;

/* D2H Rxcompletion ring work items for IPC rev7 */
typedef struct host_rxbuf_cmpl {
	/** common message header */
	cmn_msg_hdr_t	cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t	compl_hdr;
	/**  filled up meta data len */
	uint16		metadata_len;
	/** filled up buffer len to receive data */
	uint16		data_len;
	/** offset in the host rx buffer where the data starts */
	uint16		data_offset;
	/** offset in the host rx buffer where the data starts */
	uint16		flags;
	/** rx status */
	uint32		rx_status_0;
	uint32		rx_status_1;
	/** XOR checksum or a magic number to audit DMA done */
	/* This is for rev6 only. For IPC rev7, this is a reserved field */
	dma_done_t	marker;
} host_rxbuf_cmpl_t;

typedef union rxbuf_complete_item {
	host_rxbuf_cmpl_t	rxcmpl;
	unsigned char		check[D2HRING_RXCMPLT_ITEMSIZE];
} rxbuf_complete_item_t;

typedef struct host_txbuf_post {
	/** common message header */
	cmn_msg_hdr_t   cmn_hdr;
	/** eth header */
	uint8		txhdr[ETHER_HDR_LEN];
	/** flags */
	uint8		flags;
	/** number of segments */
	uint8		seg_cnt;

	/** provided meta data buffer for txstatus */
	haddr64_t	metadata_buf_haddr64;
	/** provided data buffer to receive data */
	haddr64_t	data_buf_haddr64;
	/** provided meta data buffer len */
	uint16		metadata_buf_len;
	/** provided data buffer len to receive data */
	uint16		data_len;
	/** XOR checksum or a magic number to audit DMA done */
	dma_done_t	marker;
} host_txbuf_post_t;

#define BCMPCIE_PKT_FLAGS_FRAME_802_3	0x01
#define BCMPCIE_PKT_FLAGS_FRAME_802_11	0x02

#define BCMPCIE_PKT_FLAGS_FRAME_EXEMPT_MASK	0x03	/* Exempt uses 2 bits */
#define BCMPCIE_PKT_FLAGS_FRAME_EXEMPT_SHIFT	0x02	/* needs to be shifted past other bits */

#define BCMPCIE_PKT_FLAGS_PRIO_SHIFT		5
#define BCMPCIE_PKT_FLAGS_PRIO_MASK		(7 << BCMPCIE_PKT_FLAGS_PRIO_SHIFT)
#define BCMPCIE_PKT_FLAGS_MONITOR_NO_AMSDU	0x00
#define BCMPCIE_PKT_FLAGS_MONITOR_FIRST_PKT	0x01
#define BCMPCIE_PKT_FLAGS_MONITOR_INTER_PKT	0x02
#define BCMPCIE_PKT_FLAGS_MONITOR_LAST_PKT	0x03
#define BCMPCIE_PKT_FLAGS_MONITOR_SHIFT		8
#define BCMPCIE_PKT_FLAGS_MONITOR_MASK		(3 << BCMPCIE_PKT_FLAGS_MONITOR_SHIFT)

/* These are added to fix up compile issues */
#define BCMPCIE_TXPOST_FLAGS_FRAME_802_3	BCMPCIE_PKT_FLAGS_FRAME_802_3
#define BCMPCIE_TXPOST_FLAGS_FRAME_802_11	BCMPCIE_PKT_FLAGS_FRAME_802_11
#define BCMPCIE_TXPOST_FLAGS_PRIO_SHIFT		BCMPCIE_PKT_FLAGS_PRIO_SHIFT
#define BCMPCIE_TXPOST_FLAGS_PRIO_MASK		BCMPCIE_PKT_FLAGS_PRIO_MASK

/* H2D Txpost ring work items */
typedef union txbuf_submit_item {
	host_txbuf_post_t	txpost;
	unsigned char		check[H2DRING_TXPOST_ITEMSIZE];
} txbuf_submit_item_t;

/* D2H Txcompletion ring work items - extended for IOC rev7 */
typedef struct host_txbuf_cmpl {
	/** common message header */
	cmn_msg_hdr_t	cmn_hdr;
	/** completion message header */
	compl_msg_hdr_t	compl_hdr;
	union {
		struct {
			/** provided meta data len */
			uint16	metadata_len;
			/** WLAN side txstatus */
			uint16	tx_status;
		};
		/** XOR checksum or a magic number to audit DMA done */
		/* This is for rev6 only. For IPC rev7, this is not used */
		dma_done_t	marker;
	};
} host_txbuf_cmpl_t;

typedef union txbuf_complete_item {
	host_txbuf_cmpl_t	txcmpl;
	unsigned char		check[D2HRING_TXCMPLT_ITEMSIZE];
} txbuf_complete_item_t;

#define BCMPCIE_D2H_METADATA_HDRLEN	4
#define BCMPCIE_D2H_METADATA_MINLEN	(BCMPCIE_D2H_METADATA_HDRLEN + 4)

/** ret buf struct */
typedef dma64addr_t ret_buf_t;

#ifdef PCIE_API_REV1

/* ioctl specific hdr */
typedef struct ioctl_hdr {
	uint16		cmd;
	uint16		retbuf_len;
	uint32		cmd_id;
} ioctl_hdr_t;

typedef struct ioctlptr_hdr {
	uint16		cmd;
	uint16		retbuf_len;
	uint16		buflen;
	uint16		rsvd;
	uint32		cmd_id;
} ioctlptr_hdr_t;

#else /* PCIE_API_REV1 */

typedef struct ioctl_req_hdr {
	uint32		pkt_id;	/**< Packet ID */
	uint32		cmd;	/**< IOCTL ID */
	uint16		retbuf_len;
	uint16		buflen;
	uint16		xt_id;	/**< transaction ID */
	uint16		rsvd[1];
} ioctl_req_hdr_t;

#endif /* PCIE_API_REV1 */

/** Complete msgbuf hdr for ioctl from host to dongle */
typedef struct ioct_reqst_hdr {
	cmn_msg_hdr_t msg;
#ifdef PCIE_API_REV1
	ioctl_hdr_t ioct_hdr;
#else
	ioctl_req_hdr_t ioct_hdr;
#endif // endif
	ret_buf_t ret_buf;
} ioct_reqst_hdr_t;

typedef struct ioctptr_reqst_hdr {
	cmn_msg_hdr_t msg;
#ifdef PCIE_API_REV1
	ioctlptr_hdr_t ioct_hdr;
#else
	ioctl_req_hdr_t ioct_hdr;
#endif // endif
	ret_buf_t ret_buf;
	ret_buf_t ioct_buf;
} ioctptr_reqst_hdr_t;

/** ioctl response header */
typedef struct ioct_resp_hdr {
	cmn_msg_hdr_t   msg;
#ifdef PCIE_API_REV1
	uint32	cmd_id;
#else
	uint32	pkt_id;
#endif // endif
	uint32	status;
	uint32	ret_len;
	uint32  inline_data;
#ifdef PCIE_API_REV1
#else
	uint16	xt_id;	/**< transaction ID */
	uint16	rsvd[1];
#endif // endif
} ioct_resp_hdr_t;

/* ioct resp header used in dongle */
/* ret buf hdr will be stripped off inside dongle itself */
typedef struct msgbuf_ioctl_resp {
	ioct_resp_hdr_t	ioct_hdr;
	ret_buf_t	ret_buf;	/**< ret buf pointers */
} msgbuf_ioct_resp_t;

/** WL event hdr info */
typedef struct wl_event_hdr {
	cmn_msg_hdr_t   msg;
	uint16 event;
	uint8 flags;
	uint8 rsvd;
	uint16 retbuf_len;
	uint16 rsvd1;
	uint32 rxbufid;
} wl_event_hdr_t;

#define TXDESCR_FLOWID_PCIELPBK_1	0xFF
#define TXDESCR_FLOWID_PCIELPBK_2	0xFE

typedef struct txbatch_lenptr_tup {
	uint32 pktid;
	uint16 pktlen;
	uint16 rsvd;
	ret_buf_t	ret_buf;	/**< ret buf pointers */
} txbatch_lenptr_tup_t;

typedef struct txbatch_cmn_msghdr {
	cmn_msg_hdr_t   msg;
	uint8 priority;
	uint8 hdrlen;
	uint8 pktcnt;
	uint8 flowid;
	uint8 txhdr[ETHER_HDR_LEN];
	uint16 rsvd;
} txbatch_cmn_msghdr_t;

typedef struct txbatch_msghdr {
	txbatch_cmn_msghdr_t txcmn;
	txbatch_lenptr_tup_t tx_tup[0]; /**< Based on packet count */
} txbatch_msghdr_t;

/* TX desc posting header */
typedef struct tx_lenptr_tup {
	uint16 pktlen;
	uint16 rsvd;
	ret_buf_t	ret_buf;	/**< ret buf pointers */
} tx_lenptr_tup_t;

typedef struct txdescr_cmn_msghdr {
	cmn_msg_hdr_t   msg;
	uint8 priority;
	uint8 hdrlen;
	uint8 descrcnt;
	uint8 flowid;
	uint32 pktid;
} txdescr_cmn_msghdr_t;

typedef struct txdescr_msghdr {
	txdescr_cmn_msghdr_t txcmn;
	uint8 txhdr[ETHER_HDR_LEN];
	uint16 rsvd;
	tx_lenptr_tup_t tx_tup[0];	/**< Based on descriptor count */
} txdescr_msghdr_t;

/** Tx status header info */
typedef struct txstatus_hdr {
	cmn_msg_hdr_t   msg;
	uint32 pktid;
} txstatus_hdr_t;

/** RX bufid-len-ptr tuple */
typedef struct rx_lenptr_tup {
	uint32 rxbufid;
	uint16 len;
	uint16 rsvd2;
	ret_buf_t	ret_buf;	/**< ret buf pointers */
} rx_lenptr_tup_t;

/** Rx descr Post hdr info */
typedef struct rxdesc_msghdr {
	cmn_msg_hdr_t   msg;
	uint16 rsvd0;
	uint8 rsvd1;
	uint8 descnt;
	rx_lenptr_tup_t rx_tup[0];
} rxdesc_msghdr_t;

/** RX complete tuples */
typedef struct rxcmplt_tup {
	uint16 retbuf_len;
	uint16 data_offset;
	uint32 rxstatus0;
	uint32 rxstatus1;
	uint32 rxbufid;
} rxcmplt_tup_t;

/** RX complete messge hdr */
typedef struct rxcmplt_hdr {
	cmn_msg_hdr_t   msg;
	uint16 rsvd0;
	uint16 rxcmpltcnt;
	rxcmplt_tup_t rx_tup[0];
} rxcmplt_hdr_t;

typedef struct hostevent_hdr {
	cmn_msg_hdr_t   msg;
	uint32 evnt_pyld;
} hostevent_hdr_t;

typedef struct dma_xfer_params {
	uint32 src_physaddr_hi;
	uint32 src_physaddr_lo;
	uint32 dest_physaddr_hi;
	uint32 dest_physaddr_lo;
	uint32 len;
	uint32 srcdelay;
	uint32 destdelay;
} dma_xfer_params_t;

enum {
	HOST_EVENT_CONS_CMD = 1
};

/* defines for flags */
#define MSGBUF_IOC_ACTION_MASK 0x1

#define MAX_SUSPEND_REQ 15

typedef struct tx_idle_flowring_suspend_request {
	cmn_msg_hdr_t   msg;
	uint16	ring_id[MAX_SUSPEND_REQ];      /* ring Id's */
	uint16	num;	/* number of flowid's to suspend */
} tx_idle_flowring_suspend_request_t;

typedef struct tx_idle_flowring_suspend_response {
	cmn_msg_hdr_t		msg;
	compl_msg_hdr_t		cmplt;
	uint32			rsvd[2];
	dma_done_t		marker;
} tx_idle_flowring_suspend_response_t;

typedef struct tx_idle_flowring_resume_request {
	cmn_msg_hdr_t   msg;
	uint16	flow_ring_id;
	uint16	reason;
	uint32	rsvd[7];
} tx_idle_flowring_resume_request_t;

typedef struct tx_idle_flowring_resume_response {
	cmn_msg_hdr_t		msg;
	compl_msg_hdr_t		cmplt;
	uint32			rsvd[2];
	dma_done_t		marker;
} tx_idle_flowring_resume_response_t;

/* timesync related additions */

typedef struct _bcm_xtlv {
	uint16		id; /* TLV idenitifier */
	uint16		len; /* TLV length in bytes */
} _bcm_xtlv_t;

#define BCMMSGBUF_FW_CLOCK_INFO_TAG		0
#define BCMMSGBUF_HOST_CLOCK_INFO_TAG		1
#define BCMMSGBUF_HOST_CLOCK_SELECT_TAG		2
#define BCMMSGBUF_D2H_CLOCK_CORRECTION_TAG	3
#define BCMMSGBUF_HOST_TIMESTAMPING_CONFIG_TAG	4
#define BCMMSGBUF_MAX_TSYNC_TAG			5

/* Flags in fw clock info TLV */
#define CAP_DEVICE_TS		(1 << 0)
#define CAP_CORRECTED_TS	(1 << 1)
#define TS_CLK_ACTIVE		(1 << 2)

typedef struct ts_fw_clock_info {
	_bcm_xtlv_t  xtlv; /* BCMMSGBUF_FW_CLOCK_INFO_TAG */
	ts_timestamp_srcid_t  ts; /* tick count */
	uchar		clk_src[4]; /* clock source acronym ILP/AVB/TSF */
	uint32		nominal_clock_freq;
	uint32		reset_cnt;
	uint8		flags;
	uint8		rsvd[3];
} ts_fw_clock_info_t;

typedef struct ts_host_clock_info {
	_bcm_xtlv_t  xtlv; /* BCMMSGBUF_HOST_CLOCK_INFO_TAG */
	tick_count_64_t ticks; /* 64 bit host tick counter */
	ts_timestamp_ns_64_t ns; /* 64 bit host time in nano seconds */
} ts_host_clock_info_t;

typedef struct ts_host_clock_sel {
	_bcm_xtlv_t	xtlv; /* BCMMSGBUF_HOST_CLOCK_SELECT_TAG */
	uint32		seqnum; /* number of times GPIO time sync toggled */
	uint8		min_clk_idx; /* clock idenitifer configured for packet tiem stamping */
	uint8		max_clk_idx; /* clock idenitifer configured for packet tiem stamping */
	uint16		rsvd[1];
} ts_host_clock_sel_t;

typedef struct ts_d2h_clock_correction {
	_bcm_xtlv_t		xtlv; /* BCMMSGBUF_HOST_CLOCK_INFO_TAG */
	uint8			clk_id; /* clock source in the device */
	uint8			rsvd[3];
	ts_correction_m_t	m;	/* y  = 'm' x + b */
	ts_correction_b_t	b;	/* y  = 'm' x + 'c' */
} ts_d2h_clock_correction_t;

typedef struct ts_host_timestamping_config {
	_bcm_xtlv_t		xtlv; /* BCMMSGBUF_HOST_TIMESTAMPING_CONFIG_TAG */
	/* time period to capture the device time stamp and toggle WLAN_TIME_SYNC_GPIO */
	uint16			period_ms;
	uint8			flags;
	uint8			post_delay;
	uint32			reset_cnt;
} ts_host_timestamping_config_t;

/* Flags in host timestamping config TLV */
#define FLAG_HOST_RESET		(1 << 0)
#define IS_HOST_RESET(x)	((x) & FLAG_HOST_RESET)
#define CLEAR_HOST_RESET(x)	((x) & ~FLAG_HOST_RESET)

#define FLAG_CONFIG_NODROP	(1 << 1)
#define IS_CONFIG_NODROP(x)	((x) & FLAG_CONFIG_NODROP)
#define CLEAR_CONFIG_NODROP(x)	((x) & ~FLAG_CONFIG_NODROP)

/**
 *
 * Section: HWA2.0 Compatible PCIE IPC Messaging introduced in Revision 8.
 *
 * Independent of enabling the PCIE facing HWA blocks, the following Compact
 * Work items (CWI) and Aggregated Compact Work Items (ACWI), to include 64bit
 * and 32bit addresses (with fixed HI32) may be deployed.
 *
 * In the D2H direction there are no addresses exchanged, presently.
 *
 * Based on 32bit or 64bit address fields in H2D messages there are 4 formats,
 * denoted by suffix _cwi32, _cwi64, _acwi32 and _acwi64.
 *
 * As no addresses are passed in the D2H messages, there are only 2 formats,
 * denoted by suffix _cwi and _acwi.
 * There is no savings by aggregating Rx and Tx Completions, and CWI structure
 * type would have sufficed.
 *
 * In the case of H2D TxPost, while HWA2.0 is designed to parse ACWI formats,
 * aggregation requires host to collect all packets with the "SAME" ethernet
 * header upto Max ACWI aggregation factor, to post a ACWI work item.
 * This is considered experimental and will be deployed on a need basis, e.g.
 * when a low bit rate aggregator, accumulates all packets that have the same
 * 802.3 <MacDa,MacSa,EType>.
 *
 * A Max of 4 messages may be included in a ACWI. All empty slots in a ACWI will
 * be at the end of the array of messages. An empty slot in an ACWI is encoded
 * with a host_pktid value of HWA_HOST_PKTID_NULL. In an ACWI, all empty slots
 * must be at the end of aggregation.
 *
 * Use SHIFT/MASK, instead of bitfields which are listed in Little Endian format
 *
 * DHD should support both legacy and HWA compatible formats. In HWA equipped
 * devices, it is not required to have HWA enabled. Legacy (non-HWA equipped)
 * devices may also avail of HWA compatible messaging (dongle firmware changes)
 * to reduce the PCIe traffic.
 *
 */

/* XXX: Platforms with Runner offload, will ONLY support CWI32/CWI formats.
 * Bitfield listing in Little Endian form is for guiding Runner Offload port.
 * SW DHD and Dongle firmware must use SHIFT/MASK instead of bitfield access.
 */

#ifndef HWA_AGGR_MAX
#define HWA_AGGR_MAX                4 /* max aggregation factor in a ACWI */
#endif // endif

/**
 * Messaging Mode :
 * Dongle requests the messaging mode, and host select the item structure type
 * to be used within the requested mode, based on host's addressing 32bit/64bit.
 */
#define MSGBUF_WI_LEGACY        0 /* Legacy Work Items (<= Rev5 IPC) */
#define MSGBUF_WI_COMPACT       1 /* Compact Work Items (>= Rev8 IPC) */
#define MSGBUF_WI_AGGREGATE     HWA_AGGR_MAX /* Aggregation factor of max 4 */

/**
 * Messaging Work Item Structure: used in item_type
 */
#define MSGBUF_WI_WI64          0 /* Legacy Work item type using 64b haddr */
#define MSGBUF_WI_CWI32         1 /* _cwi32_t,  _cwi_t  */
#define MSGBUF_WI_CWI64         2 /* _cwi64_t           */
#define MSGBUF_WI_ACWI32        3 /* _acwi32_t          */
#define MSGBUF_WI_ACWI64        4 /* _acwi64_t, _acwi_t */

/* Message structures that do not have address fields */
#define MSGBUF_WI_CWI           MSGBUF_WI_CWI32  /* _cwi_t */
#define MSGBUF_WI_ACWI          MSGBUF_WI_ACWI64 /* _acwi_t */

/**
 * Null host_pktid is used to detect holes in an ACWI.
 * When host maps a packet pointer to a unique 32bit pktid, a NULL pktid will
 * never be used. When host uses a packet pointer (low 32bits), then also a
 * NULL pktid will never be used.
 */
#define HWA_HOST_PKTID_NULL         0x00000000
#define HWA_HOST_PKTID_ISVALID(host_pktid) \
	((host_pktid) != HWA_HOST_PKTID_NULL)

#define HWA_IFID_INVALID            0xff /* unspecified interface id */

#ifndef HWA_ETHER_ADDR_LEN
#define HWA_ETHER_ADDR_LEN          6 /* Length of a 802.3 Ethernet Address */

typedef union hwa_txpost_eth_sada
{
	uint8   u8[HWA_ETHER_ADDR_LEN * 2];
	uint32 u32[(HWA_ETHER_ADDR_LEN * 2) / NBU32];
} hwa_txpost_eth_sada_t;
#endif /* ! HWA_ETHER_ADDR_LEN */

/** SubSection: HWA 2.0 compatible RxPost supported by HWA1a block */

/** HWA-1.0 legacy PCIE IP RxPost work item host_rxbuf_post_t */
#define HWA_RXPOST_WI_WORDS         8
#define HWA_RXPOST_WI_BYTES         (HWA_RXPOST_WI_WORDS * NBU32)

/** HWA-2.0 compatible RxPost Compact 32bit host addressing format */
#define HWA_RXPOST_CWI32_WORDS      2
#define HWA_RXPOST_CWI32_BYTES      (HWA_RXPOST_CWI32_WORDS * NBU32)

typedef union hwa_rxpost_cwi32
{
	uint8   u8[HWA_RXPOST_CWI32_BYTES];       //  8 Bytes
	uint32 u32[HWA_RXPOST_CWI32_WORDS];       //  2 words

	struct {                                  //
		uint32 host_pktid;                    // +----- word#0
		uint32 data_buf_haddr32;              // +----- word#1
	};
} hwa_rxpost_cwi32_t;

/** HWA-2.0 compatible RxPost Compact 64bit host addressing format */
#define HWA_RXPOST_CWI64_WORDS      4
#define HWA_RXPOST_CWI64_BYTES      (HWA_RXPOST_CWI64_WORDS * NBU32)

typedef union hwa_rxpost_cwi64
{
	uint8   u8[HWA_RXPOST_CWI64_BYTES];       // 16 Bytes
	uint32 u32[HWA_RXPOST_CWI64_WORDS];       //  4 words

	struct {                                  //
		uint32 host_pktid;                    // +----- word#0
		uint32 PAD;                           // +----- word#1
		haddr64_t data_buf_haddr64;           // +----- word#2 #3
	};
} hwa_rxpost_cwi64_t;

/** HWA-2.0 compatible RxPost Aggregate Compact 32bit host addressing format */
#define HWA_RXPOST_ACWI32_WORDS     8
#define HWA_RXPOST_ACWI32_BYTES     (HWA_RXPOST_ACWI32_WORDS * NBU32)

typedef union hwa_rxpost_acwi32
{
	uint8   u8[HWA_RXPOST_ACWI32_BYTES];      // 32 Bytes
	uint32 u32[HWA_RXPOST_ACWI32_WORDS];      //  8 Words

	struct {
		uint32 host_pktid[HWA_AGGR_MAX];      // word#0 #1 #2 #3
		uint32 data_buf_haddr32[HWA_AGGR_MAX]; // word#4 #5 #6 #7
	};
} hwa_rxpost_acwi32_t;

/** HWA-2.0 compatible RxPost Aggregate Compact 64bit host addressing format */
#define HWA_RXPOST_ACWI64_WORDS     12
#define HWA_RXPOST_ACWI64_BYTES     (HWA_RXPOST_ACWI64_WORDS * NBU32)

typedef union hwa_rxpost_acwi64
{
	uint8   u8[HWA_RXPOST_ACWI64_BYTES];      // 48 Bytes
	uint32 u32[HWA_RXPOST_ACWI64_WORDS];      // 12 Words

	struct {
		uint32 host_pktid[HWA_AGGR_MAX];      // word#0 #1 #2 #3
		haddr64_t data_buf_haddr64[HWA_AGGR_MAX]; // word#4 .. 11
	};
} hwa_rxpost_acwi64_t;

/** SubSection: HWA 2.0 compatible TxPost supported by HWA3a block */

/* NOTE: TxPost ACWI32 and ACWI64 formats are experimental (not listed) */

/** HWA-1.0 legacy PCIE IP TxPost work item host_txbuf_post_t */
#define HWA_TXPOST_WI_WORDS         12
#define HWA_TXPOST_WI_BYTES         (HWA_TXPOST_WI_WORDS * NBU32)

/** HWA-2.0 compatible TxPost Compact 32bit host addressing format */
#define HWA_TXPOST_CWI32_WORDS      7
#define HWA_TXPOST_CWI32_BYTES      (HWA_TXPOST_CWI32_WORDS * NBU32)

typedef union hwa_txpost_cwi32
{
	uint8   u8[HWA_TXPOST_CWI32_BYTES];       // 28 Bytes
	uint32 u32[HWA_TXPOST_CWI32_WORDS];       //  7 Words

	struct {                                  //
		uint32 host_pktid;                    // +----- word#0
		union {                               // +----- word#1
			struct {                          // BF Access
				uint32 ifid             :  5; // interface id
				uint32 prio             :  3; // packet priority
				uint32 copy             :  1; // COPY as-is bit
				uint32 flags            :  7; // flags
				uint32 RSVD_FIELD       : 16; // ref: CD[data_buf_hlen]
			};                                //
			struct {                          // CD Access
				uint16 RSVD_FIELD;            // ref: BF[ifid..flags]
				uint16 data_buf_hlen;         // data buffer length
			};                                //
		}; // u32[1]
		uint32 data_buf_haddr32;              // +----- word#2
		hwa_txpost_eth_sada_t eth_sada;       // +----- word#3 #4 #5
		union {                               // +----- word#6
			struct {                          // BF Access
				uint32 RSVD_FIELD       : 16; // ref: CD[eth_type]
				uint32 info             :  4; // Host field for future use
				uint32 flowid_override  : 12; // Host flowid override
			};                                //
			struct {                          // CD Access
				uint16 eth_type;              // packet Ether-Type
				uint16 RSVD_FIELD;            // ref: BF[info..flowid_override]
			};                                //
		}; // u32[6]
	};
} hwa_txpost_cwi32_t;

/** HWA-2.0 compatible TxPost Compact 64bit host addressing format */
#define HWA_TXPOST_CWI64_WORDS      8
#define HWA_TXPOST_CWI64_BYTES      (HWA_TXPOST_CWI64_WORDS * NBU32)

typedef union hwa_txpost_cwi64
{
	uint8   u8[HWA_TXPOST_CWI64_BYTES];       // 32 Bytes
	uint32 u32[HWA_TXPOST_CWI64_WORDS];       //  8 Words

	struct {                                  //
		uint32 host_pktid;                    // +----- word#0
		union {                               // +----- word#1
			struct {                          // BF Access
				uint32 ifid             :  5; // interface id
				uint32 prio             :  3; // packet priority
				uint32 copy             :  1; // COPY as-is bit
				uint32 flags            :  7; // flags
				uint32 RSVD_FIELD       : 16; // ref: CD[data_buf_hlen]
			};                                //
			struct {                          // CD Access
				uint16 RSVD_FIELD;            // ref: BF[ifid..flags]
				uint16 data_buf_hlen;         // data buffer length
			};                                //
		}; // u32[1]
		haddr64_t data_buf_haddr64;           // +----- word#2 #3
		hwa_txpost_eth_sada_t eth_sada;       // +----- word#4 #5 #6
		union {                               // +----- word#7
			struct {                          // BF Access
				uint32 RSVD_FIELD       : 16; // ref: CD[eth_type]
				uint32 info             :  4; // Host field - future use
				uint32 flowid_override  : 12; // Host flowid override
			};                                //
			struct {                          // CD Access
				uint16 eth_type;              // packet Ether-Type
				uint16 RSVD_FIELD;            // ref: BF[info..flowid_override]
			};                                //
		}; // u32[7]
	};
} hwa_txpost_cwi64_t;

#define HWA_TXPOST_IFID_SHIFT       0 /* word1 bits 0..4 : ifid */
#define HWA_TXPOST_IFID_MASK        (0x1F << HWA_TXPOST_IFID_SHIFT)
#define HWA_TXPOST_PRIO_SHIFT       5 /* word1 bits 5..7 : prio */
#define HWA_TXPOST_PRIO_MASK        (0x7 << HWA_TXPOST_PRIO_SHIFT)
#define HWA_TXPOST_COPY_SHIFT       8 /* word1 bit 8 : copy */
#define HWA_TXPOST_COPY_MASK        (0x1 << HWA_TXPOST_COPY_SHIFT)
#define HWA_TXPOST_FLAGS_SHIFT      9 /* word1 bit 9..15 : flags */
#define HWA_TXPOST_FLAGS_MASK       (0x7F << HWA_TXPOST_FLAGS_SHIFT)
#define HWA_TXPOST_DATA_BUF_HLEN_SHIFT (16)
#define HWA_TXPOST_DATA_BUF_HLEN_MASK (0xFFFF << HWA_TXPOST_DATA_BUF_HLEN_SHIFT)

#define HWA_TXPOST_INFO_SHIFT       16 /* word7 bits 16..19 : info */
#define HWA_TXPOST_INFO_MASK        (0xF << HWA_TXPOST_INFO_SHIFT)
#define HWA_TXPOST_FLOWOVRD_SHIFT   20 /* word7 bits 20..31 : flowid_override */
#define HWA_TXPOST_FLOWOVRD_MASK    (0xFFF << HWA_TXPOST_FLOWOVRD_SHIFT)

/** SubSection: HWA 2.0 compatible RxCple supported by HWA2b block */

/** HWA-1.0 legacy PCIE IP RxCple work item host_rxbuf_cmpl_t */
#define HWA_RXCPLE_WI_WORDS         8
#define HWA_RXCPLE_WI_BYTES         (HWA_RXCPLE_WI_WORDS * NBU32)

/** HWA-2.0 compatible RxCple Compact format */
#define HWA_RXCPLE_CWI_WORDS        2
#define HWA_RXCPLE_CWI_BYTES        (HWA_RXCPLE_CWI_WORDS * NBU32)

typedef union hwa_rxcple_cwi
{
	uint8   u8[HWA_RXCPLE_CWI_BYTES];         //  8 Bytes
	uint32 u32[HWA_RXCPLE_CWI_WORDS];         //  2 Words

	struct {
		uint8 ifid                  :  5;     // rx packet interface id
		uint8 flags                 :  3;     // packet flags
		uint8 data_offset;                    // start of packet data
		uint16 data_len;                      // length of packet data
		uint32 host_pktid;                    // host pktid
	};

} hwa_rxcple_cwi_t;

/** HWA-2.0 compatible RxCple Aggregated Compact format */
/* NOTE: hwa_rxcple_acwi_t is simply an array[4] of hwa_rxcple_cwi_t */

#define HWA_RXCPLE_ACWI_WORDS       8
#define HWA_RXCPLE_ACWI_BYTES       (HWA_RXCPLE_ACWI_WORDS * NBU32)

typedef union hwa_rxcple_acwi
{
	uint8   u8[HWA_RXCPLE_ACWI_BYTES];       // 32 Bytes
	uint32 u32[HWA_RXCPLE_ACWI_WORDS];       //  8 Words

	struct {
		uint8 ifid                  :  5;     // rx packet interface id
		uint8 flags                 :  3;     // packet flags or prio
		uint8 data_offset;                    // start of packet data
		uint16 data_len;                      // length of packet data
		uint32 host_pktid;                    // host pktid
	} aggr[HWA_AGGR_MAX];

} hwa_rxcple_acwi_t;

#define HWA_RXCPLE_IFID_SHIFT       0
#define HWA_RXCPLE_IFID_MASK        (0x1F << HWA_RXCPLE_IFID_SHIFT)
#define HWA_RXCPLE_FLAGS_SHIFT      5
#define HWA_RXCPLE_FLAGS_MASK       (0x7 << HWA_RXCPLE_FLAGS_SHIFT)
#define HWA_RXCPLE_DATA_OFFSET_SHIFT 8
#define HWA_RXCPLE_DATA_OFFSET_MASK  (0xFF << HWA_RXCPLE_DATA_OFFSET_SHIFT)
#define HWA_RXCPLE_DATA_LEN_SHIFT   16
#define HWA_RXCPLE_DATA_LEN_MASK    (0xFFFF << HWA_RXCPLE_DATA_LEN_SHIFT)

/** SubSection: HWA 2.0 compatible TxCple supported by HWA4b block */

/** HWA-1.0 legacy PCIE IP TxCple work item host_txbuf_cmpl_t */
#define HWA_TXCPLE_WI_WORDS         4
#define HWA_TXCPLE_WI_BYTES         (HWA_TXCPLE_WI_WORDS * NBU32)

/** HWA-2.0 compatible TxCple Compact format */
#define HWA_TXCPLE_CWI_WORDS        2
#define HWA_TXCPLE_CWI_BYTES        (HWA_TXCPLE_CWI_WORDS * NBU32)

typedef union hwa_txcple_cwi
{
	uint8   u8[HWA_TXCPLE_CWI_BYTES];         //  8 Bytes
	uint32 u32[HWA_TXCPLE_CWI_WORDS];         //  2 Words

	struct {
		uint32 host_pktid;                    // host pktid
		union {                                   // +----- word#1
			uint32 word1;
			struct {                              // word#1
				uint16 flow_ring_id;
				uint8 if_id;
				uint8 reserved1;
			};
		}; // u32[1]
	};

} hwa_txcple_cwi_t;

/** HWA-2.0 compatible TxCple Compact format (4Byte) */
#define HWA_TXCPLE_CWI_4B_T
#define HWA_TXCPLE_CWI_1_WORD         1
#define HWA_TXCPLE_CWI_4_BYTES        (HWA_TXCPLE_CWI_1_WORD * NBU32)
typedef union hwa_txcple_cwi_4b
{
	uint8   u8[HWA_TXCPLE_CWI_4_BYTES];         //  4 Bytes
	uint32 u32[HWA_TXCPLE_CWI_1_WORD];          //  1 Word

	struct {
		uint32 host_pktid;                    // host pktid
	};

} hwa_txcple_cwi_4b_t;

/** HWA-2.0 compatible TxCple Aggregated Compact format */
/* NOTE: hwa_txcple_acwi_t is simply an array[4] of hwa_txcple_cwi_t */

#define HWA_TXCPLE_ACWI_WORDS       8
#define HWA_TXCPLE_ACWI_BYTES       (HWA_TXCPLE_ACWI_WORDS * NBU32)

typedef union hwa_txcple_acwi
{
	uint8   u8[HWA_TXCPLE_ACWI_BYTES];           // 32 Bytes
	uint32 u32[HWA_TXCPLE_ACWI_WORDS];           //  8 Words

	struct {
		uint32 host_pktid;                   // host pktid
		union {                                  // +----- word#1
			uint32 word1;
			struct {                             // word#1
				uint16 flow_ring_id;
				uint8 if_id;
				uint8 reserved1;
			};
		}; // u32[1]
	} aggr[HWA_AGGR_MAX];

} hwa_txcple_acwi_t;

#endif /* _bcmmsgbuf_h_ */
