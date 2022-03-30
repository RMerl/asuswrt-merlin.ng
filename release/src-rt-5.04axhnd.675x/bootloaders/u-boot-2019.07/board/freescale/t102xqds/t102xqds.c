// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>
#include <hwconfig.h>
#include "../common/qixis.h"
#include "t102xqds.h"
#include "t102xqds_qixis.h"
#include "../common/sleep.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	char buf[64];
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *const freq[] = {"100", "125", "156.25", "100.0"};
	int clock;
	u8 sw = QIXIS_READ(arch);

	printf("Board: %sQDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Board Arch: V%d, ", QIXIS_READ(id), sw >> 4);
	printf("Board Version: %c, boot from ", (sw & 0xf) + 'A' - 1);

#ifdef CONFIG_SDCARD
	puts("SD/MMC\n");
#elif CONFIG_SPIFLASH
	puts("SPI\n");
#else
	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("PromJet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else if (sw == 0x15)
		printf("IFC Card\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);
#endif

	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	puts("SERDES Reference: ");
	sw = QIXIS_READ(brdcfg[2]);
	clock = (sw >> 6) & 3;
	printf("Clock1=%sMHz ", freq[clock]);
	clock = (sw >> 4) & 3;
	printf("Clock2=%sMHz\n", freq[clock]);

	return 0;
}

int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

static int board_mux_lane_to_slot(void)
{
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s1;
	u8 brdcfg9;

	srds_prtcl_s1 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_prtcl_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;


	brdcfg9 = QIXIS_READ(brdcfg[9]);
	QIXIS_WRITE(brdcfg[9], brdcfg9 | BRDCFG9_XFI_TX_DISABLE);

	switch (srds_prtcl_s1) {
	case 0:
		/* SerDes1 is not enabled */
		break;
	case 0xd5:
	case 0x5b:
	case 0x6b:
	case 0x77:
	case 0x6f:
	case 0x7f:
		QIXIS_WRITE(brdcfg[12], 0x8c);
		break;
	case 0x40:
		QIXIS_WRITE(brdcfg[12], 0xfc);
		break;
	case 0xd6:
	case 0x5a:
	case 0x6a:
	case 0x56:
		QIXIS_WRITE(brdcfg[12], 0x88);
		break;
	case 0x47:
		QIXIS_WRITE(brdcfg[12], 0xcc);
		break;
	case 0x46:
		QIXIS_WRITE(brdcfg[12], 0xc8);
		break;
	case 0x95:
	case 0x99:
		brdcfg9 &= ~BRDCFG9_XFI_TX_DISABLE;
		QIXIS_WRITE(brdcfg[9], brdcfg9);
		QIXIS_WRITE(brdcfg[12], 0x8c);
		break;
	case 0x116:
		QIXIS_WRITE(brdcfg[12], 0x00);
		break;
	case 0x115:
	case 0x119:
	case 0x129:
	case 0x12b:
		/* Aurora, PCIe, SGMII, SATA */
		QIXIS_WRITE(brdcfg[12], 0x04);
		break;
	default:
		printf("WARNING: unsupported for SerDes Protocol %d\n",
		       srds_prtcl_s1);
		return -1;
	}

	return 0;
}

#ifdef CONFIG_ARCH_T1024
static void board_mux_setup(void)
{
	u8 brdcfg15;

	brdcfg15 = QIXIS_READ(brdcfg[15]);
	brdcfg15 &= ~BRDCFG15_DIUSEL_MASK;

	if (hwconfig_arg_cmp("pin_mux", "tdm")) {
		/* Route QE_TDM multiplexed signals to TDM Riser slot */
		QIXIS_WRITE(brdcfg[15], brdcfg15 | BRDCFG15_DIUSEL_TDM);
		QIXIS_WRITE(brdcfg[13], BRDCFG13_TDM_INTERFACE << 2);
		QIXIS_WRITE(brdcfg[5], (QIXIS_READ(brdcfg[5]) &
			    ~BRDCFG5_SPIRTE_MASK) | BRDCFG5_SPIRTE_TDM);
	} else if (hwconfig_arg_cmp("pin_mux", "ucc")) {
		/* to UCC (ProfiBus) interface */
		QIXIS_WRITE(brdcfg[15], brdcfg15 | BRDCFG15_DIUSEL_UCC);
	} else if (hwconfig_arg_cmp("pin_mux", "hdmi")) {
		/* to DVI (HDMI) encoder */
		QIXIS_WRITE(brdcfg[15], brdcfg15 | BRDCFG15_DIUSEL_HDMI);
	} else if (hwconfig_arg_cmp("pin_mux", "lcd")) {
		/* to DFP (LCD) encoder */
		QIXIS_WRITE(brdcfg[15], brdcfg15 | BRDCFG15_LCDFM |
			    BRDCFG15_LCDPD | BRDCFG15_DIUSEL_LCD);
	}

	if (hwconfig_arg_cmp("adaptor", "sdxc"))
		/* Route SPI_CS multiplexed signals to SD slot */
		QIXIS_WRITE(brdcfg[5], (QIXIS_READ(brdcfg[5]) &
			    ~BRDCFG5_SPIRTE_MASK) | BRDCFG5_SPIRTE_SDHC);
}
#endif

void board_retimer_ds125df111_init(void)
{
	u8 reg;

	/* Retimer DS125DF111 is connected to I2C1_CH7_CH5 */
	reg = I2C_MUX_CH7;
	i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &reg, 1);
	reg = I2C_MUX_CH5;
	i2c_write(I2C_MUX_PCA_ADDR_SEC, 0, 1, &reg, 1);

	/* Access to Control/Shared register */
	reg = 0x0;
	i2c_write(I2C_RETIMER_ADDR, 0xff, 1, &reg, 1);

	/* Read device revision and ID */
	i2c_read(I2C_RETIMER_ADDR, 1, 1, &reg, 1);
	debug("Retimer version id = 0x%x\n", reg);

	/* Enable Broadcast */
	reg = 0x0c;
	i2c_write(I2C_RETIMER_ADDR, 0xff, 1, &reg, 1);

	/* Reset Channel Registers */
	i2c_read(I2C_RETIMER_ADDR, 0, 1, &reg, 1);
	reg |= 0x4;
	i2c_write(I2C_RETIMER_ADDR, 0, 1, &reg, 1);

	/* Enable override divider select and Enable Override Output Mux */
	i2c_read(I2C_RETIMER_ADDR, 9, 1, &reg, 1);
	reg |= 0x24;
	i2c_write(I2C_RETIMER_ADDR, 9, 1, &reg, 1);

	/* Select VCO Divider to full rate (000) */
	i2c_read(I2C_RETIMER_ADDR, 0x18, 1, &reg, 1);
	reg &= 0x8f;
	i2c_write(I2C_RETIMER_ADDR, 0x18, 1, &reg, 1);

	/* Select active PFD MUX input as re-timed data (001) */
	i2c_read(I2C_RETIMER_ADDR, 0x1e, 1, &reg, 1);
	reg &= 0x3f;
	reg |= 0x20;
	i2c_write(I2C_RETIMER_ADDR, 0x1e, 1, &reg, 1);

	/* Set data rate as 10.3125 Gbps */
	reg = 0x0;
	i2c_write(I2C_RETIMER_ADDR, 0x60, 1, &reg, 1);
	reg = 0xb2;
	i2c_write(I2C_RETIMER_ADDR, 0x61, 1, &reg, 1);
	reg = 0x90;
	i2c_write(I2C_RETIMER_ADDR, 0x62, 1, &reg, 1);
	reg = 0xb3;
	i2c_write(I2C_RETIMER_ADDR, 0x63, 1, &reg, 1);
	reg = 0xcd;
	i2c_write(I2C_RETIMER_ADDR, 0x64, 1, &reg, 1);
}

int board_early_init_f(void)
{
#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_SYS_FLASH_BASE
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2; /* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);
#endif
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);
	board_mux_lane_to_slot();
	board_retimer_ds125df111_init();

	/* Increase IO drive strength to address FCS error on RGMII */
	out_be32((unsigned *)CONFIG_SYS_FSL_SCFG_IODSECR1_ADDR, 0xbfdb7800);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x0F) {
	case QIXIS_SYSCLK_64:
		return 64000000;
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 66666666;
}

#define NUM_SRDS_PLL	2
int misc_init_r(void)
{
#ifdef CONFIG_ARCH_T1024
	board_mux_setup();
#endif
	return 0;
}

void fdt_fixup_spi_mux(void *blob)
{
	int nodeoff = 0;

	if (hwconfig_arg_cmp("pin_mux", "tdm")) {
		while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
			"eon,en25s64")) >= 0) {
			fdt_del_node(blob, nodeoff);
		}
	} else {
		/* remove tdm node */
		while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
			"maxim,ds26522")) >= 0) {
			fdt_del_node(blob, nodeoff);
		}
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);

#ifdef CONFIG_HAS_FSL_DR_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif
	fdt_fixup_spi_mux(blob);

	return 0;
}

void qixis_dump_switch(void)
{
	int i, nr_of_cfgsw;

	QIXIS_WRITE(cms[0], 0x00);
	nr_of_cfgsw = QIXIS_READ(cms[1]);

	puts("DIP switch settings dump:\n");
	for (i = 1; i <= nr_of_cfgsw; i++) {
		QIXIS_WRITE(cms[0], i);
		printf("SW%d = (0x%02x)\n", i, QIXIS_READ(cms[1]));
	}
}
