// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * Based on allwinner u-boot sources rsb code which is:
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * lixiang <lixiang@allwinnertech.com>
 */

#include <axp_pmic.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <sunxi_gpio.h>
#include <reset.h>
#include <time.h>
#include <asm/arch/cpu.h>
#include <asm/arch/prcm.h>
#include <asm/arch/rsb.h>

static int sun8i_rsb_await_trans(struct sunxi_rsb_reg *base)
{
	unsigned long tmo = timer_get_us() + 1000000;
	u32 stat;
	int ret;

	while (1) {
		stat = readl(&base->stat);
		if (stat & RSB_STAT_LBSY_INT) {
			ret = -EBUSY;
			break;
		}
		if (stat & RSB_STAT_TERR_INT) {
			ret = -EIO;
			break;
		}
		if (stat & RSB_STAT_TOVER_INT) {
			ret = 0;
			break;
		}
		if (timer_get_us() > tmo) {
			ret = -ETIME;
			break;
		}
	}
	writel(stat, &base->stat); /* Clear status bits */

	return ret;
}

static int sun8i_rsb_do_trans(struct sunxi_rsb_reg *base)
{
	setbits_le32(&base->ctrl, RSB_CTRL_START_TRANS);

	return sun8i_rsb_await_trans(base);
}

static int sun8i_rsb_read(struct sunxi_rsb_reg *base, u16 runtime_addr,
			  u8 reg_addr, u8 *data)
{
	int ret;

	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_addr), &base->devaddr);
	writel(reg_addr, &base->addr);
	writel(RSB_CMD_BYTE_READ, &base->cmd);

	ret = sun8i_rsb_do_trans(base);
	if (ret)
		return ret;

	*data = readl(&base->data) & 0xff;

	return 0;
}

static int sun8i_rsb_write(struct sunxi_rsb_reg *base, u16 runtime_addr,
			   u8 reg_addr, u8 data)
{
	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_addr), &base->devaddr);
	writel(reg_addr, &base->addr);
	writel(data, &base->data);
	writel(RSB_CMD_BYTE_WRITE, &base->cmd);

	return sun8i_rsb_do_trans(base);
}

static int sun8i_rsb_set_device_address(struct sunxi_rsb_reg *base,
					u16 device_addr, u16 runtime_addr)
{
	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_addr) |
	       RSB_DEVADDR_DEVICE_ADDR(device_addr), &base->devaddr);
	writel(RSB_CMD_SET_RTSADDR, &base->cmd);

	return sun8i_rsb_do_trans(base);
}

static void sun8i_rsb_set_clk(struct sunxi_rsb_reg *base)
{
	u32 div = 0;
	u32 cd_odly = 0;

	/* Source is Hosc24M, set RSB clk to 3Mhz */
	div = 24000000 / 3000000 / 2 - 1;
	cd_odly = div >> 1;
	if (!cd_odly)
		cd_odly = 1;

	writel((cd_odly << 8) | div, &base->ccr);
}

static int sun8i_rsb_set_device_mode(struct sunxi_rsb_reg *base)
{
	unsigned long tmo = timer_get_us() + 1000000;

	writel(RSB_DMCR_DEVICE_MODE_START | RSB_DMCR_DEVICE_MODE_DATA,
	       &base->dmcr);

	while (readl(&base->dmcr) & RSB_DMCR_DEVICE_MODE_START) {
		if (timer_get_us() > tmo)
			return -ETIME;
	}

	return sun8i_rsb_await_trans(base);
}

static int sun8i_rsb_init(struct sunxi_rsb_reg *base)
{
	writel(RSB_CTRL_SOFT_RST, &base->ctrl);
	sun8i_rsb_set_clk(base);

	return sun8i_rsb_set_device_mode(base);
}

#if IS_ENABLED(CONFIG_AXP_PMIC_BUS)
int rsb_read(const u16 runtime_addr, const u8 reg_addr, u8 *data)
{
	struct sunxi_rsb_reg *base = (struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	return sun8i_rsb_read(base, runtime_addr, reg_addr, data);
}

int rsb_write(const u16 runtime_addr, const u8 reg_addr, u8 data)
{
	struct sunxi_rsb_reg *base = (struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	return sun8i_rsb_write(base, runtime_addr, reg_addr, data);
}

int rsb_set_device_address(u16 device_addr, u16 runtime_addr)
{
	struct sunxi_rsb_reg *base = (struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	return sun8i_rsb_set_device_address(base, device_addr, runtime_addr);
}

int rsb_init(void)
{
	struct sunxi_rsb_reg *base = (struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	/* Enable RSB and PIO clk, and de-assert their resets */
	prcm_apb0_enable(PRCM_APB0_GATE_PIO | PRCM_APB0_GATE_RSB);

	if (IS_ENABLED(CONFIG_MACH_SUN9I)) {
		sunxi_gpio_set_cfgpin(SUNXI_GPN(0), SUN9I_GPN_R_RSB);
		sunxi_gpio_set_cfgpin(SUNXI_GPN(1), SUN9I_GPN_R_RSB);
		sunxi_gpio_set_pull(SUNXI_GPN(0), 1);
		sunxi_gpio_set_pull(SUNXI_GPN(1), 1);
		sunxi_gpio_set_drv(SUNXI_GPN(0), 2);
		sunxi_gpio_set_drv(SUNXI_GPN(1), 2);
	} else {
		sunxi_gpio_set_cfgpin(SUNXI_GPL(0), SUN8I_GPL_R_RSB);
		sunxi_gpio_set_cfgpin(SUNXI_GPL(1), SUN8I_GPL_R_RSB);
		sunxi_gpio_set_pull(SUNXI_GPL(0), 1);
		sunxi_gpio_set_pull(SUNXI_GPL(1), 1);
		sunxi_gpio_set_drv(SUNXI_GPL(0), 2);
		sunxi_gpio_set_drv(SUNXI_GPL(1), 2);
	}

	return sun8i_rsb_init(base);
}
#endif

#if CONFIG_IS_ENABLED(DM_I2C)
struct sun8i_rsb_priv {
	struct sunxi_rsb_reg *base;
};

/*
 * The mapping from hardware address to runtime address is fixed, and shared
 * among all RSB drivers. See the comment in drivers/bus/sunxi-rsb.c in Linux.
 */
static int sun8i_rsb_get_runtime_address(u16 device_addr)
{
	if (device_addr == AXP_PMIC_PRI_DEVICE_ADDR)
		return AXP_PMIC_PRI_RUNTIME_ADDR;
	if (device_addr == AXP_PMIC_SEC_DEVICE_ADDR)
		return AXP_PMIC_SEC_RUNTIME_ADDR;

	return -ENXIO;
}

static int sun8i_rsb_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	int runtime_addr = sun8i_rsb_get_runtime_address(msg->addr);
	struct sun8i_rsb_priv *priv = dev_get_priv(bus);

	if (runtime_addr < 0)
		return runtime_addr;

	/* The hardware only supports SMBus-style transfers. */
	if (nmsgs == 2 && msg[1].flags == I2C_M_RD && msg[1].len == 1)
		return sun8i_rsb_read(priv->base, runtime_addr,
				      msg[0].buf[0], &msg[1].buf[0]);

	if (nmsgs == 1 && msg[0].len == 2)
		return sun8i_rsb_write(priv->base, runtime_addr,
				       msg[0].buf[0], msg[0].buf[1]);

	return -EINVAL;
}

static int sun8i_rsb_probe_chip(struct udevice *bus, uint chip_addr,
				uint chip_flags)
{
	int runtime_addr = sun8i_rsb_get_runtime_address(chip_addr);
	struct sun8i_rsb_priv *priv = dev_get_priv(bus);

	if (runtime_addr < 0)
		return runtime_addr;

	return sun8i_rsb_set_device_address(priv->base, chip_addr, runtime_addr);
}

static int sun8i_rsb_probe(struct udevice *bus)
{
	struct sun8i_rsb_priv *priv = dev_get_priv(bus);
	struct reset_ctl *reset;
	struct clk *clk;

	priv->base = dev_read_addr_ptr(bus);

	reset = devm_reset_control_get(bus, NULL);
	if (!IS_ERR(reset))
		reset_deassert(reset);

	clk = devm_clk_get(bus, NULL);
	if (!IS_ERR(clk))
		clk_enable(clk);

	return sun8i_rsb_init(priv->base);
}

static int sun8i_rsb_child_pre_probe(struct udevice *child)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(child);
	struct udevice *bus = child->parent;

	/* Ensure each transfer is for a single register. */
	chip->flags |= DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS;

	return sun8i_rsb_probe_chip(bus, chip->chip_addr, 0);
}

static const struct dm_i2c_ops sun8i_rsb_ops = {
	.xfer		= sun8i_rsb_xfer,
	.probe_chip	= sun8i_rsb_probe_chip,
};

static const struct udevice_id sun8i_rsb_ids[] = {
	{ .compatible = "allwinner,sun8i-a23-rsb" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sun8i_rsb) = {
	.name			= "sun8i_rsb",
	.id			= UCLASS_I2C,
	.of_match		= sun8i_rsb_ids,
	.probe			= sun8i_rsb_probe,
	.child_pre_probe	= sun8i_rsb_child_pre_probe,
	.priv_auto		= sizeof(struct sun8i_rsb_priv),
	.ops			= &sun8i_rsb_ops,
};
#endif /* CONFIG_IS_ENABLED(DM_I2C) */
