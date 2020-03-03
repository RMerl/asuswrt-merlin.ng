/*
 * This file is part of the Chelsio T4 Ethernet driver for Linux.
 *
 * Copyright (c) 2003-2014 Chelsio Communications, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __CXGB4_OFLD_H
#define __CXGB4_OFLD_H

#include <linux/cache.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/atomic.h>

/* CPL message priority levels */
enum {
	CPL_PRIORITY_DATA     = 0,  /* data messages */
	CPL_PRIORITY_SETUP    = 1,  /* connection setup messages */
	CPL_PRIORITY_TEARDOWN = 0,  /* connection teardown messages */
	CPL_PRIORITY_LISTEN   = 1,  /* listen start/stop messages */
	CPL_PRIORITY_ACK      = 1,  /* RX ACK messages */
	CPL_PRIORITY_CONTROL  = 1   /* control messages */
};

#define INIT_TP_WR(w, tid) do { \
	(w)->wr.wr_hi = htonl(FW_WR_OP_V(FW_TP_WR) | \
			      FW_WR_IMMDLEN_V(sizeof(*w) - sizeof(w->wr))); \
	(w)->wr.wr_mid = htonl(FW_WR_LEN16_V(DIV_ROUND_UP(sizeof(*w), 16)) | \
			       FW_WR_FLOWID_V(tid)); \
	(w)->wr.wr_lo = cpu_to_be64(0); \
} while (0)

#define INIT_TP_WR_CPL(w, cpl, tid) do { \
	INIT_TP_WR(w, tid); \
	OPCODE_TID(w) = htonl(MK_OPCODE_TID(cpl, tid)); \
} while (0)

#define INIT_ULPTX_WR(w, wrlen, atomic, tid) do { \
	(w)->wr.wr_hi = htonl(FW_WR_OP_V(FW_ULPTX_WR) | \
			      FW_WR_ATOMIC_V(atomic)); \
	(w)->wr.wr_mid = htonl(FW_WR_LEN16_V(DIV_ROUND_UP(wrlen, 16)) | \
			       FW_WR_FLOWID_V(tid)); \
	(w)->wr.wr_lo = cpu_to_be64(0); \
} while (0)

/* Special asynchronous notification message */
#define CXGB4_MSG_AN ((void *)1)

struct serv_entry {
	void *data;
};

union aopen_entry {
	void *data;
	union aopen_entry *next;
};

/*
 * Holds the size, base address, free list start, etc of the TID, server TID,
 * and active-open TID tables.  The tables themselves are allocated dynamically.
 */
struct tid_info {
	void **tid_tab;
	unsigned int ntids;

	struct serv_entry *stid_tab;
	unsigned long *stid_bmap;
	unsigned int nstids;
	unsigned int stid_base;

	union aopen_entry *atid_tab;
	unsigned int natids;
	unsigned int atid_base;

	struct filter_entry *ftid_tab;
	unsigned int nftids;
	unsigned int ftid_base;
	unsigned int aftid_base;
	unsigned int aftid_end;
	/* Server filter region */
	unsigned int sftid_base;
	unsigned int nsftids;

	spinlock_t atid_lock ____cacheline_aligned_in_smp;
	union aopen_entry *afree;
	unsigned int atids_in_use;

	spinlock_t stid_lock;
	unsigned int stids_in_use;

	atomic_t tids_in_use;
};

static inline void *lookup_tid(const struct tid_info *t, unsigned int tid)
{
	return tid < t->ntids ? t->tid_tab[tid] : NULL;
}

static inline void *lookup_atid(const struct tid_info *t, unsigned int atid)
{
	return atid < t->natids ? t->atid_tab[atid].data : NULL;
}

static inline void *lookup_stid(const struct tid_info *t, unsigned int stid)
{
	/* Is it a server filter TID? */
	if (t->nsftids && (stid >= t->sftid_base)) {
		stid -= t->sftid_base;
		stid += t->nstids;
	} else {
		stid -= t->stid_base;
	}

	return stid < (t->nstids + t->nsftids) ? t->stid_tab[stid].data : NULL;
}

static inline void cxgb4_insert_tid(struct tid_info *t, void *data,
				    unsigned int tid)
{
	t->tid_tab[tid] = data;
	atomic_inc(&t->tids_in_use);
}

int cxgb4_alloc_atid(struct tid_info *t, void *data);
int cxgb4_alloc_stid(struct tid_info *t, int family, void *data);
int cxgb4_alloc_sftid(struct tid_info *t, int family, void *data);
void cxgb4_free_atid(struct tid_info *t, unsigned int atid);
void cxgb4_free_stid(struct tid_info *t, unsigned int stid, int family);
void cxgb4_remove_tid(struct tid_info *t, unsigned int qid, unsigned int tid);

struct in6_addr;

int cxgb4_create_server(const struct net_device *dev, unsigned int stid,
			__be32 sip, __be16 sport, __be16 vlan,
			unsigned int queue);
int cxgb4_create_server6(const struct net_device *dev, unsigned int stid,
			 const struct in6_addr *sip, __be16 sport,
			 unsigned int queue);
int cxgb4_remove_server(const struct net_device *dev, unsigned int stid,
			unsigned int queue, bool ipv6);
int cxgb4_create_server_filter(const struct net_device *dev, unsigned int stid,
			       __be32 sip, __be16 sport, __be16 vlan,
			       unsigned int queue,
			       unsigned char port, unsigned char mask);
int cxgb4_remove_server_filter(const struct net_device *dev, unsigned int stid,
			       unsigned int queue, bool ipv6);

static inline void set_wr_txq(struct sk_buff *skb, int prio, int queue)
{
	skb_set_queue_mapping(skb, (queue << 1) | prio);
}

enum cxgb4_uld {
	CXGB4_ULD_RDMA,
	CXGB4_ULD_ISCSI,
	CXGB4_ULD_MAX
};

enum cxgb4_state {
	CXGB4_STATE_UP,
	CXGB4_STATE_START_RECOVERY,
	CXGB4_STATE_DOWN,
	CXGB4_STATE_DETACH
};

enum cxgb4_control {
	CXGB4_CONTROL_DB_FULL,
	CXGB4_CONTROL_DB_EMPTY,
	CXGB4_CONTROL_DB_DROP,
};

struct pci_dev;
struct l2t_data;
struct net_device;
struct pkt_gl;
struct tp_tcp_stats;

struct cxgb4_range {
	unsigned int start;
	unsigned int size;
};

struct cxgb4_virt_res {                      /* virtualized HW resources */
	struct cxgb4_range ddp;
	struct cxgb4_range iscsi;
	struct cxgb4_range stag;
	struct cxgb4_range rq;
	struct cxgb4_range pbl;
	struct cxgb4_range qp;
	struct cxgb4_range cq;
	struct cxgb4_range ocq;
};

#define OCQ_WIN_OFFSET(pdev, vres) \
	(pci_resource_len((pdev), 2) - roundup_pow_of_two((vres)->ocq.size))

/*
 * Block of information the LLD provides to ULDs attaching to a device.
 */
struct cxgb4_lld_info {
	struct pci_dev *pdev;                /* associated PCI device */
	struct l2t_data *l2t;                /* L2 table */
	struct tid_info *tids;               /* TID table */
	struct net_device **ports;           /* device ports */
	const struct cxgb4_virt_res *vr;     /* assorted HW resources */
	const unsigned short *mtus;          /* MTU table */
	const unsigned short *rxq_ids;       /* the ULD's Rx queue ids */
	const unsigned short *ciq_ids;       /* the ULD's concentrator IQ ids */
	unsigned short nrxq;                 /* # of Rx queues */
	unsigned short ntxq;                 /* # of Tx queues */
	unsigned short nciq;		     /* # of concentrator IQ */
	unsigned char nchan:4;               /* # of channels */
	unsigned char nports:4;              /* # of ports */
	unsigned char wr_cred;               /* WR 16-byte credits */
	unsigned char adapter_type;          /* type of adapter */
	unsigned char fw_api_ver;            /* FW API version */
	unsigned int fw_vers;                /* FW version */
	unsigned int iscsi_iolen;            /* iSCSI max I/O length */
	unsigned int cclk_ps;                /* Core clock period in psec */
	unsigned short udb_density;          /* # of user DB/page */
	unsigned short ucq_density;          /* # of user CQs/page */
	unsigned short filt_mode;            /* filter optional components */
	unsigned short tx_modq[NCHAN];       /* maps each tx channel to a */
					     /* scheduler queue */
	void __iomem *gts_reg;               /* address of GTS register */
	void __iomem *db_reg;                /* address of kernel doorbell */
	int dbfifo_int_thresh;		     /* doorbell fifo int threshold */
	unsigned int sge_ingpadboundary;     /* SGE ingress padding boundary */
	unsigned int sge_egrstatuspagesize;  /* SGE egress status page size */
	unsigned int sge_pktshift;           /* Padding between CPL and */
					     /*	packet data */
	unsigned int pf;		     /* Physical Function we're using */
	bool enable_fw_ofld_conn;            /* Enable connection through fw */
					     /* WR */
	unsigned int max_ordird_qp;          /* Max ORD/IRD depth per RDMA QP */
	unsigned int max_ird_adapter;        /* Max IRD memory per adapter */
	bool ulptx_memwrite_dsgl;            /* use of T5 DSGL allowed */
};

struct cxgb4_uld_info {
	const char *name;
	void *(*add)(const struct cxgb4_lld_info *p);
	int (*rx_handler)(void *handle, const __be64 *rsp,
			  const struct pkt_gl *gl);
	int (*state_change)(void *handle, enum cxgb4_state new_state);
	int (*control)(void *handle, enum cxgb4_control control, ...);
};

int cxgb4_register_uld(enum cxgb4_uld type, const struct cxgb4_uld_info *p);
int cxgb4_unregister_uld(enum cxgb4_uld type);
int cxgb4_ofld_send(struct net_device *dev, struct sk_buff *skb);
unsigned int cxgb4_dbfifo_count(const struct net_device *dev, int lpfifo);
unsigned int cxgb4_port_chan(const struct net_device *dev);
unsigned int cxgb4_port_viid(const struct net_device *dev);
unsigned int cxgb4_port_idx(const struct net_device *dev);
unsigned int cxgb4_best_mtu(const unsigned short *mtus, unsigned short mtu,
			    unsigned int *idx);
unsigned int cxgb4_best_aligned_mtu(const unsigned short *mtus,
				    unsigned short header_size,
				    unsigned short data_size_max,
				    unsigned short data_size_align,
				    unsigned int *mtu_idxp);
void cxgb4_get_tcp_stats(struct pci_dev *pdev, struct tp_tcp_stats *v4,
			 struct tp_tcp_stats *v6);
void cxgb4_iscsi_init(struct net_device *dev, unsigned int tag_mask,
		      const unsigned int *pgsz_order);
struct sk_buff *cxgb4_pktgl_to_skb(const struct pkt_gl *gl,
				   unsigned int skb_len, unsigned int pull_len);
int cxgb4_sync_txq_pidx(struct net_device *dev, u16 qid, u16 pidx, u16 size);
int cxgb4_flush_eq_cache(struct net_device *dev);
void cxgb4_disable_db_coalescing(struct net_device *dev);
void cxgb4_enable_db_coalescing(struct net_device *dev);
int cxgb4_read_tpte(struct net_device *dev, u32 stag, __be32 *tpte);
u64 cxgb4_read_sge_timestamp(struct net_device *dev);

enum cxgb4_bar2_qtype { CXGB4_BAR2_QTYPE_EGRESS, CXGB4_BAR2_QTYPE_INGRESS };
int cxgb4_bar2_sge_qregs(struct net_device *dev,
			 unsigned int qid,
			 enum cxgb4_bar2_qtype qtype,
			 u64 *pbar2_qoffset,
			 unsigned int *pbar2_qid);

#endif  /* !__CXGB4_OFLD_H */
