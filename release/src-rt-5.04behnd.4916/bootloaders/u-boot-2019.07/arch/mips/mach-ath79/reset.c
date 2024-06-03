// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 * Copyright (C) 2018-2019 Rosy Song <rosysong@rosinson.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ath79.h>
#include <mach/ar71xx_regs.h>

/* QCA956X ETH_SGMII_SERDES Registers */
#define SGMII_SERDES_RES_CALIBRATION_LSB 23
#define SGMII_SERDES_RES_CALIBRATION_MASK 0x07800000
#define SGMII_SERDES_RES_CALIBRATION_SET(x) \
	(((x) << SGMII_SERDES_RES_CALIBRATION_LSB) & SGMII_SERDES_RES_CALIBRATION_MASK)
#define SGMII_SERDES_CDR_BW_LSB 1
#define SGMII_SERDES_CDR_BW_MASK 0x00000006
#define SGMII_SERDES_CDR_BW_SET(x) \
	(((x) << SGMII_SERDES_CDR_BW_LSB) & SGMII_SERDES_CDR_BW_MASK)
#define SGMII_SERDES_TX_DR_CTRL_LSB 4
#define SGMII_SERDES_TX_DR_CTRL_MASK 0x00000070
#define SGMII_SERDES_TX_DR_CTRL_SET(x) \
	(((x) << SGMII_SERDES_TX_DR_CTRL_LSB) & SGMII_SERDES_TX_DR_CTRL_MASK)
#define SGMII_SERDES_PLL_BW_LSB 8
#define SGMII_SERDES_PLL_BW_MASK 0x00000100
#define SGMII_SERDES_PLL_BW_SET(x) \
	(((x) << SGMII_SERDES_PLL_BW_LSB) & SGMII_SERDES_PLL_BW_MASK)
#define SGMII_SERDES_EN_SIGNAL_DETECT_LSB 16
#define SGMII_SERDES_EN_SIGNAL_DETECT_MASK 0x00010000
#define SGMII_SERDES_EN_SIGNAL_DETECT_SET(x) \
	(((x) << SGMII_SERDES_EN_SIGNAL_DETECT_LSB) & SGMII_SERDES_EN_SIGNAL_DETECT_MASK)
#define SGMII_SERDES_FIBER_SDO_LSB 17
#define SGMII_SERDES_FIBER_SDO_MASK 0x00020000
#define SGMII_SERDES_FIBER_SDO_SET(x) \
	(((x) << SGMII_SERDES_FIBER_SDO_LSB) & SGMII_SERDES_FIBER_SDO_MASK)
#define SGMII_SERDES_VCO_REG_LSB 27
#define SGMII_SERDES_VCO_REG_MASK 0x78000000
#define SGMII_SERDES_VCO_REG_SET(x) \
	(((x) << SGMII_SERDES_VCO_REG_LSB) & SGMII_SERDES_VCO_REG_MASK)
#define SGMII_SERDES_VCO_FAST_LSB 9
#define SGMII_SERDES_VCO_FAST_MASK 0x00000200
#define SGMII_SERDES_VCO_FAST_GET(x) \
	(((x) & SGMII_SERDES_VCO_FAST_MASK) >> SGMII_SERDES_VCO_FAST_LSB)
#define SGMII_SERDES_VCO_SLOW_LSB 10
#define SGMII_SERDES_VCO_SLOW_MASK 0x00000400
#define SGMII_SERDES_VCO_SLOW_GET(x) \
	(((x) & SGMII_SERDES_VCO_SLOW_MASK) >> SGMII_SERDES_VCO_SLOW_LSB)

void _machine_restart(void)
{
	void __iomem *base;
	u32 reg = 0;

	base = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
			   MAP_NOCACHE);
	if (soc_is_ar71xx())
		reg = AR71XX_RESET_REG_RESET_MODULE;
	else if (soc_is_ar724x())
		reg = AR724X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar913x())
		reg = AR913X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar933x())
		reg = AR933X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar934x())
		reg = AR934X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca953x())
		reg = QCA953X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca955x())
		reg = QCA955X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca956x())
		reg = QCA956X_RESET_REG_RESET_MODULE;
	else
		puts("Reset register not defined for this SOC\n");

	if (reg)
		setbits_be32(base + reg, AR71XX_RESET_FULL_CHIP);

	while (1)
		/* NOP */;
}

u32 ath79_get_bootstrap(void)
{
	void __iomem *base;
	u32 reg = 0;

	base = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
			   MAP_NOCACHE);
	if (soc_is_ar933x())
		reg = AR933X_RESET_REG_BOOTSTRAP;
	else if (soc_is_ar934x())
		reg = AR934X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca953x())
		reg = QCA953X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca955x())
		reg = QCA955X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca956x())
		reg = QCA956X_RESET_REG_BOOTSTRAP;
	else
		puts("Bootstrap register not defined for this SOC\n");

	if (reg)
		return readl(base + reg);

	return 0;
}

static int eth_init_ar933x(void)
{
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);
	void __iomem *gregs = map_physmem(AR933X_GMAC_BASE, AR933X_GMAC_SIZE,
					  MAP_NOCACHE);
	const u32 mask = AR933X_RESET_GE0_MAC | AR933X_RESET_GE0_MDIO |
			 AR933X_RESET_GE1_MAC | AR933X_RESET_GE1_MDIO |
			 AR933X_RESET_ETH_SWITCH |
			 AR933X_RESET_ETH_SWITCH_ANALOG;

	/* Clear MDIO slave EN bit. */
	clrbits_be32(rregs + AR933X_RESET_REG_BOOTSTRAP, BIT(17));
	mdelay(10);

	/* Get Atheros S26 PHY out of reset. */
	clrsetbits_be32(pregs + AR933X_PLL_SWITCH_CLOCK_CONTROL_REG,
			0x1f, 0x10);
	mdelay(10);

	setbits_be32(rregs + AR933X_RESET_REG_RESET_MODULE, mask);
	mdelay(10);
	clrbits_be32(rregs + AR933X_RESET_REG_RESET_MODULE, mask);
	mdelay(10);

	/* Configure AR93xx GMAC register. */
	clrsetbits_be32(gregs + AR933X_GMAC_REG_ETH_CFG,
			AR933X_ETH_CFG_MII_GE0_MASTER |
			AR933X_ETH_CFG_MII_GE0_SLAVE,
			AR933X_ETH_CFG_MII_GE0_SLAVE);
	return 0;
}

static int eth_init_ar934x(void)
{
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);
	void __iomem *gregs = map_physmem(AR934X_GMAC_BASE, AR934X_GMAC_SIZE,
					  MAP_NOCACHE);
	const u32 mask = AR934X_RESET_GE0_MAC | AR934X_RESET_GE0_MDIO |
			 AR934X_RESET_GE1_MAC | AR934X_RESET_GE1_MDIO |
			 AR934X_RESET_ETH_SWITCH_ANALOG;
	u32 reg;

	reg = readl(rregs + AR934X_RESET_REG_BOOTSTRAP);
	if (reg & AR934X_BOOTSTRAP_REF_CLK_40)
		writel(0x570, pregs + AR934X_PLL_SWITCH_CLOCK_CONTROL_REG);
	else
		writel(0x271, pregs + AR934X_PLL_SWITCH_CLOCK_CONTROL_REG);
	writel(BIT(26) | BIT(25), pregs + AR934X_PLL_ETH_XMII_CONTROL_REG);

	setbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);
	clrbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);

	/* Configure AR934x GMAC register. */
	writel(AR934X_ETH_CFG_RGMII_GMAC0, gregs + AR934X_GMAC_REG_ETH_CFG);
	return 0;
}

static int eth_init_qca953x(void)
{
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	const u32 mask = QCA953X_RESET_GE0_MAC | QCA953X_RESET_GE0_MDIO |
			 QCA953X_RESET_GE1_MAC | QCA953X_RESET_GE1_MDIO |
			 QCA953X_RESET_ETH_SWITCH_ANALOG |
			 QCA953X_RESET_ETH_SWITCH;

	setbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);
	clrbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);

	return 0;
}

static int qca956x_sgmii_cal(void)
{
	int i;
	u32 reg, rev_sgmii_val;
	u32 vco_fast, vco_slow;
	u32 start_val = 0, end_val = 0;
	void __iomem *gregs = map_physmem(AR71XX_MII_BASE, AR71XX_MII_SIZE,
					  MAP_NOCACHE);
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	const u32 mask = QCA956X_RESET_SGMII_ASSERT | QCA956X_RESET_SGMII;

	writel(BIT(2) | BIT(0), pregs + QCA956X_PLL_ETH_SGMII_SERDES_REG);

	reg = readl(gregs + QCA956X_GMAC_REG_SGMII_SERDES);
	vco_fast = SGMII_SERDES_VCO_FAST_GET(reg);
	vco_slow = SGMII_SERDES_VCO_SLOW_GET(reg);

	/* Set resistor calibration from 0000 to 1111 */
	for (i = 0; i < 0x10; i++) {
		reg = (readl(gregs + QCA956X_GMAC_REG_SGMII_SERDES) &
		      ~SGMII_SERDES_RES_CALIBRATION_MASK) |
		      SGMII_SERDES_RES_CALIBRATION_SET(i);
		writel(reg, gregs + QCA956X_GMAC_REG_SGMII_SERDES);

		udelay(50);

		reg = readl(gregs + QCA956X_GMAC_REG_SGMII_SERDES);
		if (vco_fast != SGMII_SERDES_VCO_FAST_GET(reg) ||
		    vco_slow != SGMII_SERDES_VCO_SLOW_GET(reg)) {
			if (start_val == 0) {
				start_val = i;
				end_val = i;
			} else {
				end_val = i;
			}
		}
		vco_fast = SGMII_SERDES_VCO_FAST_GET(reg);
		vco_slow = SGMII_SERDES_VCO_SLOW_GET(reg);
	}

	if (start_val == 0)
		rev_sgmii_val = 0x7;
	else
		rev_sgmii_val = (start_val + end_val) >> 1;

	writel((readl(gregs + QCA956X_GMAC_REG_SGMII_SERDES) &
	       ~SGMII_SERDES_RES_CALIBRATION_MASK) |
	       SGMII_SERDES_RES_CALIBRATION_SET(rev_sgmii_val),
	       gregs + QCA956X_GMAC_REG_SGMII_SERDES);

	writel(BIT(2) | BIT(0), pregs + QCA956X_PLL_ETH_SGMII_SERDES_REG);

	reg = readl(gregs + QCA956X_GMAC_REG_SGMII_SERDES);
	writel(SGMII_SERDES_CDR_BW_SET(3) | SGMII_SERDES_TX_DR_CTRL_SET(1) |
	       SGMII_SERDES_PLL_BW_SET(1) | SGMII_SERDES_EN_SIGNAL_DETECT_SET(1) |
	       SGMII_SERDES_FIBER_SDO_SET(1) | SGMII_SERDES_VCO_REG_SET(3) | reg,
	       gregs + QCA956X_GMAC_REG_SGMII_SERDES);

	setbits_be32(rregs + QCA956X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);
	clrbits_be32(rregs + QCA956X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);

	while (!(readl(gregs + QCA956X_GMAC_REG_SGMII_SERDES) & BIT(15)))
		/* NOP */;

	return 0;
}

static int qca956x_sgmii_setup(void)
{
	int i;
	u32 s = 0, reg = 0;
	u32 _regs[] = {
		BIT(4),	/* HW_RX_125M_N */
		BIT(2),	/* RX_125M_N */
		BIT(3),	/* TX_125M_N */
		BIT(0),	/* RX_CLK_N */
		BIT(1),	/* TX_CLK_N */
	};
	void __iomem *gregs = map_physmem(AR71XX_MII_BASE, AR71XX_MII_SIZE,
					  MAP_NOCACHE);

	/* Force sgmii mode */
	writel(BIT(6) | BIT(15) | BIT(8), gregs + QCA956X_GMAC_REG_MR_AN_CTRL);
	udelay(10);
	writel(0x2 | BIT(5) | (0x2 << 6), gregs + QCA956X_GMAC_REG_SGMII_CONFIG);

	/* SGMII reset sequence sugguest by qca systems team. */
	writel(0, gregs + QCA956X_GMAC_REG_SGMII_RESET);
	for (i = 0; i < ARRAY_SIZE(_regs); i++) {
		reg |= _regs[i];
		writel(reg, gregs + QCA956X_GMAC_REG_SGMII_RESET);
	}

	writel(readl(gregs + QCA956X_GMAC_REG_MR_AN_CTRL) & ~BIT(15),
	       gregs + QCA956X_GMAC_REG_MR_AN_CTRL);

	/*
	 * WARNING: Across resets SGMII link status goes to weird state.
	 * if 0xb8070058 (SGMII_DEBUG Register) reads other than 0xf or 0x10
	 * for sure we are in bad state.
	 * Issue a PHY RESET in MR_AN_CONTROL_ADDRESS to keep going.
	 */
	i = 0;
	s = (readl(gregs + QCA956X_GMAC_REG_SGMII_DEBUG) & 0xff);
	while (!(s == 0xf || s == 0x10)) {
		writel(readl(gregs + QCA956X_GMAC_REG_MR_AN_CTRL) | BIT(15),
		       gregs + QCA956X_GMAC_REG_MR_AN_CTRL);
		udelay(100);
		writel(readl(gregs + QCA956X_GMAC_REG_MR_AN_CTRL) & ~BIT(15),
		       gregs + QCA956X_GMAC_REG_MR_AN_CTRL);
		if (i++ == 10)
			break;
		s = (readl(gregs + QCA956X_GMAC_REG_SGMII_DEBUG) & 0xff);
	}

	return 0;
}

static int qca956x_s17_reset(void)
{
	void __iomem *regs = map_physmem(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE,
					  MAP_NOCACHE);
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	const u32 mask = QCA956X_RESET_SGMII_ASSERT | QCA956X_RESET_SGMII |
			 QCA956X_RESET_EXTERNAL | QCA956X_RESET_SGMII_ANALOG |
			 QCA956X_RESET_SWITCH;
	/* Bits(Reserved in datasheet) should be set to 1 */
	const u32 mask_r = QCA956X_RESET_SGMII_ASSERT | QCA956X_RESET_SGMII |
			 QCA956X_RESET_EXTERNAL;

	setbits_be32(rregs + QCA956X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);
	clrbits_be32(rregs + QCA956X_RESET_REG_RESET_MODULE, mask_r);
	mdelay(1);

	/* Reset s17 switch(GPIO11) SYS_RST_L */
	writel(readl(regs + AR71XX_GPIO_REG_OE) & ~BIT(11),
	       regs + AR71XX_GPIO_REG_OE);
	udelay(100);

	writel(readl(regs + AR71XX_GPIO_REG_OUT) & ~BIT(11),
	       regs + AR71XX_GPIO_REG_OUT);
	udelay(100);
	writel(readl(regs + AR71XX_GPIO_REG_OUT) | BIT(11),
	       regs + AR71XX_GPIO_REG_OUT);

	return 0;
}

static int qca956x_init_mdio(void)
{
	u32 reg;
	void __iomem *regs = map_physmem(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE,
						MAP_NOCACHE);
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	const u32 mask = QCA956X_RESET_GE0_MDIO | QCA956X_RESET_GE0_MAC |
			 QCA956X_RESET_GE1_MDIO | QCA956X_RESET_GE1_MAC;

	setbits_be32(rregs + QCA956X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);
	clrbits_be32(rregs + QCA956X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);

	/* GPIO4 as MDI */
	reg = readl(regs + QCA956X_GPIO_REG_IN_ENABLE3);
	reg &= ~(0xff << 16);
	reg |= (0x4 << 16);
	writel(reg, regs + QCA956X_GPIO_REG_IN_ENABLE3);

	/* GPIO4 as MDO */
	reg = readl(regs + QCA956X_GPIO_REG_OUT_FUNC1);
	reg &= ~0xff;
	reg |= 0x20;
	writel(reg, regs + QCA956X_GPIO_REG_OUT_FUNC1);

	/* Init MDC(GPIO3) / MDIO(GPIO4) */
	reg = readl(regs + AR71XX_GPIO_REG_OE);
	reg &= ~BIT(4);
	writel(reg, regs + AR71XX_GPIO_REG_OE);
	udelay(100);

	reg = readl(regs + AR71XX_GPIO_REG_OE);
	reg &= ~BIT(3);
	writel(reg, regs + AR71XX_GPIO_REG_OE);
	udelay(100);

	/* GPIO3 as MDI */
	reg = readl(regs + QCA956X_GPIO_REG_OUT_FUNC0);
	reg &= ~(0xff << 24);
	reg |= (0x21 << 24);
	writel(reg, regs + QCA956X_GPIO_REG_OUT_FUNC0);

	return 0;
}

static int eth_init_qca956x(void)
{
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);
	void __iomem *gregs = map_physmem(AR71XX_MII_BASE, AR71XX_MII_SIZE,
					  MAP_NOCACHE);

	qca956x_sgmii_cal();
	qca956x_s17_reset();
	qca956x_init_mdio();

	if (ath79_get_bootstrap() & QCA956X_BOOTSTRAP_REF_CLK_40)
		writel(0x45500, pregs + QCA956X_PLL_SWITCH_CLK_CTRL_REG);
	else
		writel(0xc5200, pregs + QCA956X_PLL_SWITCH_CLK_CTRL_REG);

	qca956x_sgmii_setup();

	writel((3 << 16) | (3 << 14) | (1 << 0) | (1 << 6),
	       gregs + QCA956X_GMAC_REG_ETH_CFG);

	writel((1 << 31) | (2 << 28) | (2 << 26) | (1 << 25),
	       pregs + QCA956X_PLL_ETH_XMII_CTRL_REG);
	mdelay(1);

	return 0;
}

int ath79_eth_reset(void)
{
	/*
	 * Un-reset ethernet. DM still doesn't have any notion of reset
	 * framework, so we do it by hand here.
	 */
	if (soc_is_ar933x())
		return eth_init_ar933x();
	if (soc_is_ar934x())
		return eth_init_ar934x();
	if (soc_is_qca953x())
		return eth_init_qca953x();
	if (soc_is_qca956x())
		return eth_init_qca956x();

	return -EINVAL;
}

static int usb_reset_ar933x(void __iomem *reset_regs)
{
	/* Ungate the USB block */
	setbits_be32(reset_regs + AR933X_RESET_REG_RESET_MODULE,
		     AR933X_RESET_USBSUS_OVERRIDE);
	mdelay(1);
	clrbits_be32(reset_regs + AR933X_RESET_REG_RESET_MODULE,
		     AR933X_RESET_USB_HOST);
	mdelay(1);
	clrbits_be32(reset_regs + AR933X_RESET_REG_RESET_MODULE,
		     AR933X_RESET_USB_PHY);
	mdelay(1);

	return 0;
}

static int usb_reset_ar934x(void __iomem *reset_regs)
{
	/* Ungate the USB block */
	setbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USBSUS_OVERRIDE);
	mdelay(1);
	clrbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USB_PHY);
	mdelay(1);
	clrbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USB_PHY_ANALOG);
	mdelay(1);
	clrbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USB_HOST);
	mdelay(1);

	return 0;
}

static int usb_reset_qca953x(void __iomem *reset_regs)
{
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);

	clrsetbits_be32(pregs + QCA953X_PLL_SWITCH_CLOCK_CONTROL_REG,
			0xf00, 0x200);
	mdelay(10);

	/* Ungate the USB block */
	setbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USBSUS_OVERRIDE);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_PHY);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_PHY_ANALOG);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_HOST);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_PHY_PLL_PWD_EXT);
	mdelay(1);

	return 0;
}

int ath79_usb_reset(void)
{
	void __iomem *usbc_regs = map_physmem(AR71XX_USB_CTRL_BASE,
					      AR71XX_USB_CTRL_SIZE,
					      MAP_NOCACHE);
	void __iomem *reset_regs = map_physmem(AR71XX_RESET_BASE,
					       AR71XX_RESET_SIZE,
					       MAP_NOCACHE);
	/*
	 * Turn on the Buff and Desc swap bits.
	 * NOTE: This write into an undocumented register in mandatory to
	 *       get the USB controller operational in BigEndian mode.
	 */
	writel(0xf0000, usbc_regs + AR71XX_USB_CTRL_REG_CONFIG);

	if (soc_is_ar933x())
		return usb_reset_ar933x(reset_regs);
	if (soc_is_ar934x())
		return usb_reset_ar934x(reset_regs);
	if (soc_is_qca953x())
		return usb_reset_qca953x(reset_regs);

	return -EINVAL;
}
