/*
 * +--------------------------------------------------------------------------+
 * bcm_pcap.h
 *
 * External Interface for Packet Capture module, shared
 * between dongle firmware (producer) and host driver (consumer).
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 *
 * +--------------------------------------------------------------------------+
 */
#ifndef __bcm_pcap_h__
#define __bcm_pcap_h__
#include <bcmwifi_rspec.h>
#include <802.11.h>

/* test pulling interval */
#define PCAPXF_POLL_10MSEC_TICKS	(1)	/* 1 * 10 millisec = 10 millisec */
#define PCAPXF_POLL_160MSEC_TICKS	(0x10)	/* 16 * 10 millisec = 160 millisec */

/** BCM_PCAP Ring Memory */
#define PCAPXF_RING_ITEM_SIZE		(512)	/* Maximum size of element */
#define PCAPXF_RING_ITEMS_MAX		(512)
#define PCAPXF_RING_MEM_BYTES	\
	(PCAPXF_RING_ITEM_SIZE * PCAPXF_RING_ITEMS_MAX)

#define PCAPXF_ALIGNMENT		(64)
#define __pcapxf_aligned		__attribute__ ((aligned (PCAPXF_ALIGNMENT)))

/** Circular Ring write and read index */
struct pcapxf_ring_idx {
	uint32  u32;
} __pcapxf_aligned;
typedef struct pcapxf_ring_idx pcapxf_ring_idx_t;

/** Each PCAP structure posted from Dongle into the circular ring */
struct pcapxf_ring_elem {
	uint8   data[PCAPXF_RING_ITEM_SIZE];	/* Actual data may be smaller */
} __pcapxf_aligned;
typedef struct pcapxf_ring_elem pcapxf_ring_elem_t;

/** PCAP header context */
struct pcapxf_header {
	uint32			elem_size;	/* size of pcap_ring_elem */
	uint32			table_daddr32;	/* ring base of pcap_elem_t */
	pcapxf_ring_idx_t	write_idx;
	pcapxf_ring_idx_t	read_idx;
} __pcapxf_aligned;
typedef struct pcapxf_header pcapxf_header_t;

/** PCAPXF PCIE IPC shared structure in Host Memory Extension (HME) */
typedef struct pcapxf_ipc_hme {
	pcapxf_header_t		header;
	pcapxf_ring_elem_t	table[PCAPXF_RING_ITEMS_MAX];
} pcapxf_ipc_hme_t;

#define PCAP_HDR_DATA_SZ	(PCAPXF_RING_ITEM_SIZE - 4)
/* pcap header msg types */
typedef enum {
	PCAP_MSGTYPE_INVALID = 0,
	PCAP_MSGTYPE_MSDU,
	PCAP_MSGTYPE_MPDU,
	PCAP_MSGTYPE_PHYSTS,
	PCAP_MSGTYPE_TOTAL
} pcap_msgtype_t;

/* pcap rx header packet element shared by dhd and dongle */
typedef struct pcap_hdr {
	union {
		uint32	frame_hdr;
		struct {
			uint16	hdr_msgtype;
			uint16	hdr_len;
		};
	};
	uint32		pkt_id;			/* dhd host buffer id */
	uint32		physeq;			/* PhySts seq */
	uint32		flags;			/* flags */
} pcap_hdr_t;

typedef struct pcap_hdrpkt {
	bool			used;		/* available or used */
	uint8			num_frames;
	uint16			frame_len;	/* length to sent */
	uint8			pcap_data[PCAP_HDR_DATA_SZ];
} pcap_hdrpkt_t;

/* packet header and metadata */
#define MAX_MACHDR_LEN		58
#define D11RXHDR_LEN		40
#define PHYPKTHDR_LEN		20
#define PHYRXSTS_LEN		152
#define PCAPF_MSDU_FIRST	0x1
#define PCAPF_MSDU_LAST		0x2
#define PCAP_PKT_BUFSZ		2048
#define PCAP_POISON_VALUE	0xa9
/* header packet element */
typedef struct pcap_msdu {
	pcap_hdr_t	hdr;
} pcap_msdu_t;

typedef pcap_msdu_t pcap_comm_t;
typedef struct pcap_mpdu {
	pcap_comm_t	comm;
	uint16		pad_len;
	uint16		d11hdr_len;
	uint8		d11hdr[D11RXHDR_LEN];		/* ucode status */
	uint8		mac_hdr[MAX_MACHDR_LEN];	/* Mac header */
} pcap_mpdu_t;

typedef struct pcap_physts {
	pcap_comm_t	comm;
	uint16		physts_len;			/* phyrx length */
	uint8		phyrxsts[PHYRXSTS_LEN];
} pcap_physts_t;

#define PCAP_ANT_MAX	4	/* Max 4x4 WLAN interface */
#define PCAP_MAX_D11_HDR_LEN	(DOT11_A4_HDR_LEN + DOT11_QOS_LEN + \
					DOT11_HTC_LEN + DOT11_IV_MAX_LEN)

/* PCAP TX header to be stitched before the local TX packets */
typedef struct pcap_tx_hdr {
	chanspec_t	chanspec;
	uint16		datarate;		/* 500 kbps units */
	ratespec_t	rspec;			/* detailed rate info */
	uint32		tsf_hi;			/* MAC timestamp (hi 32bits) */
	uint32		tsf_lo;			/* MAC timestamp (lo 32 bits) */
	int8		txpwr[PCAP_ANT_MAX];	/* power in dBm */
	uint8		txcore_mask;		/* bit map of TX cores used */
	uint8		retries;		/* Number of retries */
	uint8		rts_cnt;		/* Number of RTS transmitted */
	uint8		acked;			/* Ack received */
	uint32		pkt_size;
} pcap_tx_hdr_t;

/* Extended PCAP TX header to be used for non-local TX packets */
typedef struct pcap_tx_hdr_ext {
	pcap_tx_hdr_t	txhdr;
	uint8		d11hdr_len;
	uint8		d11hdr[PCAP_MAX_D11_HDR_LEN];
} pcap_tx_hdr_ext_t;

/* packet capture macros */
#define	WLC_PCAP_PROMISC_EN	0x1
#define	WLC_PCAP_RXCOPY_EN	0x2
#define	WLC_PCAP_TXCOPY_EN	0x4

/* rx config */
#define	WLC_PCAP_RX_FIRST_MSDU	0x100
#define	WLC_PCAP_RX_FIRST_MPDU	0x200
#define	WLC_PCAP_RX_LAST_MPDU	0x400
#define	WLC_PCAP_RX_KEEP_BADFCS	0x800
#define	WLC_PCAP_RX_KEEP_BYTES	0x1000
/* tx config */
#define	WLC_PCAP_TX_FIRST_MSDU	0x10000
#define	WLC_PCAP_TX_FIRST_MPDU	0x20000
#define	WLC_PCAP_TX_LAST_MPDU	0x40000
#define	WLC_PCAP_TX_KEEP_BYTES	0x80000

/* Type and sub type frames to capture */
#define	PCAP_CAPT_MGMT_FRAMES	0x000000000000FFFF
#define	PCAP_CAPT_CTL_FRAMES	0x00000000FFFF0000
#define	PCAP_CAPT_DATA_FRAMES	0x0000FFFF00000000
#define	PCAP_CAPT_MISC_FRAMES	0xFFFF000000000000
#define PCAP_CAPT_ALL_FRAMES	(PCAP_CAPT_MGMT_FRAMES | PCAP_CAPT_CTL_FRAMES \
				| PCAP_CAPT_DATA_FRAMES | PCAP_CAPT_MISC_FRAMES)

/** Section: BCM_PCAP HME Sizing */
#define PCAPXF_IPC_HME_BYTES	(sizeof(pcapxf_ipc_hme_t))

/** Section: BCM_PCAP Helper Macros */
/** Compute an element index, given a table base and an element pointer */
#define PCAPXF_TABLE_ELEM2IDX(table_base, elem) \
	((int)(((struct pcapxf_ring_elem *)(elem)) \
		- ((struct pcapxf_ring_elem *)(table_base))))

/** Compute an element pointer, given a table base and the element's index */
#define PCAPXF_TABLE_IDX2ELEM(table_base, idx) \
	(((struct pcapxf_ring_elem *)(table_base)) + (idx))

#endif /* __bcm_pcap_h__ */
