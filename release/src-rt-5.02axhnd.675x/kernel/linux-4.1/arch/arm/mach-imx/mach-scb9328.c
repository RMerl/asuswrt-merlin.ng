/*
 * linux/arch/arm/mach-mx1/mach-scb9328.c
 *
 * Copyright (c) 2004 Sascha Hauer <saschahauer@web.de>
 * Copyright (c) 2006-2008 Juergen Beisert <jbeisert@netscape.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/platform_device.h>
#include <linux/mtd/physmap.h>
#include <linux/interrupt.h>
#include <linux/dm9000.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include "common.h"
#include "devices-imx1.h"
#include "hardware.h"
#include "iomux-mx1.h"

/*
 * This scb9328 has a 32MiB flash
 */
static struct resource flash_resource = {
	.start	= MX1_CS0_PHYS,
	.end	= MX1_CS0_PHYS + (32 * 1024 * 1024) - 1,
	.flags	= IORESOURCE_MEM,
};

static struct physmap_flash_data scb_flash_data = {
	.width  = 2,
};

static struct platform_device scb_flash_device = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev = {
		.platform_data = &scb_flash_data,
	},
	.resource = &flash_resource,
	.num_resources = 1,
};

/*
 * scb9328 has a DM9000 network controller
 * connected to CS5, with 16 bit data path
 * and interrupt connected to GPIO 3
 */

/*
 * internal datapath is fixed 16 bit
 */
static struct dm9000_plat_data dm9000_platdata = {
	.flags	= DM9000_PLATF_16BITONLY,
};

/*
 * the DM9000 drivers wants two defined address spaces
 * to gain access to address latch registers and the data path.
 */
static struct resource dm9000x_resources[] = {
	{
		.name	= "address area",
		.start	= MX1_CS5_PHYS,
		.end	= MX1_CS5_PHYS + 1,
		.flags	= IORESOURCE_MEM,	/* address access */
	}, {
		.name	= "data area",
		.start	= MX1_CS5_PHYS + 4,
		.end	= MX1_CS5_PHYS + 5,
		.flags	= IORESOURCE_MEM,	/* data access */
	}, {
		/* irq number is run-time assigned */
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_LOWLEVEL,
	},
};

static struct platform_device dm9000x_device = {
	.name		= "dm9000",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(dm9000x_resources),
	.resource	= dm9000x_resources,
	.dev		= {
		.platform_data = &dm9000_platdata,
	}
};

static const int mxc_uart1_pins[] = {
	PC9_PF_UART1_CTS,
	PC10_PF_UART1_RTS,
	PC11_PF_UART1_TXD,
	PC12_PF_UART1_RXD,
};

static const struct imxuart_platform_data uart_pdata __initconst = {
	.flags = IMXUART_HAVE_RTSCTS,
};

static struct platform_device *devices[] __initdata = {
	&scb_flash_device,
	&dm9000x_device,
};

/*
 * scb9328_init - Init the CPU card itself
 */
static void __init scb9328_init(void)
{
	imx1_soc_init();

	mxc_gpio_setup_multiple_pins(mxc_uart1_pins,
			ARRAY_SIZE(mxc_uart1_pins), "UART1");

	imx1_add_imx_uart0(&uart_pdata);

	printk(KERN_INFO"Scb9328: Adding devices\n");
	dm9000x_resources[2].start = gpio_to_irq(IMX_GPIO_NR(3, 3));
	dm9000x_resources[2].end = gpio_to_irq(IMX_GPIO_NR(3, 3));
	platform_add_devices(devices, ARRAY_SIZE(devices));
}

static void __init scb9328_timer_init(void)
{
	mx1_clocks_init(32000);
}

MACHINE_START(SCB9328, "Synertronixx scb9328")
	/* Sascha Hauer */
	.atag_offset = 100,
	.map_io = mx1_map_io,
	.init_early = imx1_init_early,
	.init_irq = mx1_init_irq,
	.init_time	= scb9328_timer_init,
	.init_machine = scb9328_init,
	.restart	= mxc_restart,
MACHINE_END
