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

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/p2wi.h>
#include <asm/arch/prcm.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

void p2wi_init(void)
{
	struct sunxi_p2wi_reg *p2wi = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;

	/* Enable p2wi and PIO clk, and de-assert their resets */
	prcm_apb0_enable(PRCM_APB0_GATE_PIO | PRCM_APB0_GATE_P2WI);

	sunxi_gpio_set_cfgpin(SUNXI_GPL(0), SUN6I_GPL0_R_P2WI_SCK);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(1), SUN6I_GPL1_R_P2WI_SDA);

	/* Reset p2wi controller and set clock to CLKIN(12)/8 = 1.5 MHz */
	writel(P2WI_CTRL_RESET, &p2wi->ctrl);
	sdelay(0x100);
	writel(P2WI_CC_SDA_OUT_DELAY(1) | P2WI_CC_CLK_DIV(8),
	       &p2wi->cc);
}

int p2wi_change_to_p2wi_mode(u8 slave_addr, u8 ctrl_reg, u8 init_data)
{
	struct sunxi_p2wi_reg *p2wi = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;
	unsigned long tmo = timer_get_us() + 1000000;

	writel(P2WI_PM_DEV_ADDR(slave_addr) |
	       P2WI_PM_CTRL_ADDR(ctrl_reg) |
	       P2WI_PM_INIT_DATA(init_data) |
	       P2WI_PM_INIT_SEND,
	       &p2wi->pm);

	while ((readl(&p2wi->pm) & P2WI_PM_INIT_SEND)) {
		if (timer_get_us() > tmo)
			return -ETIME;
	}

	return 0;
}

static int p2wi_await_trans(void)
{
	struct sunxi_p2wi_reg *p2wi = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;
	unsigned long tmo = timer_get_us() + 1000000;
	int ret;
	u8 reg;

	while (1) {
		reg = readl(&p2wi->status);
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
	writel(reg, &p2wi->status); /* Clear status bits */
	return ret;
}

int p2wi_read(const u8 addr, u8 *data)
{
	struct sunxi_p2wi_reg *p2wi = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;
	int ret;

	writel(P2WI_DATADDR_BYTE_1(addr), &p2wi->dataddr0);
	writel(P2WI_DATA_NUM_BYTES(1) |
	       P2WI_DATA_NUM_BYTES_READ, &p2wi->numbytes);
	writel(P2WI_STAT_TRANS_DONE, &p2wi->status);
	writel(P2WI_CTRL_TRANS_START, &p2wi->ctrl);

	ret = p2wi_await_trans();

	*data = readl(&p2wi->data0) & P2WI_DATA_BYTE_1_MASK;
	return ret;
}

int p2wi_write(const u8 addr, u8 data)
{
	struct sunxi_p2wi_reg *p2wi = (struct sunxi_p2wi_reg *)SUN6I_P2WI_BASE;

	writel(P2WI_DATADDR_BYTE_1(addr), &p2wi->dataddr0);
	writel(P2WI_DATA_BYTE_1(data), &p2wi->data0);
	writel(P2WI_DATA_NUM_BYTES(1), &p2wi->numbytes);
	writel(P2WI_STAT_TRANS_DONE, &p2wi->status);
	writel(P2WI_CTRL_TRANS_START, &p2wi->ctrl);

	return p2wi_await_trans();
}
