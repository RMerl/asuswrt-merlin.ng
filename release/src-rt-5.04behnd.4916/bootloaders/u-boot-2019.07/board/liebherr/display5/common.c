// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/mx6-pins.h>
#include "common.h"

iomux_v3_cfg_t const uart_pads[] = {
	/* UART4 */
	MX6_PAD_CSI0_DAT12__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT13__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT16__UART4_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT17__UART4_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart_console_pads[] = {
	/* UART5 */
	MX6_PAD_CSI0_DAT14__UART5_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT15__UART5_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT18__UART5_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT19__UART5_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
};

void displ5_set_iomux_uart_spl(void)
{
	SETUP_IOMUX_PADS(uart_console_pads);
}

void displ5_set_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart_pads);
}

iomux_v3_cfg_t const misc_pads_spl[] = {
	/* Emergency recovery pin */
	MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

void displ5_set_iomux_misc_spl(void)
{
	SETUP_IOMUX_PADS(misc_pads_spl);
}

#ifdef CONFIG_MXC_SPI
iomux_v3_cfg_t const ecspi_pads[] = {
	/* SPI3 */
	MX6_PAD_DISP0_DAT2__ECSPI3_MISO	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT1__ECSPI3_MOSI	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT0__ECSPI3_SCLK	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT3__ECSPI3_SS0	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT4__ECSPI3_SS1	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT5__ECSPI3_SS2	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT6__ECSPI3_SS3	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT7__ECSPI3_RDY	| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const ecspi2_pads[] = {
	/* SPI2, NOR Flash nWP, CS0 */
	MX6_PAD_CSI0_DAT10__ECSPI2_MISO	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI0_DAT9__ECSPI2_MOSI	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI0_DAT8__ECSPI2_SCLK	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__GPIO5_IO29	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD3_DAT5__GPIO7_IO00	| MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	if (bus != 1 || cs != (IMX_GPIO_NR(5, 29) << 8))
		return -EINVAL;

	return IMX_GPIO_NR(5, 29);
}

void displ5_set_iomux_ecspi_spl(void)
{
	SETUP_IOMUX_PADS(ecspi2_pads);
}

void displ5_set_iomux_ecspi(void)
{
	SETUP_IOMUX_PADS(ecspi_pads);
}

#else
void displ5_set_iomux_ecspi_spl(void) {}
void displ5_set_iomux_ecspi(void) {}
#endif

#ifdef CONFIG_FSL_ESDHC
iomux_v3_cfg_t const usdhc4_pads[] = {
	MX6_PAD_SD4_CLK__SD4_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_CMD__SD4_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT0__SD4_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT1__SD4_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT2__SD4_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT3__SD4_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT4__SD4_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT5__SD4_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT6__SD4_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT7__SD4_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_ALE__SD4_RESET	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

void displ5_set_iomux_usdhc_spl(void)
{
	SETUP_IOMUX_PADS(usdhc4_pads);
}

void displ5_set_iomux_usdhc(void)
{
	SETUP_IOMUX_PADS(usdhc4_pads);
}

#else
void displ5_set_iomux_usdhc_spl(void) {}
void displ5_set_iomux_usdhc(void) {}
#endif
