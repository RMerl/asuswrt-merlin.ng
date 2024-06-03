// SPDX-License-Identifier: GPL-2.0+
/*
 * TI K3 AM65x NAVSS Ring accelerator Manager (RA) subsystem driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <asm/dma-mapping.h>
#include <asm/bitops.h>
#include <dm.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <linux/compat.h>
#include <linux/soc/ti/k3-navss-ringacc.h>
#include <linux/soc/ti/ti_sci_protocol.h>

#define set_bit(bit, bitmap)	__set_bit(bit, bitmap)
#define clear_bit(bit, bitmap)	__clear_bit(bit, bitmap)
#define dma_free_coherent(dev, size, cpu_addr, dma_handle) \
	dma_free_coherent(cpu_addr)
#define dma_zalloc_coherent(dev, size, dma_handle, flag) \
({ \
	void	*ring_mem_virt; \
	ring_mem_virt = dma_alloc_coherent((size), \
					   (unsigned long *)(dma_handle)); \
	if (ring_mem_virt) \
		memset(ring_mem_virt, 0, (size)); \
	ring_mem_virt; \
})

static LIST_HEAD(k3_nav_ringacc_list);

static	void ringacc_writel(u32 v, void __iomem *reg)
{
	pr_debug("WRITEL(32): v(%08X)-->reg(%p)\n", v, reg);
	writel(v, reg);
}

static	u32 ringacc_readl(void __iomem *reg)
{
	u32 v;

	v = readl(reg);
	pr_debug("READL(32): v(%08X)<--reg(%p)\n", v, reg);
	return v;
}

#define KNAV_RINGACC_CFG_RING_SIZE_ELCNT_MASK		GENMASK(19, 0)

/**
 * struct k3_nav_ring_rt_regs -  The RA Control/Status Registers region
 */
struct k3_nav_ring_rt_regs {
	u32	resv_16[4];
	u32	db;		/* RT Ring N Doorbell Register */
	u32	resv_4[1];
	u32	occ;		/* RT Ring N Occupancy Register */
	u32	indx;		/* RT Ring N Current Index Register */
	u32	hwocc;		/* RT Ring N Hardware Occupancy Register */
	u32	hwindx;		/* RT Ring N Current Index Register */
};

#define KNAV_RINGACC_RT_REGS_STEP	0x1000

/**
 * struct k3_nav_ring_fifo_regs -  The Ring Accelerator Queues Registers region
 */
struct k3_nav_ring_fifo_regs {
	u32	head_data[128];		/* Ring Head Entry Data Registers */
	u32	tail_data[128];		/* Ring Tail Entry Data Registers */
	u32	peek_head_data[128];	/* Ring Peek Head Entry Data Regs */
	u32	peek_tail_data[128];	/* Ring Peek Tail Entry Data Regs */
};

/**
 * struct k3_ringacc_proxy_gcfg_regs - RA Proxy Global Config MMIO Region
 */
struct k3_ringacc_proxy_gcfg_regs {
	u32	revision;	/* Revision Register */
	u32	config;		/* Config Register */
};

#define K3_RINGACC_PROXY_CFG_THREADS_MASK		GENMASK(15, 0)

/**
 * struct k3_ringacc_proxy_target_regs -  RA Proxy Datapath MMIO Region
 */
struct k3_ringacc_proxy_target_regs {
	u32	control;	/* Proxy Control Register */
	u32	status;		/* Proxy Status Register */
	u8	resv_512[504];
	u32	data[128];	/* Proxy Data Register */
};

#define K3_RINGACC_PROXY_TARGET_STEP	0x1000
#define K3_RINGACC_PROXY_NOT_USED	(-1)

enum k3_ringacc_proxy_access_mode {
	PROXY_ACCESS_MODE_HEAD = 0,
	PROXY_ACCESS_MODE_TAIL = 1,
	PROXY_ACCESS_MODE_PEEK_HEAD = 2,
	PROXY_ACCESS_MODE_PEEK_TAIL = 3,
};

#define KNAV_RINGACC_FIFO_WINDOW_SIZE_BYTES  (512U)
#define KNAV_RINGACC_FIFO_REGS_STEP	0x1000
#define KNAV_RINGACC_MAX_DB_RING_CNT    (127U)

/**
 * struct k3_nav_ring_ops -  Ring operations
 */
struct k3_nav_ring_ops {
	int (*push_tail)(struct k3_nav_ring *ring, void *elm);
	int (*push_head)(struct k3_nav_ring *ring, void *elm);
	int (*pop_tail)(struct k3_nav_ring *ring, void *elm);
	int (*pop_head)(struct k3_nav_ring *ring, void *elm);
};

/**
 * struct k3_nav_ring - RA Ring descriptor
 *
 * @rt - Ring control/status registers
 * @fifos - Ring queues registers
 * @proxy - Ring Proxy Datapath registers
 * @ring_mem_dma - Ring buffer dma address
 * @ring_mem_virt - Ring buffer virt address
 * @ops - Ring operations
 * @size - Ring size in elements
 * @elm_size - Size of the ring element
 * @mode - Ring mode
 * @flags - flags
 * @free - Number of free elements
 * @occ - Ring occupancy
 * @windex - Write index (only for @K3_NAV_RINGACC_RING_MODE_RING)
 * @rindex - Read index (only for @K3_NAV_RINGACC_RING_MODE_RING)
 * @ring_id - Ring Id
 * @parent - Pointer on struct @k3_nav_ringacc
 * @use_count - Use count for shared rings
 * @proxy_id - RA Ring Proxy Id (only if @K3_NAV_RINGACC_RING_USE_PROXY)
 */
struct k3_nav_ring {
	struct k3_nav_ring_rt_regs __iomem *rt;
	struct k3_nav_ring_fifo_regs __iomem *fifos;
	struct k3_ringacc_proxy_target_regs  __iomem *proxy;
	dma_addr_t	ring_mem_dma;
	void		*ring_mem_virt;
	struct k3_nav_ring_ops *ops;
	u32		size;
	enum k3_nav_ring_size elm_size;
	enum k3_nav_ring_mode mode;
	u32		flags;
#define KNAV_RING_FLAG_BUSY	BIT(1)
#define K3_NAV_RING_FLAG_SHARED	BIT(2)
	u32		free;
	u32		occ;
	u32		windex;
	u32		rindex;
	u32		ring_id;
	struct k3_nav_ringacc	*parent;
	u32		use_count;
	int		proxy_id;
};

/**
 * struct k3_nav_ringacc - Rings accelerator descriptor
 *
 * @dev - pointer on RA device
 * @proxy_gcfg - RA proxy global config registers
 * @proxy_target_base - RA proxy datapath region
 * @num_rings - number of ring in RA
 * @rm_gp_range - general purpose rings range from tisci
 * @dma_ring_reset_quirk - DMA reset w/a enable
 * @num_proxies - number of RA proxies
 * @rings - array of rings descriptors (struct @k3_nav_ring)
 * @list - list of RAs in the system
 * @tisci - pointer ti-sci handle
 * @tisci_ring_ops - ti-sci rings ops
 * @tisci_dev_id - ti-sci device id
 */
struct k3_nav_ringacc {
	struct udevice *dev;
	struct k3_ringacc_proxy_gcfg_regs __iomem *proxy_gcfg;
	void __iomem *proxy_target_base;
	u32 num_rings; /* number of rings in Ringacc module */
	unsigned long *rings_inuse;
	struct ti_sci_resource *rm_gp_range;
	bool dma_ring_reset_quirk;
	u32 num_proxies;
	unsigned long *proxy_inuse;

	struct k3_nav_ring *rings;
	struct list_head list;

	const struct ti_sci_handle *tisci;
	const struct ti_sci_rm_ringacc_ops *tisci_ring_ops;
	u32  tisci_dev_id;
};

static long k3_nav_ringacc_ring_get_fifo_pos(struct k3_nav_ring *ring)
{
	return KNAV_RINGACC_FIFO_WINDOW_SIZE_BYTES -
	       (4 << ring->elm_size);
}

static void *k3_nav_ringacc_get_elm_addr(struct k3_nav_ring *ring, u32 idx)
{
	return (idx * (4 << ring->elm_size) + ring->ring_mem_virt);
}

static int k3_nav_ringacc_ring_push_mem(struct k3_nav_ring *ring, void *elem);
static int k3_nav_ringacc_ring_pop_mem(struct k3_nav_ring *ring, void *elem);

static struct k3_nav_ring_ops k3_nav_mode_ring_ops = {
		.push_tail = k3_nav_ringacc_ring_push_mem,
		.pop_head = k3_nav_ringacc_ring_pop_mem,
};

static int k3_nav_ringacc_ring_push_io(struct k3_nav_ring *ring, void *elem);
static int k3_nav_ringacc_ring_pop_io(struct k3_nav_ring *ring, void *elem);
static int k3_nav_ringacc_ring_push_head_io(struct k3_nav_ring *ring,
					    void *elem);
static int k3_nav_ringacc_ring_pop_tail_io(struct k3_nav_ring *ring,
					   void *elem);

static struct k3_nav_ring_ops k3_nav_mode_msg_ops = {
		.push_tail = k3_nav_ringacc_ring_push_io,
		.push_head = k3_nav_ringacc_ring_push_head_io,
		.pop_tail = k3_nav_ringacc_ring_pop_tail_io,
		.pop_head = k3_nav_ringacc_ring_pop_io,
};

static int k3_ringacc_ring_push_head_proxy(struct k3_nav_ring *ring,
					   void *elem);
static int k3_ringacc_ring_push_tail_proxy(struct k3_nav_ring *ring,
					   void *elem);
static int k3_ringacc_ring_pop_head_proxy(struct k3_nav_ring *ring, void *elem);
static int k3_ringacc_ring_pop_tail_proxy(struct k3_nav_ring *ring, void *elem);

static struct k3_nav_ring_ops k3_nav_mode_proxy_ops = {
		.push_tail = k3_ringacc_ring_push_tail_proxy,
		.push_head = k3_ringacc_ring_push_head_proxy,
		.pop_tail = k3_ringacc_ring_pop_tail_proxy,
		.pop_head = k3_ringacc_ring_pop_head_proxy,
};

struct udevice *k3_nav_ringacc_get_dev(struct k3_nav_ringacc *ringacc)
{
	return ringacc->dev;
}

struct k3_nav_ring *k3_nav_ringacc_request_ring(struct k3_nav_ringacc *ringacc,
						int id, u32 flags)
{
	int proxy_id = K3_RINGACC_PROXY_NOT_USED;

	if (id == K3_NAV_RINGACC_RING_ID_ANY) {
		/* Request for any general purpose ring */
		struct ti_sci_resource_desc *gp_rings =
					&ringacc->rm_gp_range->desc[0];
		unsigned long size;

		size = gp_rings->start + gp_rings->num;
		id = find_next_zero_bit(ringacc->rings_inuse,
					size, gp_rings->start);
		if (id == size)
			goto error;
	} else if (id < 0) {
		goto error;
	}

	if (test_bit(id, ringacc->rings_inuse) &&
	    !(ringacc->rings[id].flags & K3_NAV_RING_FLAG_SHARED))
		goto error;
	else if (ringacc->rings[id].flags & K3_NAV_RING_FLAG_SHARED)
		goto out;

	if (flags & K3_NAV_RINGACC_RING_USE_PROXY) {
		proxy_id = find_next_zero_bit(ringacc->proxy_inuse,
					      ringacc->num_proxies, 0);
		if (proxy_id == ringacc->num_proxies)
			goto error;
	}

	if (!try_module_get(ringacc->dev->driver->owner))
		goto error;

	if (proxy_id != K3_RINGACC_PROXY_NOT_USED) {
		set_bit(proxy_id, ringacc->proxy_inuse);
		ringacc->rings[id].proxy_id = proxy_id;
		pr_debug("Giving ring#%d proxy#%d\n",
			 id, proxy_id);
	} else {
		pr_debug("Giving ring#%d\n", id);
	}

	set_bit(id, ringacc->rings_inuse);
out:
	ringacc->rings[id].use_count++;
	return &ringacc->rings[id];

error:
	return NULL;
}

static void k3_ringacc_ring_reset_sci(struct k3_nav_ring *ring)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	int ret;

	ret = ringacc->tisci_ring_ops->config(
			ringacc->tisci,
			TI_SCI_MSG_VALUE_RM_RING_COUNT_VALID,
			ringacc->tisci_dev_id,
			ring->ring_id,
			0,
			0,
			ring->size,
			0,
			0,
			0);
	if (ret)
		dev_err(ringacc->dev, "TISCI reset ring fail (%d) ring_idx %d\n",
			ret, ring->ring_id);
}

void k3_nav_ringacc_ring_reset(struct k3_nav_ring *ring)
{
	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return;

	ring->occ = 0;
	ring->free = 0;
	ring->rindex = 0;
	ring->windex = 0;

	k3_ringacc_ring_reset_sci(ring);
}

static void k3_ringacc_ring_reconfig_qmode_sci(struct k3_nav_ring *ring,
					       enum k3_nav_ring_mode mode)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	int ret;

	ret = ringacc->tisci_ring_ops->config(
			ringacc->tisci,
			TI_SCI_MSG_VALUE_RM_RING_MODE_VALID,
			ringacc->tisci_dev_id,
			ring->ring_id,
			0,
			0,
			0,
			mode,
			0,
			0);
	if (ret)
		dev_err(ringacc->dev, "TISCI reconf qmode fail (%d) ring_idx %d\n",
			ret, ring->ring_id);
}

void k3_nav_ringacc_ring_reset_dma(struct k3_nav_ring *ring, u32 occ)
{
	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return;

	if (!ring->parent->dma_ring_reset_quirk)
		return;

	if (!occ)
		occ = ringacc_readl(&ring->rt->occ);

	if (occ) {
		u32 db_ring_cnt, db_ring_cnt_cur;

		pr_debug("%s %u occ: %u\n", __func__,
			 ring->ring_id, occ);
		/* 2. Reset the ring */
		k3_ringacc_ring_reset_sci(ring);

		/*
		 * 3. Setup the ring in ring/doorbell mode
		 * (if not already in this mode)
		 */
		if (ring->mode != K3_NAV_RINGACC_RING_MODE_RING)
			k3_ringacc_ring_reconfig_qmode_sci(
					ring, K3_NAV_RINGACC_RING_MODE_RING);
		/*
		 * 4. Ring the doorbell 2**22 – ringOcc times.
		 * This will wrap the internal UDMAP ring state occupancy
		 * counter (which is 21-bits wide) to 0.
		 */
		db_ring_cnt = (1U << 22) - occ;

		while (db_ring_cnt != 0) {
			/*
			 * Ring the doorbell with the maximum count each
			 * iteration if possible to minimize the total
			 * of writes
			 */
			if (db_ring_cnt > KNAV_RINGACC_MAX_DB_RING_CNT)
				db_ring_cnt_cur = KNAV_RINGACC_MAX_DB_RING_CNT;
			else
				db_ring_cnt_cur = db_ring_cnt;

			writel(db_ring_cnt_cur, &ring->rt->db);
			db_ring_cnt -= db_ring_cnt_cur;
		}

		/* 5. Restore the original ring mode (if not ring mode) */
		if (ring->mode != K3_NAV_RINGACC_RING_MODE_RING)
			k3_ringacc_ring_reconfig_qmode_sci(ring, ring->mode);
	}

	/* 2. Reset the ring */
	k3_nav_ringacc_ring_reset(ring);
}

static void k3_ringacc_ring_free_sci(struct k3_nav_ring *ring)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	int ret;

	ret = ringacc->tisci_ring_ops->config(
			ringacc->tisci,
			TI_SCI_MSG_VALUE_RM_ALL_NO_ORDER,
			ringacc->tisci_dev_id,
			ring->ring_id,
			0,
			0,
			0,
			0,
			0,
			0);
	if (ret)
		dev_err(ringacc->dev, "TISCI ring free fail (%d) ring_idx %d\n",
			ret, ring->ring_id);
}

int k3_nav_ringacc_ring_free(struct k3_nav_ring *ring)
{
	struct k3_nav_ringacc *ringacc;

	if (!ring)
		return -EINVAL;

	ringacc = ring->parent;

	pr_debug("%s flags: 0x%08x\n", __func__, ring->flags);

	if (!test_bit(ring->ring_id, ringacc->rings_inuse))
		return -EINVAL;

	if (--ring->use_count)
		goto out;

	if (!(ring->flags & KNAV_RING_FLAG_BUSY))
		goto no_init;

	k3_ringacc_ring_free_sci(ring);

	dma_free_coherent(ringacc->dev,
			  ring->size * (4 << ring->elm_size),
			  ring->ring_mem_virt, ring->ring_mem_dma);
	ring->flags &= ~KNAV_RING_FLAG_BUSY;
	ring->ops = NULL;
	if (ring->proxy_id != K3_RINGACC_PROXY_NOT_USED) {
		clear_bit(ring->proxy_id, ringacc->proxy_inuse);
		ring->proxy = NULL;
		ring->proxy_id = K3_RINGACC_PROXY_NOT_USED;
	}

no_init:
	clear_bit(ring->ring_id, ringacc->rings_inuse);

	module_put(ringacc->dev->driver->owner);

out:
	return 0;
}

u32 k3_nav_ringacc_get_ring_id(struct k3_nav_ring *ring)
{
	if (!ring)
		return -EINVAL;

	return ring->ring_id;
}

static int k3_nav_ringacc_ring_cfg_sci(struct k3_nav_ring *ring)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	u32 ring_idx;
	int ret;

	if (!ringacc->tisci)
		return -EINVAL;

	ring_idx = ring->ring_id;
	ret = ringacc->tisci_ring_ops->config(
			ringacc->tisci,
			TI_SCI_MSG_VALUE_RM_ALL_NO_ORDER,
			ringacc->tisci_dev_id,
			ring_idx,
			lower_32_bits(ring->ring_mem_dma),
			upper_32_bits(ring->ring_mem_dma),
			ring->size,
			ring->mode,
			ring->elm_size,
			0);
	if (ret)
		dev_err(ringacc->dev, "TISCI config ring fail (%d) ring_idx %d\n",
			ret, ring_idx);

	return ret;
}

int k3_nav_ringacc_ring_cfg(struct k3_nav_ring *ring,
			    struct k3_nav_ring_cfg *cfg)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	int ret = 0;

	if (!ring || !cfg)
		return -EINVAL;
	if (cfg->elm_size > K3_NAV_RINGACC_RING_ELSIZE_256 ||
	    cfg->mode > K3_NAV_RINGACC_RING_MODE_QM ||
	    cfg->size & ~KNAV_RINGACC_CFG_RING_SIZE_ELCNT_MASK ||
	    !test_bit(ring->ring_id, ringacc->rings_inuse))
		return -EINVAL;

	if (ring->use_count != 1)
		return 0;

	ring->size = cfg->size;
	ring->elm_size = cfg->elm_size;
	ring->mode = cfg->mode;
	ring->occ = 0;
	ring->free = 0;
	ring->rindex = 0;
	ring->windex = 0;

	if (ring->proxy_id != K3_RINGACC_PROXY_NOT_USED)
		ring->proxy = ringacc->proxy_target_base +
			      ring->proxy_id * K3_RINGACC_PROXY_TARGET_STEP;

	switch (ring->mode) {
	case K3_NAV_RINGACC_RING_MODE_RING:
		ring->ops = &k3_nav_mode_ring_ops;
		break;
	case K3_NAV_RINGACC_RING_MODE_QM:
		/*
		 * In Queue mode elm_size can be 8 only and each operation
		 * uses 2 element slots
		 */
		if (cfg->elm_size != K3_NAV_RINGACC_RING_ELSIZE_8 ||
		    cfg->size % 2)
			goto err_free_proxy;
	case K3_NAV_RINGACC_RING_MODE_MESSAGE:
		if (ring->proxy)
			ring->ops = &k3_nav_mode_proxy_ops;
		else
			ring->ops = &k3_nav_mode_msg_ops;
		break;
	default:
		ring->ops = NULL;
		ret = -EINVAL;
		goto err_free_proxy;
	};

	ring->ring_mem_virt =
			dma_zalloc_coherent(ringacc->dev,
					    ring->size * (4 << ring->elm_size),
					    &ring->ring_mem_dma, GFP_KERNEL);
	if (!ring->ring_mem_virt) {
		dev_err(ringacc->dev, "Failed to alloc ring mem\n");
		ret = -ENOMEM;
		goto err_free_ops;
	}

	ret = k3_nav_ringacc_ring_cfg_sci(ring);

	if (ret)
		goto err_free_mem;

	ring->flags |= KNAV_RING_FLAG_BUSY;
	ring->flags |= (cfg->flags & K3_NAV_RINGACC_RING_SHARED) ?
			K3_NAV_RING_FLAG_SHARED : 0;

	return 0;

err_free_mem:
	dma_free_coherent(ringacc->dev,
			  ring->size * (4 << ring->elm_size),
			  ring->ring_mem_virt,
			  ring->ring_mem_dma);
err_free_ops:
	ring->ops = NULL;
err_free_proxy:
	ring->proxy = NULL;
	return ret;
}

u32 k3_nav_ringacc_ring_get_size(struct k3_nav_ring *ring)
{
	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	return ring->size;
}

u32 k3_nav_ringacc_ring_get_free(struct k3_nav_ring *ring)
{
	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	if (!ring->free)
		ring->free = ring->size - ringacc_readl(&ring->rt->occ);

	return ring->free;
}

u32 k3_nav_ringacc_ring_get_occ(struct k3_nav_ring *ring)
{
	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	return ringacc_readl(&ring->rt->occ);
}

u32 k3_nav_ringacc_ring_is_full(struct k3_nav_ring *ring)
{
	return !k3_nav_ringacc_ring_get_free(ring);
}

enum k3_ringacc_access_mode {
	K3_RINGACC_ACCESS_MODE_PUSH_HEAD,
	K3_RINGACC_ACCESS_MODE_POP_HEAD,
	K3_RINGACC_ACCESS_MODE_PUSH_TAIL,
	K3_RINGACC_ACCESS_MODE_POP_TAIL,
	K3_RINGACC_ACCESS_MODE_PEEK_HEAD,
	K3_RINGACC_ACCESS_MODE_PEEK_TAIL,
};

static int k3_ringacc_ring_cfg_proxy(struct k3_nav_ring *ring,
				     enum k3_ringacc_proxy_access_mode mode)
{
	u32 val;

	val = ring->ring_id;
	val |= mode << 16;
	val |= ring->elm_size << 24;
	ringacc_writel(val, &ring->proxy->control);
	return 0;
}

static int k3_nav_ringacc_ring_access_proxy(
			struct k3_nav_ring *ring, void *elem,
			enum k3_ringacc_access_mode access_mode)
{
	void __iomem *ptr;

	ptr = (void __iomem *)&ring->proxy->data;

	switch (access_mode) {
	case K3_RINGACC_ACCESS_MODE_PUSH_HEAD:
	case K3_RINGACC_ACCESS_MODE_POP_HEAD:
		k3_ringacc_ring_cfg_proxy(ring, PROXY_ACCESS_MODE_HEAD);
		break;
	case K3_RINGACC_ACCESS_MODE_PUSH_TAIL:
	case K3_RINGACC_ACCESS_MODE_POP_TAIL:
		k3_ringacc_ring_cfg_proxy(ring, PROXY_ACCESS_MODE_TAIL);
		break;
	default:
		return -EINVAL;
	}

	ptr += k3_nav_ringacc_ring_get_fifo_pos(ring);

	switch (access_mode) {
	case K3_RINGACC_ACCESS_MODE_POP_HEAD:
	case K3_RINGACC_ACCESS_MODE_POP_TAIL:
		pr_debug("proxy:memcpy_fromio(x): --> ptr(%p), mode:%d\n",
			 ptr, access_mode);
		memcpy_fromio(elem, ptr, (4 << ring->elm_size));
		ring->occ--;
		break;
	case K3_RINGACC_ACCESS_MODE_PUSH_TAIL:
	case K3_RINGACC_ACCESS_MODE_PUSH_HEAD:
		pr_debug("proxy:memcpy_toio(x): --> ptr(%p), mode:%d\n",
			 ptr, access_mode);
		memcpy_toio(ptr, elem, (4 << ring->elm_size));
		ring->free--;
		break;
	default:
		return -EINVAL;
	}

	pr_debug("proxy: free%d occ%d\n",
		 ring->free, ring->occ);
	return 0;
}

static int k3_ringacc_ring_push_head_proxy(struct k3_nav_ring *ring, void *elem)
{
	return k3_nav_ringacc_ring_access_proxy(
			ring, elem, K3_RINGACC_ACCESS_MODE_PUSH_HEAD);
}

static int k3_ringacc_ring_push_tail_proxy(struct k3_nav_ring *ring, void *elem)
{
	return k3_nav_ringacc_ring_access_proxy(
			ring, elem, K3_RINGACC_ACCESS_MODE_PUSH_TAIL);
}

static int k3_ringacc_ring_pop_head_proxy(struct k3_nav_ring *ring, void *elem)
{
	return k3_nav_ringacc_ring_access_proxy(
			ring, elem, K3_RINGACC_ACCESS_MODE_POP_HEAD);
}

static int k3_ringacc_ring_pop_tail_proxy(struct k3_nav_ring *ring, void *elem)
{
	return k3_nav_ringacc_ring_access_proxy(
			ring, elem, K3_RINGACC_ACCESS_MODE_POP_HEAD);
}

static int k3_nav_ringacc_ring_access_io(
		struct k3_nav_ring *ring, void *elem,
		enum k3_ringacc_access_mode access_mode)
{
	void __iomem *ptr;

	switch (access_mode) {
	case K3_RINGACC_ACCESS_MODE_PUSH_HEAD:
	case K3_RINGACC_ACCESS_MODE_POP_HEAD:
		ptr = (void __iomem *)&ring->fifos->head_data;
		break;
	case K3_RINGACC_ACCESS_MODE_PUSH_TAIL:
	case K3_RINGACC_ACCESS_MODE_POP_TAIL:
		ptr = (void __iomem *)&ring->fifos->tail_data;
		break;
	default:
		return -EINVAL;
	}

	ptr += k3_nav_ringacc_ring_get_fifo_pos(ring);

	switch (access_mode) {
	case K3_RINGACC_ACCESS_MODE_POP_HEAD:
	case K3_RINGACC_ACCESS_MODE_POP_TAIL:
		pr_debug("memcpy_fromio(x): --> ptr(%p), mode:%d\n",
			 ptr, access_mode);
		memcpy_fromio(elem, ptr, (4 << ring->elm_size));
		ring->occ--;
		break;
	case K3_RINGACC_ACCESS_MODE_PUSH_TAIL:
	case K3_RINGACC_ACCESS_MODE_PUSH_HEAD:
		pr_debug("memcpy_toio(x): --> ptr(%p), mode:%d\n",
			 ptr, access_mode);
		memcpy_toio(ptr, elem, (4 << ring->elm_size));
		ring->free--;
		break;
	default:
		return -EINVAL;
	}

	pr_debug("free%d index%d occ%d index%d\n",
		 ring->free, ring->windex, ring->occ, ring->rindex);
	return 0;
}

static int k3_nav_ringacc_ring_push_head_io(struct k3_nav_ring *ring,
					    void *elem)
{
	return k3_nav_ringacc_ring_access_io(
			ring, elem, K3_RINGACC_ACCESS_MODE_PUSH_HEAD);
}

static int k3_nav_ringacc_ring_push_io(struct k3_nav_ring *ring, void *elem)
{
	return k3_nav_ringacc_ring_access_io(
			ring, elem, K3_RINGACC_ACCESS_MODE_PUSH_TAIL);
}

static int k3_nav_ringacc_ring_pop_io(struct k3_nav_ring *ring, void *elem)
{
	return k3_nav_ringacc_ring_access_io(
			ring, elem, K3_RINGACC_ACCESS_MODE_POP_HEAD);
}

static int k3_nav_ringacc_ring_pop_tail_io(struct k3_nav_ring *ring, void *elem)
{
	return k3_nav_ringacc_ring_access_io(
			ring, elem, K3_RINGACC_ACCESS_MODE_POP_HEAD);
}

static int k3_nav_ringacc_ring_push_mem(struct k3_nav_ring *ring, void *elem)
{
	void *elem_ptr;

	elem_ptr = k3_nav_ringacc_get_elm_addr(ring, ring->windex);

	memcpy(elem_ptr, elem, (4 << ring->elm_size));

	ring->windex = (ring->windex + 1) % ring->size;
	ring->free--;
	ringacc_writel(1, &ring->rt->db);

	pr_debug("ring_push_mem: free%d index%d\n",
		 ring->free, ring->windex);

	return 0;
}

static int k3_nav_ringacc_ring_pop_mem(struct k3_nav_ring *ring, void *elem)
{
	void *elem_ptr;

	elem_ptr = k3_nav_ringacc_get_elm_addr(ring, ring->rindex);

	memcpy(elem, elem_ptr, (4 << ring->elm_size));

	ring->rindex = (ring->rindex + 1) % ring->size;
	ring->occ--;
	ringacc_writel(-1, &ring->rt->db);

	pr_debug("ring_pop_mem: occ%d index%d pos_ptr%p\n",
		 ring->occ, ring->rindex, elem_ptr);
	return 0;
}

int k3_nav_ringacc_ring_push(struct k3_nav_ring *ring, void *elem)
{
	int ret = -EOPNOTSUPP;

	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	pr_debug("ring_push%d: free%d index%d\n",
		 ring->ring_id, ring->free, ring->windex);

	if (k3_nav_ringacc_ring_is_full(ring))
		return -ENOMEM;

	if (ring->ops && ring->ops->push_tail)
		ret = ring->ops->push_tail(ring, elem);

	return ret;
}

int k3_nav_ringacc_ring_push_head(struct k3_nav_ring *ring, void *elem)
{
	int ret = -EOPNOTSUPP;

	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	pr_debug("ring_push_head: free%d index%d\n",
		 ring->free, ring->windex);

	if (k3_nav_ringacc_ring_is_full(ring))
		return -ENOMEM;

	if (ring->ops && ring->ops->push_head)
		ret = ring->ops->push_head(ring, elem);

	return ret;
}

int k3_nav_ringacc_ring_pop(struct k3_nav_ring *ring, void *elem)
{
	int ret = -EOPNOTSUPP;

	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	if (!ring->occ)
		ring->occ = k3_nav_ringacc_ring_get_occ(ring);

	pr_debug("ring_pop%d: occ%d index%d\n",
		 ring->ring_id, ring->occ, ring->rindex);

	if (!ring->occ)
		return -ENODATA;

	if (ring->ops && ring->ops->pop_head)
		ret = ring->ops->pop_head(ring, elem);

	return ret;
}

int k3_nav_ringacc_ring_pop_tail(struct k3_nav_ring *ring, void *elem)
{
	int ret = -EOPNOTSUPP;

	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	if (!ring->occ)
		ring->occ = k3_nav_ringacc_ring_get_occ(ring);

	pr_debug("ring_pop_tail: occ%d index%d\n",
		 ring->occ, ring->rindex);

	if (!ring->occ)
		return -ENODATA;

	if (ring->ops && ring->ops->pop_tail)
		ret = ring->ops->pop_tail(ring, elem);

	return ret;
}

static int k3_nav_ringacc_probe_dt(struct k3_nav_ringacc *ringacc)
{
	struct udevice *dev = ringacc->dev;
	struct udevice *tisci_dev = NULL;
	int ret;

	ringacc->num_rings = dev_read_u32_default(dev, "ti,num-rings", 0);
	if (!ringacc->num_rings) {
		dev_err(dev, "ti,num-rings read failure %d\n", ret);
		return -EINVAL;
	}

	ringacc->dma_ring_reset_quirk =
			dev_read_bool(dev, "ti,dma-ring-reset-quirk");

	ret = uclass_get_device_by_name(UCLASS_FIRMWARE, "dmsc", &tisci_dev);
	if (ret) {
		pr_debug("TISCI RA RM get failed (%d)\n", ret);
		ringacc->tisci = NULL;
		return -ENODEV;
	}
	ringacc->tisci = (struct ti_sci_handle *)
			 (ti_sci_get_handle_from_sysfw(tisci_dev));

	ret = dev_read_u32_default(dev, "ti,sci", 0);
	if (!ret) {
		dev_err(dev, "TISCI RA RM disabled\n");
		ringacc->tisci = NULL;
		return ret;
	}

	ret = dev_read_u32(dev, "ti,sci-dev-id", &ringacc->tisci_dev_id);
	if (ret) {
		dev_err(dev, "ti,sci-dev-id read failure %d\n", ret);
		ringacc->tisci = NULL;
		return ret;
	}

	ringacc->rm_gp_range = devm_ti_sci_get_of_resource(
					ringacc->tisci, dev,
					ringacc->tisci_dev_id,
					"ti,sci-rm-range-gp-rings");
	if (IS_ERR(ringacc->rm_gp_range))
		ret = PTR_ERR(ringacc->rm_gp_range);

	return 0;
}

static int k3_nav_ringacc_probe(struct udevice *dev)
{
	struct k3_nav_ringacc *ringacc;
	void __iomem *base_fifo, *base_rt;
	int ret, i;

	ringacc = dev_get_priv(dev);
	if (!ringacc)
		return -ENOMEM;

	ringacc->dev = dev;

	ret = k3_nav_ringacc_probe_dt(ringacc);
	if (ret)
		return ret;

	base_rt = (uint32_t *)devfdt_get_addr_name(dev, "rt");
	pr_debug("rt %p\n", base_rt);
	if (IS_ERR(base_rt))
		return PTR_ERR(base_rt);

	base_fifo = (uint32_t *)devfdt_get_addr_name(dev, "fifos");
	pr_debug("fifos %p\n", base_fifo);
	if (IS_ERR(base_fifo))
		return PTR_ERR(base_fifo);

	ringacc->proxy_gcfg = (struct k3_ringacc_proxy_gcfg_regs __iomem *)
		devfdt_get_addr_name(dev, "proxy_gcfg");
	if (IS_ERR(ringacc->proxy_gcfg))
		return PTR_ERR(ringacc->proxy_gcfg);
	ringacc->proxy_target_base =
		(struct k3_ringacc_proxy_gcfg_regs __iomem *)
		devfdt_get_addr_name(dev, "proxy_target");
	if (IS_ERR(ringacc->proxy_target_base))
		return PTR_ERR(ringacc->proxy_target_base);

	ringacc->num_proxies = ringacc_readl(&ringacc->proxy_gcfg->config) &
					 K3_RINGACC_PROXY_CFG_THREADS_MASK;

	ringacc->rings = devm_kzalloc(dev,
				      sizeof(*ringacc->rings) *
				      ringacc->num_rings,
				      GFP_KERNEL);
	ringacc->rings_inuse = devm_kcalloc(dev,
					    BITS_TO_LONGS(ringacc->num_rings),
					    sizeof(unsigned long), GFP_KERNEL);
	ringacc->proxy_inuse = devm_kcalloc(dev,
					    BITS_TO_LONGS(ringacc->num_proxies),
					    sizeof(unsigned long), GFP_KERNEL);

	if (!ringacc->rings || !ringacc->rings_inuse || !ringacc->proxy_inuse)
		return -ENOMEM;

	for (i = 0; i < ringacc->num_rings; i++) {
		ringacc->rings[i].rt = base_rt +
				       KNAV_RINGACC_RT_REGS_STEP * i;
		ringacc->rings[i].fifos = base_fifo +
					  KNAV_RINGACC_FIFO_REGS_STEP * i;
		ringacc->rings[i].parent = ringacc;
		ringacc->rings[i].ring_id = i;
		ringacc->rings[i].proxy_id = K3_RINGACC_PROXY_NOT_USED;
	}
	dev_set_drvdata(dev, ringacc);

	ringacc->tisci_ring_ops = &ringacc->tisci->ops.rm_ring_ops;

	list_add_tail(&ringacc->list, &k3_nav_ringacc_list);

	dev_info(dev, "Ring Accelerator probed rings:%u, gp-rings[%u,%u] sci-dev-id:%u\n",
		 ringacc->num_rings,
		 ringacc->rm_gp_range->desc[0].start,
		 ringacc->rm_gp_range->desc[0].num,
		 ringacc->tisci_dev_id);
	dev_info(dev, "dma-ring-reset-quirk: %s\n",
		 ringacc->dma_ring_reset_quirk ? "enabled" : "disabled");
	dev_info(dev, "RA Proxy rev. %08x, num_proxies:%u\n",
		 ringacc_readl(&ringacc->proxy_gcfg->revision),
		 ringacc->num_proxies);
	return 0;
}

static const struct udevice_id knav_ringacc_ids[] = {
	{ .compatible = "ti,am654-navss-ringacc" },
	{},
};

U_BOOT_DRIVER(k3_navss_ringacc) = {
	.name	= "k3-navss-ringacc",
	.id	= UCLASS_MISC,
	.of_match = knav_ringacc_ids,
	.probe = k3_nav_ringacc_probe,
	.priv_auto_alloc_size = sizeof(struct k3_nav_ringacc),
};
