/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef __NIX_H__
#define	__NIX_H__

#include <asm/arch/csrs/csrs-npa.h>
#include <asm/arch/csrs/csrs-nix.h>
#include "rvu.h"

/** Maximum number of LMACs supported */
#define MAX_LMAC			12

/* NIX RX action operation*/
#define NIX_RX_ACTIONOP_DROP		(0x0ull)
#define NIX_RX_ACTIONOP_UCAST		(0x1ull)
#define NIX_RX_ACTIONOP_UCAST_IPSEC	(0x2ull)
#define NIX_RX_ACTIONOP_MCAST		(0x3ull)
#define NIX_RX_ACTIONOP_RSS		(0x4ull)

/* NIX TX action operation*/
#define NIX_TX_ACTIONOP_DROP		(0x0ull)
#define NIX_TX_ACTIONOP_UCAST_DEFAULT	(0x1ull)
#define NIX_TX_ACTIONOP_UCAST_CHAN	(0x2ull)
#define NIX_TX_ACTIONOP_MCAST		(0x3ull)
#define NIX_TX_ACTIONOP_DROP_VIOL	(0x5ull)

#define NIX_INTF_RX			0
#define NIX_INTF_TX			1

#define NIX_INTF_TYPE_CGX		0
#define NIX_INTF_TYPE_LBK		1
#define NIX_MAX_HW_MTU			9212
#define NIX_MIN_HW_MTU			40
#define MAX_MTU				1536

#define NPA_POOL_COUNT			3
#define NPA_AURA_COUNT(x)		(1ULL << ((x) + 6))
#define NPA_POOL_RX			0ULL
#define NPA_POOL_TX			1ULL
#define NPA_POOL_SQB			2ULL
#define RQ_QLEN				Q_COUNT(Q_SIZE_1K)
#define SQ_QLEN				Q_COUNT(Q_SIZE_1K)
#define SQB_QLEN			Q_COUNT(Q_SIZE_16)

#define NIX_CQ_RX			0ULL
#define NIX_CQ_TX			1ULL
#define NIX_CQ_COUNT			2ULL
#define NIX_CQE_SIZE_W16		(16 * sizeof(u64))
#define NIX_CQE_SIZE_W64		(64 * sizeof(u64))

/** Size of aura hardware context */
#define NPA_AURA_HW_CTX_SIZE		48
/** Size of pool hardware context */
#define NPA_POOL_HW_CTX_SIZE		64

#define NPA_DEFAULT_PF_FUNC		0xffff

#define NIX_CHAN_CGX_LMAC_CHX(a, b, c)	(0x800 + 0x100 * (a) + 0x10 * (b) + (c))
#define NIX_LINK_CGX_LMAC(a, b)		(0 + 4 * (a) + (b))
#define NIX_LINK_LBK(a)			(12 + (a))
#define NIX_CHAN_LBK_CHX(a, b)		(0 + 0x100 * (a) + (b))
#define MAX_LMAC_PKIND			12

/** Number of Admin queue entries */
#define AQ_RING_SIZE	Q_COUNT(Q_SIZE_16)

/** Each completion queue contains 256 entries, see NIC_CQ_CTX_S[qsize] */
#define CQS_QSIZE			Q_SIZE_256
#define CQ_ENTRIES			Q_COUNT(CQS_QSIZE)
/**
 * Each completion queue entry contains 128 bytes, see
 * NIXX_AF_LFX_CFG[xqe_size]
 */
#define CQ_ENTRY_SIZE			NIX_CQE_SIZE_W16

enum npa_aura_size {
	NPA_AURA_SZ_0,
	NPA_AURA_SZ_128,
	NPA_AURA_SZ_256,
	NPA_AURA_SZ_512,
	NPA_AURA_SZ_1K,
	NPA_AURA_SZ_2K,
	NPA_AURA_SZ_4K,
	NPA_AURA_SZ_8K,
	NPA_AURA_SZ_16K,
	NPA_AURA_SZ_32K,
	NPA_AURA_SZ_64K,
	NPA_AURA_SZ_128K,
	NPA_AURA_SZ_256K,
	NPA_AURA_SZ_512K,
	NPA_AURA_SZ_1M,
	NPA_AURA_SZ_MAX,
};

#define NPA_AURA_SIZE_DEFAULT		NPA_AURA_SZ_128

/* NIX Transmit schedulers */
enum nix_scheduler {
	NIX_TXSCH_LVL_SMQ = 0x0,
	NIX_TXSCH_LVL_MDQ = 0x0,
	NIX_TXSCH_LVL_TL4 = 0x1,
	NIX_TXSCH_LVL_TL3 = 0x2,
	NIX_TXSCH_LVL_TL2 = 0x3,
	NIX_TXSCH_LVL_TL1 = 0x4,
	NIX_TXSCH_LVL_CNT = 0x5,
};

struct cgx;

struct nix_stats {
	u64	num_packets;
	u64	num_bytes;
};

struct nix;
struct lmac;

struct npa_af {
	void __iomem		*npa_af_base;
	struct admin_queue	aq;
	u32			aura;
};

struct npa {
	struct npa_af		*npa_af;
	void __iomem		*npa_base;
	void __iomem		*npc_base;
	void __iomem		*lmt_base;
	/** Hardware aura context */
	void			*aura_ctx;
	/** Hardware pool context */
	void			*pool_ctx[NPA_POOL_COUNT];
	void			*pool_stack[NPA_POOL_COUNT];
	void                    **buffers[NPA_POOL_COUNT];
	u32                     pool_stack_pages[NPA_POOL_COUNT];
	u32			pool_stack_pointers;
	u32			q_len[NPA_POOL_COUNT];
	u32			buf_size[NPA_POOL_COUNT];
	u32			stack_pages[NPA_POOL_COUNT];
};

struct nix_af {
	struct udevice			*dev;
	struct nix			*lmacs[MAX_LMAC];
	struct npa_af			*npa_af;
	void __iomem			*nix_af_base;
	void __iomem			*npc_af_base;
	struct admin_queue		aq;
	u8				num_lmacs;
	s8				index;
	u8				xqe_size;
	u32				sqb_size;
	u32				qints;
	u32				cints;
	u32				sq_ctx_sz;
	u32				rq_ctx_sz;
	u32				cq_ctx_sz;
	u32				rsse_ctx_sz;
	u32				cint_ctx_sz;
	u32				qint_ctx_sz;
};

struct nix_tx_dr {
	union nix_send_hdr_s	hdr;
	union nix_send_sg_s	tx_sg;
	dma_addr_t			sg1_addr;
	dma_addr_t			sg2_addr;
	dma_addr_t			sg3_addr;
	u64				in_use;
};

struct nix_rx_dr {
	union nix_cqe_hdr_s hdr;
	union nix_rx_parse_s rx_parse;
	union nix_rx_sg_s rx_sg;
};

struct nix {
	struct udevice			*dev;
	struct eth_device		*netdev;
	struct nix_af			*nix_af;
	struct npa			*npa;
	struct lmac			*lmac;
	union nix_cint_hw_s	*cint_base;
	union nix_cq_ctx_s		*cq_ctx_base;
	union nix_qint_hw_s	*qint_base;
	union nix_rq_ctx_s		*rq_ctx_base;
	union nix_rsse_s		*rss_base;
	union nix_sq_ctx_s		*sq_ctx_base;
	void				*cqe_base;
	struct qmem			sq;
	struct qmem			cq[NIX_CQ_COUNT];
	struct qmem			rq;
	struct qmem			rss;
	struct qmem			cq_ints;
	struct qmem			qints;
	char				name[16];
	void __iomem			*nix_base;	/** PF reg base */
	void __iomem			*npc_base;
	void __iomem			*lmt_base;
	struct nix_stats		tx_stats;
	struct nix_stats		rx_stats;
	u32				aura;
	int				pknd;
	int				lf;
	int				pf;
	u16				pf_func;
	u32				rq_cnt;	/** receive queues count */
	u32				sq_cnt;	/** send queues count */
	u32				cq_cnt;	/** completion queues count */
	u16				rss_sz;
	u16				sqb_size;
	u8				rss_grps;
	u8				xqe_sz;
};

struct nix_aq_cq_dis {
	union nix_aq_res_s	resp ALIGNED;
	union nix_cq_ctx_s	cq ALIGNED;
	union nix_cq_ctx_s	mcq ALIGNED;
};

struct nix_aq_rq_dis {
	union nix_aq_res_s	resp ALIGNED;
	union nix_rq_ctx_s	rq ALIGNED;
	union nix_rq_ctx_s	mrq ALIGNED;
};

struct nix_aq_sq_dis {
	union nix_aq_res_s	resp ALIGNED;
	union nix_sq_ctx_s	sq ALIGNED;
	union nix_sq_ctx_s	msq ALIGNED;
};

struct nix_aq_cq_request {
	union nix_aq_res_s	resp ALIGNED;
	union nix_cq_ctx_s	cq ALIGNED;
};

struct nix_aq_rq_request {
	union nix_aq_res_s	resp ALIGNED;
	union nix_rq_ctx_s	rq ALIGNED;
};

struct nix_aq_sq_request {
	union nix_aq_res_s	resp ALIGNED;
	union nix_sq_ctx_s	sq ALIGNED;
};

static inline u64 nix_af_reg_read(struct nix_af *nix_af, u64 offset)
{
	u64 val = readq(nix_af->nix_af_base + offset);

	debug("%s reg %p val %llx\n", __func__, nix_af->nix_af_base + offset,
	      val);
	return val;
}

static inline void nix_af_reg_write(struct nix_af *nix_af, u64 offset,
				    u64 val)
{
	debug("%s reg %p val %llx\n", __func__, nix_af->nix_af_base + offset,
	      val);
	writeq(val, nix_af->nix_af_base + offset);
}

static inline u64 nix_pf_reg_read(struct nix *nix, u64 offset)
{
	u64 val = readq(nix->nix_base + offset);

	debug("%s reg %p val %llx\n", __func__, nix->nix_base + offset,
	      val);
	return val;
}

static inline void nix_pf_reg_write(struct nix *nix, u64 offset,
				    u64 val)
{
	debug("%s reg %p val %llx\n", __func__, nix->nix_base + offset,
	      val);
	writeq(val, nix->nix_base + offset);
}

static inline u64 npa_af_reg_read(struct npa_af *npa_af, u64 offset)
{
	u64 val = readq(npa_af->npa_af_base + offset);

	debug("%s reg %p val %llx\n", __func__, npa_af->npa_af_base + offset,
	      val);
	return val;
}

static inline void npa_af_reg_write(struct npa_af *npa_af, u64 offset,
				    u64 val)
{
	debug("%s reg %p val %llx\n", __func__, npa_af->npa_af_base + offset,
	      val);
	writeq(val, npa_af->npa_af_base + offset);
}

static inline u64 npc_af_reg_read(struct nix_af *nix_af, u64 offset)
{
	u64 val = readq(nix_af->npc_af_base + offset);

	debug("%s reg %p val %llx\n", __func__, nix_af->npc_af_base + offset,
	      val);
	return val;
}

static inline void npc_af_reg_write(struct nix_af *nix_af, u64 offset,
				    u64 val)
{
	debug("%s reg %p val %llx\n", __func__, nix_af->npc_af_base + offset,
	      val);
	writeq(val, nix_af->npc_af_base + offset);
}

int npa_attach_aura(struct nix_af *nix_af, int lf,
		    const union npa_aura_s *desc, u32 aura_id);
int npa_attach_pool(struct nix_af *nix_af, int lf,
		    const union npa_pool_s *desc, u32 pool_id);
int npa_af_setup(struct npa_af *npa_af);
int npa_af_shutdown(struct npa_af *npa_af);
int npa_lf_setup(struct nix *nix);
int npa_lf_shutdown(struct nix *nix);
int npa_lf_admin_setup(struct npa *npa, int lf, dma_addr_t aura_base);
int npa_lf_admin_shutdown(struct nix_af *nix_af, int lf, u32 pool_count);

int npc_lf_admin_setup(struct nix *nix);
int npc_af_shutdown(struct nix_af *nix_af);

int nix_af_setup(struct nix_af *nix_af);
int nix_af_shutdown(struct nix_af *nix_af);
int nix_lf_setup(struct nix *nix);
int nix_lf_shutdown(struct nix *nix);
struct nix *nix_lf_alloc(struct udevice *dev);
int nix_lf_admin_setup(struct nix *nix);
int nix_lf_admin_shutdown(struct nix_af *nix_af, int lf,
			  u32 cq_count, u32 rq_count, u32 sq_count);
struct rvu_af *get_af(void);

int nix_lf_setup_mac(struct udevice *dev);
int nix_lf_read_rom_mac(struct udevice *dev);
void nix_lf_halt(struct udevice *dev);
int nix_lf_free_pkt(struct udevice *dev, uchar *pkt, int pkt_len);
int nix_lf_recv(struct udevice *dev, int flags, uchar **packetp);
int nix_lf_init(struct udevice *dev);
int nix_lf_xmit(struct udevice *dev, void *pkt, int pkt_len);

#endif /* __NIX_H__ */
