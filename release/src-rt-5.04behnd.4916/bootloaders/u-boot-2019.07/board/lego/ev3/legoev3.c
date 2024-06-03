// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 David Lechner <david@lechnology.com>
 *
 * Based on da850evm.c
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#include <common.h>
#include <i2c.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/hardware.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <linux/errno.h>
#include <hwconfig.h>
#include <asm/mach-types.h>
#include <asm/setup.h>

#ifdef CONFIG_MMC_DAVINCI
#include <mmc.h>
#include <asm/arch/sdmmc_defs.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

u8 board_rev;

#define EEPROM_I2C_ADDR		0x50
#define EEPROM_REV_OFFSET	0x3F00
#define EEPROM_MAC_OFFSET	0x3F06

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

const struct pinmux_resource pinmuxes[] = {
	PINMUX_ITEM(spi0_pins_base),
	PINMUX_ITEM(spi0_pins_scs0),
	PINMUX_ITEM(uart1_pins_txrx),
	PINMUX_ITEM(i2c0_pins),
	PINMUX_ITEM(mmc0_pins),
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_SPI0 },	/* Serial Flash */
	{ DAVINCI_LPSC_UART1 },	/* console */
	{ DAVINCI_LPSC_MMC_SD },
};

const int lpsc_size = ARRAY_SIZE(lpsc);

u32 get_board_rev(void)
{
	u8 buf[2];

	if (!board_rev) {
		if (i2c_read(EEPROM_I2C_ADDR, EEPROM_REV_OFFSET, 2, buf, 2)) {
			printf("\nBoard revision read failed!\n");
		} else {
			/*
			 * Board rev 3 has MAC address at EEPROM_REV_OFFSET.
			 * Other revisions have checksum at EEPROM_REV_OFFSET+1
			 * to detect this.
			 */
			if ((buf[0] ^ buf[1]) == 0xFF)
				board_rev = buf[0];
			else
				board_rev = 3;
		}
	}

	return board_rev;
}

/*
 * The Bluetooth MAC address serves as the board serial number.
 */
void get_board_serial(struct tag_serialnr *serialnr)
{
	u32 offset;
	u8 buf[6];

	if (!board_rev)
		board_rev = get_board_rev();

	/* Board rev 3 has MAC address where rev should be */
	offset = (board_rev == 3) ? EEPROM_REV_OFFSET : EEPROM_MAC_OFFSET;

	if (i2c_read(EEPROM_I2C_ADDR, offset, 2, buf, 6)) {
		printf("\nBoard serial read failed!\n");
	} else {
		u8 *nr;

		nr = (u8 *)&serialnr->low;
		nr[0] = buf[5];
		nr[1] = buf[4];
		nr[2] = buf[3];
		nr[3] = buf[2];
		nr = (u8 *)&serialnr->high;
		nr[0] = buf[1];
		nr[1] = buf[0];
		nr[2] = 0;
		nr[3] = 0;
	}
}

int board_early_init_f(void)
{
	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart1_ctrl_regs->pwremu_mgmt);

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
	/* LEGO didn't register for a unique number and uses da850evm */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DA850_EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI0 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART1),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

	return 0;
}
