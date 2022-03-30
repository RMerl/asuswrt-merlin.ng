// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 * Aneesh V       <aneesh@ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 */
#include <common.h>
#include <palmas.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <tca642x.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <dwc3-uboot.h>
#include <dwc3-omap-uboot.h>
#include <ti-usb-phy-uboot.h>

#include "mux_data.h"

#if defined(CONFIG_USB_EHCI_HCD) || defined(CONFIG_USB_XHCI_OMAP)
#include <sata.h>
#include <usb.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <asm/arch/clock.h>
#include <asm/arch/ehci.h>
#include <asm/ehci-omap.h>
#include <asm/arch/sata.h>

#define DIE_ID_REG_BASE     (OMAP54XX_L4_CORE_BASE + 0x2000)
#define DIE_ID_REG_OFFSET	0x200

#endif

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	"Board: OMAP5432 uEVM\n"
};

/**
 * @brief tca642x_init - uEVM default values for the GPIO expander
 * input reg, output reg, polarity reg, configuration reg
 */
struct tca642x_bank_info tca642x_init[] = {
	{ .input_reg = 0x00,
	  .output_reg = 0x04,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x80 },
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0xff },
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x40 },
};

#ifdef CONFIG_USB_DWC3
static struct dwc3_device usb_otg_ss = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = OMAP5XX_USB_OTG_SS_BASE,
	.tx_fifo_resize = false,
	.index = 0,
};

static struct dwc3_omap_device usb_otg_ss_glue = {
	.base = (void *)OMAP5XX_USB_OTG_SS_GLUE_BASE,
	.utmi_mode = DWC3_OMAP_UTMI_MODE_SW,
	.index = 0,
};

static struct ti_usb_phy_device usb_phy_device = {
	.pll_ctrl_base = (void *)OMAP5XX_USB3_PHY_PLL_CTRL,
	.usb2_phy_power = (void *)OMAP5XX_USB2_PHY_POWER,
	.usb3_phy_power = (void *)OMAP5XX_USB3_PHY_POWER,
	.index = 0,
};

int board_usb_init(int index, enum usb_init_type init)
{
	if (index) {
		printf("Invalid Controller Index\n");
		return -EINVAL;
	}

	if (init == USB_INIT_DEVICE) {
		usb_otg_ss.dr_mode = USB_DR_MODE_PERIPHERAL;
		usb_otg_ss_glue.vbus_id_status = OMAP_DWC3_VBUS_VALID;
	} else {
		usb_otg_ss.dr_mode = USB_DR_MODE_HOST;
		usb_otg_ss_glue.vbus_id_status = OMAP_DWC3_ID_GROUND;
	}

	enable_usb_clocks(index);
	ti_usb_phy_uboot_init(&usb_phy_device);
	dwc3_omap_uboot_init(&usb_otg_ss_glue);
	dwc3_uboot_init(&usb_otg_ss);

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	if (index) {
		printf("Invalid Controller Index\n");
		return -EINVAL;
	}

	ti_usb_phy_uboot_exit(index);
	dwc3_uboot_exit(index);
	dwc3_omap_uboot_exit(index);
	disable_usb_clocks(index);

	return 0;
}

int usb_gadget_handle_interrupts(int index)
{
	u32 status;

	status = dwc3_omap_uboot_interrupt_status(index);
	if (status)
		dwc3_uboot_handle_interrupt(index);

	return 0;
}
#endif

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	gpmc_init();
	gd->bd->bi_arch_number = MACH_TYPE_OMAP5_SEVM;
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	tca642x_set_inital_state(CONFIG_SYS_I2C_TCA642X_ADDR, tca642x_init);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return 0;
}

#if defined(CONFIG_USB_EHCI_HCD) || defined(CONFIG_USB_XHCI_OMAP)
static void enable_host_clocks(void)
{
	int auxclk;
	int hs_clk_ctrl_val = (OPTFCLKEN_HSIC60M_P3_CLK |
				OPTFCLKEN_HSIC480M_P3_CLK |
				OPTFCLKEN_HSIC60M_P2_CLK |
				OPTFCLKEN_HSIC480M_P2_CLK |
				OPTFCLKEN_UTMI_P3_CLK | OPTFCLKEN_UTMI_P2_CLK);

	/* Enable port 2 and 3 clocks*/
	setbits_le32((*prcm)->cm_l3init_hsusbhost_clkctrl, hs_clk_ctrl_val);

	/* Enable port 2 and 3 usb host ports tll clocks*/
	setbits_le32((*prcm)->cm_l3init_hsusbtll_clkctrl,
			(OPTFCLKEN_USB_CH1_CLK_ENABLE | OPTFCLKEN_USB_CH2_CLK_ENABLE));
#ifdef CONFIG_USB_XHCI_OMAP
	/* Enable the USB OTG Super speed clocks */
	setbits_le32((*prcm)->cm_l3init_usb_otg_ss_clkctrl,
			(OPTFCLKEN_REFCLK960M | OTG_SS_CLKCTRL_MODULEMODE_HW));
#endif

	auxclk = readl((*prcm)->scrm_auxclk1);
	/* Request auxilary clock */
	auxclk |= AUXCLK_ENABLE_MASK;
	writel(auxclk, (*prcm)->scrm_auxclk1);
}
#endif

/**
 * @brief misc_init_r - Configure EVM board specific configurations
 * such as power configurations, ethernet initialization as phase2 of
 * boot sequence
 *
 * @return 0
 */
int misc_init_r(void)
{
#ifdef CONFIG_PALMAS_POWER
	palmas_init_settings();
#endif

	omap_die_id_usbethaddr();

	return 0;
}

void set_muxconf_regs(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_essential,
		   sizeof(core_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_essential,
		   sizeof(wkup_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#ifdef CONFIG_USB_EHCI_HCD
static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_HSIC,
	.port_mode[2] = OMAP_EHCI_PORT_MODE_HSIC,
};

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	int ret;

	enable_host_clocks();

	ret = omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
	if (ret < 0) {
		puts("Failed to initialize ehci\n");
		return ret;
	}

	return 0;
}

int ehci_hcd_stop(void)
{
	return omap_ehci_hcd_stop();
}

void usb_hub_reset_devices(struct usb_hub_device *hub, int port)
{
	/* The LAN9730 needs to be reset after the port power has been set. */
	if (port == 3) {
		gpio_direction_output(CONFIG_OMAP_EHCI_PHY3_RESET_GPIO, 0);
		udelay(10);
		gpio_direction_output(CONFIG_OMAP_EHCI_PHY3_RESET_GPIO, 1);
	}
}
#endif

#ifdef CONFIG_USB_XHCI_OMAP
/**
 * @brief board_usb_init - Configure EVM board specific configurations
 * for the LDO's and clocks for the USB blocks.
 *
 * @return 0
 */
int board_usb_init(int index, enum usb_init_type init)
{
	int ret;
#ifdef CONFIG_PALMAS_USB_SS_PWR
	ret = palmas_enable_ss_ldo();
#endif

	enable_host_clocks();

	return 0;
}
#endif
