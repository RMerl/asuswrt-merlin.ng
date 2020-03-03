/*
 * Copyright 2004-2009 Analog Devices Inc.
 *               2008-2009 Bluetechnix
 *               2005 National ICT Australia (NICTA)
 *                    Aidan Williams <aidan@nicta.com.au>
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#if IS_ENABLED(CONFIG_USB_ISP1362_HCD)
#include <linux/usb/isp1362.h>
#endif
#include <linux/ata_platform.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <asm/dma.h>
#include <asm/bfin5xx_spi.h>
#include <asm/portmux.h>
#include <asm/dpmc.h>
#include <linux/mtd/physmap.h>

/*
 * Name the Board for the /proc/cpuinfo
 */
const char bfin_board_name[] = "Bluetechnix CM BF561";

#if IS_ENABLED(CONFIG_SPI_BFIN5XX)
/* all SPI peripherals info goes here */

#if IS_ENABLED(CONFIG_MTD_M25P80)
static struct mtd_partition bfin_spi_flash_partitions[] = {
	{
		.name = "bootloader(spi)",
		.size = 0x00020000,
		.offset = 0,
		.mask_flags = MTD_CAP_ROM
	}, {
		.name = "linux kernel(spi)",
		.size = 0xe0000,
		.offset = 0x20000
	}, {
		.name = "file system(spi)",
		.size = 0x700000,
		.offset = 0x00100000,
	}
};

static struct flash_platform_data bfin_spi_flash_data = {
	.name = "m25p80",
	.parts = bfin_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(bfin_spi_flash_partitions),
	.type = "m25p64",
};

/* SPI flash chip (m25p64) */
static struct bfin5xx_spi_chip spi_flash_chip_info = {
	.enable_dma = 0,         /* use dma transfer with this chip*/
};
#endif

static struct spi_board_info bfin_spi_board_info[] __initdata = {
#if IS_ENABLED(CONFIG_MTD_M25P80)
	{
		/* the modalias must be the same as spi device driver name */
		.modalias = "m25p80", /* Name of spi_driver for this device */
		.max_speed_hz = 25000000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 0, /* Framework bus number */
		.chip_select = 1, /* Framework chip select. On STAMP537 it is SPISSEL1*/
		.platform_data = &bfin_spi_flash_data,
		.controller_data = &spi_flash_chip_info,
		.mode = SPI_MODE_3,
	},
#endif

#if IS_ENABLED(CONFIG_SND_BF5XX_SOC_AD183X)
	{
		.modalias = "ad183x",
		.max_speed_hz = 3125000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 0,
		.chip_select = 4,
	},
#endif
#if IS_ENABLED(CONFIG_MMC_SPI)
	{
		.modalias = "mmc_spi",
		.max_speed_hz = 20000000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 0,
		.chip_select = 1,
		.mode = SPI_MODE_3,
	},
#endif
};

/* SPI (0) */
static struct resource bfin_spi0_resource[] = {
	[0] = {
		.start = SPI0_REGBASE,
		.end   = SPI0_REGBASE + 0xFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = CH_SPI,
		.end   = CH_SPI,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = IRQ_SPI,
		.end   = IRQ_SPI,
		.flags = IORESOURCE_IRQ,
	},
};

/* SPI controller data */
static struct bfin5xx_spi_master bfin_spi0_info = {
	.num_chipselect = 8,
	.enable_dma = 1,  /* master has the ability to do dma transfer */
	.pin_req = {P_SPI0_SCK, P_SPI0_MISO, P_SPI0_MOSI, 0},
};

static struct platform_device bfin_spi0_device = {
	.name = "bfin-spi",
	.id = 0, /* Bus number */
	.num_resources = ARRAY_SIZE(bfin_spi0_resource),
	.resource = bfin_spi0_resource,
	.dev = {
		.platform_data = &bfin_spi0_info, /* Passed to driver */
	},
};
#endif  /* spi master and devices */


#if IS_ENABLED(CONFIG_FB_HITACHI_TX09)
static struct platform_device hitachi_fb_device = {
	.name = "hitachi-tx09",
};
#endif


#if IS_ENABLED(CONFIG_SMC91X)
#include <linux/smc91x.h>

static struct smc91x_platdata smc91x_info = {
	.flags = SMC91X_USE_32BIT | SMC91X_NOWAIT,
	.leda = RPC_LED_100_10,
	.ledb = RPC_LED_TX_RX,
};

static struct resource smc91x_resources[] = {
	{
		.name = "smc91x-regs",
		.start = 0x28000300,
		.end = 0x28000300 + 16,
		.flags = IORESOURCE_MEM,
	}, {
		.start = IRQ_PF0,
		.end = IRQ_PF0,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};
static struct platform_device smc91x_device = {
	.name = "smc91x",
	.id = 0,
	.num_resources = ARRAY_SIZE(smc91x_resources),
	.resource = smc91x_resources,
	.dev	= {
		.platform_data	= &smc91x_info,
	},
};
#endif

#if IS_ENABLED(CONFIG_SMSC911X)
#include <linux/smsc911x.h>

static struct resource smsc911x_resources[] = {
	{
		.name = "smsc911x-memory",
		.start = 0x24008000,
		.end = 0x24008000 + 0xFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_PF43,
		.end = IRQ_PF43,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWLEVEL,
	},
};

static struct smsc911x_platform_config smsc911x_config = {
	.flags = SMSC911X_USE_16BIT,
	.irq_polarity = SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type = SMSC911X_IRQ_TYPE_OPEN_DRAIN,
	.phy_interface = PHY_INTERFACE_MODE_MII,
};

static struct platform_device smsc911x_device = {
	.name = "smsc911x",
	.id = 0,
	.num_resources = ARRAY_SIZE(smsc911x_resources),
	.resource = smsc911x_resources,
	.dev = {
		.platform_data = &smsc911x_config,
	},
};
#endif

#if IS_ENABLED(CONFIG_USB_NET2272)
static struct resource net2272_bfin_resources[] = {
	{
		.start = 0x24000000,
		.end = 0x24000000 + 0x100,
		.flags = IORESOURCE_MEM,
	}, {
		.start = IRQ_PF45,
		.end = IRQ_PF45,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct platform_device net2272_bfin_device = {
	.name = "net2272",
	.id = -1,
	.num_resources = ARRAY_SIZE(net2272_bfin_resources),
	.resource = net2272_bfin_resources,
};
#endif

#if IS_ENABLED(CONFIG_USB_ISP1362_HCD)
static struct resource isp1362_hcd_resources[] = {
	{
		.start = 0x24008000,
		.end = 0x24008000,
		.flags = IORESOURCE_MEM,
	}, {
		.start = 0x24008004,
		.end = 0x24008004,
		.flags = IORESOURCE_MEM,
	}, {
		.start = IRQ_PF47,
		.end = IRQ_PF47,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWEDGE,
	},
};

static struct isp1362_platform_data isp1362_priv = {
	.sel15Kres = 1,
	.clknotstop = 0,
	.oc_enable = 0,
	.int_act_high = 0,
	.int_edge_triggered = 0,
	.remote_wakeup_connected = 0,
	.no_power_switching = 1,
	.power_switching_mode = 0,
};

static struct platform_device isp1362_hcd_device = {
	.name = "isp1362-hcd",
	.id = 0,
	.dev = {
		.platform_data = &isp1362_priv,
	},
	.num_resources = ARRAY_SIZE(isp1362_hcd_resources),
	.resource = isp1362_hcd_resources,
};
#endif

#if IS_ENABLED(CONFIG_SERIAL_BFIN)
#ifdef CONFIG_SERIAL_BFIN_UART0
static struct resource bfin_uart0_resources[] = {
	{
		.start = BFIN_UART_THR,
		.end = BFIN_UART_GCTL+2,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_UART_TX,
		.end = IRQ_UART_TX,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = IRQ_UART_RX,
		.end = IRQ_UART_RX,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = IRQ_UART_ERROR,
		.end = IRQ_UART_ERROR,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = CH_UART_TX,
		.end = CH_UART_TX,
		.flags = IORESOURCE_DMA,
	},
	{
		.start = CH_UART_RX,
		.end = CH_UART_RX,
		.flags = IORESOURCE_DMA,
	},
};

static unsigned short bfin_uart0_peripherals[] = {
	P_UART0_TX, P_UART0_RX, 0
};

static struct platform_device bfin_uart0_device = {
	.name = "bfin-uart",
	.id = 0,
	.num_resources = ARRAY_SIZE(bfin_uart0_resources),
	.resource = bfin_uart0_resources,
	.dev = {
		.platform_data = &bfin_uart0_peripherals, /* Passed to driver */
	},
};
#endif
#endif

#if IS_ENABLED(CONFIG_BFIN_SIR)
#ifdef CONFIG_BFIN_SIR0
static struct resource bfin_sir0_resources[] = {
	{
		.start = 0xFFC00400,
		.end = 0xFFC004FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_UART0_RX,
		.end = IRQ_UART0_RX+1,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = CH_UART0_RX,
		.end = CH_UART0_RX+1,
		.flags = IORESOURCE_DMA,
	},
};

static struct platform_device bfin_sir0_device = {
	.name = "bfin_sir",
	.id = 0,
	.num_resources = ARRAY_SIZE(bfin_sir0_resources),
	.resource = bfin_sir0_resources,
};
#endif
#endif

#if IS_ENABLED(CONFIG_PATA_PLATFORM)
#define PATA_INT	IRQ_PF46

static struct pata_platform_info bfin_pata_platform_data = {
	.ioport_shift = 2,
};

static struct resource bfin_pata_resources[] = {
	{
		.start = 0x2400C000,
		.end = 0x2400C001F,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = 0x2400D018,
		.end = 0x2400D01B,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = PATA_INT,
		.end = PATA_INT,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct platform_device bfin_pata_device = {
	.name = "pata_platform",
	.id = -1,
	.num_resources = ARRAY_SIZE(bfin_pata_resources),
	.resource = bfin_pata_resources,
	.dev = {
		.platform_data = &bfin_pata_platform_data,
	}
};
#endif

#if IS_ENABLED(CONFIG_MTD_PHYSMAP)
static struct mtd_partition para_partitions[] = {
	{
		.name       = "bootloader(nor)",
		.size       = 0x40000,
		.offset     = 0,
	}, {
		.name       = "linux kernel(nor)",
		.size       = 0x100000,
		.offset     = MTDPART_OFS_APPEND,
	}, {
		.name       = "file system(nor)",
		.size       = MTDPART_SIZ_FULL,
		.offset     = MTDPART_OFS_APPEND,
	}
};

static struct physmap_flash_data para_flash_data = {
	.width      = 2,
	.parts      = para_partitions,
	.nr_parts   = ARRAY_SIZE(para_partitions),
};

static struct resource para_flash_resource = {
	.start = 0x20000000,
	.end   = 0x207fffff,
	.flags = IORESOURCE_MEM,
};

static struct platform_device para_flash_device = {
	.name          = "physmap-flash",
	.id            = 0,
	.dev = {
		.platform_data = &para_flash_data,
	},
	.num_resources = 1,
	.resource      = &para_flash_resource,
};
#endif

static const unsigned int cclk_vlev_datasheet[] =
{
	VRPAIR(VLEV_085, 250000000),
	VRPAIR(VLEV_090, 300000000),
	VRPAIR(VLEV_095, 313000000),
	VRPAIR(VLEV_100, 350000000),
	VRPAIR(VLEV_105, 400000000),
	VRPAIR(VLEV_110, 444000000),
	VRPAIR(VLEV_115, 450000000),
	VRPAIR(VLEV_120, 475000000),
	VRPAIR(VLEV_125, 500000000),
	VRPAIR(VLEV_130, 600000000),
};

static struct bfin_dpmc_platform_data bfin_dmpc_vreg_data = {
	.tuple_tab = cclk_vlev_datasheet,
	.tabsize = ARRAY_SIZE(cclk_vlev_datasheet),
	.vr_settling_time = 25 /* us */,
};

static struct platform_device bfin_dpmc = {
	.name = "bfin dpmc",
	.dev = {
		.platform_data = &bfin_dmpc_vreg_data,
	},
};

static struct platform_device *cm_bf561_devices[] __initdata = {

	&bfin_dpmc,

#if IS_ENABLED(CONFIG_FB_HITACHI_TX09)
	&hitachi_fb_device,
#endif

#if IS_ENABLED(CONFIG_SERIAL_BFIN)
#ifdef CONFIG_SERIAL_BFIN_UART0
	&bfin_uart0_device,
#endif
#endif

#if IS_ENABLED(CONFIG_BFIN_SIR)
#ifdef CONFIG_BFIN_SIR0
	&bfin_sir0_device,
#endif
#endif

#if IS_ENABLED(CONFIG_USB_ISP1362_HCD)
	&isp1362_hcd_device,
#endif

#if IS_ENABLED(CONFIG_SMC91X)
	&smc91x_device,
#endif

#if IS_ENABLED(CONFIG_SMSC911X)
	&smsc911x_device,
#endif

#if IS_ENABLED(CONFIG_USB_NET2272)
	&net2272_bfin_device,
#endif

#if IS_ENABLED(CONFIG_SPI_BFIN5XX)
	&bfin_spi0_device,
#endif

#if IS_ENABLED(CONFIG_PATA_PLATFORM)
	&bfin_pata_device,
#endif

#if IS_ENABLED(CONFIG_MTD_PHYSMAP)
	&para_flash_device,
#endif
};

static int __init net2272_init(void)
{
#if IS_ENABLED(CONFIG_USB_NET2272)
	int ret;

	ret = gpio_request(GPIO_PF46, "net2272");
	if (ret)
		return ret;

	/* Reset USB Chip, PF46 */
	gpio_direction_output(GPIO_PF46, 0);
	mdelay(2);
	gpio_set_value(GPIO_PF46, 1);
#endif

	return 0;
}

static int __init cm_bf561_init(void)
{
	printk(KERN_INFO "%s(): registering device resources\n", __func__);
	platform_add_devices(cm_bf561_devices, ARRAY_SIZE(cm_bf561_devices));
#if IS_ENABLED(CONFIG_SPI_BFIN5XX)
	spi_register_board_info(bfin_spi_board_info, ARRAY_SIZE(bfin_spi_board_info));
#endif

#if IS_ENABLED(CONFIG_PATA_PLATFORM)
	irq_set_status_flags(PATA_INT, IRQ_NOAUTOEN);
#endif

	if (net2272_init())
		pr_warning("unable to configure net2272; it probably won't work\n");

	return 0;
}

arch_initcall(cm_bf561_init);

static struct platform_device *cm_bf561_early_devices[] __initdata = {
#if defined(CONFIG_SERIAL_BFIN_CONSOLE) || defined(CONFIG_EARLY_PRINTK)
#ifdef CONFIG_SERIAL_BFIN_UART0
	&bfin_uart0_device,
#endif
#endif
};

void __init native_machine_early_platform_add_devices(void)
{
	printk(KERN_INFO "register early platform devices\n");
	early_platform_add_devices(cm_bf561_early_devices,
		ARRAY_SIZE(cm_bf561_early_devices));
}
