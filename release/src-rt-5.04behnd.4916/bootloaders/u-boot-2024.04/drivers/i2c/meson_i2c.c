// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 - Beniamino Galvani <b.galvani@gmail.com>
 */
#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

#define I2C_TIMEOUT_MS		100

/* Control register fields */
#define REG_CTRL_START		BIT(0)
#define REG_CTRL_ACK_IGNORE	BIT(1)
#define REG_CTRL_STATUS		BIT(2)
#define REG_CTRL_ERROR		BIT(3)
#define REG_CTRL_CLKDIV_SHIFT	12
#define REG_CTRL_CLKDIV_MASK	GENMASK(21, 12)
#define REG_CTRL_CLKDIVEXT_SHIFT 28
#define REG_CTRL_CLKDIVEXT_MASK	GENMASK(29, 28)

enum {
	TOKEN_END = 0,
	TOKEN_START,
	TOKEN_SLAVE_ADDR_WRITE,
	TOKEN_SLAVE_ADDR_READ,
	TOKEN_DATA,
	TOKEN_DATA_LAST,
	TOKEN_STOP,
};

struct i2c_regs {
	u32 ctrl;
	u32 slave_addr;
	u32 tok_list0;
	u32 tok_list1;
	u32 tok_wdata0;
	u32 tok_wdata1;
	u32 tok_rdata0;
	u32 tok_rdata1;
};

struct meson_i2c_data {
	unsigned char div_factor;
};

struct meson_i2c {
	const struct meson_i2c_data *data;
	struct clk clk;
	struct i2c_regs *regs;
	struct i2c_msg *msg;	/* Current I2C message */
	bool last;		/* Whether the message is the last */
	uint count;		/* Number of bytes in the current transfer */
	uint pos;		/* Position of current transfer in message */
	u32 tokens[2];		/* Sequence of tokens to be written */
	uint num_tokens;	/* Number of tokens to be written */
};

static void meson_i2c_reset_tokens(struct meson_i2c *i2c)
{
	i2c->tokens[0] = 0;
	i2c->tokens[1] = 0;
	i2c->num_tokens = 0;
}

static void meson_i2c_add_token(struct meson_i2c *i2c, int token)
{
	if (i2c->num_tokens < 8)
		i2c->tokens[0] |= (token & 0xf) << (i2c->num_tokens * 4);
	else
		i2c->tokens[1] |= (token & 0xf) << ((i2c->num_tokens % 8) * 4);

	i2c->num_tokens++;
}

/*
 * Retrieve data for the current transfer (which can be at most 8
 * bytes) from the device internal buffer.
 */
static void meson_i2c_get_data(struct meson_i2c *i2c, u8 *buf, int len)
{
	u32 rdata0, rdata1;
	int i;

	rdata0 = readl(&i2c->regs->tok_rdata0);
	rdata1 = readl(&i2c->regs->tok_rdata1);

	debug("meson i2c: read data %08x %08x len %d\n", rdata0, rdata1, len);

	for (i = 0; i < min(4, len); i++)
		*buf++ = (rdata0 >> i * 8) & 0xff;

	for (i = 4; i < min(8, len); i++)
		*buf++ = (rdata1 >> (i - 4) * 8) & 0xff;
}

/*
 * Write data for the current transfer (which can be at most 8 bytes)
 * to the device internal buffer.
 */
static void meson_i2c_put_data(struct meson_i2c *i2c, u8 *buf, int len)
{
	u32 wdata0 = 0, wdata1 = 0;
	int i;

	for (i = 0; i < min(4, len); i++)
		wdata0 |= *buf++ << (i * 8);

	for (i = 4; i < min(8, len); i++)
		wdata1 |= *buf++ << ((i - 4) * 8);

	writel(wdata0, &i2c->regs->tok_wdata0);
	writel(wdata1, &i2c->regs->tok_wdata1);

	debug("meson i2c: write data %08x %08x len %d\n", wdata0, wdata1, len);
}

/*
 * Prepare the next transfer: pick the next 8 bytes in the remaining
 * part of message and write tokens and data (if needed) to the
 * device.
 */
static void meson_i2c_prepare_xfer(struct meson_i2c *i2c)
{
	bool write = !(i2c->msg->flags & I2C_M_RD);
	int i;

	i2c->count = min(i2c->msg->len - i2c->pos, 8u);

	for (i = 0; i + 1 < i2c->count; i++)
		meson_i2c_add_token(i2c, TOKEN_DATA);

	if (i2c->count) {
		if (write || i2c->pos + i2c->count < i2c->msg->len)
			meson_i2c_add_token(i2c, TOKEN_DATA);
		else
			meson_i2c_add_token(i2c, TOKEN_DATA_LAST);
	}

	if (write)
		meson_i2c_put_data(i2c, i2c->msg->buf + i2c->pos, i2c->count);

	if (i2c->last && i2c->pos + i2c->count >= i2c->msg->len)
		meson_i2c_add_token(i2c, TOKEN_STOP);

	writel(i2c->tokens[0], &i2c->regs->tok_list0);
	writel(i2c->tokens[1], &i2c->regs->tok_list1);
}

static void meson_i2c_do_start(struct meson_i2c *i2c, struct i2c_msg *msg)
{
	int token;

	token = (msg->flags & I2C_M_RD) ? TOKEN_SLAVE_ADDR_READ :
		TOKEN_SLAVE_ADDR_WRITE;

	writel(msg->addr << 1, &i2c->regs->slave_addr);
	meson_i2c_add_token(i2c, TOKEN_START);
	meson_i2c_add_token(i2c, token);
}

static int meson_i2c_xfer_msg(struct meson_i2c *i2c, struct i2c_msg *msg,
			      int last)
{
	ulong start;

	debug("meson i2c: %s addr %u len %u\n",
	      (msg->flags & I2C_M_RD) ? "read" : "write",
	      msg->addr, msg->len);

	i2c->msg = msg;
	i2c->last = last;
	i2c->pos = 0;
	i2c->count = 0;

	meson_i2c_reset_tokens(i2c);
	meson_i2c_do_start(i2c, msg);

	do {
		meson_i2c_prepare_xfer(i2c);

		/* start the transfer */
		setbits_le32(&i2c->regs->ctrl, REG_CTRL_START);
		start = get_timer(0);
		while (readl(&i2c->regs->ctrl) & REG_CTRL_STATUS) {
			if (get_timer(start) > I2C_TIMEOUT_MS) {
				clrbits_le32(&i2c->regs->ctrl, REG_CTRL_START);
				debug("meson i2c: timeout\n");
				return -ETIMEDOUT;
			}
			udelay(1);
		}
		meson_i2c_reset_tokens(i2c);
		clrbits_le32(&i2c->regs->ctrl, REG_CTRL_START);

		if (readl(&i2c->regs->ctrl) & REG_CTRL_ERROR) {
			debug("meson i2c: error\n");
			return -EREMOTEIO;
		}

		if ((msg->flags & I2C_M_RD) && i2c->count) {
			meson_i2c_get_data(i2c, i2c->msg->buf + i2c->pos,
					   i2c->count);
		}
		i2c->pos += i2c->count;
	} while (i2c->pos < msg->len);

	return 0;
}

static int meson_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			  int nmsgs)
{
	struct meson_i2c *i2c = dev_get_priv(bus);
	int i, ret = 0;

	for (i = 0; i < nmsgs; i++) {
		ret = meson_i2c_xfer_msg(i2c, msg + i, i == nmsgs - 1);
		if (ret)
			return ret;
	}

	return 0;
}

static int meson_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct meson_i2c *i2c = dev_get_priv(bus);
	ulong clk_rate;
	unsigned int div;

	clk_rate = clk_get_rate(&i2c->clk);
	if (IS_ERR_VALUE(clk_rate))
		return -EINVAL;

	div = DIV_ROUND_UP(clk_rate, speed * i2c->data->div_factor);

	/* clock divider has 12 bits */
	if (div >= (1 << 12)) {
		debug("meson i2c: requested bus frequency too low\n");
		div = (1 << 12) - 1;
	}

	clrsetbits_le32(&i2c->regs->ctrl, REG_CTRL_CLKDIV_MASK,
			(div & GENMASK(9, 0)) << REG_CTRL_CLKDIV_SHIFT);

	clrsetbits_le32(&i2c->regs->ctrl, REG_CTRL_CLKDIVEXT_MASK,
			(div >> 10) << REG_CTRL_CLKDIVEXT_SHIFT);

	debug("meson i2c: set clk %u, src %lu, div %u\n", speed, clk_rate, div);

	return 0;
}

static int meson_i2c_probe(struct udevice *bus)
{
	struct meson_i2c *i2c = dev_get_priv(bus);
	int ret;

	i2c->data = (const struct meson_i2c_data *)dev_get_driver_data(bus);

	ret = clk_get_by_index(bus, 0, &i2c->clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&i2c->clk);
	if (ret)
		return ret;

	i2c->regs = dev_read_addr_ptr(bus);
	clrbits_le32(&i2c->regs->ctrl, REG_CTRL_START);

	return 0;
}

static const struct dm_i2c_ops meson_i2c_ops = {
	.xfer          = meson_i2c_xfer,
	.set_bus_speed = meson_i2c_set_bus_speed,
};

static const struct meson_i2c_data i2c_meson6_data = {
	.div_factor = 4,
};

static const struct meson_i2c_data i2c_gxbb_data = {
	.div_factor = 4,
};

static const struct meson_i2c_data i2c_axg_data = {
	.div_factor = 3,
};

static const struct udevice_id meson_i2c_ids[] = {
	{.compatible = "amlogic,meson6-i2c", .data = (ulong)&i2c_meson6_data},
	{.compatible = "amlogic,meson-gx-i2c", .data = (ulong)&i2c_gxbb_data},
	{.compatible = "amlogic,meson-gxbb-i2c", .data = (ulong)&i2c_gxbb_data},
	{.compatible = "amlogic,meson-axg-i2c", .data = (ulong)&i2c_axg_data},
	{}
};

U_BOOT_DRIVER(i2c_meson) = {
	.name = "i2c_meson",
	.id   = UCLASS_I2C,
	.of_match = meson_i2c_ids,
	.probe = meson_i2c_probe,
	.priv_auto	= sizeof(struct meson_i2c),
	.ops = &meson_i2c_ops,
};
