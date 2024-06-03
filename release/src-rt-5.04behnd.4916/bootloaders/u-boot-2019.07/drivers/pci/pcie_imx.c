// SPDX-License-Identifier: GPL-2.0
/*
 * Freescale i.MX6 PCI Express Root-Complex driver
 *
 * Copyright (C) 2013 Marek Vasut <marex@denx.de>
 *
 * Based on upstream Linux kernel driver:
 * pci-imx6.c:		Sean Cross <xobs@kosagi.com>
 * pcie-designware.c:	Jingoo Han <jg1.han@samsung.com>
 */

#include <common.h>
#include <pci.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/crm_regs.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/sizes.h>
#include <errno.h>
#include <asm/arch/sys_proto.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

#ifdef CONFIG_MX6SX
#define MX6_DBI_ADDR	0x08ffc000
#define MX6_IO_ADDR	0x08000000
#define MX6_MEM_ADDR	0x08100000
#define MX6_ROOT_ADDR	0x08f00000
#else
#define MX6_DBI_ADDR	0x01ffc000
#define MX6_IO_ADDR	0x01000000
#define MX6_MEM_ADDR	0x01100000
#define MX6_ROOT_ADDR	0x01f00000
#endif
#define MX6_DBI_SIZE	0x4000
#define MX6_IO_SIZE	0x100000
#define MX6_MEM_SIZE	0xe00000
#define MX6_ROOT_SIZE	0xfc000

/* PCIe Port Logic registers (memory-mapped) */
#define PL_OFFSET 0x700
#define PCIE_PL_PFLR (PL_OFFSET + 0x08)
#define PCIE_PL_PFLR_LINK_STATE_MASK		(0x3f << 16)
#define PCIE_PL_PFLR_FORCE_LINK			(1 << 15)
#define PCIE_PHY_DEBUG_R0 (PL_OFFSET + 0x28)
#define PCIE_PHY_DEBUG_R1 (PL_OFFSET + 0x2c)
#define PCIE_PHY_DEBUG_R1_LINK_UP		(1 << 4)
#define PCIE_PHY_DEBUG_R1_LINK_IN_TRAINING	(1 << 29)

#define PCIE_PHY_CTRL (PL_OFFSET + 0x114)
#define PCIE_PHY_CTRL_DATA_LOC 0
#define PCIE_PHY_CTRL_CAP_ADR_LOC 16
#define PCIE_PHY_CTRL_CAP_DAT_LOC 17
#define PCIE_PHY_CTRL_WR_LOC 18
#define PCIE_PHY_CTRL_RD_LOC 19

#define PCIE_PHY_STAT (PL_OFFSET + 0x110)
#define PCIE_PHY_STAT_DATA_LOC 0
#define PCIE_PHY_STAT_ACK_LOC 16

/* PHY registers (not memory-mapped) */
#define PCIE_PHY_RX_ASIC_OUT 0x100D

#define PHY_RX_OVRD_IN_LO 0x1005
#define PHY_RX_OVRD_IN_LO_RX_DATA_EN (1 << 5)
#define PHY_RX_OVRD_IN_LO_RX_PLL_EN (1 << 3)

#define PCIE_PHY_PUP_REQ		(1 << 7)

/* iATU registers */
#define PCIE_ATU_VIEWPORT		0x900
#define PCIE_ATU_REGION_INBOUND		(0x1 << 31)
#define PCIE_ATU_REGION_OUTBOUND	(0x0 << 31)
#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_CR1			0x904
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_CR2			0x908
#define PCIE_ATU_ENABLE			(0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_LOWER_BASE		0x90C
#define PCIE_ATU_UPPER_BASE		0x910
#define PCIE_ATU_LIMIT			0x914
#define PCIE_ATU_LOWER_TARGET		0x918
#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)
#define PCIE_ATU_UPPER_TARGET		0x91C

struct imx_pcie_priv {
	void __iomem		*dbi_base;
	void __iomem		*cfg_base;
};

/*
 * PHY access functions
 */
static int pcie_phy_poll_ack(void __iomem *dbi_base, int exp_val)
{
	u32 val;
	u32 max_iterations = 10;
	u32 wait_counter = 0;

	do {
		val = readl(dbi_base + PCIE_PHY_STAT);
		val = (val >> PCIE_PHY_STAT_ACK_LOC) & 0x1;
		wait_counter++;

		if (val == exp_val)
			return 0;

		udelay(1);
	} while (wait_counter < max_iterations);

	return -ETIMEDOUT;
}

static int pcie_phy_wait_ack(void __iomem *dbi_base, int addr)
{
	u32 val;
	int ret;

	val = addr << PCIE_PHY_CTRL_DATA_LOC;
	writel(val, dbi_base + PCIE_PHY_CTRL);

	val |= (0x1 << PCIE_PHY_CTRL_CAP_ADR_LOC);
	writel(val, dbi_base + PCIE_PHY_CTRL);

	ret = pcie_phy_poll_ack(dbi_base, 1);
	if (ret)
		return ret;

	val = addr << PCIE_PHY_CTRL_DATA_LOC;
	writel(val, dbi_base + PCIE_PHY_CTRL);

	ret = pcie_phy_poll_ack(dbi_base, 0);
	if (ret)
		return ret;

	return 0;
}

/* Read from the 16-bit PCIe PHY control registers (not memory-mapped) */
static int pcie_phy_read(void __iomem *dbi_base, int addr , int *data)
{
	u32 val, phy_ctl;
	int ret;

	ret = pcie_phy_wait_ack(dbi_base, addr);
	if (ret)
		return ret;

	/* assert Read signal */
	phy_ctl = 0x1 << PCIE_PHY_CTRL_RD_LOC;
	writel(phy_ctl, dbi_base + PCIE_PHY_CTRL);

	ret = pcie_phy_poll_ack(dbi_base, 1);
	if (ret)
		return ret;

	val = readl(dbi_base + PCIE_PHY_STAT);
	*data = val & 0xffff;

	/* deassert Read signal */
	writel(0x00, dbi_base + PCIE_PHY_CTRL);

	ret = pcie_phy_poll_ack(dbi_base, 0);
	if (ret)
		return ret;

	return 0;
}

static int pcie_phy_write(void __iomem *dbi_base, int addr, int data)
{
	u32 var;
	int ret;

	/* write addr */
	/* cap addr */
	ret = pcie_phy_wait_ack(dbi_base, addr);
	if (ret)
		return ret;

	var = data << PCIE_PHY_CTRL_DATA_LOC;
	writel(var, dbi_base + PCIE_PHY_CTRL);

	/* capture data */
	var |= (0x1 << PCIE_PHY_CTRL_CAP_DAT_LOC);
	writel(var, dbi_base + PCIE_PHY_CTRL);

	ret = pcie_phy_poll_ack(dbi_base, 1);
	if (ret)
		return ret;

	/* deassert cap data */
	var = data << PCIE_PHY_CTRL_DATA_LOC;
	writel(var, dbi_base + PCIE_PHY_CTRL);

	/* wait for ack de-assertion */
	ret = pcie_phy_poll_ack(dbi_base, 0);
	if (ret)
		return ret;

	/* assert wr signal */
	var = 0x1 << PCIE_PHY_CTRL_WR_LOC;
	writel(var, dbi_base + PCIE_PHY_CTRL);

	/* wait for ack */
	ret = pcie_phy_poll_ack(dbi_base, 1);
	if (ret)
		return ret;

	/* deassert wr signal */
	var = data << PCIE_PHY_CTRL_DATA_LOC;
	writel(var, dbi_base + PCIE_PHY_CTRL);

	/* wait for ack de-assertion */
	ret = pcie_phy_poll_ack(dbi_base, 0);
	if (ret)
		return ret;

	writel(0x0, dbi_base + PCIE_PHY_CTRL);

	return 0;
}

static int imx6_pcie_link_up(struct imx_pcie_priv *priv)
{
	u32 rc, ltssm;
	int rx_valid, temp;

	/* link is debug bit 36, debug register 1 starts at bit 32 */
	rc = readl(priv->dbi_base + PCIE_PHY_DEBUG_R1);
	if ((rc & PCIE_PHY_DEBUG_R1_LINK_UP) &&
	    !(rc & PCIE_PHY_DEBUG_R1_LINK_IN_TRAINING))
		return -EAGAIN;

	/*
	 * From L0, initiate MAC entry to gen2 if EP/RC supports gen2.
	 * Wait 2ms (LTSSM timeout is 24ms, PHY lock is ~5us in gen2).
	 * If (MAC/LTSSM.state == Recovery.RcvrLock)
	 * && (PHY/rx_valid==0) then pulse PHY/rx_reset. Transition
	 * to gen2 is stuck
	 */
	pcie_phy_read(priv->dbi_base, PCIE_PHY_RX_ASIC_OUT, &rx_valid);
	ltssm = readl(priv->dbi_base + PCIE_PHY_DEBUG_R0) & 0x3F;

	if (rx_valid & 0x01)
		return 0;

	if (ltssm != 0x0d)
		return 0;

	printf("transition to gen2 is stuck, reset PHY!\n");

	pcie_phy_read(priv->dbi_base, PHY_RX_OVRD_IN_LO, &temp);
	temp |= (PHY_RX_OVRD_IN_LO_RX_DATA_EN | PHY_RX_OVRD_IN_LO_RX_PLL_EN);
	pcie_phy_write(priv->dbi_base, PHY_RX_OVRD_IN_LO, temp);

	udelay(3000);

	pcie_phy_read(priv->dbi_base, PHY_RX_OVRD_IN_LO, &temp);
	temp &= ~(PHY_RX_OVRD_IN_LO_RX_DATA_EN | PHY_RX_OVRD_IN_LO_RX_PLL_EN);
	pcie_phy_write(priv->dbi_base, PHY_RX_OVRD_IN_LO, temp);

	return 0;
}

/*
 * iATU region setup
 */
static int imx_pcie_regions_setup(struct imx_pcie_priv *priv)
{
	/*
	 * i.MX6 defines 16MB in the AXI address map for PCIe.
	 *
	 * That address space excepted the pcie registers is
	 * split and defined into different regions by iATU,
	 * with sizes and offsets as follows:
	 *
	 * 0x0100_0000 --- 0x010F_FFFF 1MB IORESOURCE_IO
	 * 0x0110_0000 --- 0x01EF_FFFF 14MB IORESOURCE_MEM
	 * 0x01F0_0000 --- 0x01FF_FFFF 1MB Cfg + Registers
	 */

	/* CMD reg:I/O space, MEM space, and Bus Master Enable */
	setbits_le32(priv->dbi_base + PCI_COMMAND,
		     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	/* Set the CLASS_REV of RC CFG header to PCI_CLASS_BRIDGE_PCI */
	setbits_le32(priv->dbi_base + PCI_CLASS_REVISION,
		     PCI_CLASS_BRIDGE_PCI << 16);

	/* Region #0 is used for Outbound CFG space access. */
	writel(0, priv->dbi_base + PCIE_ATU_VIEWPORT);

	writel(lower_32_bits((uintptr_t)priv->cfg_base),
	       priv->dbi_base + PCIE_ATU_LOWER_BASE);
	writel(upper_32_bits((uintptr_t)priv->cfg_base),
	       priv->dbi_base + PCIE_ATU_UPPER_BASE);
	writel(lower_32_bits((uintptr_t)priv->cfg_base + MX6_ROOT_SIZE),
	       priv->dbi_base + PCIE_ATU_LIMIT);

	writel(0, priv->dbi_base + PCIE_ATU_LOWER_TARGET);
	writel(0, priv->dbi_base + PCIE_ATU_UPPER_TARGET);
	writel(PCIE_ATU_TYPE_CFG0, priv->dbi_base + PCIE_ATU_CR1);
	writel(PCIE_ATU_ENABLE, priv->dbi_base + PCIE_ATU_CR2);

	return 0;
}

/*
 * PCI Express accessors
 */
static void __iomem *get_bus_address(struct imx_pcie_priv *priv,
				     pci_dev_t d, int where)
{
	void __iomem *va_address;

	/* Reconfigure Region #0 */
	writel(0, priv->dbi_base + PCIE_ATU_VIEWPORT);

	if (PCI_BUS(d) < 2)
		writel(PCIE_ATU_TYPE_CFG0, priv->dbi_base + PCIE_ATU_CR1);
	else
		writel(PCIE_ATU_TYPE_CFG1, priv->dbi_base + PCIE_ATU_CR1);

	if (PCI_BUS(d) == 0) {
		va_address = priv->dbi_base;
	} else {
		writel(d << 8, priv->dbi_base + PCIE_ATU_LOWER_TARGET);
		va_address = priv->cfg_base;
	}

	va_address += (where & ~0x3);

	return va_address;
}

static int imx_pcie_addr_valid(pci_dev_t d)
{
	if ((PCI_BUS(d) == 0) && (PCI_DEV(d) > 1))
		return -EINVAL;
	if ((PCI_BUS(d) == 1) && (PCI_DEV(d) > 0))
		return -EINVAL;
	return 0;
}

/*
 * Replace the original ARM DABT handler with a simple jump-back one.
 *
 * The problem here is that if we have a PCIe bridge attached to this PCIe
 * controller, but no PCIe device is connected to the bridges' downstream
 * port, the attempt to read/write from/to the config space will produce
 * a DABT. This is a behavior of the controller and can not be disabled
 * unfortuatelly.
 *
 * To work around the problem, we backup the current DABT handler address
 * and replace it with our own DABT handler, which only bounces right back
 * into the code.
 */
static void imx_pcie_fix_dabt_handler(bool set)
{
	extern uint32_t *_data_abort;
	uint32_t *data_abort_addr = (uint32_t *)&_data_abort;

	static const uint32_t data_abort_bounce_handler = 0xe25ef004;
	uint32_t data_abort_bounce_addr = (uint32_t)&data_abort_bounce_handler;

	static uint32_t data_abort_backup;

	if (set) {
		data_abort_backup = *data_abort_addr;
		*data_abort_addr = data_abort_bounce_addr;
	} else {
		*data_abort_addr = data_abort_backup;
	}
}

static int imx_pcie_read_cfg(struct imx_pcie_priv *priv, pci_dev_t d,
			     int where, u32 *val)
{
	void __iomem *va_address;
	int ret;

	ret = imx_pcie_addr_valid(d);
	if (ret) {
		*val = 0xffffffff;
		return 0;
	}

	va_address = get_bus_address(priv, d, where);

	/*
	 * Read the PCIe config space. We must replace the DABT handler
	 * here in case we got data abort from the PCIe controller, see
	 * imx_pcie_fix_dabt_handler() description. Note that writing the
	 * "val" with valid value is also imperative here as in case we
	 * did got DABT, the val would contain random value.
	 */
	imx_pcie_fix_dabt_handler(true);
	writel(0xffffffff, val);
	*val = readl(va_address);
	imx_pcie_fix_dabt_handler(false);

	return 0;
}

static int imx_pcie_write_cfg(struct imx_pcie_priv *priv, pci_dev_t d,
			      int where, u32 val)
{
	void __iomem *va_address = NULL;
	int ret;

	ret = imx_pcie_addr_valid(d);
	if (ret)
		return ret;

	va_address = get_bus_address(priv, d, where);

	/*
	 * Write the PCIe config space. We must replace the DABT handler
	 * here in case we got data abort from the PCIe controller, see
	 * imx_pcie_fix_dabt_handler() description.
	 */
	imx_pcie_fix_dabt_handler(true);
	writel(val, va_address);
	imx_pcie_fix_dabt_handler(false);

	return 0;
}

/*
 * Initial bus setup
 */
static int imx6_pcie_assert_core_reset(struct imx_pcie_priv *priv,
				       bool prepare_for_boot)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	if (is_mx6dqp())
		setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_PCIE_SW_RST);

#if defined(CONFIG_MX6SX)
	struct gpc *gpc_regs = (struct gpc *)GPC_BASE_ADDR;

	/* SSP_EN is not used on MX6SX anymore */
	setbits_le32(&iomuxc_regs->gpr[12], IOMUXC_GPR12_TEST_POWERDOWN);
	/* Force PCIe PHY reset */
	setbits_le32(&iomuxc_regs->gpr[5], IOMUXC_GPR5_PCIE_BTNRST);
	/* Power up PCIe PHY */
	setbits_le32(&gpc_regs->cntr, PCIE_PHY_PUP_REQ);
#else
	/*
	 * If the bootloader already enabled the link we need some special
	 * handling to get the core back into a state where it is safe to
	 * touch it for configuration.  As there is no dedicated reset signal
	 * wired up for MX6QDL, we need to manually force LTSSM into "detect"
	 * state before completely disabling LTSSM, which is a prerequisite
	 * for core configuration.
	 *
	 * If both LTSSM_ENABLE and REF_SSP_ENABLE are active we have a strong
	 * indication that the bootloader activated the link.
	 */
	if (is_mx6dq() && prepare_for_boot) {
		u32 val, gpr1, gpr12;

		gpr1 = readl(&iomuxc_regs->gpr[1]);
		gpr12 = readl(&iomuxc_regs->gpr[12]);
		if ((gpr1 & IOMUXC_GPR1_PCIE_REF_CLK_EN) &&
		    (gpr12 & IOMUXC_GPR12_PCIE_CTL_2)) {
			val = readl(priv->dbi_base + PCIE_PL_PFLR);
			val &= ~PCIE_PL_PFLR_LINK_STATE_MASK;
			val |= PCIE_PL_PFLR_FORCE_LINK;

			imx_pcie_fix_dabt_handler(true);
			writel(val, priv->dbi_base + PCIE_PL_PFLR);
			imx_pcie_fix_dabt_handler(false);

			gpr12 &= ~IOMUXC_GPR12_PCIE_CTL_2;
			writel(val, &iomuxc_regs->gpr[12]);
		}
	}
	setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_TEST_POWERDOWN);
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_REF_SSP_EN);
#endif

	return 0;
}

static int imx6_pcie_init_phy(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	clrbits_le32(&iomuxc_regs->gpr[12], IOMUXC_GPR12_APPS_LTSSM_ENABLE);

	clrsetbits_le32(&iomuxc_regs->gpr[12],
			IOMUXC_GPR12_DEVICE_TYPE_MASK,
			IOMUXC_GPR12_DEVICE_TYPE_RC);
	clrsetbits_le32(&iomuxc_regs->gpr[12],
			IOMUXC_GPR12_LOS_LEVEL_MASK,
			IOMUXC_GPR12_LOS_LEVEL_9);

#ifdef CONFIG_MX6SX
	clrsetbits_le32(&iomuxc_regs->gpr[12],
			IOMUXC_GPR12_RX_EQ_MASK,
			IOMUXC_GPR12_RX_EQ_2);
#endif

	writel((0x0 << IOMUXC_GPR8_PCS_TX_DEEMPH_GEN1_OFFSET) |
	       (0x0 << IOMUXC_GPR8_PCS_TX_DEEMPH_GEN2_3P5DB_OFFSET) |
	       (20 << IOMUXC_GPR8_PCS_TX_DEEMPH_GEN2_6DB_OFFSET) |
	       (127 << IOMUXC_GPR8_PCS_TX_SWING_FULL_OFFSET) |
	       (127 << IOMUXC_GPR8_PCS_TX_SWING_LOW_OFFSET),
	       &iomuxc_regs->gpr[8]);

	return 0;
}

__weak int imx6_pcie_toggle_power(void)
{
#ifdef CONFIG_PCIE_IMX_POWER_GPIO
	gpio_request(CONFIG_PCIE_IMX_POWER_GPIO, "pcie_power");
	gpio_direction_output(CONFIG_PCIE_IMX_POWER_GPIO, 0);
	mdelay(20);
	gpio_set_value(CONFIG_PCIE_IMX_POWER_GPIO, 1);
	mdelay(20);
	gpio_free(CONFIG_PCIE_IMX_POWER_GPIO);
#endif
	return 0;
}

__weak int imx6_pcie_toggle_reset(void)
{
	/*
	 * See 'PCI EXPRESS BASE SPECIFICATION, REV 3.0, SECTION 6.6.1'
	 * for detailed understanding of the PCIe CR reset logic.
	 *
	 * The PCIe #PERST reset line _MUST_ be connected, otherwise your
	 * design does not conform to the specification. You must wait at
	 * least 20 ms after de-asserting the #PERST so the EP device can
	 * do self-initialisation.
	 *
	 * In case your #PERST pin is connected to a plain GPIO pin of the
	 * CPU, you can define CONFIG_PCIE_IMX_PERST_GPIO in your board's
	 * configuration file and the condition below will handle the rest
	 * of the reset toggling.
	 *
	 * In case your #PERST toggling logic is more complex, for example
	 * connected via CPLD or somesuch, you can override this function
	 * in your board file and implement reset logic as needed. You must
	 * not forget to wait at least 20 ms after de-asserting #PERST in
	 * this case either though.
	 *
	 * In case your #PERST line of the PCIe EP device is not connected
	 * at all, your design is broken and you should fix your design,
	 * otherwise you will observe problems like for example the link
	 * not coming up after rebooting the system back from running Linux
	 * that uses the PCIe as well OR the PCIe link might not come up in
	 * Linux at all in the first place since it's in some non-reset
	 * state due to being previously used in U-Boot.
	 */
#ifdef CONFIG_PCIE_IMX_PERST_GPIO
	gpio_request(CONFIG_PCIE_IMX_PERST_GPIO, "pcie_reset");
	gpio_direction_output(CONFIG_PCIE_IMX_PERST_GPIO, 0);
	mdelay(20);
	gpio_set_value(CONFIG_PCIE_IMX_PERST_GPIO, 1);
	mdelay(20);
	gpio_free(CONFIG_PCIE_IMX_PERST_GPIO);
#else
	puts("WARNING: Make sure the PCIe #PERST line is connected!\n");
#endif
	return 0;
}

static int imx6_pcie_deassert_core_reset(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	imx6_pcie_toggle_power();

	enable_pcie_clock();

	if (is_mx6dqp())
		clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_PCIE_SW_RST);

	/*
	 * Wait for the clock to settle a bit, when the clock are sourced
	 * from the CPU, we need about 30 ms to settle.
	 */
	mdelay(50);

#if defined(CONFIG_MX6SX)
	/* SSP_EN is not used on MX6SX anymore */
	clrbits_le32(&iomuxc_regs->gpr[12], IOMUXC_GPR12_TEST_POWERDOWN);
	/* Clear PCIe PHY reset bit */
	clrbits_le32(&iomuxc_regs->gpr[5], IOMUXC_GPR5_PCIE_BTNRST);
#else
	/* Enable PCIe */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_TEST_POWERDOWN);
	setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_REF_SSP_EN);
#endif

	imx6_pcie_toggle_reset();

	return 0;
}

static int imx_pcie_link_up(struct imx_pcie_priv *priv)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	uint32_t tmp;
	int count = 0;

	imx6_pcie_assert_core_reset(priv, false);
	imx6_pcie_init_phy();
	imx6_pcie_deassert_core_reset();

	imx_pcie_regions_setup(priv);

	/*
	 * By default, the subordinate is set equally to the secondary
	 * bus (0x01) when the RC boots.
	 * This means that theoretically, only bus 1 is reachable from the RC.
	 * Force the PCIe RC subordinate to 0xff, otherwise no downstream
	 * devices will be detected if the enumeration is applied strictly.
	 */
	tmp = readl(priv->dbi_base + 0x18);
	tmp |= (0xff << 16);
	writel(tmp, priv->dbi_base + 0x18);

	/*
	 * FIXME: Force the PCIe RC to Gen1 operation
	 * The RC must be forced into Gen1 mode before bringing the link
	 * up, otherwise no downstream devices are detected. After the
	 * link is up, a managed Gen1->Gen2 transition can be initiated.
	 */
	tmp = readl(priv->dbi_base + 0x7c);
	tmp &= ~0xf;
	tmp |= 0x1;
	writel(tmp, priv->dbi_base + 0x7c);

	/* LTSSM enable, starting link. */
	setbits_le32(&iomuxc_regs->gpr[12], IOMUXC_GPR12_APPS_LTSSM_ENABLE);

	while (!imx6_pcie_link_up(priv)) {
		udelay(10);
		count++;
		if (count >= 4000) {
#ifdef CONFIG_PCI_SCAN_SHOW
			puts("PCI:   pcie phy link never came up\n");
#endif
			debug("DEBUG_R0: 0x%08x, DEBUG_R1: 0x%08x\n",
			      readl(priv->dbi_base + PCIE_PHY_DEBUG_R0),
			      readl(priv->dbi_base + PCIE_PHY_DEBUG_R1));
			return -EINVAL;
		}
	}

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_PCI)
static struct imx_pcie_priv imx_pcie_priv = {
	.dbi_base	= (void __iomem *)MX6_DBI_ADDR,
	.cfg_base	= (void __iomem *)MX6_ROOT_ADDR,
};

static struct imx_pcie_priv *priv = &imx_pcie_priv;

static int imx_pcie_read_config(struct pci_controller *hose, pci_dev_t d,
				int where, u32 *val)
{
	struct imx_pcie_priv *priv = hose->priv_data;

	return imx_pcie_read_cfg(priv, d, where, val);
}

static int imx_pcie_write_config(struct pci_controller *hose, pci_dev_t d,
				 int where, u32 val)
{
	struct imx_pcie_priv *priv = hose->priv_data;

	return imx_pcie_write_cfg(priv, d, where, val);
}

void imx_pcie_init(void)
{
	/* Static instance of the controller. */
	static struct pci_controller	pcc;
	struct pci_controller		*hose = &pcc;
	int ret;

	memset(&pcc, 0, sizeof(pcc));

	hose->priv_data = priv;

	/* PCI I/O space */
	pci_set_region(&hose->regions[0],
		       MX6_IO_ADDR, MX6_IO_ADDR,
		       MX6_IO_SIZE, PCI_REGION_IO);

	/* PCI memory space */
	pci_set_region(&hose->regions[1],
		       MX6_MEM_ADDR, MX6_MEM_ADDR,
		       MX6_MEM_SIZE, PCI_REGION_MEM);

	/* System memory space */
	pci_set_region(&hose->regions[2],
		       MMDC0_ARB_BASE_ADDR, MMDC0_ARB_BASE_ADDR,
		       0xefffffff, PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	hose->region_count = 3;

	pci_set_ops(hose,
		    pci_hose_read_config_byte_via_dword,
		    pci_hose_read_config_word_via_dword,
		    imx_pcie_read_config,
		    pci_hose_write_config_byte_via_dword,
		    pci_hose_write_config_word_via_dword,
		    imx_pcie_write_config);

	/* Start the controller. */
	ret = imx_pcie_link_up(priv);

	if (!ret) {
		pci_register_hose(hose);
		hose->last_busno = pci_hose_scan(hose);
	}
}

void imx_pcie_remove(void)
{
	imx6_pcie_assert_core_reset(priv, true);
}

/* Probe function. */
void pci_init_board(void)
{
	imx_pcie_init();
}
#else
static int imx_pcie_dm_read_config(struct udevice *dev, pci_dev_t bdf,
				   uint offset, ulong *value,
				   enum pci_size_t size)
{
	struct imx_pcie_priv *priv = dev_get_priv(dev);
	u32 tmpval;
	int ret;

	ret = imx_pcie_read_cfg(priv, bdf, offset, &tmpval);
	if (ret)
		return ret;

	*value = pci_conv_32_to_size(tmpval, offset, size);
	return 0;
}

static int imx_pcie_dm_write_config(struct udevice *dev, pci_dev_t bdf,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	struct imx_pcie_priv *priv = dev_get_priv(dev);
	u32 tmpval, newval;
	int ret;

	ret = imx_pcie_read_cfg(priv, bdf, offset, &tmpval);
	if (ret)
		return ret;

	newval = pci_conv_size_to_32(tmpval, value, offset, size);
	return imx_pcie_write_cfg(priv, bdf, offset, newval);
}

static int imx_pcie_dm_probe(struct udevice *dev)
{
	struct imx_pcie_priv *priv = dev_get_priv(dev);

	return imx_pcie_link_up(priv);
}

static int imx_pcie_dm_remove(struct udevice *dev)
{
	struct imx_pcie_priv *priv = dev_get_priv(dev);

	imx6_pcie_assert_core_reset(priv, true);

	return 0;
}

static int imx_pcie_ofdata_to_platdata(struct udevice *dev)
{
	struct imx_pcie_priv *priv = dev_get_priv(dev);

	priv->dbi_base = (void __iomem *)devfdt_get_addr_index(dev, 0);
	priv->cfg_base = (void __iomem *)devfdt_get_addr_index(dev, 1);
	if (!priv->dbi_base || !priv->cfg_base)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops imx_pcie_ops = {
	.read_config	= imx_pcie_dm_read_config,
	.write_config	= imx_pcie_dm_write_config,
};

static const struct udevice_id imx_pcie_ids[] = {
	{ .compatible = "fsl,imx6q-pcie" },
	{ }
};

U_BOOT_DRIVER(imx_pcie) = {
	.name			= "imx_pcie",
	.id			= UCLASS_PCI,
	.of_match		= imx_pcie_ids,
	.ops			= &imx_pcie_ops,
	.probe			= imx_pcie_dm_probe,
	.remove			= imx_pcie_dm_remove,
	.ofdata_to_platdata	= imx_pcie_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct imx_pcie_priv),
	.flags			= DM_FLAG_OS_PREPARE,
};
#endif
