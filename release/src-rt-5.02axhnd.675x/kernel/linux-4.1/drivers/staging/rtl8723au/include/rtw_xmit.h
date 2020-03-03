/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 ******************************************************************************/
#ifndef _RTW_XMIT_H_
#define _RTW_XMIT_H_

#include <osdep_service.h>
#include <drv_types.h>

#define MAX_XMITBUF_SZ		2048
#define NR_XMITBUFF		4

#define XMITBUF_ALIGN_SZ	512

/*  xmit extension buff defination */
#define MAX_XMIT_EXTBUF_SZ	1536
#define NR_XMIT_EXTBUFF		32

#define MAX_NUMBLKS		1

#define XMIT_VO_QUEUE		0
#define XMIT_VI_QUEUE		1
#define XMIT_BE_QUEUE		2
#define XMIT_BK_QUEUE		3

#define VO_QUEUE_INX		0
#define VI_QUEUE_INX		1
#define BE_QUEUE_INX		2
#define BK_QUEUE_INX		3
#define BCN_QUEUE_INX		4
#define MGT_QUEUE_INX		5
#define HIGH_QUEUE_INX		6
#define TXCMD_QUEUE_INX		7

#define HW_QUEUE_ENTRY		8

#define WEP_IV(pattrib_iv, dot11txpn, keyidx)				\
do {									\
	pattrib_iv[0] = dot11txpn._byte_.TSC0;				\
	pattrib_iv[1] = dot11txpn._byte_.TSC1;				\
	pattrib_iv[2] = dot11txpn._byte_.TSC2;				\
	pattrib_iv[3] = ((keyidx & 0x3) << 6);				\
	dot11txpn.val = (dot11txpn.val == 0xffffff) ? 0 :		\
			 (dot11txpn.val+1);				\
} while (0)

#define TKIP_IV(pattrib_iv, dot11txpn, keyidx)				\
do {									\
	pattrib_iv[0] = dot11txpn._byte_.TSC1;				\
	pattrib_iv[1] = (dot11txpn._byte_.TSC1 | 0x20) & 0x7f;		\
	pattrib_iv[2] = dot11txpn._byte_.TSC0;				\
	pattrib_iv[3] = BIT(5) | ((keyidx & 0x3)<<6);			\
	pattrib_iv[4] = dot11txpn._byte_.TSC2;				\
	pattrib_iv[5] = dot11txpn._byte_.TSC3;				\
	pattrib_iv[6] = dot11txpn._byte_.TSC4;				\
	pattrib_iv[7] = dot11txpn._byte_.TSC5;				\
	dot11txpn.val = dot11txpn.val == 0xffffffffffffULL ? 0 :	\
					 (dot11txpn.val+1);		\
} while (0)

#define AES_IV(pattrib_iv, dot11txpn, keyidx)\
do {									\
	pattrib_iv[0] = dot11txpn._byte_.TSC0;				\
	pattrib_iv[1] = dot11txpn._byte_.TSC1;				\
	pattrib_iv[2] = 0;						\
	pattrib_iv[3] = BIT(5) | ((keyidx & 0x3) << 6);			\
	pattrib_iv[4] = dot11txpn._byte_.TSC2;				\
	pattrib_iv[5] = dot11txpn._byte_.TSC3;				\
	pattrib_iv[6] = dot11txpn._byte_.TSC4;				\
	pattrib_iv[7] = dot11txpn._byte_.TSC5;				\
	dot11txpn.val = dot11txpn.val == 0xffffffffffffULL ? 0 :	\
					 (dot11txpn.val+1);		\
} while (0)

#define HWXMIT_ENTRY	4

#define TXDESC_SIZE 32

#define PACKET_OFFSET_SZ 8
#define TXDESC_OFFSET (TXDESC_SIZE + PACKET_OFFSET_SZ)

struct tx_desc {
	/* DWORD 0 */
	__le32 txdw0;
	__le32 txdw1;
	__le32 txdw2;
	__le32 txdw3;
	__le32 txdw4;
	__le32 txdw5;
	__le32 txdw6;
	__le32 txdw7;
};

union txdesc {
	struct tx_desc txdesc;
	unsigned int value[TXDESC_SIZE>>2];
};

struct	hw_xmit	{
	struct rtw_queue *sta_queue;
	int	accnt;
};

/* reduce size */
struct pkt_attrib {
	u16	type;
	u8	bswenc;
	u8	dhcp_pkt;
	u16	ether_type;
	u16	seqnum;
	u16	pkt_hdrlen;	/* the original 802.3 pkt header len */
	u16	hdrlen;		/* the WLAN Header Len */
	u32	pktlen;		/* the original 802.3 pkt raw_data len */
	u32	last_txcmdsz;
	u32	encrypt;	/* when 0 indicate no encrypt. */
	u8	nr_frags;
	u8	iv_len;
	u8	icv_len;
	u8	iv[18];
	u8	icv[16];
	u8	priority;
	u8	ack_policy;
	u8	mac_id;
	u8	vcs_mode;	/* virtual carrier sense method */
	u8	dst[ETH_ALEN];
	u8	src[ETH_ALEN];
	u8	ta[ETH_ALEN];
	u8	ra[ETH_ALEN];
	u8	key_idx;
	u8	qos_en;
	u8	ht_en;
	u8	raid;/* rate adpative id */
	u8	bwmode;
	u8	ch_offset;/* PRIME_CHNL_OFFSET */
	u8	sgi;/* short GI */
	u8	ampdu_en;/* tx ampdu enable */
	u8	mdata;/* more data bit */
	u8	pctrl;/* per packet txdesc control enable */
	u8	triggered;/* for ap mode handling Power Saving sta */
	u8	qsel;
	u8	eosp;
	u8	rate;
	u8	retry_ctrl;
	struct sta_info *psta;
};

#define WLANHDR_OFFSET		64

#define NULL_FRAMETAG		0x0
#define DATA_FRAMETAG		0x01
#define L2_FRAMETAG		0x02
#define MGNT_FRAMETAG		0x03
#define AMSDU_FRAMETAG		0x04

#define EII_FRAMETAG		0x05
#define IEEE8023_FRAMETAG 	0x06

#define MP_FRAMETAG		0x07

#define TXAGG_FRAMETAG		0x08

struct  submit_ctx {
	u32 timeout_ms; /* <0: not synchronous, 0: wait forever,
			 * >0: up to ms waiting
			 */
	int status; /* status for operation */
	struct completion done;
};

enum {
	RTW_SCTX_SUBMITTED = -1,
	RTW_SCTX_DONE_SUCCESS = 0,
	RTW_SCTX_DONE_UNKNOWN,
	RTW_SCTX_DONE_TIMEOUT,
	RTW_SCTX_DONE_BUF_ALLOC,
	RTW_SCTX_DONE_BUF_FREE,
	RTW_SCTX_DONE_WRITE_PORT_ERR,
	RTW_SCTX_DONE_TX_DESC_NA,
	RTW_SCTX_DONE_TX_DENY,
	RTW_SCTX_DONE_CCX_PKT_FAIL,
	RTW_SCTX_DONE_DRV_STOP,
	RTW_SCTX_DONE_DEV_REMOVE,
};

void rtw_sctx_init23a(struct submit_ctx *sctx, int timeout_ms);
int rtw_sctx_wait23a(struct submit_ctx *sctx);
void rtw23a_sctx_done_err(struct submit_ctx **sctx, int status);

struct xmit_buf {
	struct list_head list, list2;
	struct rtw_adapter *padapter;

	u8 *pallocated_buf;
	u8 *pbuf;
	void *priv_data;

	u16 ext_tag; /*  0: Normal xmitbuf, 1: extension xmitbuf. */
	u16 flags;
	u32 alloc_sz;
	u32  len;
	struct submit_ctx *sctx;
	u32	ff_hwaddr;
	struct urb *pxmit_urb[8];
	u8 bpending[8];
	int last[8];
#if defined(DBG_XMIT_BUF) || defined(DBG_XMIT_BUF_EXT)
	u8 no;
#endif
};

struct xmit_frame {
	struct list_head	list;
	struct pkt_attrib attrib;
	struct sk_buff *pkt;
	int	frame_tag;
	struct rtw_adapter *padapter;
	u8	*buf_addr;
	struct xmit_buf *pxmitbuf;

	s8	pkt_offset;

	u8 ack_report;

	u8 ext_tag; /* 0:data, 1:mgmt */
};

struct tx_servq {
	struct list_head	tx_pending;
	struct rtw_queue	sta_pending;
	int qcnt;
};

struct sta_xmit_priv {
	spinlock_t	lock;
	int	option;
	int	apsd_setting;	/* When bit mask is on, the associated edca
				 * queue supports APSD.
				 */
	struct tx_servq	be_q;			/* priority == 0,3 */
	struct tx_servq	bk_q;			/* priority == 1,2 */
	struct tx_servq	vi_q;			/* priority == 4,5 */
	struct tx_servq	vo_q;			/* priority == 6,7 */
	struct list_head legacy_dz;
	struct list_head apsd;
	u16 txseq_tid[16];
};

struct	hw_txqueue {
	volatile int head;
	volatile int tail;
	volatile int free_sz;	/* in units of 64 bytes */
	volatile int free_cmdsz;
	volatile int txsz[8];
	uint	ff_hwaddr;
	uint	cmd_hwaddr;
	int	ac_tag;
};

struct agg_pkt_info {
	u16 offset;
	u16 pkt_len;
};

struct	xmit_priv {
	spinlock_t	lock;

	struct semaphore	xmit_sema;
	struct semaphore	terminate_xmitthread_sema;

	struct rtw_queue	be_pending;
	struct rtw_queue	bk_pending;
	struct rtw_queue	vi_pending;
	struct rtw_queue	vo_pending;
	struct rtw_queue	bm_pending;

	int free_xmitframe_cnt;
	struct rtw_queue	free_xmit_queue;

	int free_xframe_ext_cnt;
	struct rtw_queue free_xframe_ext_queue;

	uint	frag_len;

	struct rtw_adapter	*adapter;

	u64	tx_bytes;
	u64	tx_pkts;
	u64	tx_drop;
	u64	last_tx_bytes;
	u64	last_tx_pkts;

	struct hw_xmit *hwxmits;
	u8	hwxmit_entry;
	u8	vcs;
	u8	nqos_ssn;

	u8	wmm_para_seq[4];/* sequence for wmm ac parameter strength from
				 * large to small. it's value is 0->vo, 1->vi,
				 * 2->be, 3->bk.
				 */

	struct semaphore	tx_retevt;/* all tx return event; */

	struct tasklet_struct xmit_tasklet;

	struct rtw_queue free_xmitbuf_queue;
	struct list_head xmitbuf_list;		/* track buffers for cleanup */
	struct rtw_queue pending_xmitbuf_queue;
	uint free_xmitbuf_cnt;

	struct rtw_queue free_xmit_extbuf_queue;
	struct list_head xmitextbuf_list;	/* track buffers for cleanup */
	uint free_xmit_extbuf_cnt;

	int	ack_tx;
	struct mutex ack_tx_mutex;
	struct submit_ctx ack_tx_ops;
	spinlock_t lock_sctx;
};

struct xmit_buf *rtw_alloc_xmitbuf23a_ext(struct xmit_priv *pxmitpriv);
s32 rtw_free_xmitbuf_ext23a(struct xmit_priv *pxmitpriv,
			 struct xmit_buf *pxmitbuf);

struct xmit_buf *rtw_alloc_xmitbuf23a(struct xmit_priv *pxmitpriv);
s32 rtw_free_xmitbuf23a(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);

void rtw_count_tx_stats23a(struct rtw_adapter *padapter,
			struct xmit_frame *pxmitframe, int sz);
void rtw_update_protection23a(struct rtw_adapter *padapter, u8 *ie, uint ie_len);
struct xmit_frame *rtw_alloc_xmitframe23a_ext(struct xmit_priv *pxmitpriv);
struct xmit_frame *rtw_alloc_xmitframe23a_once(struct xmit_priv *pxmitpriv);
s32 rtw_free_xmitframe23a(struct xmit_priv *pxmitpriv,
		       struct xmit_frame *pxmitframe);
void rtw_free_xmitframe_queue23a(struct xmit_priv *pxmitpriv, struct rtw_queue *pframequeue);
struct tx_servq *rtw_get_sta_pending23a(struct rtw_adapter *padapter,
				     struct sta_info *psta, int up, u8 *ac);
s32 rtw_xmitframe_enqueue23a(struct rtw_adapter *padapter,
			     struct xmit_frame *pxmitframe);
struct xmit_frame *rtw_dequeue_xframe23a(struct xmit_priv *pxmitpriv,
				      struct hw_xmit *phwxmit_i, int entry);
s32 rtw_xmit23a_classifier(struct rtw_adapter *padapter,
			struct xmit_frame *pxmitframe);
s32 rtw_xmitframe_coalesce23a(struct rtw_adapter *padapter, struct sk_buff *pkt,
			      struct xmit_frame *pxmitframe);
s32 _rtw_init_hw_txqueue(struct hw_txqueue *phw_txqueue, u8 ac_tag);
void _rtw_init_sta_xmit_priv23a(struct sta_xmit_priv *psta_xmitpriv);

s32 rtw_txframes_pending23a(struct rtw_adapter *padapter);
s32 rtw_txframes_sta_ac_pending23a(struct rtw_adapter *padapter,
				struct pkt_attrib *pattrib);
void rtw_init_hwxmits23a(struct hw_xmit *phwxmit, int entry);
int _rtw_init_xmit_priv23a(struct xmit_priv *pxmitpriv,
			struct rtw_adapter *padapter);
void _rtw_free_xmit_priv23a(struct xmit_priv *pxmitpriv);
void rtw_alloc_hwxmits23a(struct rtw_adapter *padapter);
void rtw_free_hwxmits23a(struct rtw_adapter *padapter);
int rtw_xmit23a(struct rtw_adapter *padapter, struct sk_buff *pkt);
#if defined(CONFIG_8723AU_AP_MODE)
int xmitframe_enqueue_for_sleeping_sta23a(struct rtw_adapter *padapter,
				       struct xmit_frame *pxmitframe);
void stop_sta_xmit23a(struct rtw_adapter *padapter, struct sta_info *psta);
void wakeup_sta_to_xmit23a(struct rtw_adapter *padapter, struct sta_info *psta);
void xmit_delivery_enabled_frames23a(struct rtw_adapter *padapter,
				  struct sta_info *psta);
#endif
u8	qos_acm23a(u8 acm_mask, u8 priority);
u32	rtw_get_ff_hwaddr23a(struct xmit_frame	*pxmitframe);
int rtw_ack_tx_wait23a(struct xmit_priv *pxmitpriv, u32 timeout_ms);

/* include after declaring struct xmit_buf, in order to avoid warning */
#include <xmit_osdep.h>

#endif	/* _RTL871X_XMIT_H_ */
