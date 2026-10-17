// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright ASPEED Technology Inc.
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <i2c.h>
#include <log.h>
#include <regmap.h>
#include <reset.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include "ast2600_i2c.h"

/* Device private data */
struct ast2600_i2c_priv {
	struct clk clk;
	struct ast2600_i2c_regs *regs;
	void __iomem *global;
};

static int ast2600_i2c_read_data(struct ast2600_i2c_priv *priv, u8 chip_addr,
				 u8 *buffer, size_t len, bool send_stop)
{
	int rx_cnt, ret = 0;
	u32 cmd, isr;

	for (rx_cnt = 0; rx_cnt < len; rx_cnt++, buffer++) {
		cmd = I2CM_PKT_EN | I2CM_PKT_ADDR(chip_addr) |
		      I2CM_RX_CMD;
		if (!rx_cnt)
			cmd |= I2CM_START_CMD;

		if ((len - 1) == rx_cnt)
			cmd |= I2CM_RX_CMD_LAST;

		if (send_stop && ((len - 1) == rx_cnt))
			cmd |= I2CM_STOP_CMD;

		writel(cmd, &priv->regs->cmd_sts);

		ret = readl_poll_timeout(&priv->regs->isr, isr,
					 isr & I2CM_PKT_DONE,
					 I2C_TIMEOUT_US);
		if (ret)
			return -ETIMEDOUT;

		*buffer =
			I2CC_GET_RX_BUFF(readl(&priv->regs->trx_buff));

		writel(I2CM_PKT_DONE, &priv->regs->isr);

		if (isr & I2CM_TX_NAK)
			return -EREMOTEIO;
	}

	return 0;
}

static int ast2600_i2c_write_data(struct ast2600_i2c_priv *priv, u8 chip_addr,
				  u8 *buffer, size_t len, bool send_stop)
{
	int tx_cnt, ret = 0;
	u32 cmd, isr;

	if (!len) {
		cmd = I2CM_PKT_EN | I2CM_PKT_ADDR(chip_addr) |
		      I2CM_START_CMD;
		writel(cmd, &priv->regs->cmd_sts);
		ret = readl_poll_timeout(&priv->regs->isr, isr,
					 isr & I2CM_PKT_DONE,
					 I2C_TIMEOUT_US);
		if (ret)
			return -ETIMEDOUT;

		writel(I2CM_PKT_DONE, &priv->regs->isr);

		if (isr & I2CM_TX_NAK)
			return -EREMOTEIO;
	}

	for (tx_cnt = 0; tx_cnt < len; tx_cnt++, buffer++) {
		cmd = I2CM_PKT_EN | I2CM_PKT_ADDR(chip_addr);
		cmd |= I2CM_TX_CMD;

		if (!tx_cnt)
			cmd |= I2CM_START_CMD;

		if (send_stop && ((len - 1) == tx_cnt))
			cmd |= I2CM_STOP_CMD;

		writel(*buffer, &priv->regs->trx_buff);
		writel(cmd, &priv->regs->cmd_sts);
		ret = readl_poll_timeout(&priv->regs->isr, isr,
					 isr & I2CM_PKT_DONE,
					 I2C_TIMEOUT_US);
		if (ret)
			return -ETIMEDOUT;

		writel(I2CM_PKT_DONE, &priv->regs->isr);

		if (isr & I2CM_TX_NAK)
			return -EREMOTEIO;
	}

	return 0;
}

static int ast2600_i2c_deblock(struct udevice *dev)
{
	struct ast2600_i2c_priv *priv = dev_get_priv(dev);
	u32 csr = readl(&priv->regs->cmd_sts);
	u32 isr;
	int ret;

	/* reinit */
	writel(0, &priv->regs->fun_ctrl);
	/* Enable Master Mode. Assuming single-master */
	writel(I2CC_BUS_AUTO_RELEASE | I2CC_MASTER_EN |
		       I2CC_MULTI_MASTER_DIS,
	       &priv->regs->fun_ctrl);

	csr = readl(&priv->regs->cmd_sts);

	if (!(csr & I2CC_SDA_LINE_STS) &&
	    (csr & I2CC_SCL_LINE_STS)) {
		debug("Bus stuck (%x), attempting recovery\n", csr);
		writel(I2CM_RECOVER_CMD_EN, &priv->regs->cmd_sts);
		ret = readl_poll_timeout(&priv->regs->isr, isr,
					 isr & (I2CM_BUS_RECOVER_FAIL |
						I2CM_BUS_RECOVER),
					 I2C_TIMEOUT_US);
		writel(~0, &priv->regs->isr);
		if (ret)
			return -EREMOTEIO;
	}

	return 0;
}

static int ast2600_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct ast2600_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	if (readl(&priv->regs->trx_buff) & I2CC_BUS_BUSY_STS)
		return -EREMOTEIO;

	for (; nmsgs > 0; nmsgs--, msg++) {
		if (msg->flags & I2C_M_RD) {
			debug("i2c_read: chip=0x%x, len=0x%x, flags=0x%x\n",
			      msg->addr, msg->len, msg->flags);
			ret = ast2600_i2c_read_data(priv, msg->addr, msg->buf,
						    msg->len, (nmsgs == 1));
		} else {
			debug("i2c_write: chip=0x%x, len=0x%x, flags=0x%x\n",
			      msg->addr, msg->len, msg->flags);
			ret = ast2600_i2c_write_data(priv, msg->addr, msg->buf,
						     msg->len, (nmsgs == 1));
		}
		if (ret) {
			debug("%s: error (%d)\n", __func__, ret);
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int ast2600_i2c_set_speed(struct udevice *dev, unsigned int speed)
{
	struct ast2600_i2c_priv *priv = dev_get_priv(dev);
	unsigned long base_clk1, base_clk2, base_clk3, base_clk4;
	int multiply = 10;
	int baseclk_idx;
	u32 clk_div_reg;
	u32 apb_clk;
	u32 scl_low;
	u32 scl_high;
	int divisor;
	int inc = 0;
	u32 data;

	debug("Setting speed for I2C%d to <%u>\n", dev->seq_, speed);
	if (!speed) {
		debug("No valid speed specified\n");
		return -EINVAL;
	}

	apb_clk = clk_get_rate(&priv->clk);

	clk_div_reg = readl(priv->global + I2CG_CLK_DIV_CTRL);

	base_clk1 = (apb_clk * multiply) / (((GET_CLK1_DIV(clk_div_reg) + 2) * multiply) / 2);
	base_clk2 = (apb_clk * multiply) / (((GET_CLK2_DIV(clk_div_reg) + 2) * multiply) / 2);
	base_clk3 = (apb_clk * multiply) / (((GET_CLK3_DIV(clk_div_reg) + 2) * multiply) / 2);
	base_clk4 = (apb_clk * multiply) / (((GET_CLK4_DIV(clk_div_reg) + 2) * multiply) / 2);

	if ((apb_clk / speed) <= 32) {
		baseclk_idx = 0;
		divisor = DIV_ROUND_UP(apb_clk, speed);
	} else if ((base_clk1 / speed) <= 32) {
		baseclk_idx = 1;
		divisor = DIV_ROUND_UP(base_clk1, speed);
	} else if ((base_clk2 / speed) <= 32) {
		baseclk_idx = 2;
		divisor = DIV_ROUND_UP(base_clk2, speed);
	} else if ((base_clk3 / speed) <= 32) {
		baseclk_idx = 3;
		divisor = DIV_ROUND_UP(base_clk3, speed);
	} else {
		baseclk_idx = 4;
		divisor = DIV_ROUND_UP(base_clk4, speed);
		inc = 0;
		while ((divisor + inc) > 32) {
			inc |= divisor & 0x1;
			divisor >>= 1;
			baseclk_idx++;
		}
		divisor += inc;
	}
	divisor = min_t(int, divisor, 32);
	baseclk_idx &= 0xf;
	scl_low = ((divisor * 9) / 16) - 1;
	scl_low = min_t(u32, scl_low, 0xf);
	scl_high = (divisor - scl_low - 2) & 0xf;
	/* Divisor : Base Clock : tCKHighMin : tCK High : tCK Low  */
	data = ((scl_high - 1) << 20) | (scl_high << 16) | (scl_low << 12) |
	       baseclk_idx;
	/* Set AC Timing */
	writel(data, &priv->regs->ac_timing);

	return 0;
}

static int ast2600_i2c_probe(struct udevice *dev)
{
	struct ast2600_i2c_priv *priv = dev_get_priv(dev);
	ofnode i2c_global_node;

	/* find global base address */
	i2c_global_node = ofnode_get_parent(dev_ofnode(dev));
	priv->global = (void *)ofnode_get_addr(i2c_global_node);
	if (IS_ERR(priv->global)) {
		debug("%s(): can't get global\n", __func__);
		return PTR_ERR(priv->global);
	}

	/* Reset device */
	writel(0, &priv->regs->fun_ctrl);
	/* Enable Master Mode. Assuming single-master */
	writel(I2CC_BUS_AUTO_RELEASE | I2CC_MASTER_EN |
		       I2CC_MULTI_MASTER_DIS,
	       &priv->regs->fun_ctrl);

	writel(0, &priv->regs->ier);
	/* Clear Interrupt */
	writel(~0, &priv->regs->isr);

	return 0;
}

static int ast2600_i2c_of_to_plat(struct udevice *dev)
{
	struct ast2600_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0) {
		debug("%s: Can't get clock for %s: %d\n", __func__, dev->name,
		      ret);
		return ret;
	}

	return 0;
}

static const struct dm_i2c_ops ast2600_i2c_ops = {
	.xfer = ast2600_i2c_xfer,
	.deblock = ast2600_i2c_deblock,
	.set_bus_speed = ast2600_i2c_set_speed,
};

static const struct udevice_id ast2600_i2c_ids[] = {
	{ .compatible = "aspeed,ast2600-i2c" },
	{},
};

U_BOOT_DRIVER(ast2600_i2c) = {
	.name = "ast2600_i2c",
	.id = UCLASS_I2C,
	.of_match = ast2600_i2c_ids,
	.probe = ast2600_i2c_probe,
	.of_to_plat = ast2600_i2c_of_to_plat,
	.priv_auto = sizeof(struct ast2600_i2c_priv),
	.ops = &ast2600_i2c_ops,
};

struct ast2600_i2c_global_priv {
	void __iomem *regs;
	struct reset_ctl reset;
};

/*
 * APB clk : 100Mhz
 * div  : scl       : baseclk [APB/((div/2) + 1)] : tBuf [1/bclk * 16]
 * I2CG10[31:24] base clk4 for i2c auto recovery timeout counter (0xC6)
 * I2CG10[23:16] base clk3 for Standard-mode (100Khz) min tBuf 4.7us
 * 0x3c : 100.8Khz  : 3.225Mhz                    : 4.96us
 * 0x3d : 99.2Khz   : 3.174Mhz                    : 5.04us
 * 0x3e : 97.65Khz  : 3.125Mhz                    : 5.12us
 * 0x40 : 97.75Khz  : 3.03Mhz                     : 5.28us
 * 0x41 : 99.5Khz   : 2.98Mhz                     : 5.36us (default)
 * I2CG10[15:8] base clk2 for Fast-mode (400Khz) min tBuf 1.3us
 * 0x12 : 400Khz    : 10Mhz                       : 1.6us
 * I2CG10[7:0] base clk1 for Fast-mode Plus (1Mhz) min tBuf 0.5us
 * 0x08 : 1Mhz      : 20Mhz                       : 0.8us
 */

static int aspeed_i2c_global_probe(struct udevice *dev)
{
	struct ast2600_i2c_global_priv *i2c_global = dev_get_priv(dev);
	void __iomem *regs;
	int ret = 0;

	i2c_global->regs = dev_read_addr_ptr(dev);
	if (!i2c_global->regs)
		return -EINVAL;

	debug("%s(dev=%p)\n", __func__, dev);

	regs = i2c_global->regs;

	ret = reset_get_by_index(dev, 0, &i2c_global->reset);
	if (ret) {
		printf("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	reset_deassert(&i2c_global->reset);

	writel(GLOBAL_INIT, regs + I2CG_CTRL);
	writel(I2CCG_DIV_CTRL, regs + I2CG_CLK_DIV_CTRL);

	return 0;
}

static const struct udevice_id aspeed_i2c_global_ids[] = {
	{	.compatible = "aspeed,ast2600-i2c-global",	},
	{ }
};

U_BOOT_DRIVER(aspeed_i2c_global) = {
	.name		= "aspeed_i2c_global",
	.id			= UCLASS_MISC,
	.of_match	= aspeed_i2c_global_ids,
	.probe		= aspeed_i2c_global_probe,
	.priv_auto  = sizeof(struct ast2600_i2c_global_priv),
};
