// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2022 NXP
 */

#include <common.h>
#include <display_options.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <hwconfig.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <env_internal.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#include <fsl_immap.h>
#include <netdev.h>

#include <fdtdec.h>
#include <miiphy.h>
#include "../common/qixis.h"
#include "../drivers/net/fsl_enetc.h"

DECLARE_GLOBAL_DATA_PTR;

int config_board_mux(void)
{
#ifndef CONFIG_LPUART
#if defined(CONFIG_TARGET_LS1028AQDS) && defined(CONFIG_FSL_QIXIS)
	u8 reg;

	reg = QIXIS_READ(brdcfg[13]);
	/* Field| Function
	 * 7-6  | Controls I2C3 routing (net CFG_MUX_I2C3):
	 * I2C3 | 10= Routes {SCL, SDA} to CAN1 transceiver as {TX, RX}.
	 * 5-4  | Controls I2C4 routing (net CFG_MUX_I2C4):
	 * I2C4 |11= Routes {SCL, SDA} to CAN2 transceiver as {TX, RX}.
	 */
	reg &= ~(0xf0);
	reg |= 0xb0;
	QIXIS_WRITE(brdcfg[13], reg);

	reg = QIXIS_READ(brdcfg[15]);
	/* Field| Function
	 * 7    | Controls the CAN1 transceiver (net CFG_CAN1_STBY):
	 * CAN1 | 0= CAN #1 transceiver enabled
	 * 6    | Controls the CAN2 transceiver (net CFG_CAN2_STBY):
	 * CAN2 | 0= CAN #2 transceiver enabled
	 */
	reg &= ~(0xc0);
	QIXIS_WRITE(brdcfg[15], reg);
#endif
#endif

	return 0;
}

#ifdef CONFIG_LPUART
u32 get_lpuart_clk(void)
{
	return gd->bus_clk / CONFIG_SYS_FSL_LPUART_CLK_DIV;
}
#endif

int board_init(void)
{
#ifndef CONFIG_SYS_EARLY_PCI_INIT
	pci_init();
#endif

#if defined(CONFIG_TARGET_LS1028ARDB)
	u8 val = I2C_MUX_CH_DEFAULT;

#if !CONFIG_IS_ENABLED(DM_I2C)
	i2c_write(I2C_MUX_PCA_ADDR_PRI, 0x0b, 1, &val, 1);
#else
	struct udevice *dev;

	if (!i2c_get_chip_for_busnum(0, I2C_MUX_PCA_ADDR_PRI, 1, &dev))
		dm_i2c_write(dev, 0x0b, &val, 1);
#endif
#endif

#if defined(CONFIG_TARGET_LS1028ARDB)
	u8 reg;

	reg = QIXIS_READ(brdcfg[4]);
	/*
	 * Field | Function
	 * 3     | DisplayPort Power Enable (net DP_PWR_EN):
	 * DPPWR | 0= DP_PWR is enabled.
	 */
	reg &= ~(DP_PWD_EN_DEFAULT_MASK);
	QIXIS_WRITE(brdcfg[4], reg);
#endif
	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return pci_eth_init(bis);
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	config_board_mux();

	return 0;
}
#endif

int board_early_init_f(void)
{
#ifdef CONFIG_LPUART
	u8 uart;
#endif

#if defined(CONFIG_SYS_I2C_EARLY_INIT) && defined(CONFIG_SPL_BUILD)
	i2c_early_init_f();
#endif

	fsl_lsch3_early_init_f();

#ifdef CONFIG_LPUART
	/*
	 * Field| Function
	 * --------------------------------------------------------------
	 * 7-6  | Controls I2C3 routing (net CFG_MUX_I2C3):
	 * I2C3 | 11= Routes {SCL, SDA} to LPUART1 header as {SOUT, SIN}.
	 * --------------------------------------------------------------
	 * 5-4  | Controls I2C4 routing (net CFG_MUX_I2C4):
	 * I2C4 |11= Routes {SCL, SDA} to LPUART1 header as {CTS_B, RTS_B}.
	 */
	/* use lpuart0 as system console */
	uart = QIXIS_READ(brdcfg[13]);
	uart &= ~CFG_LPUART_MUX_MASK;
	uart |= CFG_LPUART_EN;
	QIXIS_WRITE(brdcfg[13], uart);
#endif

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
}

int esdhc_status_fixup(void *blob, const char *compat)
{
	void __iomem *dcfg_ccsr = (void __iomem *)DCFG_BASE;
	char esdhc1_path[] = "/soc/mmc@2140000";
	char esdhc2_path[] = "/soc/mmc@2150000";
	char dspi1_path[] = "/soc/spi@2100000";
	char dspi2_path[] = "/soc/spi@2110000";
	u32 mux_sdhc1, mux_sdhc2;
	u32 io = 0;

	/*
	 * The PMUX IO-expander for mux select is used to control
	 * the muxing of various onboard interfaces.
	 */

	io = in_le32(dcfg_ccsr + DCFG_RCWSR12);
	mux_sdhc1 = (io >> DCFG_RCWSR12_SDHC_SHIFT) & DCFG_RCWSR12_SDHC_MASK;

	/* Disable esdhc1/dspi1 if not selected. */
	if (mux_sdhc1 != 0)
		do_fixup_by_path(blob, esdhc1_path, "status", "disabled",
				 sizeof("disabled"), 1);
	if (mux_sdhc1 != 2)
		do_fixup_by_path(blob, dspi1_path, "status", "disabled",
				 sizeof("disabled"), 1);

	io = in_le32(dcfg_ccsr + DCFG_RCWSR13);
	mux_sdhc2 = (io >> DCFG_RCWSR13_SDHC_SHIFT) & DCFG_RCWSR13_SDHC_MASK;

	/* Disable esdhc2/dspi2 if not selected. */
	if (mux_sdhc2 != 0)
		do_fixup_by_path(blob, esdhc2_path, "status", "disabled",
				 sizeof("disabled"), 1);
	if (mux_sdhc2 != 2)
		do_fixup_by_path(blob, dspi2_path, "status", "disabled",
				 sizeof("disabled"), 1);

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
#endif

	fdt_fixup_memory_banks(blob, base, size, 2);

	fdt_fixup_icid(blob);

#ifdef CONFIG_FSL_ENETC
	fdt_fixup_enetc_mac(blob);
#endif

	return 0;
}
#endif

#ifdef CONFIG_FSL_QIXIS
int checkboard(void)
{
#ifdef CONFIG_TFABOOT
	enum boot_src src = get_boot_src();
#endif
	u8 sw;

	int clock;
	char *board;
	char buf[64] = {0};
	static const char *freq[6] = {"100.00", "125.00", "156.25",
					"161.13", "322.26", "100.00 SS"};

	cpu_name(buf);
	/* find the board details */
	sw = QIXIS_READ(id);

	switch (sw) {
	case 0x46:
		board = "QDS";
		break;
	case 0x47:
		board = "RDB";
		break;
	case 0x49:
		board = "HSSI";
		break;
	default:
		board = "unknown";
		break;
	}

	sw = QIXIS_READ(arch);
	printf("Board: %s-%s, Version: %c, boot from ",
	       buf, board, (sw & 0xf) + 'A' - 1);

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

#ifdef CONFIG_TFABOOT
	if (src == BOOT_SOURCE_SD_MMC) {
		puts("SD\n");
	} else if (src == BOOT_SOURCE_SD_MMC2) {
		puts("eMMC\n");
	} else {
#endif
#ifdef CONFIG_SD_BOOT
		puts("SD\n");
#elif defined(CONFIG_EMMC_BOOT)
		puts("eMMC\n");
#else
		switch (sw) {
		case 0:
		case 4:
			printf("NOR\n");
			break;
		case 1:
			printf("NAND\n");
			break;
		default:
			printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);
			break;
		}
#endif
#ifdef CONFIG_TFABOOT
	}
#endif

	printf("FPGA: v%d (%s)\n", QIXIS_READ(scver), board);
	puts("SERDES1 Reference : ");

	sw = QIXIS_READ(brdcfg[2]);
#ifdef CONFIG_TARGET_LS1028ARDB
	clock = (sw >> 6) & 3;
#else
	clock = (sw >> 4) & 0xf;
#endif

	printf("Clock1 = %sMHz ", freq[clock]);
#ifdef CONFIG_TARGET_LS1028ARDB
	clock = (sw >> 4) & 3;
#else
	clock = sw & 0xf;
#endif
	printf("Clock2 = %sMHz\n", freq[clock]);

	return 0;
}
#endif

void *video_hw_init(void)
{
	return NULL;
}
