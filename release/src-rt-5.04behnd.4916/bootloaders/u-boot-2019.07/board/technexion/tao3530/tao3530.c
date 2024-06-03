// SPDX-License-Identifier: GPL-2.0+
/*
 * Maintainer :
 *      Tapani Utriainen <linuxfae@technexion.com>
 */
#include <common.h>
#include <netdev.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>

#include <usb.h>
#include <asm/ehci-omap.h>

#include "tao3530.h"

DECLARE_GLOBAL_DATA_PTR;

int tao3530_revision(void)
{
	int ret = 0;

	/* char *label argument is unused in gpio_request() */
	ret = gpio_request(65, "");
	if (ret) {
		puts("Error: GPIO 65 not available\n");
		goto out;
	}
	MUX_VAL(CP(GPMC_WAIT3),	(IEN  | PTU | EN  | M4));

	ret = gpio_request(1, "");
	if (ret) {
		puts("Error: GPIO 1 not available\n");
		goto out2;
	}
	MUX_VAL(CP(SYS_CLKREQ), (IEN  | PTU | EN | M4));

	ret = gpio_direction_input(65);
	if (ret) {
		puts("Error: GPIO 65 not available for input\n");
		goto out3;
	}

	ret =  gpio_direction_input(1);
	if (ret) {
		puts("Error: GPIO 1 not available for input\n");
		goto out3;
	}

	ret = gpio_get_value(65) << 1 | gpio_get_value(1);

out3:
	MUX_VAL(CP(SYS_CLKREQ), (IEN  | PTU | EN | M0));
	gpio_free(1);
out2:
	MUX_VAL(CP(GPMC_WAIT3),	(IEN  | PTU | EN  | M0));
	gpio_free(65);
out:

	return ret;
}

#ifdef CONFIG_SPL_BUILD
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on both banks.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
#if defined(CONFIG_SYS_BOARD_OMAP3_HA)
	/*
	 * Switch baseboard LED to red upon power-on
	 */
	MUX_OMAP3_HA();

	/* Request a gpio before using it */
	gpio_request(111, "");
	/* Sets the gpio as output and its value to 1, switch LED to red */
	gpio_direction_output(111, 1);
#endif

	if (tao3530_revision() < 3) {
		/* 256MB / Bank */
		timings->mcfg = MCFG(256 << 20, 14);	/* RAS-width 14 */
		timings->ctrla = HYNIX_V_ACTIMA_165;
		timings->ctrlb = HYNIX_V_ACTIMB_165;
	} else {
		/* 128MB / Bank */
		timings->mcfg = MCFG(128 << 20, 13);	/* RAS-width 13 */
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
	}

	timings->mr = MICRON_V_MR_165;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
}
#endif

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_TAO3530;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

	/* Configure GPIOs to output */
	/* GPIO23 */
	writel(~(GPIO10 | GPIO8 | GPIO2 | GPIO1), &gpio6_base->oe);
	writel(~(GPIO31 | GPIO30 | GPIO22 | GPIO21 |
		 GPIO15 | GPIO14 | GPIO13 | GPIO12), &gpio5_base->oe);

	/* Set GPIOs */
	writel(GPIO10 | GPIO8 | GPIO2 | GPIO1,
	       &gpio6_base->setdataout);
	writel(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
	       GPIO15 | GPIO14 | GPIO13 | GPIO12, &gpio5_base->setdataout);

	switch (tao3530_revision()) {
	case 0:
		puts("TAO-3530 REV Reserve 1\n");
		break;
	case 1:
		puts("TAO-3530 REV Reserve 2\n");
		break;
	case 2:
		puts("TAO-3530 REV Cx\n");
		break;
	case 3:
		puts("TAO-3530 REV Ax/Bx\n");
		break;
	default:
		puts("Unknown board revision\n");
	}

	omap_die_id_display();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_TAO3530();
#if defined(CONFIG_SYS_BOARD_OMAP3_HA)
	MUX_OMAP3_HA();
#endif
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);

	return 0;
}
#endif

#if defined(CONFIG_MMC)
void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

#if defined(CONFIG_USB_EHCI_HCD) && !defined(CONFIG_SPL_BUILD)
/* Call usb_stop() before starting the kernel */
void show_boot_progress(int val)
{
	if (val == BOOTSTAGE_ID_RUN_OS)
		usb_stop();
}

static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED
};

int ehci_hcd_init(int index, enum usb_init_type init,
		  struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	return omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(int index)
{
	return omap_ehci_hcd_stop();
}
#endif /* CONFIG_USB_EHCI_HCD */
