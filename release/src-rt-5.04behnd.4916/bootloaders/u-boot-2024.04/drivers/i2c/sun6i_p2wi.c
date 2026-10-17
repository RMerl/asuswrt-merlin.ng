// SPDX-License-Identifier: GPL-2.0+
/*
 * Sunxi A31 Power Management Unit
 *
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 * http://linux-sunxi.org
 *
 * Based on sun6i sources and earlier U-Boot Allwinner A10 SPL work
 *
 * (C) Copyright 2006-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#include <axp_pmic.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <reset.h>
#include <sunxi_gpio.h>
#include <time.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/p2wi.h>
#include <asm/arch/prcm.h>
#include <asm/arch/sys_proto.h>

static int sun6i_p2wi_await_trans(struct sunxi_p2wi_reg *base)
{
	unsigned long tmo = timer_get_us() + 1000000;
	int ret;
	u8 reg;

	while (1) {
		reg = readl(&base->status);
		if (reg & P2WI_STAT_TRANS_ERR) {
			ret = -EIO;
			break;
		}
		if (reg & P2WI_STAT_TRANS_DONE) {
			ret = 0;
			break;
		}
		if (timer_get_us() > tmo) {
			ret = -ETIME;
			break;
		}
	}
	writel(reg, &base->status); /* Clear status bits */

	return ret;
}

static int sun6i_p2wi_read(struct sunxi_p2wi_reg *base, const u8 addr, u8 *data)
{
	int ret;

	writel(P2WI_DATADDR_BYTE_1(addr), &base->dataddr0);
	writel(P2WI_DATA_NUM_BYTES(1) |
	       P2WI_DATA_NUM_BYTES_READ, &base->numbytes);
	writel(P2WI_STAT_TRANS_DONE, &base->status);
	writel(P2WI_CTRL_TRANS_START, &base->ctrl);

	ret = sun6i_p2wi_await_trans(base);

	*data = readl(&base->data0) & P2WI_DATA_BYTE_1_MASK;

	return ret;
}

static int sun6i_p2wi_write(struct sunxi_p2wi_reg *base, const u8 addr, u8 data)
{
	writel(P2WI_DATADDR_BYTE_1(addr), &base->dataddr0);
	writel(P2WI_DATA_BYTE_1(data), &base->data0);
	writel(P2WI_DATA_NUM_BYTES(1), &base->numbytes);
	writel(P2WI_STAT_TRANS_DONE, &base->status);
	writel(P2WI_CTRL_TRANS_START, &base->ctrl);

	return sun6i_p2wi_await_trans(base);
}

static int sun6i_p2wi_change_to_p2wi_mode(struct sunxi_p2wi_reg *base,
					  u8 slave_addr, u8 ctrl_reg,
					  u8 init_data)
{
	unsigned long tmo = timer_get_us() + 1000000;

	writel(P2WI_PM_DEV_ADDR(slave_addr) |
	       P2WI_PM_CTRL_ADDR(ctrl_reg) |
	       P2WI_PM_INIT_DATA(init_data) |
	       P2WI_PM_INIT_SEND,
	       &base->pm);

	while ((readl(&base->pm) & P2WI_PM_INIT_SEND)) {
		if (timer_get_us() > tmo)
			return -ETIME;
	}

	return 0;
}

static void sun6i_p2wi_init(struct sunxi_p2wi_reg *base)
{
	/* Reset p2wi controller and set clock to CLKIN(12)/8 = 1.5 MHz */
	writel(P2WI_CTRL_RESET, &base->ctrl);
	sdelay(0x100);
	writel(P2WI_CC_SDA_OUT_DELAY(1) | P2WI_CC_CLK_DIV(8),
	       &base->cc);
}

#if IS_ENABLED(CONFIG_AXP_PMIC_BUS)
int p2wi_read(const u8 addr, u8 *data)
{
	struct sunxi_p2wi_reg *base = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;

	return sun6i_p2wi_read(base, addr, data);
}

int p2wi_write(const u8 addr, u8 data)
{
	struct sunxi_p2wi_reg *base = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;

	return sun6i_p2wi_write(base, addr, data);
}

int p2wi_change_to_p2wi_mode(u8 slave_addr, u8 ctrl_reg, u8 init_data)
{
	struct sunxi_p2wi_reg *base = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;

	return sun6i_p2wi_change_to_p2wi_mode(base, slave_addr, ctrl_reg,
					      init_data);
}

void p2wi_init(void)
{
	struct sunxi_p2wi_reg *base = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;

	/* Enable p2wi and PIO clk, and de-assert their resets */
	prcm_apb0_enable(PRCM_APB0_GATE_PIO | PRCM_APB0_GATE_P2WI);

	sunxi_gpio_set_cfgpin(SUNXI_GPL(0), SUN6I_GPL0_R_P2WI_SCK);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(1), SUN6I_GPL1_R_P2WI_SDA);

	sun6i_p2wi_init(base);
}
#endif

#if CONFIG_IS_ENABLED(DM_I2C)
struct sun6i_p2wi_priv {
	struct sunxi_p2wi_reg *base;
};

static int sun6i_p2wi_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct sun6i_p2wi_priv *priv = dev_get_priv(bus);

	/* The hardware only supports SMBus-style transfers. */
	if (nmsgs == 2 && msg[1].flags == I2C_M_RD && msg[1].len == 1)
		return sun6i_p2wi_read(priv->base,
				       msg[0].buf[0], &msg[1].buf[0]);

	if (nmsgs == 1 && msg[0].len == 2)
		return sun6i_p2wi_write(priv->base,
					msg[0].buf[0], msg[0].buf[1]);

	return -EINVAL;
}

static int sun6i_p2wi_probe_chip(struct udevice *bus, uint chip_addr,
				 uint chip_flags)
{
	struct sun6i_p2wi_priv *priv = dev_get_priv(bus);

	return sun6i_p2wi_change_to_p2wi_mode(priv->base, chip_addr,
					      AXP_PMIC_MODE_REG,
					      AXP_PMIC_MODE_P2WI);
}

static int sun6i_p2wi_probe(struct udevice *bus)
{
	struct sun6i_p2wi_priv *priv = dev_get_priv(bus);
	struct reset_ctl *reset;
	struct clk *clk;

	priv->base = dev_read_addr_ptr(bus);

	reset = devm_reset_control_get(bus, NULL);
	if (!IS_ERR(reset))
		reset_deassert(reset);

	clk = devm_clk_get(bus, NULL);
	if (!IS_ERR(clk))
		clk_enable(clk);

	sun6i_p2wi_init(priv->base);

	return 0;
}

static int sun6i_p2wi_child_pre_probe(struct udevice *child)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(child);
	struct udevice *bus = child->parent;

	/* Ensure each transfer is for a single register. */
	chip->flags |= DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS;

	return sun6i_p2wi_probe_chip(bus, chip->chip_addr, 0);
}

static const struct dm_i2c_ops sun6i_p2wi_ops = {
	.xfer		= sun6i_p2wi_xfer,
	.probe_chip	= sun6i_p2wi_probe_chip,
};

static const struct udevice_id sun6i_p2wi_ids[] = {
	{ .compatible = "allwinner,sun6i-a31-p2wi" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sun6i_p2wi) = {
	.name			= "sun6i_p2wi",
	.id			= UCLASS_I2C,
	.of_match		= sun6i_p2wi_ids,
	.probe			= sun6i_p2wi_probe,
	.child_pre_probe	= sun6i_p2wi_child_pre_probe,
	.priv_auto		= sizeof(struct sun6i_p2wi_priv),
	.ops			= &sun6i_p2wi_ops,
};
#endif /* CONFIG_IS_ENABLED(DM_I2C) */
