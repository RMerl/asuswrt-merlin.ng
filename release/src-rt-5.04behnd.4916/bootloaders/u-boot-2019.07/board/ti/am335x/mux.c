/*
 * mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "../common/board_detect.h"
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux uart1_pin_mux[] = {
	{OFFSET(uart1_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART1_RXD */
	{OFFSET(uart1_txd), (MODE(0) | PULLUDEN)},		/* UART1_TXD */
	{-1},
};

static struct module_pin_mux uart2_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART2_RXD */
	{OFFSET(spi0_d0), (MODE(1) | PULLUDEN)},		/* UART2_TXD */
	{-1},
};

static struct module_pin_mux uart3_pin_mux[] = {
	{OFFSET(spi0_cs1), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(ecap0_in_pwm0_out), (MODE(1) | PULLUDEN)},	/* UART3_TXD */
	{-1},
};

static struct module_pin_mux uart4_pin_mux[] = {
	{OFFSET(gpmc_wait0), (MODE(6) | PULLUP_EN | RXACTIVE)},	/* UART4_RXD */
	{OFFSET(gpmc_wpn), (MODE(6) | PULLUDEN)},		/* UART4_TXD */
	{-1},
};

static struct module_pin_mux uart5_pin_mux[] = {
	{OFFSET(lcd_data9), (MODE(4) | PULLUP_EN | RXACTIVE)},	/* UART5_RXD */
	{OFFSET(lcd_data8), (MODE(4) | PULLUDEN)},		/* UART5_TXD */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
	{OFFSET(spi0_cs1), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* GPIO0_6 */
	{-1},
};

static struct module_pin_mux mmc0_no_cd_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux_sk_evm[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUP_EN)},	/* MMC0_CD */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(gpmc_ad7), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT7 */
	{OFFSET(gpmc_ad6), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT6 */
	{OFFSET(gpmc_ad5), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT5 */
	{OFFSET(gpmc_ad4), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT4 */
	{OFFSET(gpmc_ad3), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad2), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
	{OFFSET(gpmc_ad1), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
	{OFFSET(gpmc_ad0), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
	{OFFSET(gpmc_csn1), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CLK */
	{OFFSET(gpmc_csn2), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
	{OFFSET(gpmc_csn0), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC1_WP */
	{OFFSET(gpmc_advn_ale), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC1_CD */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux i2c1_pin_mux[] = {
	{OFFSET(spi0_d1), (MODE(2) | RXACTIVE |
			PULLUDEN | SLEWCTRL)},	/* I2C_DATA */
	{OFFSET(spi0_cs0), (MODE(2) | RXACTIVE |
			PULLUDEN | SLEWCTRL)},	/* I2C_SCLK */
	{-1},
};

static struct module_pin_mux spi0_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(0) | RXACTIVE | PULLUDEN)},	/* SPI0_SCLK */
	{OFFSET(spi0_d0), (MODE(0) | RXACTIVE |
			PULLUDEN | PULLUP_EN)},			/* SPI0_D0 */
	{OFFSET(spi0_d1), (MODE(0) | RXACTIVE | PULLUDEN)},	/* SPI0_D1 */
	{OFFSET(spi0_cs0), (MODE(0) | RXACTIVE |
			PULLUDEN | PULLUP_EN)},			/* SPI0_CS0 */
	{-1},
};

static struct module_pin_mux gpio0_7_pin_mux[] = {
	{OFFSET(ecap0_in_pwm0_out), (MODE(7) | PULLUDEN)},	/* GPIO0_7 */
	{-1},
};

static struct module_pin_mux gpio0_18_pin_mux[] = {
	{OFFSET(usb0_drvvbus), (MODE(7) | PULLUDEN)},	/* GPIO0_18 */
	{-1},
};

static struct module_pin_mux rgmii1_pin_mux[] = {
	{OFFSET(mii1_txen), MODE(2)},			/* RGMII1_TCTL */
	{OFFSET(mii1_rxdv), MODE(2) | RXACTIVE},	/* RGMII1_RCTL */
	{OFFSET(mii1_txd3), MODE(2)},			/* RGMII1_TD3 */
	{OFFSET(mii1_txd2), MODE(2)},			/* RGMII1_TD2 */
	{OFFSET(mii1_txd1), MODE(2)},			/* RGMII1_TD1 */
	{OFFSET(mii1_txd0), MODE(2)},			/* RGMII1_TD0 */
	{OFFSET(mii1_txclk), MODE(2)},			/* RGMII1_TCLK */
	{OFFSET(mii1_rxclk), MODE(2) | RXACTIVE},	/* RGMII1_RCLK */
	{OFFSET(mii1_rxd3), MODE(2) | RXACTIVE},	/* RGMII1_RD3 */
	{OFFSET(mii1_rxd2), MODE(2) | RXACTIVE},	/* RGMII1_RD2 */
	{OFFSET(mii1_rxd1), MODE(2) | RXACTIVE},	/* RGMII1_RD1 */
	{OFFSET(mii1_rxd0), MODE(2) | RXACTIVE},	/* RGMII1_RD0 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},/* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux mii1_pin_mux[] = {
	{OFFSET(mii1_rxerr), MODE(0) | RXACTIVE},	/* MII1_RXERR */
	{OFFSET(mii1_txen), MODE(0)},			/* MII1_TXEN */
	{OFFSET(mii1_rxdv), MODE(0) | RXACTIVE},	/* MII1_RXDV */
	{OFFSET(mii1_txd3), MODE(0)},			/* MII1_TXD3 */
	{OFFSET(mii1_txd2), MODE(0)},			/* MII1_TXD2 */
	{OFFSET(mii1_txd1), MODE(0)},			/* MII1_TXD1 */
	{OFFSET(mii1_txd0), MODE(0)},			/* MII1_TXD0 */
	{OFFSET(mii1_txclk), MODE(0) | RXACTIVE},	/* MII1_TXCLK */
	{OFFSET(mii1_rxclk), MODE(0) | RXACTIVE},	/* MII1_RXCLK */
	{OFFSET(mii1_rxd3), MODE(0) | RXACTIVE},	/* MII1_RXD3 */
	{OFFSET(mii1_rxd2), MODE(0) | RXACTIVE},	/* MII1_RXD2 */
	{OFFSET(mii1_rxd1), MODE(0) | RXACTIVE},	/* MII1_RXD1 */
	{OFFSET(mii1_rxd0), MODE(0) | RXACTIVE},	/* MII1_RXD0 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux rmii1_pin_mux[] = {
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mii1_crs), MODE(1) | RXACTIVE},		/* MII1_CRS */
	{OFFSET(mii1_rxerr), MODE(1) | RXACTIVE},	/* MII1_RXERR */
	{OFFSET(mii1_txen), MODE(1)},			/* MII1_TXEN */
	{OFFSET(mii1_txd1), MODE(1)},			/* MII1_TXD1 */
	{OFFSET(mii1_txd0), MODE(1)},			/* MII1_TXD0 */
	{OFFSET(mii1_rxd1), MODE(1) | RXACTIVE},	/* MII1_RXD1 */
	{OFFSET(mii1_rxd0), MODE(1) | RXACTIVE},	/* MII1_RXD0 */
	{OFFSET(rmii1_refclk), MODE(0) | RXACTIVE},	/* RMII1_REFCLK */
	{-1},
};

#ifdef CONFIG_NAND
static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD0  */
	{OFFSET(gpmc_ad1),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD1  */
	{OFFSET(gpmc_ad2),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD2  */
	{OFFSET(gpmc_ad3),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD3  */
	{OFFSET(gpmc_ad4),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD4  */
	{OFFSET(gpmc_ad5),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD5  */
	{OFFSET(gpmc_ad6),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD6  */
	{OFFSET(gpmc_ad7),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD7  */
#ifdef CONFIG_SYS_NAND_BUSWIDTH_16BIT
	{OFFSET(gpmc_ad8),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD8  */
	{OFFSET(gpmc_ad9),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD9  */
	{OFFSET(gpmc_ad10),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD10 */
	{OFFSET(gpmc_ad11),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD11 */
	{OFFSET(gpmc_ad12),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD12 */
	{OFFSET(gpmc_ad13),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD13 */
	{OFFSET(gpmc_ad14),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD14 */
	{OFFSET(gpmc_ad15),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD15 */
#endif
	{OFFSET(gpmc_wait0),	(MODE(0) | PULLUP_EN | RXACTIVE)}, /* nWAIT */
	{OFFSET(gpmc_wpn),	(MODE(7) | PULLUP_EN)},		   /* nWP */
	{OFFSET(gpmc_csn0),	(MODE(0) | PULLUP_EN)},		   /* nCS */
	{OFFSET(gpmc_wen),	(MODE(0) | PULLDOWN_EN)},	   /* WEN */
	{OFFSET(gpmc_oen_ren),	(MODE(0) | PULLDOWN_EN)},	   /* OE */
	{OFFSET(gpmc_advn_ale),	(MODE(0) | PULLDOWN_EN)},	   /* ADV_ALE */
	{OFFSET(gpmc_be0n_cle),	(MODE(0) | PULLDOWN_EN)},	   /* BE_CLE */
	{-1},
};
#elif defined(CONFIG_NOR)
static struct module_pin_mux bone_norcape_pin_mux[] = {
	{OFFSET(gpmc_a0), MODE(0) | PULLUDDIS},			/* NOR_A0 */
	{OFFSET(gpmc_a1), MODE(0) | PULLUDDIS},			/* NOR_A1 */
	{OFFSET(gpmc_a2), MODE(0) | PULLUDDIS},			/* NOR_A2 */
	{OFFSET(gpmc_a3), MODE(0) | PULLUDDIS},			/* NOR_A3 */
	{OFFSET(gpmc_a4), MODE(0) | PULLUDDIS},			/* NOR_A4 */
	{OFFSET(gpmc_a5), MODE(0) | PULLUDDIS},			/* NOR_A5 */
	{OFFSET(gpmc_a6), MODE(0) | PULLUDDIS},			/* NOR_A6 */
	{OFFSET(gpmc_a7), MODE(0) | PULLUDDIS},			/* NOR_A7 */
	{OFFSET(gpmc_ad0), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD0 */
	{OFFSET(gpmc_ad1), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD1 */
	{OFFSET(gpmc_ad2), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD2 */
	{OFFSET(gpmc_ad3), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD3 */
	{OFFSET(gpmc_ad4), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD4 */
	{OFFSET(gpmc_ad5), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD5 */
	{OFFSET(gpmc_ad6), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD6 */
	{OFFSET(gpmc_ad7), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD7 */
	{OFFSET(gpmc_ad8), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD8 */
	{OFFSET(gpmc_ad9), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD9 */
	{OFFSET(gpmc_ad10), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD10 */
	{OFFSET(gpmc_ad11), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD11 */
	{OFFSET(gpmc_ad12), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD12 */
	{OFFSET(gpmc_ad13), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD13 */
	{OFFSET(gpmc_ad14), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD14 */
	{OFFSET(gpmc_ad15), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD15 */
	{OFFSET(gpmc_csn0), MODE(0) | PULLUDEN | PULLUP_EN},     /* CE */
	{OFFSET(gpmc_advn_ale), MODE(0) | PULLUDEN | PULLDOWN_EN}, /* ALE */
	{OFFSET(gpmc_oen_ren), MODE(0) | PULLUDEN | PULLDOWN_EN},/* OEn_REN */
	{OFFSET(gpmc_be0n_cle), MODE(0) | PULLUDEN | PULLDOWN_EN},/* unused */
	{OFFSET(gpmc_wen), MODE(0) | PULLUDEN | PULLDOWN_EN},    /* WEN */
	{OFFSET(gpmc_wait0), MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE},/*WAIT*/
	{-1},
};
#endif

static struct module_pin_mux uart3_icev2_pin_mux[] = {
	{OFFSET(mii1_rxd3), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(mii1_rxd2), MODE(1) | PULLUDEN},		/* UART3_TXD */
	{-1},
};

#if defined(CONFIG_NOR_BOOT)
void enable_norboot_pin_mux(void)
{
	configure_module_pin_mux(bone_norcape_pin_mux);
}
#endif

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_uart1_pin_mux(void)
{
	configure_module_pin_mux(uart1_pin_mux);
}

void enable_uart2_pin_mux(void)
{
	configure_module_pin_mux(uart2_pin_mux);
}

void enable_uart3_pin_mux(void)
{
	configure_module_pin_mux(uart3_pin_mux);
}

void enable_uart4_pin_mux(void)
{
	configure_module_pin_mux(uart4_pin_mux);
}

void enable_uart5_pin_mux(void)
{
	configure_module_pin_mux(uart5_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

/*
 * The AM335x GP EVM, if daughter card(s) are connected, can have 8
 * different profiles.  These profiles determine what peripherals are
 * valid and need pinmux to be configured.
 */
#define PROFILE_NONE	0x0
#define PROFILE_0	(1 << 0)
#define PROFILE_1	(1 << 1)
#define PROFILE_2	(1 << 2)
#define PROFILE_3	(1 << 3)
#define PROFILE_4	(1 << 4)
#define PROFILE_5	(1 << 5)
#define PROFILE_6	(1 << 6)
#define PROFILE_7	(1 << 7)
#define PROFILE_MASK	0x7
#define PROFILE_ALL	0xFF

/* CPLD registers */
#define I2C_CPLD_ADDR	0x35
#define CFG_REG		0x10

static unsigned short detect_daughter_board_profile(void)
{
	unsigned short val;

#ifndef CONFIG_DM_I2C
	if (i2c_probe(I2C_CPLD_ADDR))
		return PROFILE_NONE;

	if (i2c_read(I2C_CPLD_ADDR, CFG_REG, 1, (unsigned char *)(&val), 2))
		return PROFILE_NONE;
#else
	struct udevice *dev = NULL;
	int rc;

	rc = i2c_get_chip_for_busnum(0, I2C_CPLD_ADDR, 1, &dev);
	if (rc)
		return PROFILE_NONE;
	rc = dm_i2c_read(dev, CFG_REG, (unsigned char *)(&val), 2);
	if (rc)
		return PROFILE_NONE;
#endif
	return (1 << (val & PROFILE_MASK));
}

void enable_board_pin_mux(void)
{
	/* Do board-specific muxes. */
	if (board_is_bone()) {
		/* Beaglebone pinmux */
		configure_module_pin_mux(mii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux);
#if defined(CONFIG_NAND)
		configure_module_pin_mux(nand_pin_mux);
#elif defined(CONFIG_NOR)
		configure_module_pin_mux(bone_norcape_pin_mux);
#else
		configure_module_pin_mux(mmc1_pin_mux);
#endif
	} else if (board_is_gp_evm()) {
		/* General Purpose EVM */
		unsigned short profile = detect_daughter_board_profile();
		configure_module_pin_mux(rgmii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux);
		/* In profile #2 i2c1 and spi0 conflict. */
		if (profile & ~PROFILE_2)
			configure_module_pin_mux(i2c1_pin_mux);
		/* Profiles 2 & 3 don't have NAND */
#ifdef CONFIG_NAND
		if (profile & ~(PROFILE_2 | PROFILE_3))
			configure_module_pin_mux(nand_pin_mux);
#endif
		else if (profile == PROFILE_2) {
			configure_module_pin_mux(mmc1_pin_mux);
			configure_module_pin_mux(spi0_pin_mux);
		}
	} else if (board_is_idk()) {
		/* Industrial Motor Control (IDK) */
		configure_module_pin_mux(mii1_pin_mux);
		configure_module_pin_mux(mmc0_no_cd_pin_mux);
	} else if (board_is_evm_sk()) {
		/* Starter Kit EVM */
		configure_module_pin_mux(i2c1_pin_mux);
		configure_module_pin_mux(gpio0_7_pin_mux);
		configure_module_pin_mux(rgmii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux_sk_evm);
	} else if (board_is_bone_lt()) {
		if (board_is_bben()) {
			/* SanCloud Beaglebone LT Enhanced pinmux */
			configure_module_pin_mux(rgmii1_pin_mux);
		} else {
			/* Beaglebone LT pinmux */
			configure_module_pin_mux(mii1_pin_mux);
		}
		/* Beaglebone LT pinmux */
		configure_module_pin_mux(mmc0_pin_mux);
#if defined(CONFIG_NAND) && defined(CONFIG_EMMC_BOOT)
		configure_module_pin_mux(nand_pin_mux);
#elif defined(CONFIG_NOR) && defined(CONFIG_EMMC_BOOT)
		configure_module_pin_mux(bone_norcape_pin_mux);
#else
		configure_module_pin_mux(mmc1_pin_mux);
#endif
	} else if (board_is_pb()) {
		configure_module_pin_mux(mii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux);
	} else if (board_is_icev2()) {
		configure_module_pin_mux(mmc0_pin_mux);
		configure_module_pin_mux(gpio0_18_pin_mux);
		configure_module_pin_mux(uart3_icev2_pin_mux);
		configure_module_pin_mux(rmii1_pin_mux);
		configure_module_pin_mux(spi0_pin_mux);
	} else {
		/* Unknown board. We might still be able to boot. */
		puts("Bad EEPROM or unknown board, cannot configure pinmux.");
	}
}
