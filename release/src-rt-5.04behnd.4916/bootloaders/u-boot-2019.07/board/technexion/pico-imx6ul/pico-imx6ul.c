// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Technexion Ltd.
 *
 * Author: Richard Hu <richard.hu@technexion.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <common.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/sizes.h>
#include <usb.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include "../../freescale/common/pfuze.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define OTG_ID_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define MDIO_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST | PAD_CTL_ODE)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_SPEED_HIGH   |                                   \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST)

#define ENET_CLK_PAD_CTRL  (PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define RMII_PHY_RESET IMX_GPIO_NR(1, 28)

static iomux_v3_cfg_t const fec_pads[] = {
	MX6_PAD_ENET1_TX_EN__ENET2_MDC		| MUX_PAD_CTRL(MDIO_PAD_CTRL),
	MX6_PAD_ENET1_TX_DATA1__ENET2_MDIO	| MUX_PAD_CTRL(MDIO_PAD_CTRL),
	MX6_PAD_ENET2_TX_DATA0__ENET2_TDATA00	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET2_TX_DATA1__ENET2_TDATA01	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET2_TX_CLK__ENET2_REF_CLK2	| MUX_PAD_CTRL(ENET_CLK_PAD_CTRL),
	MX6_PAD_ENET2_TX_EN__ENET2_TX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET2_RX_DATA0__ENET2_RDATA00	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET2_RX_DATA1__ENET2_RDATA01	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET2_RX_EN__ENET2_RX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET2_RX_ER__ENET2_RX_ER	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_UART4_TX_DATA__GPIO1_IO28	| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_fec();

	gpio_request(RMII_PHY_RESET, "enet_phy_reset");
	gpio_direction_output(RMII_PHY_RESET, 0);
	/*
	 * According to KSZ8081MNX-RNB manual:
	 * For warm reset, the reset (RST#) pin should be asserted low for a
	 * minimum of 500μs.  The strap-in pin values are read and updated
	 * at the de-assertion of reset.
	 */
	udelay(500);

	gpio_direction_output(RMII_PHY_RESET, 1);
	/*
	 * According to KSZ8081MNX-RNB manual:
	 * After the de-assertion of reset, wait a minimum of 100μs before
	 * starting programming on the MIIM (MDC/MDIO) interface.
	 */
	udelay(100);

	return fecmxc_initialize(bis);
}

static int setup_fec(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
			IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

	ret = enable_fec_anatop_clock(1, ENET_50MHZ);
	if (ret)
		return ret;

	enable_enet_clk(1);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0x8190);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart6_pads[] = {
	MX6_PAD_CSI_MCLK__UART6_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI_PIXCLK__UART6_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

static iomux_v3_cfg_t const usb_otg_pad[] = {
	MX6_PAD_GPIO1_IO00__ANATOP_OTG1_ID | MUX_PAD_CTRL(OTG_ID_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart6_pads, ARRAY_SIZE(uart6_pads));
}

static void setup_usb(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_otg_pad, ARRAY_SIZE(usb_otg_pad));
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

#ifdef CONFIG_DM_PMIC
int power_init_board(void)
{
	struct udevice *dev;
	int ret, dev_id, rev_id;

	ret = pmic_get("pfuze3000", &dev);
	if (ret == -ENODEV)
		return 0;
	if (ret != 0)
		return ret;

	dev_id = pmic_reg_read(dev, PFUZE3000_DEVICEID);
	rev_id = pmic_reg_read(dev, PFUZE3000_REVID);
	printf("PMIC: PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", dev_id, rev_id);

	/* disable Low Power Mode during standby mode */
	pmic_reg_write(dev, PFUZE3000_LDOGCTL, 0x1);

	/* SW1B step ramp up time from 2us to 4us/25mV */
	pmic_reg_write(dev, PFUZE3000_SW1BCONF, 0x40);

	/* SW1B mode to APS/PFM */
	pmic_reg_write(dev, PFUZE3000_SW1BMODE, 0xc);

	/* SW1B standby voltage set to 0.975V */
	pmic_reg_write(dev, PFUZE3000_SW1BSTBY, 0xb);

	return 0;
}
#endif

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return USB_INIT_DEVICE;
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	/* Set Power polarity */
	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_fec();
	setup_usb();

	return 0;
}

int checkboard(void)
{
	puts("Board: PICO-IMX6UL-EMMC\n");

	return 0;
}
