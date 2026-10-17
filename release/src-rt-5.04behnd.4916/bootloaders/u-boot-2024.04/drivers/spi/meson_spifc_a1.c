// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Amlogic A1 SPI flash controller (SPIFC)
 *
 * Copyright (c) 2023, SberDevices. All Rights Reserved.
 *
 * Author: Martin Kurbanov <mmkurbanov@sberdevices.ru>
 *
 * Ported to u-boot:
 * Author: Igor Prusov <ivprusov@sberdevices.ru>
 */

#include <clk.h>
#include <dm.h>
#include <spi.h>
#include <spi-mem.h>
#include <asm/io.h>
#include <linux/log2.h>
#include <linux/time.h>
#include <linux/iopoll.h>
#include <linux/bitfield.h>

#define SPIFC_A1_AHB_CTRL_REG		0x0
#define SPIFC_A1_AHB_BUS_EN		BIT(31)

#define SPIFC_A1_USER_CTRL0_REG		0x200
#define SPIFC_A1_USER_REQUEST_ENABLE	BIT(31)
#define SPIFC_A1_USER_REQUEST_FINISH	BIT(30)
#define SPIFC_A1_USER_DATA_UPDATED	BIT(0)

#define SPIFC_A1_USER_CTRL1_REG		0x204
#define SPIFC_A1_USER_CMD_ENABLE	BIT(30)
#define SPIFC_A1_USER_CMD_MODE		GENMASK(29, 28)
#define SPIFC_A1_USER_CMD_CODE		GENMASK(27, 20)
#define SPIFC_A1_USER_ADDR_ENABLE	BIT(19)
#define SPIFC_A1_USER_ADDR_MODE		GENMASK(18, 17)
#define SPIFC_A1_USER_ADDR_BYTES	GENMASK(16, 15)
#define SPIFC_A1_USER_DOUT_ENABLE	BIT(14)
#define SPIFC_A1_USER_DOUT_MODE		GENMASK(11, 10)
#define SPIFC_A1_USER_DOUT_BYTES	GENMASK(9, 0)

#define SPIFC_A1_USER_CTRL2_REG		0x208
#define SPIFC_A1_USER_DUMMY_ENABLE	BIT(31)
#define SPIFC_A1_USER_DUMMY_MODE	GENMASK(30, 29)
#define SPIFC_A1_USER_DUMMY_CLK_SYCLES	GENMASK(28, 23)

#define SPIFC_A1_USER_CTRL3_REG		0x20c
#define SPIFC_A1_USER_DIN_ENABLE	BIT(31)
#define SPIFC_A1_USER_DIN_MODE		GENMASK(28, 27)
#define SPIFC_A1_USER_DIN_BYTES		GENMASK(25, 16)

#define SPIFC_A1_USER_ADDR_REG		0x210

#define SPIFC_A1_AHB_REQ_CTRL_REG	0x214
#define SPIFC_A1_AHB_REQ_ENABLE		BIT(31)

#define SPIFC_A1_ACTIMING0_REG		(0x0088 << 2)
#define SPIFC_A1_TSLCH			GENMASK(31, 30)
#define SPIFC_A1_TCLSH			GENMASK(29, 28)
#define SPIFC_A1_TSHWL			GENMASK(20, 16)
#define SPIFC_A1_TSHSL2			GENMASK(15, 12)
#define SPIFC_A1_TSHSL1			GENMASK(11, 8)
#define SPIFC_A1_TWHSL			GENMASK(7, 0)

#define SPIFC_A1_DBUF_CTRL_REG		0x240
#define SPIFC_A1_DBUF_DIR		BIT(31)
#define SPIFC_A1_DBUF_AUTO_UPDATE_ADDR	BIT(30)
#define SPIFC_A1_DBUF_ADDR		GENMASK(7, 0)

#define SPIFC_A1_DBUF_DATA_REG		0x244

#define SPIFC_A1_USER_DBUF_ADDR_REG	0x248

#define SPIFC_A1_BUFFER_SIZE		512U

#define SPIFC_A1_MAX_HZ			200000000
#define SPIFC_A1_MIN_HZ			1000000

#define SPIFC_A1_USER_CMD(op) ( \
	SPIFC_A1_USER_CMD_ENABLE | \
	FIELD_PREP(SPIFC_A1_USER_CMD_CODE, (op)->cmd.opcode) | \
	FIELD_PREP(SPIFC_A1_USER_CMD_MODE, ilog2((op)->cmd.buswidth)))

#define SPIFC_A1_USER_ADDR(op) ( \
	SPIFC_A1_USER_ADDR_ENABLE | \
	FIELD_PREP(SPIFC_A1_USER_ADDR_MODE, ilog2((op)->addr.buswidth)) | \
	FIELD_PREP(SPIFC_A1_USER_ADDR_BYTES, (op)->addr.nbytes - 1))

#define SPIFC_A1_USER_DUMMY(op) ( \
	SPIFC_A1_USER_DUMMY_ENABLE | \
	FIELD_PREP(SPIFC_A1_USER_DUMMY_MODE, ilog2((op)->dummy.buswidth)) | \
	FIELD_PREP(SPIFC_A1_USER_DUMMY_CLK_SYCLES, (op)->dummy.nbytes << 3))

#define SPIFC_A1_TSLCH_VAL	FIELD_PREP(SPIFC_A1_TSLCH, 1)
#define SPIFC_A1_TCLSH_VAL	FIELD_PREP(SPIFC_A1_TCLSH, 1)
#define SPIFC_A1_TSHWL_VAL	FIELD_PREP(SPIFC_A1_TSHWL, 7)
#define SPIFC_A1_TSHSL2_VAL	FIELD_PREP(SPIFC_A1_TSHSL2, 7)
#define SPIFC_A1_TSHSL1_VAL	FIELD_PREP(SPIFC_A1_TSHSL1, 7)
#define SPIFC_A1_TWHSL_VAL	FIELD_PREP(SPIFC_A1_TWHSL, 2)
#define SPIFC_A1_ACTIMING0_VAL	(SPIFC_A1_TSLCH_VAL | SPIFC_A1_TCLSH_VAL | \
				 SPIFC_A1_TSHWL_VAL | SPIFC_A1_TSHSL2_VAL | \
				 SPIFC_A1_TSHSL1_VAL | SPIFC_A1_TWHSL_VAL)

struct amlogic_spifc_a1 {
	struct clk clk;
	void __iomem *base;
	u32 curr_speed_hz;
};

static int amlogic_spifc_a1_request(struct amlogic_spifc_a1 *spifc, bool read)
{
	u32 mask = SPIFC_A1_USER_REQUEST_FINISH |
		   (read ? SPIFC_A1_USER_DATA_UPDATED : 0);
	u32 val;

	writel(SPIFC_A1_USER_REQUEST_ENABLE,
	       spifc->base + SPIFC_A1_USER_CTRL0_REG);

	return readl_poll_timeout(spifc->base + SPIFC_A1_USER_CTRL0_REG,
				  val, (val & mask) == mask,
				  200 * USEC_PER_MSEC);
}

static void amlogic_spifc_a1_drain_buffer(struct amlogic_spifc_a1 *spifc,
					  char *buf, u32 len)
{
	u32 data;
	const u32 count = len / sizeof(data);
	const u32 pad = len % sizeof(data);

	writel(SPIFC_A1_DBUF_AUTO_UPDATE_ADDR,
	       spifc->base + SPIFC_A1_DBUF_CTRL_REG);
	ioread32_rep(spifc->base + SPIFC_A1_DBUF_DATA_REG, buf, count);

	if (pad) {
		data = readl(spifc->base + SPIFC_A1_DBUF_DATA_REG);
		memcpy(buf + len - pad, &data, pad);
	}
}

static void amlogic_spifc_a1_fill_buffer(struct amlogic_spifc_a1 *spifc,
					 const char *buf, u32 len)
{
	u32 data;
	const u32 count = len / sizeof(data);
	const u32 pad = len % sizeof(data);

	writel(SPIFC_A1_DBUF_DIR | SPIFC_A1_DBUF_AUTO_UPDATE_ADDR,
	       spifc->base + SPIFC_A1_DBUF_CTRL_REG);
	iowrite32_rep(spifc->base + SPIFC_A1_DBUF_DATA_REG, buf, count);

	if (pad) {
		memcpy(&data, buf + len - pad, pad);
		writel(data, spifc->base + SPIFC_A1_DBUF_DATA_REG);
	}
}

static void amlogic_spifc_a1_user_init(struct amlogic_spifc_a1 *spifc)
{
	writel(0, spifc->base + SPIFC_A1_USER_CTRL0_REG);
	writel(0, spifc->base + SPIFC_A1_USER_CTRL1_REG);
	writel(0, spifc->base + SPIFC_A1_USER_CTRL2_REG);
	writel(0, spifc->base + SPIFC_A1_USER_CTRL3_REG);
}

static void amlogic_spifc_a1_set_cmd(struct amlogic_spifc_a1 *spifc,
				     u32 cmd_cfg)
{
	u32 val;

	val = readl(spifc->base + SPIFC_A1_USER_CTRL1_REG);
	val &= ~(SPIFC_A1_USER_CMD_MODE | SPIFC_A1_USER_CMD_CODE);
	val |= cmd_cfg;
	writel(val, spifc->base + SPIFC_A1_USER_CTRL1_REG);
}

static void amlogic_spifc_a1_set_addr(struct amlogic_spifc_a1 *spifc, u32 addr,
				      u32 addr_cfg)
{
	u32 val;

	writel(addr, spifc->base + SPIFC_A1_USER_ADDR_REG);

	val = readl(spifc->base + SPIFC_A1_USER_CTRL1_REG);
	val &= ~(SPIFC_A1_USER_ADDR_MODE | SPIFC_A1_USER_ADDR_BYTES);
	val |= addr_cfg;
	writel(val, spifc->base + SPIFC_A1_USER_CTRL1_REG);
}

static void amlogic_spifc_a1_set_dummy(struct amlogic_spifc_a1 *spifc,
				       u32 dummy_cfg)
{
	u32 val = readl(spifc->base + SPIFC_A1_USER_CTRL2_REG);

	val &= ~(SPIFC_A1_USER_DUMMY_MODE | SPIFC_A1_USER_DUMMY_CLK_SYCLES);
	val |= dummy_cfg;
	writel(val, spifc->base + SPIFC_A1_USER_CTRL2_REG);
}

static int amlogic_spifc_a1_read(struct amlogic_spifc_a1 *spifc, void *buf,
				 u32 size, u32 mode)
{
	u32 val = readl(spifc->base + SPIFC_A1_USER_CTRL3_REG);
	int ret;

	val &= ~(SPIFC_A1_USER_DIN_MODE | SPIFC_A1_USER_DIN_BYTES);
	val |= SPIFC_A1_USER_DIN_ENABLE;
	val |= FIELD_PREP(SPIFC_A1_USER_DIN_MODE, mode);
	val |= FIELD_PREP(SPIFC_A1_USER_DIN_BYTES, size);
	writel(val, spifc->base + SPIFC_A1_USER_CTRL3_REG);

	ret = amlogic_spifc_a1_request(spifc, true);
	if (!ret)
		amlogic_spifc_a1_drain_buffer(spifc, buf, size);

	return ret;
}

static int amlogic_spifc_a1_write(struct amlogic_spifc_a1 *spifc,
				  const void *buf, u32 size, u32 mode)
{
	u32 val;

	amlogic_spifc_a1_fill_buffer(spifc, buf, size);

	val = readl(spifc->base + SPIFC_A1_USER_CTRL1_REG);
	val &= ~(SPIFC_A1_USER_DOUT_MODE | SPIFC_A1_USER_DOUT_BYTES);
	val |= FIELD_PREP(SPIFC_A1_USER_DOUT_MODE, mode);
	val |= FIELD_PREP(SPIFC_A1_USER_DOUT_BYTES, size);
	val |= SPIFC_A1_USER_DOUT_ENABLE;
	writel(val, spifc->base + SPIFC_A1_USER_CTRL1_REG);

	return amlogic_spifc_a1_request(spifc, false);
}

static int amlogic_spifc_a1_set_freq(struct amlogic_spifc_a1 *spifc, u32 freq)
{
	int ret;

	if (freq == spifc->curr_speed_hz)
		return 0;

	ret = clk_set_rate(&spifc->clk, freq);
	if (ret)
		return ret;

	spifc->curr_speed_hz = freq;
	return 0;
}

static int amlogic_spifc_a1_exec_op(struct spi_slave *slave,
				    const struct spi_mem_op *op)
{
	struct amlogic_spifc_a1 *spifc = dev_get_priv(slave->dev->parent);
	size_t data_size = op->data.nbytes;
	int ret;

	ret = amlogic_spifc_a1_set_freq(spifc, slave->max_hz);
	if (ret)
		return ret;

	amlogic_spifc_a1_user_init(spifc);
	amlogic_spifc_a1_set_cmd(spifc, SPIFC_A1_USER_CMD(op));

	if (op->addr.nbytes)
		amlogic_spifc_a1_set_addr(spifc, op->addr.val,
					  SPIFC_A1_USER_ADDR(op));

	if (op->dummy.nbytes)
		amlogic_spifc_a1_set_dummy(spifc, SPIFC_A1_USER_DUMMY(op));

	if (data_size) {
		u32 mode = ilog2(op->data.buswidth);

		writel(0, spifc->base + SPIFC_A1_USER_DBUF_ADDR_REG);

		if (op->data.dir == SPI_MEM_DATA_IN)
			ret = amlogic_spifc_a1_read(spifc, op->data.buf.in,
						    data_size, mode);
		else
			ret = amlogic_spifc_a1_write(spifc, op->data.buf.out,
						     data_size, mode);
	} else {
		ret = amlogic_spifc_a1_request(spifc, false);
	}

	return ret;
}

static int amlogic_spifc_a1_adjust_op_size(struct spi_slave *slave,
					   struct spi_mem_op *op)
{
	op->data.nbytes = min(op->data.nbytes, SPIFC_A1_BUFFER_SIZE);
	return 0;
}

static void amlogic_spifc_a1_hw_init(struct amlogic_spifc_a1 *spifc)
{
	u32 regv;

	regv = readl(spifc->base + SPIFC_A1_AHB_REQ_CTRL_REG);
	regv &= ~(SPIFC_A1_AHB_REQ_ENABLE);
	writel(regv, spifc->base + SPIFC_A1_AHB_REQ_CTRL_REG);

	regv = readl(spifc->base + SPIFC_A1_AHB_CTRL_REG);
	regv &= ~(SPIFC_A1_AHB_BUS_EN);
	writel(regv, spifc->base + SPIFC_A1_AHB_CTRL_REG);

	writel(SPIFC_A1_ACTIMING0_VAL, spifc->base + SPIFC_A1_ACTIMING0_REG);

	writel(0, spifc->base + SPIFC_A1_USER_DBUF_ADDR_REG);
}

static const struct spi_controller_mem_ops amlogic_spifc_a1_mem_ops = {
	.exec_op = amlogic_spifc_a1_exec_op,
	.adjust_op_size = amlogic_spifc_a1_adjust_op_size,
};

static int amlogic_spifc_a1_probe(struct udevice *dev)
{
	struct amlogic_spifc_a1 *spifc = dev_get_priv(dev);
	int ret;
	struct udevice *bus = dev;

	spifc->base = dev_read_addr_ptr(dev);
	if (!spifc->base)
		return -EINVAL;

	ret = clk_get_by_index(bus, 0, &spifc->clk);
	if (ret) {
		pr_err("can't get clk spifc_gate!\n");
		return ret;
	}

	ret = clk_enable(&spifc->clk);
	if (ret) {
		pr_err("enable clk fail\n");
		return ret;
	}

	amlogic_spifc_a1_hw_init(spifc);

	return 0;
}

static const struct udevice_id meson_spifc_ids[] = {
	{ .compatible = "amlogic,a1-spifc", },
	{ }
};

int amlogic_spifc_a1_set_speed(struct udevice *bus, uint hz)
{
	return 0;
}

int amlogic_spifc_a1_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static const struct dm_spi_ops amlogic_spifc_a1_ops = {
	.mem_ops = &amlogic_spifc_a1_mem_ops,
	.set_speed = amlogic_spifc_a1_set_speed,
	.set_mode = amlogic_spifc_a1_set_mode,
};

U_BOOT_DRIVER(meson_spifc_a1) = {
	.name		= "meson_spifc_a1",
	.id		= UCLASS_SPI,
	.of_match	= meson_spifc_ids,
	.ops		= &amlogic_spifc_a1_ops,
	.probe		= amlogic_spifc_a1_probe,
	.priv_auto	= sizeof(struct amlogic_spifc_a1),
};
