// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * Based on allwinner u-boot sources rsb code which is:
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * lixiang <lixiang@allwinnertech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/prcm.h>
#include <asm/arch/rsb.h>

static int rsb_set_device_mode(void);

static void rsb_cfg_io(void)
{
#ifdef CONFIG_MACH_SUN8I
	sunxi_gpio_set_cfgpin(SUNXI_GPL(0), SUN8I_GPL_R_RSB);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(1), SUN8I_GPL_R_RSB);
	sunxi_gpio_set_pull(SUNXI_GPL(0), 1);
	sunxi_gpio_set_pull(SUNXI_GPL(1), 1);
	sunxi_gpio_set_drv(SUNXI_GPL(0), 2);
	sunxi_gpio_set_drv(SUNXI_GPL(1), 2);
#elif defined CONFIG_MACH_SUN9I
	sunxi_gpio_set_cfgpin(SUNXI_GPN(0), SUN9I_GPN_R_RSB);
	sunxi_gpio_set_cfgpin(SUNXI_GPN(1), SUN9I_GPN_R_RSB);
	sunxi_gpio_set_pull(SUNXI_GPN(0), 1);
	sunxi_gpio_set_pull(SUNXI_GPN(1), 1);
	sunxi_gpio_set_drv(SUNXI_GPN(0), 2);
	sunxi_gpio_set_drv(SUNXI_GPN(1), 2);
#else
#error unsupported MACH_SUNXI
#endif
}

static void rsb_set_clk(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	u32 div = 0;
	u32 cd_odly = 0;

	/* Source is Hosc24M, set RSB clk to 3Mhz */
	div = 24000000 / 3000000 / 2 - 1;
	cd_odly = div >> 1;
	if (!cd_odly)
		cd_odly = 1;

	writel((cd_odly << 8) | div, &rsb->ccr);
}

int rsb_init(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	/* Enable RSB and PIO clk, and de-assert their resets */
	prcm_apb0_enable(PRCM_APB0_GATE_PIO | PRCM_APB0_GATE_RSB);

	/* Setup external pins */
	rsb_cfg_io();

	writel(RSB_CTRL_SOFT_RST, &rsb->ctrl);
	rsb_set_clk();

	return rsb_set_device_mode();
}

static int rsb_await_trans(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	unsigned long tmo = timer_get_us() + 1000000;
	u32 stat;
	int ret;

	while (1) {
		stat = readl(&rsb->stat);
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
	writel(stat, &rsb->stat); /* Clear status bits */

	return ret;
}

static int rsb_set_device_mode(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	unsigned long tmo = timer_get_us() + 1000000;

	writel(RSB_DMCR_DEVICE_MODE_START | RSB_DMCR_DEVICE_MODE_DATA,
	       &rsb->dmcr);

	while (readl(&rsb->dmcr) & RSB_DMCR_DEVICE_MODE_START) {
		if (timer_get_us() > tmo)
			return -ETIME;
	}

	return rsb_await_trans();
}

static int rsb_do_trans(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	setbits_le32(&rsb->ctrl, RSB_CTRL_START_TRANS);
	return rsb_await_trans();
}

int rsb_set_device_address(u16 device_addr, u16 runtime_addr)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_addr) |
	       RSB_DEVADDR_DEVICE_ADDR(device_addr), &rsb->devaddr);
	writel(RSB_CMD_SET_RTSADDR, &rsb->cmd);

	return rsb_do_trans();
}

int rsb_write(const u16 runtime_device_addr, const u8 reg_addr, u8 data)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_device_addr), &rsb->devaddr);
	writel(reg_addr, &rsb->addr);
	writel(data, &rsb->data);
	writel(RSB_CMD_BYTE_WRITE, &rsb->cmd);

	return rsb_do_trans();
}

int rsb_read(const u16 runtime_device_addr, const u8 reg_addr, u8 *data)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	int ret;

	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_device_addr), &rsb->devaddr);
	writel(reg_addr, &rsb->addr);
	writel(RSB_CMD_BYTE_READ, &rsb->cmd);

	ret = rsb_do_trans();
	if (ret)
		return ret;

	*data = readl(&rsb->data) & 0xff;

	return 0;
}
