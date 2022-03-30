// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Savoir-faire Linux Inc.
 *
 * Derived from MX51EVK code by
 *   Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx51.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/mx5_video.h>
#include <environment.h>
#include <mmc.h>
#include <input.h>
#include <fsl_esdhc.h>
#include <mc13892.h>

#include <malloc.h>
#include <netdev.h>
#include <phy.h>
#include "ts4800.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

u32 get_board_rev(void)
{
	u32 rev = get_cpu_rev();
	if (!gpio_get_value(IMX_GPIO_NR(1, 22)))
		rev |= BOARD_REV_2_0 << BOARD_VER_OFFSET;
	return rev;
}

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_DSE_HIGH)

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		MX51_PAD_UART1_RXD__UART1_RXD,
		MX51_PAD_UART1_TXD__UART1_TXD,
		NEW_PAD_CTRL(MX51_PAD_UART1_RTS__UART1_RTS, UART_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_UART1_CTS__UART1_CTS, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_EIM_EB2__FEC_MDIO,
				PAD_CTL_HYS |
				PAD_CTL_PUS_22K_UP |
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
		MX51_PAD_EIM_EB3__FEC_RDATA1,
		NEW_PAD_CTRL(MX51_PAD_EIM_CS2__FEC_RDATA2, PAD_CTL_HYS),
		MX51_PAD_EIM_CS3__FEC_RDATA3,
		MX51_PAD_NANDF_CS2__FEC_TX_ER,
		MX51_PAD_EIM_CS5__FEC_CRS,
		MX51_PAD_EIM_CS4__FEC_RX_ER,
		/* PAD used on TS4800 */
		MX51_PAD_DI2_PIN2__FEC_MDC,
		MX51_PAD_DISP2_DAT14__FEC_RDAT0,
		MX51_PAD_DISP2_DAT10__FEC_COL,
		MX51_PAD_DISP2_DAT11__FEC_RXCLK,
		MX51_PAD_DISP2_DAT15__FEC_TDAT0,
		MX51_PAD_DISP2_DAT6__FEC_TDAT1,
		MX51_PAD_DISP2_DAT7__FEC_TDAT2,
		MX51_PAD_DISP2_DAT8__FEC_TDAT3,
		MX51_PAD_DISP2_DAT9__FEC_TX_EN,
		MX51_PAD_DISP2_DAT13__FEC_TX_CLK,
		MX51_PAD_DISP2_DAT12__FEC_RX_DV,
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_0__GPIO1_0,
						NO_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(1, 0));
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_6__GPIO1_6,
						NO_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(1, 6));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(1, 0));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(1, 6));

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_SD1_CMD__SD1_CMD, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_CLK__SD1_CLK, PAD_CTL_DSE_MAX |
			PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA0__SD1_DATA0, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA1__SD1_DATA1, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA2__SD1_DATA2, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA3__SD1_DATA3, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_0__SD1_CD, PAD_CTL_HYS),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_1__SD1_WP, PAD_CTL_HYS),
	};

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	imx_iomux_v3_setup_multiple_pads(sd1_pads, ARRAY_SIZE(sd1_pads));

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

/*
 * Read the MAC address from FEC's registers PALR PAUR.
 * User is supposed to configure these registers when MAC address is known
 * from another source (fuse), but on TS4800, MAC address is not fused and
 * the bootrom configure these registers on startup.
 */
static int fec_get_mac_from_register(uint32_t base_addr)
{
	unsigned char ethaddr[6];
	u32 reg_mac[2];
	int i;

	reg_mac[0] = in_be32(base_addr + 0xE4);
	reg_mac[1] = in_be32(base_addr + 0xE8);

	for(i = 0; i < 6; i++)
		ethaddr[i] = (reg_mac[i / 4] >> ((i % 4) * 8)) & 0xFF;

	if (is_valid_ethaddr(ethaddr)) {
		eth_env_set_enetaddr("ethaddr", ethaddr);
		return 0;
	}

	return -1;
}

#define TS4800_GPIO_FEC_PHY_RES         IMX_GPIO_NR(2, 14)
int board_eth_init(bd_t *bd)
{
	int dev_id = -1;
	int phy_id = 0xFF;
	uint32_t addr = IMX_FEC_BASE;

	uint32_t base_mii;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	/* reset FEC phy */
	imx_iomux_v3_setup_pad(MX51_PAD_EIM_A20__GPIO2_14);
	gpio_direction_output(TS4800_GPIO_FEC_PHY_RES, 0);
	mdelay(1);
	gpio_set_value(TS4800_GPIO_FEC_PHY_RES, 1);
	mdelay(1);

	base_mii = addr;
	debug("eth_init: fec_probe(bd, %i, %i) @ %08x\n", dev_id, phy_id, addr);
	bus = fec_get_miibus(base_mii, dev_id);
	if (!bus)
		return -ENOMEM;

	phydev = phy_find_by_mask(bus, phy_id, PHY_INTERFACE_MODE_MII);
	if (!phydev) {
		free(bus);
		return -ENOMEM;
	}

	if (fec_get_mac_from_register(addr))
		printf("eth_init: failed to get MAC address\n");

	ret = fec_probe(bd, dev_id, addr, bus, phydev);
	if (ret) {
		free(phydev);
		free(bus);
	}

	return ret;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int checkboard(void)
{
	puts("Board: TS4800\n");

	return 0;
}

void hw_watchdog_reset(void)
{
	struct ts4800_wtd_regs *wtd = (struct ts4800_wtd_regs *) (TS4800_SYSCON_BASE + 0xE);
	/* feed the watchdog for another 10s */
	writew(0x2, &wtd->feed);
}

void hw_watchdog_init(void)
{
	return;
}
