// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Gumstix Inc. <www.gumstix.com>
 * Maintainer: Ash Charles  <ash@gumstix.com>
 */
#include <common.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <twl6030.h>
#include <asm/emif.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>

#include "duovero_mux_data.h"

#define WIFI_EN	43

#if defined(CONFIG_CMD_NET)
#define SMSC_NRESET	45
static void setup_net_chip(void);
#endif

#ifdef CONFIG_USB_EHCI_HCD
#include <usb.h>
#include <asm/arch/ehci.h>
#include <asm/ehci-omap.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	"Board: duovero\n"
};

struct omap4_scrm_regs *const scrm = (struct omap4_scrm_regs *)0x4a30a000;

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	gpmc_init();

	gd->bd->bi_arch_number = MACH_TYPE_DUOVERO;
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

/**
 * @brief misc_init_r - Configure board specific configurations
 * such as power configurations, ethernet initialization as phase2 of
 * boot sequence
 *
 * @return 0
 */
int misc_init_r(void)
{
	int ret = 0;
	u8 val;

	/* wifi setup: first enable 32Khz clock from 6030 pmic */
	val = 0xe1;
	ret = i2c_write(TWL6030_CHIP_PM, 0xbe, 1, &val, 1);
	if (ret)
		printf("Failed to enable 32Khz clock to wifi module\n");

	/* then setup WIFI_EN as an output pin and send reset pulse */
	if (!gpio_request(WIFI_EN, "")) {
		gpio_direction_output(WIFI_EN, 0);
		gpio_set_value(WIFI_EN, 1);
		udelay(1);
		gpio_set_value(WIFI_EN, 0);
		udelay(1);
		gpio_set_value(WIFI_EN, 1);
	}

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif
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

	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_non_essential,
		   sizeof(core_padconf_array_non_essential) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_non_essential,
		   sizeof(wkup_padconf_array_non_essential) /
		   sizeof(struct pad_conf_entry));
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}

#if !defined(CONFIG_SPL_BUILD)
void board_mmc_power_init(void)
{
	twl6030_power_mmc_init(0);
}
#endif
#endif

#if defined(CONFIG_CMD_NET)

#define GPMC_SIZE_16M	0xF
#define GPMC_BASEADDR_MASK	0x3F
#define GPMC_CS_ENABLE		0x1

static void enable_gpmc_net_config(const u32 *gpmc_config, const struct gpmc_cs *cs,
		u32 base, u32 size)
{
	writel(0, &cs->config7);
	sdelay(1000);
	/* Delay for settling */
	writel(gpmc_config[0], &cs->config1);
	writel(gpmc_config[1], &cs->config2);
	writel(gpmc_config[2], &cs->config3);
	writel(gpmc_config[3], &cs->config4);
	writel(gpmc_config[4], &cs->config5);
	writel(gpmc_config[5], &cs->config6);

	/*
	 * Enable the config.  size is the CS size and goes in
	 * bits 11:8.  We set bit 6 to enable this CS and the base
	 * address goes into bits 5:0.
	 */
	writel((size << 8) | (GPMC_CS_ENABLE << 6) |
				 ((base >> 24) & GPMC_BASEADDR_MASK),
				 &cs->config7);

	sdelay(2000);
}

/* GPMC CS configuration for an SMSC LAN9221 ethernet controller */
#define NET_LAN9221_GPMC_CONFIG1    0x2a001203
#define NET_LAN9221_GPMC_CONFIG2    0x000a0a02
#define NET_LAN9221_GPMC_CONFIG3    0x00020200
#define NET_LAN9221_GPMC_CONFIG4    0x0a030a03
#define NET_LAN9221_GPMC_CONFIG5    0x000a0a0a
#define NET_LAN9221_GPMC_CONFIG6    0x8a070707
#define NET_LAN9221_GPMC_CONFIG7    0x00000f6c

/* GPMC definitions for LAN9221 chips on expansion boards */
static const u32 gpmc_lan_config[] = {
	NET_LAN9221_GPMC_CONFIG1,
	NET_LAN9221_GPMC_CONFIG2,
	NET_LAN9221_GPMC_CONFIG3,
	NET_LAN9221_GPMC_CONFIG4,
	NET_LAN9221_GPMC_CONFIG5,
	NET_LAN9221_GPMC_CONFIG6,
	/*CONFIG7- computed as params */
};

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *	      Ethernet hardware.
 */
static void setup_net_chip(void)
{
	enable_gpmc_net_config(gpmc_lan_config, &gpmc_cfg->cs[5], 0x2C000000,
			      GPMC_SIZE_16M);

	/* Make GPIO SMSC_NRESET as output pin and send reset pulse */
	if (!gpio_request(SMSC_NRESET, "")) {
		gpio_direction_output(SMSC_NRESET, 0);
		gpio_set_value(SMSC_NRESET, 1);
		udelay(1);
		gpio_set_value(SMSC_NRESET, 0);
		udelay(1);
		gpio_set_value(SMSC_NRESET, 1);
	}
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

#ifdef CONFIG_USB_EHCI_HCD

static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	int ret;
	unsigned int utmi_clk;
	u32 auxclk, altclksrc;

	/* Now we can enable our port clocks */
	utmi_clk = readl((void *)CM_L3INIT_HSUSBHOST_CLKCTRL);
	utmi_clk |= HSUSBHOST_CLKCTRL_CLKSEL_UTMI_P1_MASK;
	setbits_le32((void *)CM_L3INIT_HSUSBHOST_CLKCTRL, utmi_clk);

	auxclk = readl(&scrm->auxclk3);
	/* Select sys_clk */
	auxclk &= ~AUXCLK_SRCSELECT_MASK;
	auxclk |=  AUXCLK_SRCSELECT_SYS_CLK << AUXCLK_SRCSELECT_SHIFT;
	/* Set the divisor to 2 */
	auxclk &= ~AUXCLK_CLKDIV_MASK;
	auxclk |= AUXCLK_CLKDIV_2 << AUXCLK_CLKDIV_SHIFT;
	/* Request auxilary clock #3 */
	auxclk |= AUXCLK_ENABLE_MASK;
	writel(auxclk, &scrm->auxclk3);

	altclksrc = readl(&scrm->altclksrc);

	/* Activate alternate system clock supplier */
	altclksrc &= ~ALTCLKSRC_MODE_MASK;
	altclksrc |= ALTCLKSRC_MODE_ACTIVE;

	/* enable clocks */
	altclksrc |= ALTCLKSRC_ENABLE_INT_MASK | ALTCLKSRC_ENABLE_EXT_MASK;

	writel(altclksrc, &scrm->altclksrc);

	ret = omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
	if (ret < 0)
		return ret;

	return 0;
}

int ehci_hcd_stop(int index)
{
	return omap_ehci_hcd_stop();
}
#endif

/*
 * get_board_rev() - get board revision
 */
u32 get_board_rev(void)
{
	return 0x20;
}
