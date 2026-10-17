// SPDX-License-Identifier: GPL-2.0
/*
 * MMIO register bitfield-controlled multiplexer driver
 * Based on the linux mmio multiplexer driver
 *
 * Copyright (C) 2017 Pengutronix, Philipp Zabel <kernel@pengutronix.de>
 * Copyright (C) 2019 Texas Instrument, Jean-jacques Hiblot <jjhiblot@ti.com>
 */
#include <common.h>
#include <dm.h>
#include <mux-internal.h>
#include <regmap.h>
#include <syscon.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/read.h>
#include <dm/devres.h>
#include <dt-bindings/mux/mux.h>
#include <linux/bitops.h>

static int mux_mmio_set(struct mux_control *mux, int state)
{
	struct regmap_field **fields = dev_get_priv(mux->dev);

	return regmap_field_write(fields[mux_control_get_index(mux)], state);
}

static const struct mux_control_ops mux_mmio_ops = {
	.set = mux_mmio_set,
};

static const struct udevice_id mmio_mux_of_match[] = {
	{ .compatible = "mmio-mux" },
	{ /* sentinel */ },
};

static int mmio_mux_probe(struct udevice *dev)
{
	struct regmap_field **fields;
	struct mux_chip *mux_chip = dev_get_uclass_priv(dev);
	struct regmap *regmap;
	u32 *mux_reg_masks;
	u32 *idle_states;
	int num_fields;
	int ret;
	int i;

	regmap = syscon_node_to_regmap(dev_ofnode(dev->parent));
	if (IS_ERR(regmap)) {
		ret = PTR_ERR(regmap);
		dev_err(dev, "failed to get regmap: %d\n", ret);
		return ret;
	}

	num_fields = dev_read_size(dev, "mux-reg-masks");
	if (num_fields < 0)
		return log_msg_ret("mux-reg-masks missing", -EINVAL);

	num_fields /= sizeof(u32);
	if (num_fields == 0 || num_fields % 2)
		ret = -EINVAL;
	num_fields = num_fields / 2;

	ret = mux_alloc_controllers(dev, num_fields);
	if (ret < 0)
		return log_msg_ret("mux_alloc_controllers", ret);

	fields = devm_kmalloc(dev, num_fields * sizeof(*fields), __GFP_ZERO);
	if (!fields)
		return -ENOMEM;
	dev_set_priv(dev, fields);

	mux_reg_masks = devm_kmalloc(dev, num_fields * 2 * sizeof(u32),
				     __GFP_ZERO);
	if (!mux_reg_masks)
		return -ENOMEM;

	ret = dev_read_u32_array(dev, "mux-reg-masks", mux_reg_masks,
				 num_fields * 2);
	if (ret < 0)
		return log_msg_ret("mux-reg-masks read", ret);

	idle_states = devm_kmalloc(dev, num_fields * sizeof(u32), __GFP_ZERO);
	if (!idle_states)
		return -ENOMEM;

	ret = dev_read_u32_array(dev, "idle-states", idle_states, num_fields);
	if (ret < 0) {
		log_err("idle-states");
		devm_kfree(dev, idle_states);
		idle_states = NULL;
	}

	for (i = 0; i < num_fields; i++) {
		struct mux_control *mux = &mux_chip->mux[i];
		struct reg_field field;
		u32 reg, mask;
		int bits;

		reg = mux_reg_masks[2 * i];
		mask = mux_reg_masks[2 * i + 1];

		field.reg = reg;
		field.msb = fls(mask) - 1;
		field.lsb = ffs(mask) - 1;

		if (mask != GENMASK(field.msb, field.lsb))
			return log_msg_ret("invalid mask", -EINVAL);

		fields[i] = devm_regmap_field_alloc(dev, regmap, field);
		if (IS_ERR(fields[i])) {
			ret = PTR_ERR(fields[i]);
			return log_msg_ret("regmap_field_alloc", ret);
		}

		bits = 1 + field.msb - field.lsb;
		mux->states = 1 << bits;

		if (!idle_states)
			continue;

		if (idle_states[i] != MUX_IDLE_AS_IS &&
		    idle_states[i] >= mux->states)
			return log_msg_ret("idle-states range", -EINVAL);

		mux->idle_state = idle_states[i];
	}

	devm_kfree(dev, mux_reg_masks);
	if (idle_states)
		devm_kfree(dev, idle_states);

	return 0;
}

U_BOOT_DRIVER(mmio_mux) = {
	.name = "mmio-mux",
	.id = UCLASS_MUX,
	.of_match = mmio_mux_of_match,
	.probe = mmio_mux_probe,
	.ops = &mux_mmio_ops,
};
