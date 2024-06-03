// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <linux/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/arch/gpio.h>

#define DB_88F6281_OE_LOW	~(BIT(7))
#define DB_88F6281_OE_HIGH	~(BIT(15) | BIT(14) | BIT(13) | BIT(4))
#define DB_88F6281_OE_VAL_LOW	BIT(7)
#define DB_88F6281_OE_VAL_HIGH	0

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	mvebu_config_gpio(DB_88F6281_OE_VAL_LOW,
			  DB_88F6281_OE_VAL_HIGH,
			  DB_88F6281_OE_LOW, DB_88F6281_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
#ifdef CONFIG_CMD_NAND
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
#else
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
#endif
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_SD_CLK,
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_SATA1_ACTn,
		MPP21_SATA0_ACTn,
		MPP22_GPIO,
		MPP23_GPIO,
		MPP24_GPIO,
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,
		MPP29_GPIO,
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,
		MPP36_GPIO,
		MPP37_GPIO,
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_GPIO,
		MPP41_GPIO,
		MPP42_GPIO,
		MPP43_GPIO,
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,
		MPP48_GPIO,
		MPP49_GPIO,
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

#ifdef CONFIG_RESET_PHY_R
/* automatically defined by kirkwood config.h */
void reset_phy(void)
{
}
#endif
