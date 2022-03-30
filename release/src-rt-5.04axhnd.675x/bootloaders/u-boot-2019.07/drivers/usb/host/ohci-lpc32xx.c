// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2008 by NXP Semiconductors
 * @Author: Based on code by Kevin Wells
 * @Descr: USB driver - Embedded Artists LPC3250 OEM Board support functions
 *
 * Copyright (c) 2015 Tyco Fire Protection Products.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/i2c.h>
#include <usb.h>
#include <i2c.h>

/* OTG I2C controller module register structures */
struct otgi2c_regs {
	u32 otg_i2c_txrx;   /* OTG I2C Tx/Rx Data FIFO */
	u32 otg_i2c_stat;   /* OTG I2C Status Register */
	u32 otg_i2c_ctrl;   /* OTG I2C Control Register */
	u32 otg_i2c_clk_hi; /* OTG I2C Clock Divider high */
	u32 otg_i2c_clk_lo; /* OTG I2C Clock Divider low */
};

/* OTG controller module register structures */
struct otg_regs {
	u32 reserved1[64];
	u32 otg_int_sts;    /* OTG int status register */
	u32 otg_int_enab;   /* OTG int enable register */
	u32 otg_int_set;    /* OTG int set register */
	u32 otg_int_clr;    /* OTG int clear register */
	u32 otg_sts_ctrl;   /* OTG status/control register */
	u32 otg_timer;      /* OTG timer register */
	u32 reserved2[122];
	struct otgi2c_regs otg_i2c;
	u32 reserved3[824];
	u32 otg_clk_ctrl;   /* OTG clock control reg */
	u32 otg_clk_sts;    /* OTG clock status reg */
};

/* otg_sts_ctrl register definitions */
#define OTG_HOST_EN			(1 << 0) /* Enable host mode */

/* otg_clk_ctrl and otg_clk_sts register definitions */
#define OTG_CLK_AHB_EN			(1 << 4) /* Enable AHB clock */
#define OTG_CLK_OTG_EN			(1 << 3) /* Enable OTG clock */
#define OTG_CLK_I2C_EN			(1 << 2) /* Enable I2C clock */
#define OTG_CLK_HOST_EN			(1 << 0) /* Enable host clock */

/* ISP1301 USB transceiver I2C registers */
#define MC1_SPEED_REG			(1 << 0)
#define MC1_DAT_SE0			(1 << 2)
#define MC1_UART_EN			(1 << 6)

#define MC2_SPD_SUSP_CTRL		(1 << 1)
#define MC2_BI_DI			(1 << 2)
#define MC2_PSW_EN			(1 << 6)

#define OTG1_DP_PULLUP			(1 << 0)
#define OTG1_DM_PULLUP			(1 << 1)
#define OTG1_DP_PULLDOWN		(1 << 2)
#define OTG1_DM_PULLDOWN		(1 << 3)
#define OTG1_VBUS_DRV			(1 << 5)

#define ISP1301_I2C_ADDR		CONFIG_USB_ISP1301_I2C_ADDR

#define ISP1301_I2C_MODE_CONTROL_1_SET		0x04
#define ISP1301_I2C_MODE_CONTROL_1_CLR		0x05
#define ISP1301_I2C_MODE_CONTROL_2_SET		0x12
#define ISP1301_I2C_MODE_CONTROL_2_CLR		0x13
#define ISP1301_I2C_OTG_CONTROL_1_SET		0x06
#define ISP1301_I2C_OTG_CONTROL_1_CLR		0x07
#define ISP1301_I2C_INTERRUPT_LATCH_CLR		0x0B
#define ISP1301_I2C_INTERRUPT_FALLING_CLR	0x0D
#define ISP1301_I2C_INTERRUPT_RISING_CLR	0x0F

static struct otg_regs *otg = (struct otg_regs *)USB_BASE;
static struct clk_pm_regs *clk_pwr = (struct clk_pm_regs *)CLK_PM_BASE;

static int isp1301_set_value(struct udevice *dev, int reg, u8 value)
{
#ifndef CONFIG_DM_I2C
	return i2c_write(ISP1301_I2C_ADDR, reg, 1, &value, 1);
#else
	return dm_i2c_write(dev, reg, &value, 1);
#endif
}

static void isp1301_configure(struct udevice *dev)
{
#ifndef CONFIG_DM_I2C
	i2c_set_bus_num(I2C_2);
#endif

	/*
	 * LPC32XX only supports DAT_SE0 USB mode
	 * This sequence is important
	 */

	/* Disable transparent UART mode first */
	isp1301_set_value(dev, ISP1301_I2C_MODE_CONTROL_1_CLR, MC1_UART_EN);

	isp1301_set_value(dev, ISP1301_I2C_MODE_CONTROL_1_CLR, ~MC1_SPEED_REG);
	isp1301_set_value(dev, ISP1301_I2C_MODE_CONTROL_1_SET, MC1_SPEED_REG);
	isp1301_set_value(dev, ISP1301_I2C_MODE_CONTROL_2_CLR, ~0);
	isp1301_set_value(dev, ISP1301_I2C_MODE_CONTROL_2_SET,
			  MC2_BI_DI | MC2_PSW_EN | MC2_SPD_SUSP_CTRL);

	isp1301_set_value(dev, ISP1301_I2C_OTG_CONTROL_1_CLR, ~0);
	isp1301_set_value(dev, ISP1301_I2C_MODE_CONTROL_1_SET, MC1_DAT_SE0);
	isp1301_set_value(dev, ISP1301_I2C_OTG_CONTROL_1_SET,
			  OTG1_DM_PULLDOWN | OTG1_DP_PULLDOWN);
	isp1301_set_value(dev, ISP1301_I2C_OTG_CONTROL_1_CLR,
			  OTG1_DM_PULLUP | OTG1_DP_PULLUP);
	isp1301_set_value(dev, ISP1301_I2C_INTERRUPT_LATCH_CLR, ~0);
	isp1301_set_value(dev, ISP1301_I2C_INTERRUPT_FALLING_CLR, ~0);
	isp1301_set_value(dev, ISP1301_I2C_INTERRUPT_RISING_CLR, ~0);

	/* Enable usb_need_clk clock after transceiver is initialized */
	setbits_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_USBDVND_EN);
}

static int usbpll_setup(void)
{
	u32 ret;

	/* make sure clocks are disabled */
	clrbits_le32(&clk_pwr->usb_ctrl,
		     CLK_USBCTRL_CLK_EN1 | CLK_USBCTRL_CLK_EN2);

	/* start PLL clock input */
	setbits_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_CLK_EN1);

	/* Setup PLL. */
	setbits_le32(&clk_pwr->usb_ctrl,
		     CLK_USBCTRL_FDBK_PLUS1(192 - 1));
	setbits_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_POSTDIV_2POW(0x01));
	setbits_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_PLL_PWRUP);

	ret = wait_for_bit_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_PLL_STS,
				true, CONFIG_SYS_HZ, false);
	if (ret)
		return ret;

	/* enable PLL output */
	setbits_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_CLK_EN2);

	return 0;
}

int usb_cpu_init(void)
{
	u32 ret;
	struct udevice *dev = NULL;

#ifdef CONFIG_DM_I2C
	ret = i2c_get_chip_for_busnum(I2C_2, ISP1301_I2C_ADDR, 1, &dev);
	if (ret) {
		debug("%s: No bus %d\n", __func__, I2C_2);
		return ret;
	}
#endif

	/*
	 * USB pins routing setup is done by "lpc32xx_usb_init()" and should
	 * be call by board "board_init()" or "misc_init_r()" functions.
	 */

	/* enable AHB slave USB clock */
	setbits_le32(&clk_pwr->usb_ctrl,
		     CLK_USBCTRL_HCLK_EN | CLK_USBCTRL_BUS_KEEPER);

	/* enable I2C clock */
	writel(OTG_CLK_I2C_EN, &otg->otg_clk_ctrl);
	ret = wait_for_bit_le32(&otg->otg_clk_sts, OTG_CLK_I2C_EN, true,
				CONFIG_SYS_HZ, false);
	if (ret)
		return ret;

	/* Configure ISP1301 */
	isp1301_configure(dev);

	/* setup USB clocks and PLL */
	ret = usbpll_setup();
	if (ret)
		return ret;

	/* enable usb_host_need_clk */
	setbits_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_USBHSTND_EN);

	/* enable all needed USB clocks */
	const u32 mask = OTG_CLK_AHB_EN | OTG_CLK_OTG_EN |
			 OTG_CLK_I2C_EN | OTG_CLK_HOST_EN;
	writel(mask, &otg->otg_clk_ctrl);

	ret = wait_for_bit_le32(&otg->otg_clk_sts, mask, true,
				CONFIG_SYS_HZ, false);
	if (ret)
		return ret;

	setbits_le32(&otg->otg_sts_ctrl, OTG_HOST_EN);
	isp1301_set_value(dev, ISP1301_I2C_OTG_CONTROL_1_SET, OTG1_VBUS_DRV);

	return 0;
}

int usb_cpu_stop(void)
{
	struct udevice *dev = NULL;
	int ret = 0;

#ifdef CONFIG_DM_I2C
	ret = i2c_get_chip_for_busnum(I2C_2, ISP1301_I2C_ADDR, 1, &dev);
	if (ret) {
		debug("%s: No bus %d\n", __func__, I2C_2);
		return ret;
	}
#endif

	/* vbus off */
	isp1301_set_value(dev, ISP1301_I2C_OTG_CONTROL_1_SET, OTG1_VBUS_DRV);

	clrbits_le32(&otg->otg_sts_ctrl, OTG_HOST_EN);

	clrbits_le32(&clk_pwr->usb_ctrl, CLK_USBCTRL_HCLK_EN);

	return ret;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}
