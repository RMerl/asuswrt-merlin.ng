// SPDX-License-Identifier: GPL-2.0+
// (C) 2021 Pali Rohár <pali@kernel.org>

#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <asm/io.h>

#define MVEBU_SOC_CONTROL_1_REG 0x4

#define MVEBU_PCIE_ID 0

struct mvebu_reset_data {
	void *base;
};

static int mvebu_reset_of_xlate(struct reset_ctl *rst,
				struct ofnode_phandle_args *args)
{
	if (args->args_count < 2)
		return -EINVAL;

	rst->id = args->args[0];
	rst->data = args->args[1];

	/* Currently only PCIe is implemented */
	if (rst->id != MVEBU_PCIE_ID)
		return -EINVAL;

	/* Four PCIe enable bits are shared across more PCIe links */
	if (!(rst->data >= 0 && rst->data <= 3))
		return -EINVAL;

	return 0;
}

static int mvebu_reset_request(struct reset_ctl *rst)
{
	return 0;
}

static int mvebu_reset_free(struct reset_ctl *rst)
{
	return 0;
}

static int mvebu_reset_assert(struct reset_ctl *rst)
{
	struct mvebu_reset_data *data = dev_get_priv(rst->dev);

	clrbits_32(data->base + MVEBU_SOC_CONTROL_1_REG, BIT(rst->data));
	return 0;
}

static int mvebu_reset_deassert(struct reset_ctl *rst)
{
	struct mvebu_reset_data *data = dev_get_priv(rst->dev);

	setbits_32(data->base + MVEBU_SOC_CONTROL_1_REG, BIT(rst->data));
	return 0;
}

static int mvebu_reset_status(struct reset_ctl *rst)
{
	struct mvebu_reset_data *data = dev_get_priv(rst->dev);

	return !(readl(data->base + MVEBU_SOC_CONTROL_1_REG) & BIT(rst->data));
}

static int mvebu_reset_of_to_plat(struct udevice *dev)
{
	struct mvebu_reset_data *data = dev_get_priv(dev);

	data->base = dev_read_addr_ptr(dev);
	if (!data->base)
		return -EINVAL;

	return 0;
}

static const struct udevice_id mvebu_reset_of_match[] = {
	{ .compatible = "marvell,armada-370-xp-system-controller" },
	{ .compatible = "marvell,armada-375-system-controller" },
	{ .compatible = "marvell,armada-380-system-controller" },
	{ .compatible = "marvell,armada-390-system-controller" },
	{ },
};

static const struct reset_ops mvebu_reset_ops = {
	.of_xlate = mvebu_reset_of_xlate,
	.request = mvebu_reset_request,
	.rfree = mvebu_reset_free,
	.rst_assert = mvebu_reset_assert,
	.rst_deassert = mvebu_reset_deassert,
	.rst_status = mvebu_reset_status,
};

U_BOOT_DRIVER(mvebu_reset) = {
	.name = "mvebu-reset",
	.id = UCLASS_RESET,
	.of_match = mvebu_reset_of_match,
	.of_to_plat = mvebu_reset_of_to_plat,
	.priv_auto = sizeof(struct mvebu_reset_data),
	.ops = &mvebu_reset_ops,
};
