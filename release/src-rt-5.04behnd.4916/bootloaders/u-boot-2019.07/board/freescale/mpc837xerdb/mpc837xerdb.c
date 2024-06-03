// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Kevin Lam <kevin.lam@freescale.com>
 * Joe D'Abbraccio <joe.d'abbraccio@freescale.com>
 */

#include <common.h>
#include <hwconfig.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/fsl_mpc83xx_serdes.h>
#include <fdt_support.h>
#include <spd_sdram.h>
#include <vsc7385.h>
#include <fsl_esdhc.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SYS_DRAM_TEST)
int
testdram(void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf("Testing DRAM from 0x%08x to 0x%08x\n",
	       CONFIG_SYS_MEMTEST_START,
	       CONFIG_SYS_MEMTEST_END);

	printf("DRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test passed.\n");
	return 0;
}
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
void ddr_enable_ecc(unsigned int dram_size);
#endif
int fixed_sdram(void);

int dram_init(void)
{
	immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -ENXIO;

#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/* Initialize DDR ECC byte */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif
	/* return total bus DDR size(bytes) */
	gd->ram_size = msize * 1024 * 1024;

	return 0;
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = CONFIG_SYS_DDR_SIZE * 1024 * 1024;
	u32 msize_log2 = __ilog2(msize);

	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_SDRAM_BASE & 0xfffff000;
	im->sysconf.ddrlaw[0].ar = LBLAWAR_EN | (msize_log2 - 1);

	im->sysconf.ddrcdr = CONFIG_SYS_DDRCDR_VALUE;
	udelay(50000);

	im->ddr.sdram_clk_cntl = CONFIG_SYS_DDR_SDRAM_CLK_CNTL;
	udelay(1000);

	im->ddr.csbnds[0].csbnds = CONFIG_SYS_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CONFIG_SYS_DDR_CS0_CONFIG;
	udelay(1000);

	im->ddr.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	im->ddr.sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CONFIG_SYS_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_mode2 = CONFIG_SYS_DDR_MODE2;
	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	sync();
	udelay(1000);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
	udelay(2000);
	return CONFIG_SYS_DDR_SIZE;
}
#endif	/*!CONFIG_SYS_SPD_EEPROM */

int checkboard(void)
{
	puts("Board: Freescale MPC837xERDB\n");
	return 0;
}

int board_early_init_f(void)
{
#ifdef CONFIG_FSL_SERDES
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	u32 spridr = in_be32(&immr->sysconf.spridr);

	/* we check only part num, and don't look for CPU revisions */
	switch (PARTID_NO_E(spridr)) {
	case SPR_8377:
		fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_SATA,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		fsl_setup_serdes(CONFIG_FSL_SERDES2, FSL_SERDES_PROTO_PEX,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		break;
	case SPR_8378:
		fsl_setup_serdes(CONFIG_FSL_SERDES2, FSL_SERDES_PROTO_PEX,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		break;
	case SPR_8379:
		fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_SATA,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		fsl_setup_serdes(CONFIG_FSL_SERDES2, FSL_SERDES_PROTO_SATA,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		break;
	default:
		printf("serdes not configured: unknown CPU part number: "
		       "%04x\n", spridr >> 16);
		break;
	}
#endif /* CONFIG_FSL_SERDES */
	return 0;
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_init(bd_t *bd)
{
	struct immap __iomem *im = (struct immap __iomem *)CONFIG_SYS_IMMR;
	char buffer[HWCONFIG_BUFFER_SIZE] = {0};
	int esdhc_hwconfig_enabled = 0;

	if (env_get_f("hwconfig", buffer, sizeof(buffer)) > 0)
		esdhc_hwconfig_enabled = hwconfig_f("esdhc", buffer);

	if (esdhc_hwconfig_enabled == 0)
		return 0;

	clrsetbits_be32(&im->sysconf.sicrl, SICRL_USB_B, SICRL_USB_B_SD);
	clrsetbits_be32(&im->sysconf.sicrh, SICRH_SPI, SICRH_SPI_SD);

	return fsl_esdhc_mmc_init(bd);
}
#endif

/*
 * Miscellaneous late-boot configurations
 *
 * If a VSC7385 microcode image is present, then upload it.
*/
int misc_init_r(void)
{
	int rc = 0;

#ifdef CONFIG_VSC7385_IMAGE
	if (vsc7385_upload_firmware((void *) CONFIG_VSC7385_IMAGE,
		CONFIG_VSC7385_IMAGE_SIZE)) {
		puts("Failure uploading VSC7385 microcode.\n");
		rc = 1;
	}
#endif

	return rc;
}

#if defined(CONFIG_OF_BOARD_SETUP)

int ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	ft_cpu_setup(blob, bd);
	fsl_fdt_fixup_dr_usb(blob, bd);
	fdt_fixup_esdhc(blob, bd);

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */
