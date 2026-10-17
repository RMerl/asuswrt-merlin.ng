// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
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

/* OTP Register Offsets */
#define OTPC_SBPI_CTRL			0x0020
#define OTPC_SBPI_CMD_VALID_PRE		0x0024
#define OTPC_SBPI_CS_VALID_PRE		0x0028
#define OTPC_SBPI_STATUS		0x002C
#define OTPC_USER_CTRL			0x0100
#define OTPC_USER_ADDR			0x0104
#define OTPC_USER_ENABLE		0x0108
#define OTPC_USER_QP			0x0120
#define OTPC_USER_Q			0x0124
#define OTPC_INT_STATUS			0x0304
#define OTPC_SBPI_CMD0_OFFSET		0x1000
#define OTPC_SBPI_CMD1_OFFSET		0x1004

/* OTP Register bits and masks */
#define OTPC_USER_ADDR_MASK		GENMASK(31, 16)
#define OTPC_USE_USER			BIT(0)
#define OTPC_USE_USER_MASK		GENMASK(16, 16)
#define OTPC_USER_FSM_ENABLE		BIT(0)
#define OTPC_USER_FSM_ENABLE_MASK	GENMASK(16, 16)
#define OTPC_SBPI_DONE			BIT(1)
#define OTPC_USER_DONE			BIT(2)

#define SBPI_DAP_ADDR			0x02
#define SBPI_DAP_ADDR_SHIFT		8
#define SBPI_DAP_ADDR_MASK		GENMASK(31, 24)
#define SBPI_CMD_VALID_MASK		GENMASK(31, 16)
#define SBPI_DAP_CMD_WRF		0xC0
#define SBPI_DAP_REG_ECC		0x3A
#define SBPI_ECC_ENABLE			0x00
#define SBPI_ECC_DISABLE		0x09
#define SBPI_ENABLE			BIT(0)
#define SBPI_ENABLE_MASK		GENMASK(16, 16)

#define OTPC_TIMEOUT			10000

#define RK3588_OTPC_AUTO_CTRL		0x0004
#define RK3588_ADDR_SHIFT		16
#define RK3588_ADDR(n)			((n) << RK3588_ADDR_SHIFT)
#define RK3588_BURST_SHIFT		8
#define RK3588_BURST(n)			((n) << RK3588_BURST_SHIFT)
#define RK3588_OTPC_AUTO_EN		0x0008
#define RK3588_AUTO_EN			BIT(0)
#define RK3588_OTPC_DOUT0		0x0020
#define RK3588_OTPC_INT_ST		0x0084
#define RK3588_RD_DONE			BIT(1)

#define RV1126_OTP_NVM_CEB		0x00
#define RV1126_OTP_NVM_RSTB		0x04
#define RV1126_OTP_NVM_ST		0x18
#define RV1126_OTP_NVM_RADDR		0x1C
#define RV1126_OTP_NVM_RSTART		0x20
#define RV1126_OTP_NVM_RDATA		0x24
#define RV1126_OTP_READ_ST		0x30

struct rockchip_otp_plat {
	void __iomem *base;
};

struct rockchip_otp_data {
	int (*init)(struct udevice *dev);
	int (*read)(struct udevice *dev, int offset, void *buf, int size);
	int offset;
	int size;
	int block_size;
};

#if defined(DEBUG)
static int dump_otp(struct cmd_tbl *cmdtp, int flag,
		    int argc, char *const argv[])
{
	struct udevice *dev;
	u8 data[4];
	int ret, i;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
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
	dump_otp, 1, 1, dump_otp,
	"Dump the content of the otp",
	""
);
#endif

static int rockchip_otp_poll_timeout(struct rockchip_otp_plat *otp,
				     u32 flag, u32 reg)
{
	u32 status;
	int ret;

	ret = readl_poll_sleep_timeout(otp->base + reg, status,
				       (status & flag), 1, OTPC_TIMEOUT);
	if (ret)
		return ret;

	/* Clear int flag */
	writel(flag, otp->base + reg);

	return 0;
}

static int rockchip_otp_ecc_enable(struct rockchip_otp_plat *otp, bool enable)
{
	writel(SBPI_DAP_ADDR_MASK | (SBPI_DAP_ADDR << SBPI_DAP_ADDR_SHIFT),
	       otp->base + OTPC_SBPI_CTRL);

	writel(SBPI_CMD_VALID_MASK | 0x1, otp->base + OTPC_SBPI_CMD_VALID_PRE);
	writel(SBPI_DAP_CMD_WRF | SBPI_DAP_REG_ECC,
	       otp->base + OTPC_SBPI_CMD0_OFFSET);

	if (enable)
		writel(SBPI_ECC_ENABLE, otp->base + OTPC_SBPI_CMD1_OFFSET);
	else
		writel(SBPI_ECC_DISABLE, otp->base + OTPC_SBPI_CMD1_OFFSET);

	writel(SBPI_ENABLE_MASK | SBPI_ENABLE, otp->base + OTPC_SBPI_CTRL);

	return rockchip_otp_poll_timeout(otp, OTPC_SBPI_DONE, OTPC_INT_STATUS);
}

static int rockchip_px30_otp_read(struct udevice *dev, int offset,
				  void *buf, int size)
{
	struct rockchip_otp_plat *otp = dev_get_plat(dev);
	u8 *buffer = buf;
	int ret;

	ret = rockchip_otp_ecc_enable(otp, false);
	if (ret)
		return ret;

	writel(OTPC_USE_USER | OTPC_USE_USER_MASK, otp->base + OTPC_USER_CTRL);
	udelay(5);

	while (size--) {
		writel(offset++ | OTPC_USER_ADDR_MASK,
		       otp->base + OTPC_USER_ADDR);
		writel(OTPC_USER_FSM_ENABLE | OTPC_USER_FSM_ENABLE_MASK,
		       otp->base + OTPC_USER_ENABLE);

		ret = rockchip_otp_poll_timeout(otp, OTPC_USER_DONE,
						OTPC_INT_STATUS);
		if (ret)
			goto read_end;

		*buffer++ = (u8)(readl(otp->base + OTPC_USER_Q) & 0xFF);
	}

read_end:
	writel(0x0 | OTPC_USE_USER_MASK, otp->base + OTPC_USER_CTRL);

	return ret;
}

static int rockchip_rk3568_otp_read(struct udevice *dev, int offset,
				    void *buf, int size)
{
	struct rockchip_otp_plat *otp = dev_get_plat(dev);
	u16 *buffer = buf;
	int ret;

	ret = rockchip_otp_ecc_enable(otp, false);
	if (ret)
		return ret;

	writel(OTPC_USE_USER | OTPC_USE_USER_MASK, otp->base + OTPC_USER_CTRL);
	udelay(5);

	while (size--) {
		writel(offset++ | OTPC_USER_ADDR_MASK,
		       otp->base + OTPC_USER_ADDR);
		writel(OTPC_USER_FSM_ENABLE | OTPC_USER_FSM_ENABLE_MASK,
		       otp->base + OTPC_USER_ENABLE);

		ret = rockchip_otp_poll_timeout(otp, OTPC_USER_DONE,
						OTPC_INT_STATUS);
		if (ret)
			goto read_end;

		*buffer++ = (u16)(readl(otp->base + OTPC_USER_Q) & 0xFFFF);
	}

read_end:
	writel(0x0 | OTPC_USE_USER_MASK, otp->base + OTPC_USER_CTRL);

	return ret;
}

static int rockchip_rk3588_otp_read(struct udevice *dev, int offset,
				    void *buf, int size)
{
	struct rockchip_otp_plat *otp = dev_get_plat(dev);
	u32 *buffer = buf;
	int ret;

	while (size--) {
		writel(RK3588_ADDR(offset++) | RK3588_BURST(1),
		       otp->base + RK3588_OTPC_AUTO_CTRL);
		writel(RK3588_AUTO_EN, otp->base + RK3588_OTPC_AUTO_EN);

		ret = rockchip_otp_poll_timeout(otp, RK3588_RD_DONE,
						RK3588_OTPC_INT_ST);
		if (ret)
			return ret;

		*buffer++ = readl(otp->base + RK3588_OTPC_DOUT0);
	}

	return 0;
}

static int rockchip_rv1126_otp_init(struct udevice *dev)
{
	struct rockchip_otp_plat *otp = dev_get_plat(dev);
	int ret;

	writel(0x0, otp->base + RV1126_OTP_NVM_CEB);
	ret = rockchip_otp_poll_timeout(otp, 0x1, RV1126_OTP_NVM_ST);

	if (ret)
		return ret;

	writel(0x1, otp->base + RV1126_OTP_NVM_RSTB);
	ret = rockchip_otp_poll_timeout(otp, 0x4, RV1126_OTP_NVM_ST);

	if (ret)
		return ret;

	return 0;
}

static int rockchip_rv1126_otp_read(struct udevice *dev, int offset, void *buf,
				    int size)
{
	struct rockchip_otp_plat *otp = dev_get_plat(dev);
	u32 status = 0;
	u8 *buffer = buf;
	int ret = 0;

	while (size--) {
		writel(offset++, otp->base + RV1126_OTP_NVM_RADDR);
		writel(0x1, otp->base + RV1126_OTP_NVM_RSTART);
		ret = readl_poll_timeout(otp->base + RV1126_OTP_READ_ST,
					 status, !status, OTPC_TIMEOUT);
		if (ret)
			return ret;

		*buffer++ = (u8)(readl(otp->base + RV1126_OTP_NVM_RDATA) & 0xFF);
	}

	return 0;
}

static int rockchip_otp_read(struct udevice *dev, int offset,
			     void *buf, int size)
{
	const struct rockchip_otp_data *data =
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

static const struct misc_ops rockchip_otp_ops = {
	.read = rockchip_otp_read,
};

static int rockchip_otp_of_to_plat(struct udevice *dev)
{
	struct rockchip_otp_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);

	return 0;
}

static int rockchip_otp_probe(struct udevice *dev)
{
	struct rockchip_otp_data *data;

	data = (struct rockchip_otp_data *)dev_get_driver_data(dev);
	if (!data)
		return -EINVAL;

	if (data->init)
		return data->init(dev);

	return 0;
}

static const struct rockchip_otp_data px30_data = {
	.read = rockchip_px30_otp_read,
	.size = 0x40,
};

static const struct rockchip_otp_data rk3568_data = {
	.read = rockchip_rk3568_otp_read,
	.size = 0x80,
	.block_size = 2,
};

static const struct rockchip_otp_data rk3588_data = {
	.read = rockchip_rk3588_otp_read,
	.offset = 0xC00,
	.size = 0x400,
	.block_size = 4,
};

static const struct rockchip_otp_data rv1126_data = {
	.init = rockchip_rv1126_otp_init,
	.read = rockchip_rv1126_otp_read,
	.size = 0x40,
};

static const struct udevice_id rockchip_otp_ids[] = {
	{
		.compatible = "rockchip,px30-otp",
		.data = (ulong)&px30_data,
	},
	{
		.compatible = "rockchip,rk3308-otp",
		.data = (ulong)&px30_data,
	},
	{
		.compatible = "rockchip,rk3568-otp",
		.data = (ulong)&rk3568_data,
	},
	{
		.compatible = "rockchip,rk3588-otp",
		.data = (ulong)&rk3588_data,
	},
	{
		.compatible = "rockchip,rv1126-otp",
		.data = (ulong)&rv1126_data,
	},
	{}
};

U_BOOT_DRIVER(rockchip_otp) = {
	.name = "rockchip_otp",
	.id = UCLASS_MISC,
	.of_match = rockchip_otp_ids,
	.of_to_plat = rockchip_otp_of_to_plat,
	.plat_auto = sizeof(struct rockchip_otp_plat),
	.ops = &rockchip_otp_ops,
	.probe = rockchip_otp_probe,
};
