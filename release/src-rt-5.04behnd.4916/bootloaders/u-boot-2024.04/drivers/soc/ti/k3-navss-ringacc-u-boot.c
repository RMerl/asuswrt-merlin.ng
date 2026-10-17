// SPDX-License-Identifier: GPL-2.0+
/*
 * U-Boot specific helpers for TI K3 AM65x NAVSS Ring accelerator
 * Manager (RA) subsystem driver
 *
 * Copyright (C) 2021 Texas Instruments Incorporated - https://www.ti.com
 */

struct k3_nav_ring_cfg_regs {
	u32	resv_64[16];
	u32	ba_lo;		/* Ring Base Address Lo Register */
	u32	ba_hi;		/* Ring Base Address Hi Register */
	u32	size;		/* Ring Size Register */
	u32	event;		/* Ring Event Register */
	u32	orderid;	/* Ring OrderID Register */
};

#define KNAV_RINGACC_CFG_REGS_STEP	0x100

#define KNAV_RINGACC_CFG_RING_BA_HI_ADDR_HI_MASK	GENMASK(15, 0)

#define KNAV_RINGACC_CFG_RING_SIZE_QMODE_MASK		GENMASK(31, 30)
#define KNAV_RINGACC_CFG_RING_SIZE_QMODE_SHIFT		(30)

#define KNAV_RINGACC_CFG_RING_SIZE_ELSIZE_MASK		GENMASK(26, 24)
#define KNAV_RINGACC_CFG_RING_SIZE_ELSIZE_SHIFT		(24)

static void k3_ringacc_ring_reset_raw(struct k3_nav_ring *ring)
{
	writel(0, &ring->cfg->size);
}

static void k3_ringacc_ring_reconfig_qmode_raw(struct k3_nav_ring *ring, enum k3_nav_ring_mode mode)
{
	u32 val;

	val = readl(&ring->cfg->size);
	val &= KNAV_RINGACC_CFG_RING_SIZE_QMODE_MASK;
	val |= mode << KNAV_RINGACC_CFG_RING_SIZE_QMODE_SHIFT;
	writel(val, &ring->cfg->size);
}

static void k3_ringacc_ring_free_raw(struct k3_nav_ring *ring)
{
	writel(0, &ring->cfg->ba_hi);
	writel(0, &ring->cfg->ba_lo);
	writel(0, &ring->cfg->size);
}

static void k3_nav_ringacc_ring_cfg_raw(struct k3_nav_ring *ring)
{
	u32 val;

	writel(lower_32_bits(ring->ring_mem_dma), &ring->cfg->ba_lo);
	writel(upper_32_bits(ring->ring_mem_dma), &ring->cfg->ba_hi);

	val = ring->mode << KNAV_RINGACC_CFG_RING_SIZE_QMODE_SHIFT |
	      ring->elm_size << KNAV_RINGACC_CFG_RING_SIZE_ELSIZE_SHIFT |
	      ring->size;
	writel(val, &ring->cfg->size);
}
