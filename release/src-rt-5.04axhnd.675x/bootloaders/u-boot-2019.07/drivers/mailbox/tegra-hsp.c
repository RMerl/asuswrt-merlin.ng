// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <dt-bindings/mailbox/tegra186-hsp.h>

#define TEGRA_HSP_INT_DIMENSIONING		0x380
#define TEGRA_HSP_INT_DIMENSIONING_NSI_SHIFT	16
#define TEGRA_HSP_INT_DIMENSIONING_NSI_MASK	0xf
#define TEGRA_HSP_INT_DIMENSIONING_NDB_SHIFT	12
#define TEGRA_HSP_INT_DIMENSIONING_NDB_MASK	0xf
#define TEGRA_HSP_INT_DIMENSIONING_NAS_SHIFT	8
#define TEGRA_HSP_INT_DIMENSIONING_NAS_MASK	0xf
#define TEGRA_HSP_INT_DIMENSIONING_NSS_SHIFT	4
#define TEGRA_HSP_INT_DIMENSIONING_NSS_MASK	0xf
#define TEGRA_HSP_INT_DIMENSIONING_NSM_SHIFT	0
#define TEGRA_HSP_INT_DIMENSIONING_NSM_MASK	0xf

#define TEGRA_HSP_DB_REG_TRIGGER	0x0
#define TEGRA_HSP_DB_REG_ENABLE		0x4
#define TEGRA_HSP_DB_REG_RAW		0x8
#define TEGRA_HSP_DB_REG_PENDING	0xc

#define TEGRA_HSP_DB_ID_CCPLEX		1
#define TEGRA_HSP_DB_ID_BPMP		3
#define TEGRA_HSP_DB_ID_NUM		7

struct tegra_hsp {
	fdt_addr_t regs;
	uint32_t db_base;
};

static uint32_t *tegra_hsp_reg(struct tegra_hsp *thsp, uint32_t db_id,
			       uint32_t reg)
{
	return (uint32_t *)(thsp->regs + thsp->db_base + (db_id * 0x100) + reg);
}

static uint32_t tegra_hsp_readl(struct tegra_hsp *thsp, uint32_t db_id,
				uint32_t reg)
{
	uint32_t *r = tegra_hsp_reg(thsp, db_id, reg);
	return readl(r);
}

static void tegra_hsp_writel(struct tegra_hsp *thsp, uint32_t val,
			     uint32_t db_id, uint32_t reg)
{
	uint32_t *r = tegra_hsp_reg(thsp, db_id, reg);

	writel(val, r);
	readl(r);
}

static int tegra_hsp_db_id(ulong chan_id)
{
	switch (chan_id) {
	case (HSP_MBOX_TYPE_DB << 16) | HSP_DB_MASTER_BPMP:
		return TEGRA_HSP_DB_ID_BPMP;
	default:
		debug("Invalid channel ID\n");
		return -EINVAL;
	}
}

static int tegra_hsp_of_xlate(struct mbox_chan *chan,
			      struct ofnode_phandle_args *args)
{
	debug("%s(chan=%p)\n", __func__, chan);

	if (args->args_count != 2) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	chan->id = (args->args[0] << 16) | args->args[1];

	return 0;
}

static int tegra_hsp_request(struct mbox_chan *chan)
{
	int db_id;

	debug("%s(chan=%p)\n", __func__, chan);

	db_id = tegra_hsp_db_id(chan->id);
	if (db_id < 0) {
		debug("tegra_hsp_db_id() failed: %d\n", db_id);
		return -EINVAL;
	}

	return 0;
}

static int tegra_hsp_free(struct mbox_chan *chan)
{
	debug("%s(chan=%p)\n", __func__, chan);

	return 0;
}

static int tegra_hsp_send(struct mbox_chan *chan, const void *data)
{
	struct tegra_hsp *thsp = dev_get_priv(chan->dev);
	int db_id;

	debug("%s(chan=%p, data=%p)\n", __func__, chan, data);

	db_id = tegra_hsp_db_id(chan->id);
	tegra_hsp_writel(thsp, 1, db_id, TEGRA_HSP_DB_REG_TRIGGER);

	return 0;
}

static int tegra_hsp_recv(struct mbox_chan *chan, void *data)
{
	struct tegra_hsp *thsp = dev_get_priv(chan->dev);
	uint32_t db_id = TEGRA_HSP_DB_ID_CCPLEX;
	uint32_t val;

	debug("%s(chan=%p, data=%p)\n", __func__, chan, data);

	val = tegra_hsp_readl(thsp, db_id, TEGRA_HSP_DB_REG_RAW);
	if (!(val & BIT(chan->id)))
		return -ENODATA;

	tegra_hsp_writel(thsp, BIT(chan->id), db_id, TEGRA_HSP_DB_REG_RAW);

	return 0;
}

static int tegra_hsp_bind(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static int tegra_hsp_probe(struct udevice *dev)
{
	struct tegra_hsp *thsp = dev_get_priv(dev);
	u32 val;
	int nr_sm, nr_ss, nr_as;

	debug("%s(dev=%p)\n", __func__, dev);

	thsp->regs = devfdt_get_addr(dev);
	if (thsp->regs == FDT_ADDR_T_NONE)
		return -ENODEV;

	val = readl(thsp->regs + TEGRA_HSP_INT_DIMENSIONING);
	nr_sm = (val >> TEGRA_HSP_INT_DIMENSIONING_NSM_SHIFT) &
		TEGRA_HSP_INT_DIMENSIONING_NSM_MASK;
	nr_ss = (val >> TEGRA_HSP_INT_DIMENSIONING_NSS_SHIFT) &
		TEGRA_HSP_INT_DIMENSIONING_NSS_MASK;
	nr_as = (val >> TEGRA_HSP_INT_DIMENSIONING_NAS_SHIFT) &
		TEGRA_HSP_INT_DIMENSIONING_NAS_MASK;

	thsp->db_base = (1 + (nr_sm >> 1) + nr_ss + nr_as) << 16;

	return 0;
}

static const struct udevice_id tegra_hsp_ids[] = {
	{ .compatible = "nvidia,tegra186-hsp" },
	{ }
};

struct mbox_ops tegra_hsp_mbox_ops = {
	.of_xlate = tegra_hsp_of_xlate,
	.request = tegra_hsp_request,
	.free = tegra_hsp_free,
	.send = tegra_hsp_send,
	.recv = tegra_hsp_recv,
};

U_BOOT_DRIVER(tegra_hsp) = {
	.name = "tegra-hsp",
	.id = UCLASS_MAILBOX,
	.of_match = tegra_hsp_ids,
	.bind = tegra_hsp_bind,
	.probe = tegra_hsp_probe,
	.priv_auto_alloc_size = sizeof(struct tegra_hsp),
	.ops = &tegra_hsp_mbox_ops,
};
