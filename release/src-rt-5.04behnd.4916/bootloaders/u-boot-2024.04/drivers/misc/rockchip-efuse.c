// SPDX-License-Identifier: GPL-2.0+
/*
 * eFuse driver for Rockchip devices
 *
 * Copyright 2017, Theobroma Systems Design und Consulting GmbH
 * Written by Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <display_options.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <malloc.h>
#include <misc.h>

#define EFUSE_CTRL		0x0000
#define RK3036_A_SHIFT		8
#define RK3036_A_MASK		GENMASK(15, 8)
#define RK3036_ADDR(n)		((n) << RK3036_A_SHIFT)
#define RK3128_A_SHIFT		7
#define RK3128_A_MASK		GENMASK(15, 7)
#define RK3128_ADDR(n)		((n) << RK3128_A_SHIFT)
#define RK3288_A_SHIFT		6
#define RK3288_A_MASK		GENMASK(15, 6)
#define RK3288_ADDR(n)		((n) << RK3288_A_SHIFT)
#define RK3399_A_SHIFT		16
#define RK3399_A_MASK		GENMASK(25, 16)
#define RK3399_ADDR(n)		((n) << RK3399_A_SHIFT)
#define RK3399_STROBSFTSEL	BIT(9)
#define RK3399_RSB		BIT(7)
#define RK3399_PD		BIT(5)
#define EFUSE_PGENB		BIT(3)
#define EFUSE_LOAD		BIT(2)
#define EFUSE_STROBE		BIT(1)
#define EFUSE_CSB		BIT(0)
#define EFUSE_DOUT		0x0004
#define RK3328_INT_STATUS	0x0018
#define RK3328_INT_FINISH	BIT(0)
#define RK3328_DOUT		0x0020
#define RK3328_AUTO_CTRL	0x0024
#define RK3328_AUTO_RD		BIT(1)
#define RK3328_AUTO_ENB		BIT(0)

struct rockchip_efuse_plat {
	void __iomem *base;
};

struct rockchip_efuse_data {
	int (*read)(struct udevice *dev, int offset, void *buf, int size);
	int offset;
	int size;
	int block_size;
};

#if defined(DEBUG)
static int dump_efuse(struct cmd_tbl *cmdtp, int flag,
		      int argc, char *const argv[])
{
	struct udevice *dev;
	u8 data[4];
	int ret, i;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_efuse), &dev);
	if (ret) {
		printf("%s: no misc-device found\n", __func__);
		return 0;
	}

	for (i = 0; true; i += sizeof(data)) {
		ret = misc_read(dev, i, &data, sizeof(data));
		if (ret <= 0)
			return 0;

		print_buffer(i, data, 1, sizeof(data), sizeof(data));
	}

	return 0;
}

U_BOOT_CMD(
	dump_efuse, 1, 1, dump_efuse,
	"Dump the content of the efuse",
	""
);
#endif

static int rockchip_rk3036_efuse_read(struct udevice *dev, int offset,
				      void *buf, int size)
{
	struct rockchip_efuse_plat *efuse = dev_get_plat(dev);
	u8 *buffer = buf;

	/* Switch to read mode */
	writel(EFUSE_LOAD, efuse->base + EFUSE_CTRL);
	udelay(2);

	while (size--) {
		clrsetbits_le32(efuse->base + EFUSE_CTRL, RK3036_A_MASK,
				RK3036_ADDR(offset++));
		udelay(2);
		setbits_le32(efuse->base + EFUSE_CTRL, EFUSE_STROBE);
		udelay(2);
		*buffer++ = (u8)(readl(efuse->base + EFUSE_DOUT) & 0xFF);
		clrbits_le32(efuse->base + EFUSE_CTRL, EFUSE_STROBE);
		udelay(2);
	}

	/* Switch to inactive mode */
	writel(0x0, efuse->base + EFUSE_CTRL);

	return 0;
}

static int rockchip_rk3128_efuse_read(struct udevice *dev, int offset,
				      void *buf, int size)
{
	struct rockchip_efuse_plat *efuse = dev_get_plat(dev);
	u8 *buffer = buf;

	/* Switch to read mode */
	writel(EFUSE_LOAD, efuse->base + EFUSE_CTRL);
	udelay(2);

	while (size--) {
		clrsetbits_le32(efuse->base + EFUSE_CTRL, RK3128_A_MASK,
				RK3128_ADDR(offset++));
		udelay(2);
		setbits_le32(efuse->base + EFUSE_CTRL, EFUSE_STROBE);
		udelay(2);
		*buffer++ = (u8)(readl(efuse->base + EFUSE_DOUT) & 0xFF);
		clrbits_le32(efuse->base + EFUSE_CTRL, EFUSE_STROBE);
		udelay(2);
	}

	/* Switch to inactive mode */
	writel(0x0, efuse->base + EFUSE_CTRL);

	return 0;
}

static int rockchip_rk3288_efuse_read(struct udevice *dev, int offset,
				      void *buf, int size)
{
	struct rockchip_efuse_plat *efuse = dev_get_plat(dev);
	u8 *buffer = buf;

	/* Switch to read mode */
	writel(EFUSE_CSB, efuse->base + EFUSE_CTRL);
	writel(EFUSE_LOAD | EFUSE_PGENB, efuse->base + EFUSE_CTRL);
	udelay(2);

	while (size--) {
		clrsetbits_le32(efuse->base + EFUSE_CTRL, RK3288_A_MASK,
				RK3288_ADDR(offset++));
		udelay(2);
		setbits_le32(efuse->base + EFUSE_CTRL, EFUSE_STROBE);
		udelay(2);
		*buffer++ = (u8)(readl(efuse->base + EFUSE_DOUT) & 0xFF);
		clrbits_le32(efuse->base + EFUSE_CTRL, EFUSE_STROBE);
		udelay(2);
	}

	/* Switch to standby mode */
	writel(EFUSE_CSB | EFUSE_PGENB, efuse->base + EFUSE_CTRL);

	return 0;
}

static int rockchip_rk3328_efuse_read(struct udevice *dev, int offset,
				      void *buf, int size)
{
	struct rockchip_efuse_plat *efuse = dev_get_plat(dev);
	u32 status, *buffer = buf;
	int ret;

	while (size--) {
		writel(RK3328_AUTO_RD | RK3328_AUTO_ENB | RK3399_ADDR(offset++),
		       efuse->base + RK3328_AUTO_CTRL);
		udelay(1);

		ret = readl_poll_sleep_timeout(efuse->base + RK3328_INT_STATUS,
			status, (status & RK3328_INT_FINISH), 1, 50);
		if (ret)
			return ret;

		*buffer++ = readl(efuse->base + RK3328_DOUT);
		writel(RK3328_INT_FINISH, efuse->base + RK3328_INT_STATUS);
	}

	return 0;
}

static int rockchip_rk3399_efuse_read(struct udevice *dev, int offset,
				      void *buf, int size)
{
	struct rockchip_efuse_plat *efuse = dev_get_plat(dev);
	u32 *buffer = buf;

	/* Switch to array read mode */
	writel(EFUSE_LOAD | EFUSE_PGENB | RK3399_STROBSFTSEL | RK3399_RSB,
	       efuse->base + EFUSE_CTRL);
	udelay(1);

	while (size--) {
		setbits_le32(efuse->base + EFUSE_CTRL,
			     EFUSE_STROBE | RK3399_ADDR(offset++));
		udelay(1);
		*buffer++ = readl(efuse->base + EFUSE_DOUT);
		clrbits_le32(efuse->base + EFUSE_CTRL, EFUSE_STROBE);
		udelay(1);
	}

	/* Switch to power-down mode */
	writel(RK3399_PD | EFUSE_CSB, efuse->base + EFUSE_CTRL);

	return 0;
}

static int rockchip_efuse_read(struct udevice *dev, int offset,
			       void *buf, int size)
{
	const struct rockchip_efuse_data *data =
		(void *)dev_get_driver_data(dev);
	u32 block_start, block_end, block_offset, blocks;
	u8 *buffer;
	int ret;

	if (offset < 0 || !buf || size <= 0 || offset + size > data->size)
		return -EINVAL;

	if (!data->read)
		return -ENOSYS;

	offset += data->offset;

	if (data->block_size <= 1) {
		ret = data->read(dev, offset, buf, size);
		goto done;
	}

	block_start = offset / data->block_size;
	block_offset = offset % data->block_size;
	block_end = DIV_ROUND_UP(offset + size, data->block_size);
	blocks = block_end - block_start;

	buffer = calloc(blocks, data->block_size);
	if (!buffer)
		return -ENOMEM;

	ret = data->read(dev, block_start, buffer, blocks);
	if (!ret)
		memcpy(buf, buffer + block_offset, size);

	free(buffer);

done:
	return ret < 0 ? ret : size;
}

static const struct misc_ops rockchip_efuse_ops = {
	.read = rockchip_efuse_read,
};

static int rockchip_efuse_of_to_plat(struct udevice *dev)
{
	struct rockchip_efuse_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);

	return 0;
}

static const struct rockchip_efuse_data rk3036_data = {
	.read = rockchip_rk3036_efuse_read,
	.size = 0x20,
};

static const struct rockchip_efuse_data rk3128_data = {
	.read = rockchip_rk3128_efuse_read,
	.size = 0x40,
};

static const struct rockchip_efuse_data rk3288_data = {
	.read = rockchip_rk3288_efuse_read,
	.size = 0x20,
};

static const struct rockchip_efuse_data rk3328_data = {
	.read = rockchip_rk3328_efuse_read,
	.offset = 0x60,
	.size = 0x20,
	.block_size = 4,
};

static const struct rockchip_efuse_data rk3399_data = {
	.read = rockchip_rk3399_efuse_read,
	.size = 0x80,
	.block_size = 4,
};

static const struct udevice_id rockchip_efuse_ids[] = {
	{
		.compatible = "rockchip,rk3036-efuse",
		.data = (ulong)&rk3036_data,
	},
	{
		.compatible = "rockchip,rk3066a-efuse",
		.data = (ulong)&rk3288_data,
	},
	{
		.compatible = "rockchip,rk3128-efuse",
		.data = (ulong)&rk3128_data,
	},
	{
		.compatible = "rockchip,rk3188-efuse",
		.data = (ulong)&rk3288_data,
	},
	{
		.compatible = "rockchip,rk3228-efuse",
		.data = (ulong)&rk3288_data,
	},
	{
		.compatible = "rockchip,rk3288-efuse",
		.data = (ulong)&rk3288_data,
	},
	{
		.compatible = "rockchip,rk3328-efuse",
		.data = (ulong)&rk3328_data,
	},
	{
		.compatible = "rockchip,rk3399-efuse",
		.data = (ulong)&rk3399_data,
	},
	{}
};

U_BOOT_DRIVER(rockchip_efuse) = {
	.name = "rockchip_efuse",
	.id = UCLASS_MISC,
	.of_match = rockchip_efuse_ids,
	.of_to_plat = rockchip_efuse_of_to_plat,
	.plat_auto = sizeof(struct rockchip_efuse_plat),
	.ops = &rockchip_efuse_ops,
};
