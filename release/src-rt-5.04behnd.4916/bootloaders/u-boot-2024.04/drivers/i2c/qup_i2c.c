// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2009-2013, 2016-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2014, Sony Mobile Communications AB.
 * Copyright (c) 2022-2023, Sumit Garg <sumit.garg@linaro.org>
 *
 * Inspired by corresponding driver in Linux: drivers/i2c/busses/i2c-qup.c
 */

#include <init.h>
#include <env.h>
#include <common.h>
#include <log.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/compat.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <i2c.h>
#include <watchdog.h>
#include <fdtdec.h>
#include <clk.h>
#include <reset.h>
#include <asm/arch/gpio.h>
#include <cpu_func.h>
#include <asm/system.h>
#include <asm/gpio.h>
#include <dm.h>
#include <dm/pinctrl.h>

/* QUP Registers */
#define QUP_CONFIG				0x000
#define QUP_STATE				0x004
#define QUP_IO_MODE				0x008
#define QUP_SW_RESET				0x00c
#define QUP_OPERATIONAL				0x018
#define QUP_ERROR_FLAGS				0x01c /* NOT USED */
#define QUP_ERROR_FLAGS_EN			0x020 /* NOT USED */
#define QUP_TEST_CTRL				0x024 /* NOT USED */
#define QUP_OPERATIONAL_MASK			0x028 /* NOT USED */
#define QUP_HW_VERSION				0x030
#define QUP_MX_OUTPUT_CNT			0x100
#define QUP_OUT_DEBUG				0x108 /* NOT USED */
#define QUP_OUT_FIFO_CNT			0x10C /* NOT USED */
#define QUP_OUT_FIFO_BASE			0x110
#define QUP_MX_WRITE_CNT			0x150
#define QUP_MX_INPUT_CNT			0x200
#define QUP_MX_READ_CNT				0x208
#define QUP_IN_READ_CUR				0x20C /* NOT USED */
#define QUP_IN_DEBUG				0x210 /* NOT USED */
#define QUP_IN_FIFO_CNT				0x214 /* NOT USED */
#define QUP_IN_FIFO_BASE			0x218
#define QUP_I2C_CLK_CTL				0x400
#define QUP_I2C_STATUS				0x404 /* NOT USED */
#define QUP_I2C_MASTER_GEN			0x408
#define QUP_I2C_MASTER_BUS_CLR			0x40C /* NOT USED */

/* QUP States and reset values */
#define QUP_RESET_STATE				0
#define QUP_RUN_STATE				1
#define QUP_PAUSE_STATE				3
#define QUP_STATE_MASK				3

#define QUP_STATE_VALID				BIT(2)
#define QUP_I2C_MAST_GEN			BIT(4)
#define QUP_I2C_FLUSH				BIT(6)

#define QUP_OPERATIONAL_RESET			0x000ff0
#define QUP_I2C_STATUS_RESET			0xfffffc

/* QUP OPERATIONAL FLAGS */
#define QUP_I2C_NACK_FLAG			BIT(3)
#define QUP_OUT_NOT_EMPTY			BIT(4)
#define QUP_IN_NOT_EMPTY			BIT(5)
#define QUP_OUT_FULL				BIT(6)
#define QUP_OUT_SVC_FLAG			BIT(8)
#define QUP_IN_SVC_FLAG				BIT(9)
#define QUP_MX_OUTPUT_DONE			BIT(10)
#define QUP_MX_INPUT_DONE			BIT(11)
#define OUT_BLOCK_WRITE_REQ			BIT(12)
#define IN_BLOCK_READ_REQ			BIT(13)

/*
 * QUP engine acting as I2C controller is referred to as
 * I2C mini core, following are related macros.
 */
#define QUP_NO_OUTPUT				BIT(6)
#define QUP_NO_INPUT				BIT(7)
#define QUP_CLOCK_AUTO_GATE			BIT(13)
#define QUP_I2C_MINI_CORE			(2 << 8)
#define QUP_I2C_N_VAL_V2			7

/* Packing/Unpacking words in FIFOs, and IO modes */
#define QUP_OUTPUT_BLK_MODE			BIT(10)
#define QUP_OUTPUT_BAM_MODE			(BIT(10) | BIT(11))
#define QUP_INPUT_BLK_MODE			BIT(12)
#define QUP_INPUT_BAM_MODE			(BIT(12) | BIT(13))
#define QUP_BAM_MODE				(QUP_OUTPUT_BAM_MODE | QUP_INPUT_BAM_MODE)
#define QUP_BLK_MODE				(QUP_OUTPUT_BLK_MODE | QUP_INPUT_BLK_MODE)
#define QUP_UNPACK_EN				BIT(14)
#define QUP_PACK_EN				BIT(15)

#define QUP_REPACK_EN				(QUP_UNPACK_EN | QUP_PACK_EN)
#define QUP_V2_TAGS_EN				1

#define QUP_OUTPUT_BLOCK_SIZE(x)		(((x) >> 0) & 0x03)
#define QUP_OUTPUT_FIFO_SIZE(x)			(((x) >> 2) & 0x07)
#define QUP_INPUT_BLOCK_SIZE(x)			(((x) >> 5) & 0x03)
#define QUP_INPUT_FIFO_SIZE(x)			(((x) >> 7) & 0x07)

/* QUP v2 tags */
#define QUP_TAG_V2_START			0x81
#define QUP_TAG_V2_DATAWR			0x82
#define QUP_TAG_V2_DATAWR_STOP			0x83
#define QUP_TAG_V2_DATARD			0x85
#define QUP_TAG_V2_DATARD_NACK			0x86
#define QUP_TAG_V2_DATARD_STOP			0x87

#define QUP_I2C_MX_CONFIG_DURING_RUN		BIT(31)

/* Minimum transfer timeout for i2c transfers in micro seconds */
#define TOUT_CNT				(2 * 1000 * 1000)

/* Default values. Use these if FW query fails */
#define DEFAULT_CLK_FREQ			I2C_SPEED_STANDARD_RATE
#define DEFAULT_SRC_CLK				19200000

/*
 * Max tags length (start, stop and maximum 2 bytes address) for each QUP
 * data transfer
 */
#define QUP_MAX_TAGS_LEN			4
/* Max data length for each DATARD tags */
#define RECV_MAX_DATA_LEN			254
/* TAG length for DATA READ in RX FIFO */
#define READ_RX_TAGS_LEN			2

struct qup_i2c_priv {
	phys_addr_t base;
	struct clk core;
	struct clk iface;
	u32 in_fifo_sz;
	u32 out_fifo_sz;
	u32 clk_ctl;
	u32 config_run;
};

static inline u8 i2c_8bit_addr_from_msg(const struct i2c_msg *msg)
{
	return (msg->addr << 1) | (msg->flags & I2C_M_RD ? 1 : 0);
}

static int qup_i2c_poll_state_mask(struct qup_i2c_priv *qup,
				   u32 req_state, u32 req_mask)
{
	int retries = 1;
	u32 state;

	/*
	 * State transition takes 3 AHB clocks cycles + 3 I2C master clock
	 * cycles. So retry once after a 1uS delay.
	 */
	do {
		state = readl(qup->base + QUP_STATE);

		if (state & QUP_STATE_VALID &&
		    (state & req_mask) == req_state)
			return 0;

		udelay(1);
	} while (retries--);

	return -ETIMEDOUT;
}

static int qup_i2c_poll_state(struct qup_i2c_priv *qup, u32 req_state)
{
	return qup_i2c_poll_state_mask(qup, req_state, QUP_STATE_MASK);
}

static int qup_i2c_poll_state_valid(struct qup_i2c_priv *qup)
{
	return qup_i2c_poll_state_mask(qup, 0, 0);
}

static int qup_i2c_poll_state_i2c_master(struct qup_i2c_priv *qup)
{
	return qup_i2c_poll_state_mask(qup, QUP_I2C_MAST_GEN, QUP_I2C_MAST_GEN);
}

static int qup_i2c_change_state(struct qup_i2c_priv *qup, u32 state)
{
	if (qup_i2c_poll_state_valid(qup) != 0)
		return -EIO;

	writel(state, qup->base + QUP_STATE);

	if (qup_i2c_poll_state(qup, state) != 0)
		return -EIO;
	return 0;
}

/*
 * Function to check wheather Input or Output FIFO
 * has data to be serviced
 */
static int qup_i2c_check_fifo_status(struct qup_i2c_priv *qup, u32 reg_addr,
				     u32 flags)
{
	unsigned long count = TOUT_CNT;
	u32 val, status_flag;
	int ret = 0;

	do {
		val = readl(qup->base + reg_addr);
		status_flag = val & flags;

		if (!count) {
			printf("%s, timeout\n", __func__);
			ret = -ETIMEDOUT;
			break;
		}

		count--;
		udelay(1);
	} while (!status_flag);

	return ret;
}

/*
 * Function to configure Input and Output enable/disable
 */
static void qup_i2c_enable_io_config(struct qup_i2c_priv *qup, u32 write_cnt,
				     u32 read_cnt)
{
	u32 qup_config = QUP_I2C_MINI_CORE | QUP_I2C_N_VAL_V2;

	writel(qup->config_run | write_cnt, qup->base + QUP_MX_WRITE_CNT);

	if (read_cnt)
		writel(qup->config_run | read_cnt, qup->base + QUP_MX_READ_CNT);
	else
		qup_config |= QUP_NO_INPUT;

	writel(qup_config, qup->base + QUP_CONFIG);
}

static unsigned int qup_i2c_read_word(struct qup_i2c_priv *qup)
{
	return readl(qup->base + QUP_IN_FIFO_BASE);
}

static void qup_i2c_write_word(struct qup_i2c_priv *qup, u32 word)
{
	writel(word, qup->base + QUP_OUT_FIFO_BASE);
}

static int qup_i2c_blsp_read(struct qup_i2c_priv *qup, unsigned int addr,
			     bool last, u8 *buffer, unsigned int bytes)
{
	unsigned int i, j, word;
	int ret = 0;

	/* FIFO mode size limitation, for larger size implement block mode */
	if (bytes > (qup->in_fifo_sz - READ_RX_TAGS_LEN))
		return -EINVAL;

	qup_i2c_enable_io_config(qup, QUP_MAX_TAGS_LEN,
				 bytes + READ_RX_TAGS_LEN);

	if (last)
		qup_i2c_write_word(qup, QUP_TAG_V2_START | addr << 8 |
					QUP_TAG_V2_DATARD_STOP << 16 |
					bytes << 24);
	else
		qup_i2c_write_word(qup, QUP_TAG_V2_START | addr << 8 |
					QUP_TAG_V2_DATARD << 16 | bytes << 24);

	ret = qup_i2c_change_state(qup, QUP_RUN_STATE);
	if (ret)
		return ret;

	ret = qup_i2c_check_fifo_status(qup, QUP_OPERATIONAL, QUP_OUT_SVC_FLAG);
	if (ret)
		return ret;
	writel(QUP_OUT_SVC_FLAG, qup->base + QUP_OPERATIONAL);

	ret = qup_i2c_check_fifo_status(qup, QUP_OPERATIONAL, QUP_IN_SVC_FLAG);
	if (ret)
		return ret;
	writel(QUP_IN_SVC_FLAG, qup->base + QUP_OPERATIONAL);

	word = qup_i2c_read_word(qup);
	*(buffer++) = (word >> (8 * READ_RX_TAGS_LEN)) & 0xff;
	if (bytes > 1)
		*(buffer++) = (word >> (8 * (READ_RX_TAGS_LEN + 1))) & 0xff;

	for (i = 2; i < bytes; i += 4) {
		word = qup_i2c_read_word(qup);

		for (j = 0; j < 4; j++) {
			if ((i + j) == bytes)
				break;
			*buffer = (word >> (j * 8)) & 0xff;
			buffer++;
		}
	}

	ret = qup_i2c_change_state(qup, QUP_PAUSE_STATE);
	return ret;
}

static int qup_i2c_blsp_write(struct qup_i2c_priv *qup, unsigned int addr,
			      bool first, bool last, const u8 *buffer,
			      unsigned int bytes)
{
	unsigned int i;
	u32 word = 0;
	int ret = 0;

	/* FIFO mode size limitation, for larger size implement block mode */
	if (bytes > (qup->out_fifo_sz - QUP_MAX_TAGS_LEN))
		return -EINVAL;

	qup_i2c_enable_io_config(qup, bytes + QUP_MAX_TAGS_LEN, 0);

	if (first) {
		ret = qup_i2c_change_state(qup, QUP_RUN_STATE);
		if (ret)
			return ret;

		writel(qup->clk_ctl, qup->base + QUP_I2C_CLK_CTL);

		ret = qup_i2c_change_state(qup, QUP_PAUSE_STATE);
		if (ret)
			return ret;
	}

	if (last)
		qup_i2c_write_word(qup, QUP_TAG_V2_START | addr << 8 |
					QUP_TAG_V2_DATAWR_STOP << 16 |
					bytes << 24);
	else
		qup_i2c_write_word(qup, QUP_TAG_V2_START | addr << 8 |
					QUP_TAG_V2_DATAWR << 16 | bytes << 24);

	for (i = 0; i < bytes; i++) {
		/* Write the byte of data */
		word |= *buffer << ((i % 4) * 8);
		if ((i % 4) == 3) {
			qup_i2c_write_word(qup, word);
			word = 0;
		}
		buffer++;
	}

	if ((i % 4) != 0)
		qup_i2c_write_word(qup, word);

	ret = qup_i2c_change_state(qup, QUP_RUN_STATE);
	if (ret)
		return ret;

	ret = qup_i2c_check_fifo_status(qup, QUP_OPERATIONAL, QUP_OUT_SVC_FLAG);
	if (ret)
		return ret;
	writel(QUP_OUT_SVC_FLAG, qup->base + QUP_OPERATIONAL);

	ret = qup_i2c_change_state(qup, QUP_PAUSE_STATE);
	return ret;
}

static void qup_i2c_conf_mode_v2(struct qup_i2c_priv *qup)
{
	u32 io_mode = QUP_REPACK_EN;

	writel(0, qup->base + QUP_MX_OUTPUT_CNT);
	writel(0, qup->base + QUP_MX_INPUT_CNT);

	writel(io_mode, qup->base + QUP_IO_MODE);
}

static int qup_i2c_xfer_v2(struct udevice *bus, struct i2c_msg msgs[], int num)
{
	struct qup_i2c_priv *qup = dev_get_priv(bus);
	int ret, idx = 0;
	u32 i2c_addr;

	writel(1, qup->base + QUP_SW_RESET);
	ret = qup_i2c_poll_state(qup, QUP_RESET_STATE);
	if (ret)
		goto out;

	/* Configure QUP as I2C mini core */
	writel(QUP_I2C_MINI_CORE | QUP_I2C_N_VAL_V2 | QUP_NO_INPUT,
	       qup->base + QUP_CONFIG);
	writel(QUP_V2_TAGS_EN, qup->base + QUP_I2C_MASTER_GEN);

	if (qup_i2c_poll_state_i2c_master(qup)) {
		ret = -EIO;
		goto out;
	}

	qup_i2c_conf_mode_v2(qup);

	for (idx = 0; idx < num; idx++) {
		struct i2c_msg *m = &msgs[idx];

		qup->config_run = !idx ? 0 : QUP_I2C_MX_CONFIG_DURING_RUN;
		i2c_addr = i2c_8bit_addr_from_msg(m);

		if (m->flags & I2C_M_RD)
			ret = qup_i2c_blsp_read(qup, i2c_addr, idx == (num - 1),
						m->buf, m->len);
		else
			ret = qup_i2c_blsp_write(qup, i2c_addr, idx == 0,
						 idx == (num - 1), m->buf,
						 m->len);
		if (ret)
			break;
	}
out:
	qup_i2c_change_state(qup, QUP_RESET_STATE);
	return ret;
}

static int qup_i2c_enable_clocks(struct udevice *dev, struct qup_i2c_priv *qup)
{
	int ret;

	ret = clk_enable(&qup->core);
	if (ret) {
		dev_err(dev, "clk_enable failed %d\n", ret);
		return ret;
	}

	ret = clk_enable(&qup->iface);
	if (ret) {
		dev_err(dev, "clk_enable failed %d\n", ret);
		return ret;
	}

	return 0;
}

static int qup_i2c_probe(struct udevice *dev)
{
	static const int blk_sizes[] = {4, 16, 32};
	struct qup_i2c_priv *qup = dev_get_priv(dev);
	u32 io_mode, hw_ver, size, size_idx;
	int ret;

	qup->base = (phys_addr_t)dev_read_addr_ptr(dev);
	if (!qup->base)
		return -EINVAL;

	ret = clk_get_by_name(dev, "core", &qup->core);
	if (ret) {
		pr_err("clk_get_by_name(core) failed: %d\n", ret);
		return ret;
	}
	ret = clk_get_by_name(dev, "iface", &qup->iface);
	if (ret) {
		pr_err("clk_get_by_name(iface) failed: %d\n", ret);
		return ret;
	}
	qup_i2c_enable_clocks(dev, qup);

	writel(1, qup->base + QUP_SW_RESET);
	ret = qup_i2c_poll_state_valid(qup);
	if (ret)
		return ret;

	hw_ver = readl(qup->base + QUP_HW_VERSION);
	dev_dbg(dev, "Revision %x\n", hw_ver);

	io_mode = readl(qup->base + QUP_IO_MODE);

	/*
	 * The block/fifo size w.r.t. 'actual data' is 1/2 due to 'tag'
	 * associated with each byte written/received
	 */
	size_idx = QUP_OUTPUT_BLOCK_SIZE(io_mode);
	if (size_idx >= ARRAY_SIZE(blk_sizes)) {
		ret = -EIO;
		return ret;
	}
	size = QUP_OUTPUT_FIFO_SIZE(io_mode);
	qup->out_fifo_sz = blk_sizes[size_idx] * (2 << size);

	size_idx = QUP_INPUT_BLOCK_SIZE(io_mode);
	if (size_idx >= ARRAY_SIZE(blk_sizes)) {
		ret = -EIO;
		return ret;
	}
	size = QUP_INPUT_FIFO_SIZE(io_mode);
	qup->in_fifo_sz = blk_sizes[size_idx] * (2 << size);

	dev_dbg(dev, "IN:fifo:%d, OUT:fifo:%d\n", qup->in_fifo_sz,
		qup->out_fifo_sz);

	return 0;
}

static int qup_i2c_set_bus_speed(struct udevice *dev, unsigned int clk_freq)
{
	struct qup_i2c_priv *qup = dev_get_priv(dev);
	unsigned int src_clk_freq;
	int fs_div, hs_div;

	/* We support frequencies up to FAST Mode Plus (1MHz) */
	if (!clk_freq || clk_freq > I2C_SPEED_FAST_PLUS_RATE) {
		dev_err(dev, "clock frequency not supported %d\n", clk_freq);
		return -EINVAL;
	}

	src_clk_freq = clk_get_rate(&qup->iface);
	if ((int)src_clk_freq < 0) {
		src_clk_freq = DEFAULT_SRC_CLK;
		dev_dbg(dev, "using default core freq %d\n", src_clk_freq);
	}

	dev_dbg(dev, "src_clk_freq %u\n", src_clk_freq);
	dev_dbg(dev, "clk_freq     %u\n", clk_freq);

	hs_div = 3;
	if (clk_freq <= I2C_SPEED_STANDARD_RATE) {
		fs_div = ((src_clk_freq / clk_freq) / 2) - 3;
		qup->clk_ctl = (hs_div << 8) | (fs_div & 0xff);
	} else {
		/* 33%/66% duty cycle */
		fs_div = ((src_clk_freq / clk_freq) - 6) * 2 / 3;
		qup->clk_ctl = ((fs_div / 2) << 16) | (hs_div << 8) | (fs_div & 0xff);
	}

	dev_dbg(dev, "clk_ctl      %u\n", qup->clk_ctl);

	return 0;
}

/* Probe to see if a chip is present. */
static int qup_i2c_probe_chip(struct udevice *dev, uint chip_addr,
			      uint chip_flags)
{
	struct qup_i2c_priv *qup = dev_get_priv(dev);
	u32 hw_ver = readl(qup->base + QUP_HW_VERSION);

	return hw_ver ? 0 : -1;
}

static const struct dm_i2c_ops qup_i2c_ops = {
	.xfer		= qup_i2c_xfer_v2,
	.probe_chip	= qup_i2c_probe_chip,
	.set_bus_speed	= qup_i2c_set_bus_speed,
};

/*
 * Currently this driver only supports v2.x of QUP I2C controller, hence
 * functions above are named with a _v2 suffix. So when we have the
 * v1.1.1 support added as per the Linux counterpart then it should be easy
 * to add corresponding functions named with a _v1 suffix.
 */
static const struct udevice_id qup_i2c_ids[] = {
	{ .compatible = "qcom,i2c-qup-v2.1.1" },
	{ .compatible = "qcom,i2c-qup-v2.2.1" },
	{}
};

U_BOOT_DRIVER(i2c_qup) = {
	.name	= "i2c_qup",
	.id	= UCLASS_I2C,
	.of_match = qup_i2c_ids,
	.probe	= qup_i2c_probe,
	.priv_auto = sizeof(struct qup_i2c_priv),
	.ops	= &qup_i2c_ops,
};
