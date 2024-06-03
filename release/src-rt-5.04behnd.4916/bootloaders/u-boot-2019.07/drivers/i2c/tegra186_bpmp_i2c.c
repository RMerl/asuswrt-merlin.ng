// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <misc.h>
#include <asm/arch-tegra/bpmp_abi.h>

DECLARE_GLOBAL_DATA_PTR;

struct tegra186_bpmp_i2c {
	uint32_t bpmp_bus_id;
};

static inline void serialize_u16(uint8_t **p, uint16_t val)
{
	(*p)[0] = val & 0xff;
	(*p)[1] = val >> 8;
	(*p) += 2;
}

/* These just happen to have the same values as I2C_M_* and SERIALI2C_* */
#define SUPPORTED_FLAGS \
	(I2C_M_TEN | \
	I2C_M_RD | \
	I2C_M_STOP | \
	I2C_M_NOSTART | \
	I2C_M_REV_DIR_ADDR | \
	I2C_M_IGNORE_NAK | \
	I2C_M_NO_RD_ACK | \
	I2C_M_RECV_LEN)

static int tegra186_bpmp_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
				  int nmsgs)
{
	struct tegra186_bpmp_i2c *priv = dev_get_priv(dev);
	struct mrq_i2c_request req;
	struct mrq_i2c_response resp;
	uint8_t *p;
	int left, i, ret;

	req.cmd = CMD_I2C_XFER;
	req.xfer.bus_id = priv->bpmp_bus_id;
	p = &req.xfer.data_buf[0];
	left = ARRAY_SIZE(req.xfer.data_buf);
	for (i = 0; i < nmsgs; i++) {
		int len = 6;
		if (!(msg[i].flags & I2C_M_RD))
			len += msg[i].len;
		if ((len >= BIT(16)) || (len > left))
			return -ENOSPC;

		if (msg[i].flags & ~SUPPORTED_FLAGS)
			return -EINVAL;

		serialize_u16(&p, msg[i].addr);
		serialize_u16(&p, msg[i].flags);
		serialize_u16(&p, msg[i].len);
		if (!(msg[i].flags & I2C_M_RD)) {
			memcpy(p, msg[i].buf, msg[i].len);
			p += msg[i].len;
		}
	}
	req.xfer.data_size = p - &req.xfer.data_buf[0];

	ret = misc_call(dev->parent, MRQ_I2C, &req, sizeof(req), &resp,
			sizeof(resp));
	if (ret < 0)
		return ret;

	p = &resp.xfer.data_buf[0];
	left = resp.xfer.data_size;
	if (left > ARRAY_SIZE(resp.xfer.data_buf))
		return -EINVAL;
	for (i = 0; i < nmsgs; i++) {
		if (msg[i].flags & I2C_M_RD) {
			memcpy(msg[i].buf, p, msg[i].len);
			p += msg[i].len;
		}
	}

	return 0;
}

static int tegra186_bpmp_probe_chip(struct udevice *bus, uint chip_addr,
				    uint chip_flags)
{
	return 0;
}

static int tegra186_bpmp_i2c_probe(struct udevice *dev)
{
	struct tegra186_bpmp_i2c *priv = dev_get_priv(dev);

	priv->bpmp_bus_id = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
					    "nvidia,bpmp-bus-id", U32_MAX);
	if (priv->bpmp_bus_id == U32_MAX) {
		debug("%s: could not parse nvidia,bpmp-bus-id\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static const struct dm_i2c_ops tegra186_bpmp_i2c_ops = {
	.xfer = tegra186_bpmp_i2c_xfer,
	.probe_chip = tegra186_bpmp_probe_chip,
};

static const struct udevice_id tegra186_bpmp_i2c_ids[] = {
	{ .compatible = "nvidia,tegra186-bpmp-i2c" },
	{ }
};

U_BOOT_DRIVER(i2c_gpio) = {
	.name	= "tegra186_bpmp_i2c",
	.id	= UCLASS_I2C,
	.of_match = tegra186_bpmp_i2c_ids,
	.probe	= tegra186_bpmp_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct tegra186_bpmp_i2c),
	.ops	= &tegra186_bpmp_i2c_ops,
};
