// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <hwconfig.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <environment.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#ifdef CONFIG_FSL_LS_PPA
#include <asm/arch/ppa.h>
#endif
#include <fsl_immap.h>
#include <netdev.h>

#include <fdtdec.h>
#include <miiphy.h>
#include "../common/qixis.h"

DECLARE_GLOBAL_DATA_PTR;

int config_board_mux(void)
{
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
	return 0;
}

int board_init(void)
{
#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

#ifndef CONFIG_SYS_EARLY_PCI_INIT
	pci_init();
#endif

#if defined(CONFIG_TARGET_LS1028ARDB)
	u8 val = I2C_MUX_CH_DEFAULT;

	i2c_write(I2C_MUX_PCA_ADDR_PRI, 0x0b, 1, &val, 1);
#endif
	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	config_board_mux();

	return 0;
}
#endif

int board_early_init_f(void)
{
#ifdef CONFIG_SYS_I2C_EARLY_INIT
	i2c_early_init_f();
#endif

	fsl_lsch3_early_init_f();
	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
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
