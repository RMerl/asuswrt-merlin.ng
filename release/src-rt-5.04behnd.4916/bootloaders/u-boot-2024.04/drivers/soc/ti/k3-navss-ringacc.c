// SPDX-License-Identifier: GPL-2.0+
/*
 * TI K3 AM65x NAVSS Ring accelerator Manager (RA) subsystem driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com
 */

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <malloc.h>
#include <asm/bitops.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/soc/ti/k3-navss-ringacc.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <linux/soc/ti/cppi5.h>

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
#define K3_DMARING_RING_CFG_RING_SIZE_ELCNT_MASK		GENMASK(15, 0)

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
#define K3_DMARING_RING_RT_REGS_STEP			0x2000
#define K3_DMARING_RING_RT_REGS_REVERSE_OFS		0x1000
#define KNAV_RINGACC_RT_OCC_MASK		GENMASK(20, 0)
#define K3_DMARING_RING_RT_OCC_TDOWN_COMPLETE		BIT(31)
#define K3_DMARING_RING_RT_DB_ENTRY_MASK		GENMASK(7, 0)
#define K3_DMARING_RING_RT_DB_TDOWN_ACK		BIT(31)


/**
 * struct k3_nav_ring_fifo_regs -  The Ring Accelerator Queues Registers region
 */
struct k3_nav_ring_fifo_regs {
	u32	head_data[128];		/* Ring Head Entry Data Registers */
	u32	tail_data[128];		/* Ring Tail Entry Data Registers */
	u32	peek_head_data[128];	/* Ring Peek Head Entry Data Regs */
	u32	peek_tail_data[128];	/* Ring Peek Tail Entry Data Regs */
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
 * struct k3_nav_ring_state - Internal state tracking structure
 *
 * @free: Number of free entries
 * @occ: Occupancy
 * @windex: Write index
 * @rindex: Read index
 */
struct k3_nav_ring_state {
	u32 free;
	u32 occ;
	u32 windex;
	u32 rindex;
	u32 tdown_complete:1;
};

/**
 * struct k3_nav_ring - RA Ring descriptor
 *
 * @cfg - Ring configuration registers
 * @rt - Ring control/status registers
 * @fifos - Ring queues registers
 * @ring_mem_dma - Ring buffer dma address
 * @ring_mem_virt - Ring buffer virt address
 * @ops - Ring operations
 * @size - Ring size in elements
 * @elm_size - Size of the ring element
 * @mode - Ring mode
 * @flags - flags
 * @ring_id - Ring Id
 * @parent - Pointer on struct @k3_nav_ringacc
 * @use_count - Use count for shared rings
 */
struct k3_nav_ring {
	struct k3_nav_ring_cfg_regs __iomem *cfg;
	struct k3_nav_ring_rt_regs __iomem *rt;
	struct k3_nav_ring_fifo_regs __iomem *fifos;
	dma_addr_t	ring_mem_dma;
	void		*ring_mem_virt;
	struct k3_nav_ring_ops *ops;
	u32		size;
	enum k3_nav_ring_size elm_size;
	enum k3_nav_ring_mode mode;
	u32		flags;
#define KNAV_RING_FLAG_BUSY	BIT(1)
#define K3_NAV_RING_FLAG_SHARED	BIT(2)
#define K3_NAV_RING_FLAG_REVERSE BIT(3)
	struct k3_nav_ring_state state;
	u32		ring_id;
	struct k3_nav_ringacc	*parent;
	u32		use_count;
};

struct k3_nav_ringacc_ops {
	int (*init)(struct udevice *dev, struct k3_nav_ringacc *ringacc);
};

/**
 * struct k3_nav_ringacc - Rings accelerator descriptor
 *
 * @dev - pointer on RA device
 * @num_rings - number of ring in RA
 * @rm_gp_range - general purpose rings range from tisci
 * @dma_ring_reset_quirk - DMA reset w/a enable
 * @num_proxies - number of RA proxies
 * @rings - array of rings descriptors (struct @k3_nav_ring)
 * @list - list of RAs in the system
 * @tisci - pointer ti-sci handle
 * @tisci_ring_ops - ti-sci rings ops
 * @tisci_dev_id - ti-sci device id
 * @ops: SoC specific ringacc operation
 * @dual_ring: indicate k3_dmaring dual ring support
 */
struct k3_nav_ringacc {
	struct udevice *dev;
	u32 num_rings; /* number of rings in Ringacc module */
	unsigned long *rings_inuse;
	struct ti_sci_resource *rm_gp_range;
	bool dma_ring_reset_quirk;
	u32 num_proxies;

	struct k3_nav_ring *rings;
	struct list_head list;

	const struct ti_sci_handle *tisci;
	const struct ti_sci_rm_ringacc_ops *tisci_ring_ops;
	u32  tisci_dev_id;

	const struct k3_nav_ringacc_ops *ops;
	bool dual_ring;
};

#include "k3-navss-ringacc-u-boot.c"

static int k3_nav_ringacc_ring_read_occ(struct k3_nav_ring *ring)
{
	return readl(&ring->rt->occ) & KNAV_RINGACC_RT_OCC_MASK;
}

static void k3_nav_ringacc_ring_update_occ(struct k3_nav_ring *ring)
{
	u32 val;

	val = readl(&ring->rt->occ);

	ring->state.occ = val & KNAV_RINGACC_RT_OCC_MASK;
	ring->state.tdown_complete = !!(val & K3_DMARING_RING_RT_OCC_TDOWN_COMPLETE);
}

static void *k3_nav_ringacc_get_elm_addr(struct k3_nav_ring *ring, u32 idx)
{
	return (idx * (4 << ring->elm_size) + ring->ring_mem_virt);
}

static int k3_nav_ringacc_ring_push_mem(struct k3_nav_ring *ring, void *elem);
static int k3_nav_ringacc_ring_pop_mem(struct k3_nav_ring *ring, void *elem);
static int k3_dmaring_ring_fwd_pop_mem(struct k3_nav_ring *ring, void *elem);
static int k3_dmaring_ring_reverse_pop_mem(struct k3_nav_ring *ring, void *elem);

static struct k3_nav_ring_ops k3_nav_mode_ring_ops = {
		.push_tail = k3_nav_ringacc_ring_push_mem,
		.pop_head = k3_nav_ringacc_ring_pop_mem,
};

static struct k3_nav_ring_ops k3_dmaring_fwd_ring_ops = {
		.push_tail = k3_nav_ringacc_ring_push_mem,
		.pop_head = k3_dmaring_ring_fwd_pop_mem,
};

static struct k3_nav_ring_ops k3_dmaring_reverse_ring_ops = {
		.pop_head = k3_dmaring_ring_reverse_pop_mem,
};

struct udevice *k3_nav_ringacc_get_dev(struct k3_nav_ringacc *ringacc)
{
	return ringacc->dev;
}

struct k3_nav_ring *k3_nav_ringacc_request_ring(struct k3_nav_ringacc *ringacc,
						int id)
{
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

	if (!try_module_get(ringacc->dev->driver->owner))
		goto error;

	pr_debug("Giving ring#%d\n", id);

	set_bit(id, ringacc->rings_inuse);
out:
	ringacc->rings[id].use_count++;
	return &ringacc->rings[id];

error:
	return NULL;
}

static int k3_dmaring_ring_request_rings_pair(struct k3_nav_ringacc *ringacc,
					      int fwd_id, int compl_id,
					      struct k3_nav_ring **fwd_ring,
					      struct k3_nav_ring **compl_ring)
{
	/* k3_dmaring: fwd_id == compl_id, so we ignore compl_id */
	if (fwd_id < 0)
		return -EINVAL;

	if (test_bit(fwd_id, ringacc->rings_inuse))
		return -EBUSY;

	*fwd_ring = &ringacc->rings[fwd_id];
	*compl_ring = &ringacc->rings[fwd_id + ringacc->num_rings];
	set_bit(fwd_id, ringacc->rings_inuse);
	ringacc->rings[fwd_id].use_count++;
	dev_dbg(ringacc->dev, "Giving ring#%d\n", fwd_id);

	return 0;
}

int k3_nav_ringacc_request_rings_pair(struct k3_nav_ringacc *ringacc,
				      int fwd_id, int compl_id,
				      struct k3_nav_ring **fwd_ring,
				      struct k3_nav_ring **compl_ring)
{
	int ret = 0;

	if (!fwd_ring || !compl_ring)
		return -EINVAL;

	if (ringacc->dual_ring)
		return k3_dmaring_ring_request_rings_pair(ringacc, fwd_id, compl_id,
						    fwd_ring, compl_ring);

	*fwd_ring = k3_nav_ringacc_request_ring(ringacc, fwd_id);
	if (!(*fwd_ring))
		return -ENODEV;

	*compl_ring = k3_nav_ringacc_request_ring(ringacc, compl_id);
	if (!(*compl_ring)) {
		k3_nav_ringacc_ring_free(*fwd_ring);
		ret = -ENODEV;
	}

	return ret;
}

static void k3_ringacc_ring_reset_sci(struct k3_nav_ring *ring)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	int ret;

	if (IS_ENABLED(CONFIG_K3_DM_FW))
		return k3_ringacc_ring_reset_raw(ring);

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

	memset(&ring->state, 0, sizeof(ring->state));

	k3_ringacc_ring_reset_sci(ring);
}

static void k3_ringacc_ring_reconfig_qmode_sci(struct k3_nav_ring *ring,
					       enum k3_nav_ring_mode mode)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	int ret;

	if (IS_ENABLED(CONFIG_K3_DM_FW))
		return k3_ringacc_ring_reconfig_qmode_raw(ring, mode);

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

	if (!ring->parent->dma_ring_reset_quirk) {
		k3_nav_ringacc_ring_reset(ring);
		return;
	}

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

	if (IS_ENABLED(CONFIG_K3_DM_FW))
		return k3_ringacc_ring_free_raw(ring);

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

	/*
	 * k3_dmaring: rings shared memory and configuration, only forward ring is
	 * configured and reverse ring considered as slave.
	 */
	if (ringacc->dual_ring && (ring->flags & K3_NAV_RING_FLAG_REVERSE))
		return 0;

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
	if (ret) {
		dev_err(ringacc->dev, "TISCI config ring fail (%d) ring_idx %d\n",
			ret, ring_idx);
		return ret;
	}

	/*
	 * Above TI SCI call handles firewall configuration, cfg
	 * register configuration still has to be done locally in
	 * absence of RM services.
	 */
	if (IS_ENABLED(CONFIG_K3_DM_FW))
		k3_nav_ringacc_ring_cfg_raw(ring);

	return 0;
}

static int k3_dmaring_ring_cfg(struct k3_nav_ring *ring, struct k3_nav_ring_cfg *cfg)
{
	struct k3_nav_ringacc *ringacc;
	struct k3_nav_ring *reverse_ring;
	int ret = 0;

	if (cfg->elm_size != K3_NAV_RINGACC_RING_ELSIZE_8 ||
	    cfg->mode != K3_NAV_RINGACC_RING_MODE_RING ||
	    cfg->size & ~K3_DMARING_RING_CFG_RING_SIZE_ELCNT_MASK)
		return -EINVAL;

	ringacc = ring->parent;

	/*
	 * k3_dmaring: rings shared memory and configuration, only forward ring is
	 * configured and reverse ring considered as slave.
	 */
	if (ringacc->dual_ring && (ring->flags & K3_NAV_RING_FLAG_REVERSE))
		return 0;

	if (!test_bit(ring->ring_id, ringacc->rings_inuse))
		return -EINVAL;

	ring->size = cfg->size;
	ring->elm_size = cfg->elm_size;
	ring->mode = cfg->mode;
	memset(&ring->state, 0, sizeof(ring->state));

	ring->ops = &k3_dmaring_fwd_ring_ops;

	ring->ring_mem_virt =
		dma_alloc_coherent(ring->size * (4 << ring->elm_size),
				   (unsigned long *)&ring->ring_mem_dma);
	if (!ring->ring_mem_virt) {
		dev_err(ringacc->dev, "Failed to alloc ring mem\n");
		ret = -ENOMEM;
		goto err_free_ops;
	}

	ret = k3_nav_ringacc_ring_cfg_sci(ring);
	if (ret)
		goto err_free_mem;

	ring->flags |= KNAV_RING_FLAG_BUSY;

	/* k3_dmaring: configure reverse ring */
	reverse_ring = &ringacc->rings[ring->ring_id + ringacc->num_rings];
	reverse_ring->size = cfg->size;
	reverse_ring->elm_size = cfg->elm_size;
	reverse_ring->mode = cfg->mode;
	memset(&reverse_ring->state, 0, sizeof(reverse_ring->state));
	reverse_ring->ops = &k3_dmaring_reverse_ring_ops;

	reverse_ring->ring_mem_virt = ring->ring_mem_virt;
	reverse_ring->ring_mem_dma = ring->ring_mem_dma;
	reverse_ring->flags |= KNAV_RING_FLAG_BUSY;

	return 0;

err_free_mem:
	dma_free_coherent(ringacc->dev,
			  ring->size * (4 << ring->elm_size),
			  ring->ring_mem_virt,
			  ring->ring_mem_dma);
err_free_ops:
	ring->ops = NULL;
	return ret;
}

int k3_nav_ringacc_ring_cfg(struct k3_nav_ring *ring,
			    struct k3_nav_ring_cfg *cfg)
{
	struct k3_nav_ringacc *ringacc = ring->parent;
	int ret = 0;

	if (!ring || !cfg)
		return -EINVAL;

	if (ringacc->dual_ring)
		return k3_dmaring_ring_cfg(ring, cfg);

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
	memset(&ring->state, 0, sizeof(ring->state));

	switch (ring->mode) {
	case K3_NAV_RINGACC_RING_MODE_RING:
		ring->ops = &k3_nav_mode_ring_ops;
		break;
	default:
		ring->ops = NULL;
		ret = -EINVAL;
		goto err_free_ops;
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

	if (!ring->state.free)
		ring->state.free = ring->size - ringacc_readl(&ring->rt->occ);

	return ring->state.free;
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

static int k3_dmaring_ring_fwd_pop_mem(struct k3_nav_ring *ring, void *elem)
{
	void *elem_ptr;
	u32 elem_idx;

	/*
	 * k3_dmaring: forward ring is always tied DMA channel and HW does not
	 * maintain any state data required for POP operation and its unknown
	 * how much elements were consumed by HW. So, to actually
	 * do POP, the read pointer has to be recalculated every time.
	 */
	ring->state.occ = k3_nav_ringacc_ring_read_occ(ring);
	if (ring->state.windex >= ring->state.occ)
		elem_idx = ring->state.windex - ring->state.occ;
	else
		elem_idx = ring->size - (ring->state.occ - ring->state.windex);

	elem_ptr = k3_nav_ringacc_get_elm_addr(ring, elem_idx);
	invalidate_dcache_range((unsigned long)ring->ring_mem_virt,
				ALIGN((unsigned long)ring->ring_mem_virt +
				      ring->size * (4 << ring->elm_size),
				      ARCH_DMA_MINALIGN));

	memcpy(elem, elem_ptr, (4 << ring->elm_size));

	ring->state.occ--;
	writel(-1, &ring->rt->db);

	dev_dbg(ring->parent->dev, "%s: occ%d Windex%d Rindex%d pos_ptr%px\n",
		__func__, ring->state.occ, ring->state.windex, elem_idx,
		elem_ptr);
	return 0;
}

static int k3_dmaring_ring_reverse_pop_mem(struct k3_nav_ring *ring, void *elem)
{
	void *elem_ptr;

	elem_ptr = k3_nav_ringacc_get_elm_addr(ring, ring->state.rindex);

	if (ring->state.occ) {
		invalidate_dcache_range((unsigned long)ring->ring_mem_virt,
					ALIGN((unsigned long)ring->ring_mem_virt +
					ring->size * (4 << ring->elm_size),
					ARCH_DMA_MINALIGN));

		memcpy(elem, elem_ptr, (4 << ring->elm_size));
		ring->state.rindex = (ring->state.rindex + 1) % ring->size;
		ring->state.occ--;
		writel(-1 & K3_DMARING_RING_RT_DB_ENTRY_MASK, &ring->rt->db);
	}

	dev_dbg(ring->parent->dev, "%s: occ%d index%d pos_ptr%px\n",
		__func__, ring->state.occ, ring->state.rindex, elem_ptr);
	return 0;
}

static int k3_nav_ringacc_ring_push_mem(struct k3_nav_ring *ring, void *elem)
{
	void *elem_ptr;

	elem_ptr = k3_nav_ringacc_get_elm_addr(ring, ring->state.windex);

	memcpy(elem_ptr, elem, (4 << ring->elm_size));

	flush_dcache_range((unsigned long)ring->ring_mem_virt,
			   ALIGN((unsigned long)ring->ring_mem_virt +
				 ring->size * (4 << ring->elm_size),
				 ARCH_DMA_MINALIGN));

	ring->state.windex = (ring->state.windex + 1) % ring->size;
	ring->state.free--;
	ringacc_writel(1, &ring->rt->db);

	pr_debug("ring_push_mem: free%d index%d\n",
		 ring->state.free, ring->state.windex);

	return 0;
}

static int k3_nav_ringacc_ring_pop_mem(struct k3_nav_ring *ring, void *elem)
{
	void *elem_ptr;

	elem_ptr = k3_nav_ringacc_get_elm_addr(ring, ring->state.rindex);

	invalidate_dcache_range((unsigned long)ring->ring_mem_virt,
				ALIGN((unsigned long)ring->ring_mem_virt +
				      ring->size * (4 << ring->elm_size),
				      ARCH_DMA_MINALIGN));

	memcpy(elem, elem_ptr, (4 << ring->elm_size));

	ring->state.rindex = (ring->state.rindex + 1) % ring->size;
	ring->state.occ--;
	ringacc_writel(-1, &ring->rt->db);

	pr_debug("ring_pop_mem: occ%d index%d pos_ptr%p\n",
		 ring->state.occ, ring->state.rindex, elem_ptr);
	return 0;
}

int k3_nav_ringacc_ring_push(struct k3_nav_ring *ring, void *elem)
{
	int ret = -EOPNOTSUPP;

	if (!ring || !(ring->flags & KNAV_RING_FLAG_BUSY))
		return -EINVAL;

	pr_debug("ring_push%d: free%d index%d\n",
		 ring->ring_id, ring->state.free, ring->state.windex);

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
		 ring->state.free, ring->state.windex);

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

	if (!ring->state.occ)
		k3_nav_ringacc_ring_update_occ(ring);

	pr_debug("ring_pop%d: occ%d index%d\n",
		 ring->ring_id, ring->state.occ, ring->state.rindex);

	if (!ring->state.occ && !ring->state.tdown_complete)
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

	if (!ring->state.occ)
		k3_nav_ringacc_ring_update_occ(ring);

	pr_debug("ring_pop_tail: occ%d index%d\n",
		 ring->state.occ, ring->state.rindex);

	if (!ring->state.occ)
		return -ENODATA;

	if (ring->ops && ring->ops->pop_tail)
		ret = ring->ops->pop_tail(ring, elem);

	return ret;
}

static int k3_nav_ringacc_probe_dt(struct k3_nav_ringacc *ringacc)
{
	struct udevice *dev = ringacc->dev;
	struct udevice *devp = dev;
	struct udevice *tisci_dev = NULL;
	int ret;

	ringacc->num_rings = dev_read_u32_default(dev, "ti,num-rings", 0);
	if (!ringacc->num_rings) {
		dev_err(dev, "ti,num-rings read failure %d\n", ret);
		return -EINVAL;
	}

	ringacc->dma_ring_reset_quirk =
			dev_read_bool(dev, "ti,dma-ring-reset-quirk");

	ret = uclass_get_device_by_phandle(UCLASS_FIRMWARE, devp,
					   "ti,sci", &tisci_dev);
	if (ret) {
		pr_debug("TISCI RA RM get failed (%d)\n", ret);
		ringacc->tisci = NULL;
		return -ENODEV;
	}
	ringacc->tisci = (struct ti_sci_handle *)
			 (ti_sci_get_handle_from_sysfw(tisci_dev));

	ret = dev_read_u32_default(devp, "ti,sci", 0);
	if (!ret) {
		dev_err(dev, "TISCI RA RM disabled\n");
		ringacc->tisci = NULL;
		return ret;
	}

	ret = dev_read_u32(devp, "ti,sci-dev-id", &ringacc->tisci_dev_id);
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

static int k3_nav_ringacc_init(struct udevice *dev, struct k3_nav_ringacc *ringacc)
{
	void __iomem *base_cfg, *base_rt;
	int ret, i;

	ret = k3_nav_ringacc_probe_dt(ringacc);
	if (ret)
		return ret;

	base_cfg = dev_remap_addr_name(dev, "cfg");
	pr_debug("cfg %p\n", base_cfg);
	if (!base_cfg)
		return -EINVAL;

	base_rt = dev_read_addr_name_ptr(dev, "rt");
	pr_debug("rt %p\n", base_rt);
	if (!base_rt)
		return -EINVAL;

	ringacc->rings = devm_kzalloc(dev,
				      sizeof(*ringacc->rings) *
				      ringacc->num_rings,
				      GFP_KERNEL);
	ringacc->rings_inuse = devm_kcalloc(dev,
					    BITS_TO_LONGS(ringacc->num_rings),
					    sizeof(unsigned long), GFP_KERNEL);

	if (!ringacc->rings || !ringacc->rings_inuse)
		return -ENOMEM;

	for (i = 0; i < ringacc->num_rings; i++) {
		ringacc->rings[i].cfg = base_cfg +
					KNAV_RINGACC_CFG_REGS_STEP * i;
		ringacc->rings[i].rt = base_rt +
				       KNAV_RINGACC_RT_REGS_STEP * i;
		ringacc->rings[i].parent = ringacc;
		ringacc->rings[i].ring_id = i;
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
	return 0;
}

struct k3_nav_ringacc *k3_ringacc_dmarings_init(struct udevice *dev,
						struct k3_ringacc_init_data *data)
{
	struct k3_nav_ringacc *ringacc;
	void __iomem *base_rt;
	int i;

	ringacc = devm_kzalloc(dev, sizeof(*ringacc), GFP_KERNEL);
	if (!ringacc)
		return ERR_PTR(-ENOMEM);

	ringacc->dual_ring = true;

	ringacc->dev = dev;
	ringacc->num_rings = data->num_rings;
	ringacc->tisci = data->tisci;
	ringacc->tisci_dev_id = data->tisci_dev_id;

	base_rt = dev_read_addr_name_ptr(dev, "ringrt");
	if (!base_rt)
		return ERR_PTR(-EINVAL);

	ringacc->rings = devm_kzalloc(dev,
				      sizeof(*ringacc->rings) *
				      ringacc->num_rings * 2,
				      GFP_KERNEL);
	ringacc->rings_inuse = devm_kcalloc(dev,
					    BITS_TO_LONGS(ringacc->num_rings),
					    sizeof(unsigned long), GFP_KERNEL);

	if (!ringacc->rings || !ringacc->rings_inuse)
		return ERR_PTR(-ENOMEM);

	for (i = 0; i < ringacc->num_rings; i++) {
		struct k3_nav_ring *ring = &ringacc->rings[i];

		ring->rt = base_rt + K3_DMARING_RING_RT_REGS_STEP * i;
		ring->parent = ringacc;
		ring->ring_id = i;

		ring = &ringacc->rings[ringacc->num_rings + i];
		ring->rt = base_rt + K3_DMARING_RING_RT_REGS_STEP * i +
			   K3_DMARING_RING_RT_REGS_REVERSE_OFS;
		ring->parent = ringacc;
		ring->ring_id = i;
		ring->flags = K3_NAV_RING_FLAG_REVERSE;
	}

	ringacc->tisci_ring_ops = &ringacc->tisci->ops.rm_ring_ops;

	dev_info(dev, "k3_dmaring Ring probed rings:%u, sci-dev-id:%u\n",
		 ringacc->num_rings,
		 ringacc->tisci_dev_id);
	dev_info(dev, "dma-ring-reset-quirk: %s\n",
		 ringacc->dma_ring_reset_quirk ? "enabled" : "disabled");

	return ringacc;
}

struct ringacc_match_data {
	struct k3_nav_ringacc_ops ops;
};

static struct ringacc_match_data k3_nav_ringacc_data = {
	.ops = {
		.init = k3_nav_ringacc_init,
	},
};

static const struct udevice_id knav_ringacc_ids[] = {
	{ .compatible = "ti,am654-navss-ringacc", .data = (ulong)&k3_nav_ringacc_data, },
	{},
};

static int k3_nav_ringacc_probe(struct udevice *dev)
{
	struct k3_nav_ringacc *ringacc;
	int ret;
	const struct ringacc_match_data *match_data;

	match_data = (struct ringacc_match_data *)dev_get_driver_data(dev);

	ringacc = dev_get_priv(dev);
	if (!ringacc)
		return -ENOMEM;

	ringacc->dev = dev;
	ringacc->ops = &match_data->ops;
	ret = ringacc->ops->init(dev, ringacc);
	if (ret)
		return ret;

	return 0;
}

U_BOOT_DRIVER(k3_navss_ringacc) = {
	.name	= "k3-navss-ringacc",
	.id	= UCLASS_MISC,
	.of_match = knav_ringacc_ids,
	.probe = k3_nav_ringacc_probe,
	.priv_auto	= sizeof(struct k3_nav_ringacc),
};
