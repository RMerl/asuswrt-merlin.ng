// SPDX-License-Identifier: GPL-2.0+
/*
 * am3517evm.c - board file for TI's AM3517 family of devices.
 *
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Based on ti/evm/evm.c
 *
 * Copyright (C) 2010
 * Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/omap_musb.h>
#include <asm/arch/am35x_def.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/musb.h>
#include <asm/mach-types.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include <i2c.h>
#include <netdev.h>
#include "am3517evm.h"

DECLARE_GLOBAL_DATA_PTR;

#define AM3517_IP_SW_RESET	0x48002598
#define CPGMACSS_SW_RST		(1 << 1)
#define PHY_GPIO		30

#if defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_SPL_OS_BOOT)
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	return serial_tstc() && serial_getc() == 'c';
}
#endif
#endif

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3517EVM;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

#ifdef CONFIG_USB_MUSB_AM35X
static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

static struct omap_musb_board_data musb_board_data = {
	.set_phy_power		= am35x_musb_phy_power,
	.clear_irq		= am35x_musb_clear_irq,
	.reset			= am35x_musb_reset,
};

static struct musb_hdrc_platform_data musb_plat = {
#if defined(CONFIG_USB_MUSB_HOST)
	.mode           = MUSB_HOST,
#elif defined(CONFIG_USB_MUSB_GADGET)
	.mode		= MUSB_PERIPHERAL,
#else
#error "Please define either CONFIG_USB_MUSB_HOST or CONFIG_USB_MUSB_GADGET"
#endif
	.config         = &musb_config,
	.power          = 250,
	.platform_ops	= &am35x_ops,
	.board_data	= &musb_board_data,
};

static void am3517_evm_musb_init(void)
{
	/*
	 * Set up USB clock/mode in the DEVCONF2 register.
	 * USB2.0 PHY reference clock is 13 MHz
	 */
	clrsetbits_le32(&am35x_scm_general_regs->devconf2,
			CONF2_REFFREQ | CONF2_OTGMODE | CONF2_PHY_GPIOMODE,
			CONF2_REFFREQ_13MHZ | CONF2_SESENDEN |
			CONF2_VBDTCTEN | CONF2_DATPOL);

	musb_register(&musb_plat, &musb_board_data,
			(void *)AM35XX_IPSS_USBOTGSS_BASE);
}
#else
#define am3517_evm_musb_init() do {} while (0)
#endif

/*
 * Routine: misc_init_r
 * Description: Init i2c, ethernet, etc... (done here so udelay works)
 */
int misc_init_r(void)
{
	volatile unsigned int ctr;
	u32 reset;

#if !defined(CONFIG_DM_I2C)
#ifdef CONFIG_SYS_I2C_OMAP24XX
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);
#endif
#endif
	omap_die_id_display();

	am3517_evm_musb_init();

	if (gpio_request(PHY_GPIO, "gpio_30") == 0) {
		/* activate PHY reset */
		gpio_direction_output(PHY_GPIO, 0);
		gpio_set_value(PHY_GPIO, 0);

		ctr  = 0;
		do {
			udelay(1000);
			ctr++;
		} while (ctr < 300);

		/* deactivate PHY reset */
		gpio_set_value(PHY_GPIO, 1);

		/* allow the PHY to stabilize and settle down */
		ctr = 0;
		do {
			udelay(1000);
			ctr++;
		} while (ctr < 300);

		/* ensure that the module is out of reset */
		reset = readl(AM3517_IP_SW_RESET);
		reset &= (~CPGMACSS_SW_RST);
		writel(reset, AM3517_IP_SW_RESET);

		/* Free requested GPIO */
		gpio_free(PHY_GPIO);
	}

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
	MUX_AM3517EVM();
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#if defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET)
int board_eth_init(bd_t *bis)
{
	int rv, n = 0;

	rv = cpu_eth_init(bis);
	if (rv > 0)
		n += rv;

	rv = usb_eth_initialize(bis);
	if (rv > 0)
		n += rv;

	return n;
}
#endif
