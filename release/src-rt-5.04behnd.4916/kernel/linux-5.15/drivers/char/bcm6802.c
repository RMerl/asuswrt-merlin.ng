/*
 * Copyright (C) 2017 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)            KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/printk.h>
#include <linux/scatterlist.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_net.h>
#include <linux/of_gpio.h>
#include <linux/dma-mapping.h>
#include <linux/phy.h>

#include <linux/bbsi.h>
#include <linux/bmoca.h>

const char bcm6802_driver_name[] = "bcm6802";

#define REG_RD(x)			((u32)bcm6802_read32(priv, (u32)(x)))
#define REG_WR(x, y)			bcm6802_write32(priv, (u32)(x), (y))
#define REG_WR_BLOCK(addr, src, len)	bcm6802_writebuf(priv, addr, src, len)
#define REG_RD_BLOCK(addr, dst, len)	bcm6802_readbuf(priv, addr, dst, len)

#define REG_SET(x, y)			REG_WR(x, REG_RD(x) | (y))
#define REG_UNSET(x, y)			REG_WR(x, REG_RD(x) & ~(y))

#define LEAP_HOST_L1_INTR_MASK_CLEAR		0x100b0318

#define CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3	0x1010000c
#define CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_0	0x1010003c
#define CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_3	0x10100048
#define CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5	0x10100050
#define CLKGEN_PLL_SYS1_PLL_DIV			0x10100058
#define CLKGEN_PLL_SYS1_PLL_LOCK_STATUS		0x10100060
#define CLKGEN_PLL_SYS1_PLL_RESET		0x1010006C
#define CLKGEN_PLL_SYS1_PLL_PWRDN		0x10100068
#define CLKGEN_PLL_SYS1_PLL_SSC_MODE_CONTROL_HIGH 0x10100070
#define CLKGEN_PLL_SYS1_PLL_SSC_MODE_CONTROL_LOW 0x10100074
#define CLKGEN_LEAP_TOP_INST_CLOCK_DISABLE	0x101000d4
#define CLKGEN_LEAP_TOP_INST_DATA		0x101000e4
#define CLKGEN_LEAP_TOP_INST_HAB		0x101000e8
#define CLKGEN_LEAP_TOP_INST_PROG0		0x101000ec
#define CLKGEN_LEAP_TOP_INST_PROG1		0x101000f0
#define CLKGEN_LEAP_TOP_INST_PROG2		0x101000f4
#define CLKGEN_LEAP_TOP_INST_ROM		0x101000f8
#define CLKGEN_LEAP_TOP_INST_SHARED		0x101000fc
#define CLKGEN_PAD_CLOCK_DISABLE		0x1010013c
#define CLKGEN_SYS_CTRL_INST_POWER_SWITCH_MEMORY 0x10100164

#define SUN_TOP_CTRL_CHIP_FAMILY_ID		0x10404000
#define SUN_TOP_CTRL_PRODUCT_ID			0x10404004
#define SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_0	0x104040a4
#define SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_1	0x104040a8
#define SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_5	0x104040b8
#define SUN_TOP_CTRL_PIN_MUX_PAD_CTRL_3		0x10404128
#define SUN_TOP_CTRL_SW_INIT_0_CLEAR		0x1040431C
#define SUN_TOP_CTRL_SW_INIT_1_CLEAR		0x10404334
#define SUN_TOP_CTRL_PIN_MUX_CTRL_0		0x10404100
#define SUN_TOP_CTRL_PIN_MUX_CTRL_1		0x10404104
#define SUN_TOP_CTRL_PIN_MUX_CTRL_2		0x10404108
#define SUN_TOP_CTRL_PIN_MUX_CTRL_3		0x1040410c
#define SUN_TOP_CTRL_PIN_MUX_CTRL_4		0x10404110
#define SUN_TOP_CTRL_PIN_MUX_CTRL_5		0x10404114
#define SUN_TOP_CTRL_PIN_MUX_CTRL_6		0x10404118

#define PM_CLK_CTRL				0x10406184
#define PM_CONFIG				0x10406180

#define EPORT_REG_EMUX_CNTRL			0x10800000
#define EPORT_REG_GPHY_CNTRL			0x10800004
#define EPORT_REG_RGMII_0_CNTRL			0x1080000c
#define EPORT_REG_RGMII_0_RX_CLOCK_DELAY_CNTRL	0x10800014
#define EPORT_REG_RGMII_1_CNTRL			0x10800018
#define EPORT_REG_RGMII_1_RX_CLOCK_DELAY_CNTRL	0x10800020
#define EPORT_REG_LED_CNTRL			0x10800024

#define EPORT_UMAC_RX_MAX_PKT_SIZE_OFFSET	0x10800e08
#define EPORT_UMAC_FRM_LEN_OFFSET		0x10800814

#define MOCA_DATA_MEM				0x10600000
#define MOCA_CNTL_MEM				0x10708000
#define MOCA_L2_CPU_CLEAR			0x107ffc48
#define MOCA_L2_MASK_SET			0x107ffc50

#define MOCA_PMB_MASTER_WDATA			0x107ffcc8
#define MOCA_PMB_MASTER_CMD			0x107ffccc
#define MOCA_HOSTMISC_MISC_CTRL			0x107ffd00
#define MOCA_HOSTMISC_H2M_INT_TRIG		0x107ffd0c
#define MOCA_HOSTMISC_SUBSYS_CFG		0x107ffd14


/* Private data used by the bcm6802 module */
struct bcm6802_priv {
	struct spi_device	*spi_device;
	struct device		*dev;
	u32			chip_id;
	struct mutex		copy_mutex;
	u32			eport_mux;
	u32			gphy_en;
	int			irq;
	struct gpio_desc	*gpio_desc;
};

static const struct moca_regs regs_6802 = {
	.data_mem_offset		= 0,
	.data_mem_size			= (640 * 1024),
	.cntl_mem_offset		= 0x00108000,
	.cntl_mem_size			= (384 * 1024),
	.gp0_offset			= 0,
	.gp1_offset			= 0,
	.ringbell_offset		= 0x001ffd0c,
	.l2_status_offset		= 0x001ffc40,
	.l2_clear_offset		= 0x001ffc48,
	.l2_mask_set_offset		= 0x001ffc50,
	.l2_mask_clear_offset		= 0x001ffc54,
	.sw_reset_offset		= 0x001ffd00,
	.led_ctrl_offset		= 0,
	.m2m_src_offset			= 0x001ffc00,
	.m2m_dst_offset			= 0x001ffc04,
	.m2m_cmd_offset			= 0x001ffc08,
	.m2m_status_offset		= 0x001ffc0c,
	.m2m_src_high_offset		= 0x001ffc10,
	.m2m_dst_high_offset		= 0x001ffc14,
	.moca2host_mmp_inbox_0_offset	= 0x001ffd58,
	.moca2host_mmp_inbox_1_offset	= 0x001ffd5c,
	.moca2host_mmp_inbox_2_offset	= 0x001ffd60,
	.extras_mmp_outbox_3_offset	= 0x001fec3c,
	.h2m_resp_bit[1]		= 0x10,
	.h2m_req_bit[1]			= 0x20,
	.h2m_resp_bit[0]		= 0x1,
	.h2m_req_bit[0]			= 0x2,
	.sideband_gmii_fc_offset	= 0x001fec18
};

#define MOCA_BPCM_NUM		5
#define MOCA_BPCM_ZONES_NUM	8

#define MOCA_CPU_CLOCK_NUM	1
#define MOCA_PHY_CLOCK_NUM	2

enum PMB_COMMAND_E {
	PMB_COMMAND_PHY1_ON = 0,
	PMB_COMMAND_PARTIAL_ON,
	PMB_COMMAND_PHY1_OFF,
	PMB_COMMAND_ALL_OFF,

	PMB_COMMAND_LAST
};

enum PMB_GIVE_OWNERSHIP_E {
	PMB_GIVE_OWNERSHIP_2_HOST = 0,
	PMB_GIVE_OWNERSHIP_2_FW,

	PMB_GET_OWNERSHIP_LAST
};

static u32 zone_all_off_bitmask[MOCA_BPCM_NUM] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static u32 zone_partial_on_bitmask[MOCA_BPCM_NUM] = {
	0x41, 0xFC, 0xFF, 0xFF, 0x00 };
static u32 zone_phy1_bitmask[MOCA_BPCM_NUM] = {
	0x00, 0x00, 0x00, 0x00, 0xFF };

struct moca_6802c0_clock_params {
	u32 cpu_hz;
	u32 pdiv;
	u32 ndiv;
	u32 pll_mdivs[6];
};

#define NUM_6802C0_CLOCK_OPTIONS 2

struct moca_6802c0_clock_params
	moca_6802c0_clock_params[NUM_6802C0_CLOCK_OPTIONS] = {
	{			/* VCO of 2200, default */
		440000000,		/* cpu_hz */
		1,			/* pdiv */
		44,			/* ndiv */
		{5, 22, 7, 7, 44, 44}	/* pll_mdivs[6] */
	},
	{			/* VCO of 2400 */
		400000000,		/* cpu_hz */
		1,			/* pdiv */
		48,			/* ndiv */
		{6, 24, 8, 8, 48, 48}	/* pll_mdivs[6] */
	},
};

int bcm6802_writebuf(void *hw_priv, unsigned long addr,
		     u32 *data, unsigned long len)
{
	struct bcm6802_priv *priv = hw_priv;
	int rc;

	mutex_lock(&priv->copy_mutex);
	rc = bbsi_writebuf(priv->spi_device, addr, data, len);
	mutex_unlock(&priv->copy_mutex);

	return rc;
}

int bcm6802_readbuf(void *hw_priv, unsigned long addr,
		    u32 *data, unsigned long len)
{
	struct bcm6802_priv *priv = hw_priv;
	int rc;

	mutex_lock(&priv->copy_mutex);
	rc = bbsi_readbuf(priv->spi_device, addr, data, len);
	mutex_unlock(&priv->copy_mutex);

	return rc;
}

unsigned int  bcm6802_read32(void *hw_priv, uintptr_t addr)
{
	struct bcm6802_priv *priv = hw_priv;
	unsigned int rc;

	mutex_lock(&priv->copy_mutex);
	rc = bbsi_read32(priv->spi_device, addr);
	mutex_unlock(&priv->copy_mutex);

	return rc;
}

void bcm6802_write32(void *hw_priv, uintptr_t addr, u32 data)
{
	struct bcm6802_priv *priv = hw_priv;

	mutex_lock(&priv->copy_mutex);
	bbsi_write32(priv->spi_device, addr, data);
	mutex_unlock(&priv->copy_mutex);
}

static void bcm6802_clk_set_rate(struct bcm6802_priv *priv)
{
	/* The REG_RD/REG_WR macros need a valid 'priv->pdev->dev' */
	struct moca_6802c0_clock_params *p_clock_data =
		&moca_6802c0_clock_params[0];
	u32 i;
	u32 addr;
	u32 data;

	/* 1. Set POST_DIVIDER_HOLD_CHx (bit [12] in each PLL_CHANNEL_CTRL_CH_x
	      register. This will zero the output channels */
	for (addr = CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_0;
	     addr <= CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5; addr += 4)
		REG_SET(addr, (1 << 12));

	/* 2. Program new PDIV/NDIV value, this will lose lock and
	     trigger a new PLL lock process for a new VCO frequency */
	REG_WR(CLKGEN_PLL_SYS1_PLL_DIV,
		((p_clock_data->pdiv << 10) | p_clock_data->ndiv));

	/* 3. Wait >10 usec for lock time. Max lock time per data sheet is
	      460/Fref. Alternatively monitor CLKGEN_PLL_SYS*_PLL_LOCK_STATUS
	      to check if PLL has locked */
	data = 0;
	i = 0;
	while ((data & 0x1) == 0) {
		/* This typically is only read once */
		data = REG_RD(CLKGEN_PLL_SYS1_PLL_LOCK_STATUS);

		if (i++ > 10) {
			dev_err(priv->dev, "MoCA SYS1 PLL NOT LOCKED!\n");
			break;
		}
	}

	/* 4. Configure new MDIV value along with set POST_DIVIDER_LOAD_EN_CHx
	      (bit [13]=1, while keep bit[12]=1) in each PLL_CHANNEL_CTRL_CH_x
	      register */
	i = 0;
	for (addr = CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_0;
	     addr <= CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5; addr += 4) {
		data = REG_RD(addr);
		data |= (1 << 13);
		data &= ~(0xFF << 1);
		data |= (p_clock_data->pll_mdivs[i] << 1);
		REG_WR(addr, data);
		i++;
	}

	/* 5. Clear bits [12] and bit [13] in each PLL_CHANNEL_CTRL_CH_x */
	for (addr = CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_0;
	     addr <= CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5; addr += 4)
		REG_UNSET(addr, ((1 << 13) | (1 << 12)));
}

static void bcm6802_pmb_delay(struct bcm6802_priv *priv)
{
	unsigned int data;
	unsigned int i, j;

	REG_WR(MOCA_PMB_MASTER_WDATA, 0xFF444000);

	for (i = 0; i < MOCA_BPCM_NUM; i++) {
		for (j = 0; j < MOCA_BPCM_ZONES_NUM; j++) {
			data = 0x100012 + j * 4 + i * 0x1000;
			REG_WR(MOCA_PMB_MASTER_CMD, data);
		}
	}
}

static void bcm6802_pmb_control(struct bcm6802_priv *priv,
				enum PMB_COMMAND_E cmd)
{
	unsigned int i, j;
	u32 *p_zone_control;
	u32 data;

	switch (cmd) {
	case PMB_COMMAND_ALL_OFF:
		/* Turn off zone command */
		REG_WR(MOCA_PMB_MASTER_WDATA, 0xA00);
		p_zone_control = &zone_all_off_bitmask[0];
		break;

	case PMB_COMMAND_PHY1_OFF:
		/* Turn off zone command */
		REG_WR(MOCA_PMB_MASTER_WDATA, 0xA00);
		p_zone_control = &zone_phy1_bitmask[0];
		break;

	case PMB_COMMAND_PHY1_ON:
		/* Turn on zone command */
		REG_WR(MOCA_PMB_MASTER_WDATA, 0xC00);
		p_zone_control = &zone_phy1_bitmask[0];
		break;

	case PMB_COMMAND_PARTIAL_ON:
		/* Turn on zone command */
		REG_WR(MOCA_PMB_MASTER_WDATA, 0xC00);
		p_zone_control = &zone_partial_on_bitmask[0];
		break;

	default:
		dev_err(priv->dev, "%s: illegal cmd: %08x\n",
			__func__, cmd);
		return;
	}

	for (i = 0; i < MOCA_BPCM_NUM; i++) {
		for (j = 0; j < MOCA_BPCM_ZONES_NUM; j++) {
			if (*p_zone_control & (1 << j)) {
				/* zone address in bpcms */
				data = (0x1 << 20) + 16 + (i * 4096) + (j * 4);
				REG_WR(MOCA_PMB_MASTER_CMD, data);
			}
		}
		p_zone_control++;
	}

}

static void bcm6802_pmb_give_cntrl(struct bcm6802_priv *priv,
				   enum PMB_GIVE_OWNERSHIP_E cmd)
{
	unsigned int i;
	u32 data;

	/* Pass control over the memories to the FW */
	REG_WR(MOCA_PMB_MASTER_WDATA, cmd);
	for (i = 0; i < 3; i++) {
		data = 0x100002 + i * 0x1000;
		REG_WR(MOCA_PMB_MASTER_CMD, data);
	}
}

static void bcm6802_set_reset(struct bcm6802_priv *priv, bool reset)
{
	/* Reset assertion delay is 15ms. Reset de-assertion is specified as
	   210ms in the datasheet, but this is not enough, round up to 250 ms
	 */
	unsigned int reset_delay = reset ? 15 : 250;

	if (!priv->gpio_desc)
		return;

	gpiod_set_value(priv->gpio_desc, !reset);
	msleep(reset_delay);
}

void bcm6802_hw_reset(void *hw_priv)
{
	struct bcm6802_priv *priv = hw_priv;

	/* some board-level initialization: */

	/* clear sw_init signals */
	REG_WR(SUN_TOP_CTRL_SW_INIT_0_CLEAR, 0x0FFFFFFF);

	/*pinmux, rgmii, 3450 */
	REG_WR(SUN_TOP_CTRL_PIN_MUX_CTRL_0, 0x11110022);

	REG_WR(SUN_TOP_CTRL_PIN_MUX_CTRL_1, 0x11111111); /*rgmii */
	REG_WR(SUN_TOP_CTRL_PIN_MUX_CTRL_2, 0x11111111); /*rgmii */
	/* enable sideband all,0,1,2, rgmii */
	REG_WR(SUN_TOP_CTRL_PIN_MUX_CTRL_3, 0x22221111);
	/* enable sideband 3,4 */
	REG_WR(SUN_TOP_CTRL_PIN_MUX_CTRL_4, 0x10000022);
	/* enable LED gpios */
	REG_WR(SUN_TOP_CTRL_PIN_MUX_CTRL_5, 0x11000000);
	/* mdio, mdc */
	REG_WR(SUN_TOP_CTRL_PIN_MUX_CTRL_6, 0x00001100);

	/* Set GPIO 41 to PULL_UP */
	REG_WR(SUN_TOP_CTRL_PIN_MUX_PAD_CTRL_3, 0x2402);

	/* Use 2.5V for rgmii */
	REG_WR(SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_0, 0x11);

	/* Disable GPHY LDO */
	if (priv->eport_mux == 0)
		REG_WR(SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_1, 0x3);

	/* set test_drive_sel to 16mA */
	REG_WR(SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_5, 0x47);

	/* Enable LEDs */
	REG_WR(EPORT_REG_LED_CNTRL, 0x93bc);

	/* Disable clkobsv output pin */
	REG_WR(CLKGEN_PAD_CLOCK_DISABLE, 0x1);

	/* Disable LEAP clocks */
	REG_WR(CLKGEN_LEAP_TOP_INST_CLOCK_DISABLE, 0x7);

	/* disable uarts */
	REG_WR(PM_CONFIG, 0x4000);
	/* disable second I2C port, PWMA and timers */
	REG_WR(PM_CLK_CTRL, 0x1810c);

	/* disable and clear all interrupts */
	REG_WR(MOCA_L2_MASK_SET, 0xffffffff);

	/* assert resets */

	/* reset CPU first, both CPUs for MoCA 20 HW */
	REG_SET(MOCA_HOSTMISC_MISC_CTRL, 5);

	udelay(20);

	/* reset everything else except clocks */
	REG_SET(MOCA_HOSTMISC_MISC_CTRL,
		 ~((1 << 3) | (1 << 7) | (1 << 15) | (1 << 16)));

	udelay(20);

	/* disable clocks */
	REG_SET(MOCA_HOSTMISC_MISC_CTRL,
		 ~((1 << 3) | (1 << 15) | (1 << 16)));

	REG_WR(MOCA_L2_CPU_CLEAR, 0xffffffff);

	/* Power down all zones */
	bcm6802_pmb_control(priv, PMB_COMMAND_ALL_OFF);

	/* Power down all SYS_CTRL memories */
	REG_WR(CLKGEN_PLL_SYS1_PLL_PWRDN, 1);
	REG_SET(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3, 1);
}

static void bcm6802_ps_power_ctrl_phy1(struct bcm6802_priv *priv,
				       enum PMB_COMMAND_E cmd)
{
	u32 pll_ctrl_3, pll_ctrl_5, sw_reset;

	pll_ctrl_3 = REG_RD(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_3);
	pll_ctrl_5 = REG_RD(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5);
	sw_reset = REG_RD(MOCA_HOSTMISC_MISC_CTRL);

	/* enable PLL  */
	REG_UNSET(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_3, 1);
	REG_UNSET(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5, 1);

	udelay(1);

	/* de assert moca_phy1_disable_clk */
	REG_UNSET(MOCA_HOSTMISC_MISC_CTRL, (1 << 9));

	bcm6802_pmb_control(priv, cmd);

	REG_WR(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_3, pll_ctrl_3);
	REG_WR(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5, pll_ctrl_5);

	udelay(1);

	REG_WR(MOCA_HOSTMISC_MISC_CTRL, sw_reset);
}

/* called any time we start/restart/stop MoCA */
void bcm6802_hw_init(void *hw_priv, int action, int *enabled, int bonded_mode)
{
	u32 mask;
	u32 data;
	u32 count = 0;
	struct bcm6802_priv *priv = hw_priv;

	if (action == MOCA_ENABLE && !*enabled) {
		/* SUN_TOP_CTRL_SW_INIT_0_CLEAR --> Do this at start of
		   sequence, don't touch gphy_sw_init */
		REG_WR(SUN_TOP_CTRL_SW_INIT_0_CLEAR, ~(1 << 26));
		*enabled = 1;
	}

	/* clock not enabled, register accesses will fail with bus error */
	if (!*enabled)
		return;

	bcm6802_hw_reset(priv);
	udelay(1);

	if (action == MOCA_ENABLE) {

		bcm6802_clk_set_rate(priv);

		/* Power up all zones */
		bcm6802_pmb_control(priv, PMB_COMMAND_PARTIAL_ON);

		REG_UNSET(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3, 1);

		REG_WR(CLKGEN_PLL_SYS1_PLL_RESET, 1);
		REG_WR(CLKGEN_PLL_SYS1_PLL_PWRDN, 0);
		data = 0;
		while ((data & 0x1) == 0) {
			/* This typically is only read once */
			data = REG_RD(CLKGEN_PLL_SYS1_PLL_LOCK_STATUS);

			if (count++ > 10)
				break;
		}
		REG_WR(CLKGEN_PLL_SYS1_PLL_RESET, 0);

		if (bonded_mode) {
			REG_UNSET(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_3, 1);
			REG_UNSET(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5, 1);
		} else {
			REG_SET(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_3, 1);
			REG_SET(CLKGEN_PLL_SYS1_PLL_CHANNEL_CTRL_CH_5, 1);
		}
		udelay(1);

		/* deassert moca_sys_reset, system clock, phy0, phy0 clock */
		mask = (1 << 1) | (1 << 7) | (1 << 4) | (1 << 8);

		/* deassert phy1 and phy1 clock in bonded mode */
		if (bonded_mode)
			mask |= (1 << 5) | (1 << 9);

		REG_UNSET(MOCA_HOSTMISC_MISC_CTRL, mask);

		/* Before power off the memories, moca_phy1_disable_clk */
		if (bonded_mode == 0)
			bcm6802_ps_power_ctrl_phy1(priv, PMB_COMMAND_PHY1_OFF);
		else
			bcm6802_ps_power_ctrl_phy1(priv, PMB_COMMAND_PHY1_ON);

		bcm6802_pmb_give_cntrl(priv, PMB_GIVE_OWNERSHIP_2_FW);

		data = REG_RD(CLKGEN_PLL_SYS1_PLL_SSC_MODE_CONTROL_HIGH);
		data = (data & 0xFFFF0000) | 0x7dd;
		REG_WR(CLKGEN_PLL_SYS1_PLL_SSC_MODE_CONTROL_HIGH, data);

		data = REG_RD(CLKGEN_PLL_SYS1_PLL_SSC_MODE_CONTROL_LOW);
		data = (data & 0xffc00000) | 0x3d71;
		REG_WR(CLKGEN_PLL_SYS1_PLL_SSC_MODE_CONTROL_LOW, data);

		REG_SET(CLKGEN_PLL_SYS1_PLL_SSC_MODE_CONTROL_LOW, (1 << 22));
	}

	REG_WR(MOCA_HOSTMISC_H2M_INT_TRIG, 0);

	REG_WR(LEAP_HOST_L1_INTR_MASK_CLEAR, 2);

	if (action == MOCA_DISABLE)
		*enabled = 0;
}

int bcm6802_write_mem(void *hw_priv, uintptr_t dst_offset, void *src,
		      unsigned int len)
{
	uintptr_t addr;
	struct bcm6802_priv *priv = hw_priv;

	addr = MOCA_DATA_MEM + dst_offset;

	REG_WR_BLOCK(addr, src, len);

	return 0;
}

void bcm6802_read_mem(void *hw_priv, void *dst, uintptr_t src_offset,
		      unsigned int len)
{
	uintptr_t addr = MOCA_DATA_MEM + src_offset;
	struct bcm6802_priv *priv = hw_priv;

	REG_RD_BLOCK(addr, dst, len);
}

static void bcm6802_mem_init_680xC0(struct bcm6802_priv *priv)
{
	/* De-assert reset (all memories are OFF by default
	   Force_SP_off =1, Force_Rf_off =1) */
	REG_UNSET(MOCA_HOSTMISC_MISC_CTRL,
		   ((1 << 15) | (1 << 16)));

	bcm6802_pmb_delay(priv);
	bcm6802_pmb_control(priv, PMB_COMMAND_ALL_OFF);

	/* Write Force_SP_on =0, Force_SP_off =0,
	   Force_RF_on =0, Force_RF_off =0 */
	REG_UNSET(MOCA_HOSTMISC_SUBSYS_CFG, ((1 << 10) | (1 << 11)));
	bcm6802_pmb_control(priv, PMB_COMMAND_PARTIAL_ON);
}

static int bcm6802_hw_specific_init(struct bcm6802_priv *priv, u32 eport_mux,
				    int rgmii0_mode, int rgmii1_mode)
{
	bcm6802_hw_reset(priv);

	if (((priv->chip_id & 0xFFFFFFF0) == 0x680200C0) ||
	    ((priv->chip_id & 0xFFFFFFF0) == 0x680300C0)) {
		/* Initialize 680x CO memory */
		bcm6802_mem_init_680xC0(priv);
	}

	REG_WR(EPORT_REG_EMUX_CNTRL, eport_mux);

	if (eport_mux == 0) {
		/* rgmii 1 disabled */
		REG_WR(EPORT_REG_RGMII_1_CNTRL, 0x0);
	} else {
		switch (rgmii1_mode) {
		case PHY_INTERFACE_MODE_RGMII_RXID:
			REG_WR(EPORT_REG_RGMII_1_CNTRL, 0x09);
			REG_WR(EPORT_REG_RGMII_1_RX_CLOCK_DELAY_CNTRL, 0xE4);
			break;
		case PHY_INTERFACE_MODE_RGMII_TXID:
			REG_WR(EPORT_REG_RGMII_1_CNTRL, 0x01);
			REG_WR(EPORT_REG_RGMII_1_RX_CLOCK_DELAY_CNTRL, 0x84);
			break;
		case PHY_INTERFACE_MODE_RGMII_ID:
			REG_WR(EPORT_REG_RGMII_1_CNTRL, 0x01);
			REG_WR(EPORT_REG_RGMII_1_RX_CLOCK_DELAY_CNTRL, 0xE4);
			break;
		default:
			REG_WR(EPORT_REG_RGMII_1_CNTRL, 0x09);
			REG_WR(EPORT_REG_RGMII_1_RX_CLOCK_DELAY_CNTRL, 0x84);
		}
	}

	switch (rgmii0_mode) {
	case PHY_INTERFACE_MODE_RGMII_RXID:
		REG_WR(EPORT_REG_RGMII_0_CNTRL, 0x09);
		REG_WR(EPORT_REG_RGMII_0_RX_CLOCK_DELAY_CNTRL, 0xE4);
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		REG_WR(EPORT_REG_RGMII_0_CNTRL, 0x01);
		REG_WR(EPORT_REG_RGMII_0_RX_CLOCK_DELAY_CNTRL, 0x84);
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		REG_WR(EPORT_REG_RGMII_0_CNTRL, 0x01);
		REG_WR(EPORT_REG_RGMII_0_RX_CLOCK_DELAY_CNTRL, 0xE4);
		break;
	default:
		REG_WR(EPORT_REG_RGMII_0_CNTRL, 0x09);
		REG_WR(EPORT_REG_RGMII_0_RX_CLOCK_DELAY_CNTRL, 0x84);
	}

	if (eport_mux == 0) /* Shutdown Gphy */
		REG_WR(EPORT_REG_GPHY_CNTRL, 0x02A4C00F);
	else
		REG_WR(EPORT_REG_GPHY_CNTRL, 0x02A4C000);

	/* Power down all LEAP memories */
	REG_WR(CLKGEN_LEAP_TOP_INST_DATA, 0x6);
	REG_WR(CLKGEN_LEAP_TOP_INST_HAB, 0x6);
	REG_WR(CLKGEN_LEAP_TOP_INST_PROG0, 0x6);
	REG_WR(CLKGEN_LEAP_TOP_INST_PROG1, 0x6);
	REG_WR(CLKGEN_LEAP_TOP_INST_PROG2, 0x6);
	REG_WR(CLKGEN_LEAP_TOP_INST_ROM, 0x6);
	REG_WR(CLKGEN_LEAP_TOP_INST_SHARED, 0x6);

	REG_WR(CLKGEN_SYS_CTRL_INST_POWER_SWITCH_MEMORY, 0x3);

	/* Allow jumbo frames up to 9000 bytes */
	REG_WR(EPORT_UMAC_RX_MAX_PKT_SIZE_OFFSET, 9000);
	REG_WR(EPORT_UMAC_FRM_LEN_OFFSET, 9000);

	return 0;
}

void bcm6802_enable_irq(void *hw_priv)
{
	struct bcm6802_priv *priv = hw_priv;

	enable_irq(priv->irq);
}

void bcm6802_disable_irq(void *hw_priv)
{
	struct bcm6802_priv *priv = hw_priv;

	disable_irq_nosync(priv->irq);
}

struct moca_ops bcm6802_moca_ops = {
	.read32 = bcm6802_read32,
	.write32 = bcm6802_write32,
	.hw_reset = bcm6802_hw_reset,
	.hw_init = bcm6802_hw_init,
	.write_mem = bcm6802_write_mem,
	.read_mem = bcm6802_read_mem,
	.hw_enable_irq = bcm6802_enable_irq,
	.hw_disable_irq = bcm6802_disable_irq,

	.dma = 0
};

static int bcm6802_parse_rgmii(struct bcm6802_priv *priv,
			       struct device_node *rgmii_of_node,
			       int *rgmii_mode, u8 **macaddr,
			       u32 *enet_phandle)
{
	phy_interface_t iface;
	int ret;

	ret = of_get_mac_address(rgmii_of_node, *macaddr);
	if (ret < 0)
		return ret;

	ret = of_get_phy_mode(rgmii_of_node, &iface);
	if (ret < 0)
		iface = PHY_INTERFACE_MODE_RGMII_TXID;

	*rgmii_mode = iface;

	of_property_read_u32(rgmii_of_node, "enet-id", enet_phandle);

	return 0;
}

/* An estimate of how long it should take to prepare MoCA for S2 mode.
   Measured empirically at different SPI bitrates, then doubled */
static int bcm6802_estimate_suspend_timeout(unsigned int bitrate)
{
	/* This equation is a linear fit to measured data */
	return (30 + 21562500 / bitrate ) * 200;
}

int bcm6802_probe(struct spi_device *spi_device)
{
	struct device_node *ethernet_of_node;
	struct device_node *rgmii_of_node;
	struct bcm6802_priv *priv;
	u8 *macaddr_1 = NULL;
	u8 *macaddr = NULL;
	u32 rgmii0_phandle = 0;
	u32 rgmii1_phandle = 0;
	int rgmii0_mode = 0;
	int rgmii1_mode = 0;
	u32 enet_ph = 0;
	int wol_irq;
	int rc;

	priv = devm_kzalloc(&spi_device->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->spi_device = spi_device;

	mutex_init(&priv->copy_mutex);

	priv->dev = &spi_device->dev;

	priv->gpio_desc =  devm_gpiod_get_optional(priv->dev, "reset",
						   GPIOD_OUT_LOW);

	/* Reset chip, then un-reset it */
	bcm6802_set_reset(priv, true);
	bcm6802_set_reset(priv, false);

	priv->chip_id = REG_RD(SUN_TOP_CTRL_PRODUCT_ID) + 0xA0;
	if ((priv->chip_id & 0xFFFE0000) != 0x68020000) {
		dev_err(priv->dev, "bcm6802: No 6802/3 chip found\n");
		rc = -EFAULT;
		goto error;
	}

	dev_info(priv->dev, "Found chip: 0x%08X\n", priv->chip_id);

	/* Parse RGMII nodes.  6802 has two RGMII ports */
	ethernet_of_node = of_find_node_by_name(priv->dev->of_node,
						"ethernet-ports");

	if (ethernet_of_node) {
		rgmii_of_node = of_find_node_by_name(ethernet_of_node,
						     "rgmii0");

		if (rgmii_of_node)
			bcm6802_parse_rgmii(priv, rgmii_of_node, &rgmii0_mode,
					    &macaddr, &rgmii0_phandle);

		rgmii_of_node = of_find_node_by_name(ethernet_of_node,
						     "rgmii1");

		if (rgmii_of_node)
			bcm6802_parse_rgmii(priv, rgmii_of_node, &rgmii1_mode,
					    &macaddr_1, &rgmii1_phandle);

		if (macaddr_1 && macaddr) {
			dev_err(priv->dev, "Both rgmii nodes have mac address property.  Only the MoCA-connected node should.");
			rc = -EFAULT;
			goto error;
		}

		/* If an RGMII node has a MAC address property, we use that for
		   MoCA */
		if (macaddr) {
			enet_ph = rgmii0_phandle;
			priv->eport_mux = 2;  /* RGMII0 connected to MoCA */
		} else if (macaddr_1) {
			macaddr = macaddr_1;
			enet_ph = rgmii1_phandle;
			priv->eport_mux = 1;  /* RGMII1 connected to MoCA */
		} else {
			priv->eport_mux = 4;  /* GPHY connected to MoCA,
						 external mdio control */
			priv->gphy_en = 1;
		}

		/* If both rgmii nodes are present, then GPHY must be used for
		   one of them */
		if (rgmii1_phandle && rgmii0_phandle) {
			/* enable external mdio */
			priv->eport_mux |= 4;
			priv->gphy_en = 1;
		}
	}

	bcm6802_hw_specific_init(priv, priv->eport_mux, rgmii0_mode,
				 rgmii1_mode);

	priv->irq = irq_of_parse_and_map(priv->dev->of_node, 0);

	if (priv->irq < 0) {
		dev_err(priv->dev, "can't get IRQ\n");
		rc = -EIO;
		goto error;
	}

	wol_irq = irq_of_parse_and_map(priv->dev->of_node, 1);

	bcm6802_moca_ops.suspend_timeout_ms =
		bcm6802_estimate_suspend_timeout(spi_device->max_speed_hz);

	rc = moca_initialize(&spi_device->dev, &bcm6802_moca_ops,
			     (void *)priv, (void __iomem *)MOCA_DATA_MEM,
			     priv->chip_id, 0, priv->irq, wol_irq, macaddr,
			     enet_ph, &regs_6802);

	if (rc) {
		dev_err(priv->dev, "bcm6802: probe failed\n");
		goto error;
	}

	return 0;

error:
	mutex_destroy(&priv->copy_mutex);
	return rc;
}

void bcm6802_remove(void *hw_priv)
{
	bcm6802_set_reset(hw_priv, true);
}

static struct spi_driver bcm6802_driver = {
	.driver = {
		.name = bcm6802_driver_name,
		.owner = THIS_MODULE,
		.pm = &moca_pm_ops,
	},
	.probe = bcm6802_probe,
};

module_spi_driver(bcm6802_driver);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("BCM6802 module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
