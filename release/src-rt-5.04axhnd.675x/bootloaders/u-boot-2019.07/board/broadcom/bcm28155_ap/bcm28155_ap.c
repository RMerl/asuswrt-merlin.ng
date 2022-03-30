// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Broadcom Corporation.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <mmc.h>
#include <asm/kona-common/kona_sdhci.h>
#include <asm/kona-common/clk.h>
#include <asm/arch/sysmap.h>

#include <usb.h>
#include <usb/dwc2_udc.h>
#include <g_dnl.h>

#define SECWATCHDOG_SDOGCR_OFFSET	0x00000000
#define SECWATCHDOG_SDOGCR_EN_SHIFT	27
#define SECWATCHDOG_SDOGCR_SRSTEN_SHIFT	26
#define SECWATCHDOG_SDOGCR_CLKS_SHIFT	20
#define SECWATCHDOG_SDOGCR_LD_SHIFT	0

#ifndef CONFIG_USB_SERIALNO
#define CONFIG_USB_SERIALNO "1234567890"
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * board_init - early hardware init
 */
int board_init(void)
{
	printf("Relocation Offset is: %08lx\n", gd->reloc_off);

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	clk_init();

	return 0;
}

/*
 * misc_init_r - miscellaneous platform dependent initializations
 */
int misc_init_r(void)
{
	/* Disable watchdog reset - watchdog unused */
	writel((0 << SECWATCHDOG_SDOGCR_EN_SHIFT) |
	       (0 << SECWATCHDOG_SDOGCR_SRSTEN_SHIFT) |
	       (4 << SECWATCHDOG_SDOGCR_CLKS_SHIFT) |
	       (0x5a0 << SECWATCHDOG_SDOGCR_LD_SHIFT),
	       (SECWD_BASE_ADDR + SECWATCHDOG_SDOGCR_OFFSET));

	return 0;
}

/*
 * dram_init - sets uboots idea of sdram size
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

/* This is called after dram_init() so use get_ram_size result */
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

#ifdef CONFIG_MMC_SDHCI_KONA
/*
 * mmc_init - Initializes mmc
 */
int board_mmc_init(bd_t *bis)
{
	int ret = 0;

	/* Register eMMC - SDIO2 */
	ret = kona_sdhci_init(1, 400000, 0);
	if (ret)
		return ret;

	/* Register SD Card - SDIO4 kona_mmc_init assumes 0 based index */
	ret = kona_sdhci_init(3, 400000, 0);
	return ret;
}
#endif

#ifdef CONFIG_USB_GADGET
static struct dwc2_plat_otg_data bcm_otg_data = {
	.regs_otg	= HSOTG_BASE_ADDR
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("%s: performing dwc2_udc_probe\n", __func__);
	return dwc2_udc_probe(&bcm_otg_data);
}

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	debug("%s\n", __func__);
	if (!env_get("serial#"))
		g_dnl_set_serialnumber(CONFIG_USB_SERIALNO);
	return 0;
}

int g_dnl_get_board_bcd_device_number(int gcnum)
{
	debug("%s\n", __func__);
	return 1;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	debug("%s\n", __func__);
	return 0;
}
#endif
