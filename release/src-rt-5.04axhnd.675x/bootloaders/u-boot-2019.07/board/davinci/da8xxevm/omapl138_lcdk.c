// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da850evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/davinci_nand.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/mach-types.h>
#include <asm/arch/davinci_misc.h>
#ifdef CONFIG_MMC_DAVINCI
#include <mmc.h>
#include <asm/arch/sdmmc_defs.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define pinmux(x)	(&davinci_syscfg_regs->pinmux[x])

#ifdef CONFIG_MMC_DAVINCI
/* MMC0 pin muxer settings */
const struct pinmux_config mmc0_pins[] = {
	/* GP0[11] is required for SD to work on Rev 3 EVMs */
	{ pinmux(0),  8, 4 },	/* GP0[11] */
	{ pinmux(10), 2, 0 },	/* MMCSD0_CLK */
	{ pinmux(10), 2, 1 },	/* MMCSD0_CMD */
	{ pinmux(10), 2, 2 },	/* MMCSD0_DAT_0 */
	{ pinmux(10), 2, 3 },	/* MMCSD0_DAT_1 */
	{ pinmux(10), 2, 4 },	/* MMCSD0_DAT_2 */
	{ pinmux(10), 2, 5 },	/* MMCSD0_DAT_3 */
	/* LCDK supports only 4-bit mode, remaining pins are not configured */
};
#endif

/* UART pin muxer settings */
static const struct pinmux_config uart_pins[] = {
	{ pinmux(0), 4, 6 },
	{ pinmux(0), 4, 7 },
	{ pinmux(4), 2, 4 },
	{ pinmux(4), 2, 5 }
};

#ifdef CONFIG_DRIVER_TI_EMAC
static const struct pinmux_config emac_pins[] = {
	{ pinmux(2), 8, 1 },
	{ pinmux(2), 8, 2 },
	{ pinmux(2), 8, 3 },
	{ pinmux(2), 8, 4 },
	{ pinmux(2), 8, 5 },
	{ pinmux(2), 8, 6 },
	{ pinmux(2), 8, 7 },
	{ pinmux(3), 8, 0 },
	{ pinmux(3), 8, 1 },
	{ pinmux(3), 8, 2 },
	{ pinmux(3), 8, 3 },
	{ pinmux(3), 8, 4 },
	{ pinmux(3), 8, 5 },
	{ pinmux(3), 8, 6 },
	{ pinmux(3), 8, 7 },
	{ pinmux(4), 8, 0 },
	{ pinmux(4), 8, 1 }
};
#endif /* CONFIG_DRIVER_TI_EMAC */

/* I2C pin muxer settings */
static const struct pinmux_config i2c_pins[] = {
	{ pinmux(4), 2, 2 },
	{ pinmux(4), 2, 3 }
};

#ifdef CONFIG_NAND_DAVINCI
const struct pinmux_config nand_pins[] = {
	{ pinmux(7), 1, 1 },
	{ pinmux(7), 1, 2 },
	{ pinmux(7), 1, 4 },
	{ pinmux(7), 1, 5 },
	{ pinmux(8), 1, 0 },
	{ pinmux(8), 1, 1 },
	{ pinmux(8), 1, 2 },
	{ pinmux(8), 1, 3 },
	{ pinmux(8), 1, 4 },
	{ pinmux(8), 1, 5 },
	{ pinmux(8), 1, 6 },
	{ pinmux(8), 1, 7 },
	{ pinmux(9), 1, 0 },
	{ pinmux(9), 1, 1 },
	{ pinmux(9), 1, 2 },
	{ pinmux(9), 1, 3 },
	{ pinmux(9), 1, 4 },
	{ pinmux(9), 1, 5 },
	{ pinmux(9), 1, 6 },
	{ pinmux(9), 1, 7 },
	{ pinmux(12), 1, 5 },
	{ pinmux(12), 1, 6 }
};

#endif

#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
#define HAS_RMII 1
#else
#define HAS_RMII 0
#endif

const struct pinmux_resource pinmuxes[] = {
	PINMUX_ITEM(uart_pins),
	PINMUX_ITEM(i2c_pins),
#ifdef CONFIG_NAND_DAVINCI
	PINMUX_ITEM(nand_pins),
#endif
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI1 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
#ifdef CONFIG_MMC_DAVINCI
	{ DAVINCI_LPSC_MMC_SD },
#endif
};

const int lpsc_size = ARRAY_SIZE(lpsc);

#ifndef CONFIG_DA850_EVM_MAX_CPU_CLK
#define CONFIG_DA850_EVM_MAX_CPU_CLK	456000000
#endif

/*
 * get_board_rev() - setup to pass kernel board revision information
 * Returns:
 * bit[0-3]	Maximum cpu clock rate supported by onboard SoC
 *		0000b - 300 MHz
 *		0001b - 372 MHz
 *		0010b - 408 MHz
 *		0011b - 456 MHz
 */
u32 get_board_rev(void)
{
	return 0;
}

int board_early_init_f(void)
{
	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	if (da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc)))
		return 1;

	return 0;
}

int board_init(void)
{
	irq_init();

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_OMAPL138_LCDK;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;


	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART2),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

#ifdef CONFIG_NAND_DAVINCI
	/*
	 * NAND CS setup - cycle counts based on da850evm NAND timings in the
	 * Linux kernel @ 25MHz EMIFA
	 */
	writel((DAVINCI_ABCR_WSETUP(15) |
		DAVINCI_ABCR_WSTROBE(63) |
		DAVINCI_ABCR_WHOLD(7) |
		DAVINCI_ABCR_RSETUP(15) |
		DAVINCI_ABCR_RSTROBE(63) |
		DAVINCI_ABCR_RHOLD(7) |
		DAVINCI_ABCR_TA(3) |
		DAVINCI_ABCR_ASIZE_16BIT),
	       &davinci_emif_regs->ab2cr); /* CS3 */
#endif


#ifdef CONFIG_MMC_DAVINCI
	if (davinci_configure_pin_mux(mmc0_pins, ARRAY_SIZE(mmc0_pins)) != 0)
		return 1;
#endif

#ifdef CONFIG_DRIVER_TI_EMAC
	if (davinci_configure_pin_mux(emac_pins, ARRAY_SIZE(emac_pins)) != 0)
		return 1;
	davinci_emac_mii_mode_sel(HAS_RMII);
#endif /* CONFIG_DRIVER_TI_EMAC */

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC

/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	return 0;
}

#endif /* CONFIG_DRIVER_TI_EMAC */

#define CFG_MAC_ADDR_SPI_BUS	0
#define CFG_MAC_ADDR_SPI_CS	0
#define CFG_MAC_ADDR_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED
#define CFG_MAC_ADDR_SPI_MODE	SPI_MODE_3

#define CFG_MAC_ADDR_OFFSET	(flash->size - SZ_64K)

static int  get_mac_addr(u8 *addr)
{
	/* Need to find a way to get MAC ADDRESS */
	return 0;
}

void dsp_lpsc_on(unsigned domain, unsigned int id)
{
	dv_reg_p mdstat, mdctl, ptstat, ptcmd;
	struct davinci_psc_regs *psc_regs;

	psc_regs = davinci_psc0_regs;
	mdstat = &psc_regs->psc0.mdstat[id];
	mdctl = &psc_regs->psc0.mdctl[id];
	ptstat = &psc_regs->ptstat;
	ptcmd = &psc_regs->ptcmd;

	while (*ptstat & (0x1 << domain))
		;

	if ((*mdstat & 0x1f) == 0x03)
		return;                 /* Already on and enabled */

	*mdctl |= 0x03;

	*ptcmd = 0x1 << domain;

	while (*ptstat & (0x1 << domain))
		;
	while ((*mdstat & 0x1f) != 0x03)
		;		/* Probably an overkill... */
}

static void dspwake(void)
{
	unsigned *resetvect = (unsigned *)DAVINCI_L3CBARAM_BASE;

	/* if the device is ARM only, return */
	if ((REG(CHIP_REV_ID_REG) & 0x3f) == 0x10)
		return;

	if (!strcmp(env_get("dspwake"), "no"))
		return;

	*resetvect++ = 0x1E000; /* DSP Idle */
	/* clear out the next 10 words as NOP */
	memset(resetvect, 0, sizeof(unsigned) * 10);

	/* setup the DSP reset vector */
	REG(HOST1CFG) = DAVINCI_L3CBARAM_BASE;

	dsp_lpsc_on(1, DAVINCI_LPSC_GEM);
	REG(PSC0_MDCTL + (15 * 4)) |= 0x100;
}

#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
/**
 * rmii_hw_init
 *
 */
int rmii_hw_init(void)
{
	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC_USE_RMII */

int misc_init_r(void)
{
	uint8_t tmp[20], addr[10];


	if (env_get("ethaddr") == NULL) {
		/* Read Ethernet MAC address from EEPROM */
		if (dvevm_read_mac_address(addr)) {
			/* Set Ethernet MAC address from EEPROM */
			davinci_sync_env_enetaddr(addr);
		} else {
			get_mac_addr(addr);
		}

		if (!is_multicast_ethaddr(addr) && !is_zero_ethaddr(addr)) {
			sprintf((char *)tmp, "%02x:%02x:%02x:%02x:%02x:%02x",
				addr[0], addr[1], addr[2], addr[3], addr[4],
				addr[5]);

			env_set("ethaddr", (char *)tmp);
		} else {
			printf("Invalid MAC address read.\n");
		}
	}

#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
	/* Select RMII fucntion through the expander */
	if (rmii_hw_init())
		printf("RMII hardware init failed!!!\n");
#endif

	dspwake();

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
#ifdef CONFIG_MMC_DAVINCI
static struct davinci_mmc mmc_sd0 = {
	.reg_base = (struct davinci_mmc_regs *)DAVINCI_MMC_SD0_BASE,
	.host_caps = MMC_MODE_4BIT,     /* DA850 supports only 4-bit SD/MMC */
	.voltages = MMC_VDD_32_33 | MMC_VDD_33_34,
	.version = MMC_CTLR_VERSION_2,
};

int board_mmc_init(bd_t *bis)
{
	mmc_sd0.input_clk = clk_get(DAVINCI_MMCSD_CLKID);

	/* Add slot-0 to mmc subsystem */
	return davinci_mmc_init(bis, &mmc_sd0);
}
#endif
#endif
