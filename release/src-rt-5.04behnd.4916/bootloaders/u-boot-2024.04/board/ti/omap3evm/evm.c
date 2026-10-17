// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004-2011
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Manikandan Pillai <mani.pillai@ti.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 */
#include <common.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <ns16550.h>
#include <serial.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/gpio.h>
#include <twl4030.h>
#include <asm/mach-types.h>
#include <linux/delay.h>
#include <linux/mtd/rawnand.h>
#include "evm.h"

#define OMAP3EVM_GPIO_ETH_RST_GEN1 64
#define OMAP3EVM_GPIO_ETH_RST_GEN2 7

#define CFG_SMC911X_BASE 0x2C000000

DECLARE_GLOBAL_DATA_PTR;

static u32 omap3_evm_version;

u32 get_omap3_evm_rev(void)
{
	return omap3_evm_version;
}

static void omap3_evm_get_revision(void)
{
#if defined(CONFIG_SMC911X)
	/*
	 * Board revision can be ascertained only by identifying
	 * the Ethernet chipset.
	 */
	unsigned int smsc_id;

	/* Ethernet PHY ID is stored at ID_REV register */
	smsc_id = readl(CFG_SMC911X_BASE + 0x50) & 0xFFFF0000;
	printf("Read back SMSC id 0x%x\n", smsc_id);

	switch (smsc_id) {
	/* SMSC9115 chipset */
	case 0x01150000:
		omap3_evm_version = OMAP3EVM_BOARD_GEN_1;
		break;
	/* SMSC 9220 chipset */
	case 0x92200000:
	default:
		omap3_evm_version = OMAP3EVM_BOARD_GEN_2;
       }
#else /* !CONFIG_SMC911X */
#if defined(CONFIG_STATIC_BOARD_REV)
	/* Look for static defintion of the board revision */
	omap3_evm_version = CONFIG_STATIC_BOARD_REV;
#else
	/* Fallback to the default above */
	omap3_evm_version = OMAP3EVM_BOARD_GEN_2;
#endif /* CONFIG_STATIC_BOARD_REV */
#endif /* CONFIG_SMC911X */
}

#if defined(CONFIG_USB_MUSB_GADGET) || defined(CONFIG_USB_MUSB_HOST)
/* MUSB port on OMAP3EVM Rev >= E requires extvbus programming. */
u8 omap3_evm_need_extvbus(void)
{
	u8 retval = 0;

	if (get_omap3_evm_rev() >= OMAP3EVM_BOARD_GEN_2)
		retval = 1;

	return retval;
}
#endif /* CONFIG_USB_MUSB_{GADGET,HOST} */

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3EVM;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

#if defined(CONFIG_SPL_OS_BOOT)
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

	return 0;
}
#endif /* CONFIG_SPL_OS_BOOT */

#if defined(CONFIG_SPL_BUILD)
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on the first bank.  This
 * provides the timing values back to the function that configures
 * the memory.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	int pop_mfr, pop_id;

	/*
	 * We need to identify what PoP memory is on the board so that
	 * we know what timings to use.  To map the ID values please see
	 * nand_ids.c
	 */
	identify_nand_chip(&pop_mfr, &pop_id);

	if (pop_mfr == NAND_MFR_HYNIX && pop_id == 0xbc) {
		/* 256MB DDR */
		timings->mcfg = HYNIX_V_MCFG_200(256 << 20);
		timings->ctrla = HYNIX_V_ACTIMA_200;
		timings->ctrlb = HYNIX_V_ACTIMB_200;
	} else {
		/* 128MB DDR */
		timings->mcfg = MICRON_V_MCFG_165(128 << 20);
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
	}
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	timings->mr = MICRON_V_MR_165;
}
#endif /* CONFIG_SPL_BUILD */

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
	twl4030_power_init();
	twl4030_power_mmc_init(0);

#if defined(CONFIG_SMC911X)
	setup_net_chip();
#endif
	omap3_evm_get_revision();

#if defined(CONFIG_SMC911X)
	reset_net_chip();
#endif
	omap_die_id_display();

#if defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET) && \
						!defined(CONFIG_SMC911X)
	omap_die_id_usbethaddr();
#endif
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
	MUX_EVM();
}

#if defined(CONFIG_SMC911X)
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Configure GPMC registers */
	writel(NET_GPMC_CONFIG1, &gpmc_cfg->cs[5].config1);
	writel(NET_GPMC_CONFIG2, &gpmc_cfg->cs[5].config2);
	writel(NET_GPMC_CONFIG3, &gpmc_cfg->cs[5].config3);
	writel(NET_GPMC_CONFIG4, &gpmc_cfg->cs[5].config4);
	writel(NET_GPMC_CONFIG5, &gpmc_cfg->cs[5].config5);
	writel(NET_GPMC_CONFIG6, &gpmc_cfg->cs[5].config6);
	writel(NET_GPMC_CONFIG7, &gpmc_cfg->cs[5].config7);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);
}

/**
 * Reset the ethernet chip.
 */
static void reset_net_chip(void)
{
	int ret;
	int rst_gpio;

	if (get_omap3_evm_rev() == OMAP3EVM_BOARD_GEN_1) {
		rst_gpio = OMAP3EVM_GPIO_ETH_RST_GEN1;
	} else {
		rst_gpio = OMAP3EVM_GPIO_ETH_RST_GEN2;
	}

	ret = gpio_request(rst_gpio, "");
	if (ret < 0) {
		printf("Unable to get GPIO %d\n", rst_gpio);
		return;
	}

	/* Configure as output */
	gpio_direction_output(rst_gpio, 0);

	/* Send a pulse on the GPIO pin */
	gpio_set_value(rst_gpio, 1);
	udelay(1);
	gpio_set_value(rst_gpio, 0);
	udelay(1);
	gpio_set_value(rst_gpio, 1);
}
#endif /* CONFIG_SMC911X */
