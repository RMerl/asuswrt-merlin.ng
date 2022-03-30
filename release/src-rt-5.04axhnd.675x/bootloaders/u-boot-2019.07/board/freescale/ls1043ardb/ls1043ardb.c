// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <fdt_support.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include <fsl_sec.h>
#include "cpld.h"
#ifdef CONFIG_U_QE
#include <fsl_qe.h>
#endif
#include <asm/arch/ppa.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_TFABOOT
struct ifc_regs ifc_cfg_nor_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nor",
		CONFIG_SYS_NOR_CSPR,
		CONFIG_SYS_NOR_CSPR_EXT,
		CONFIG_SYS_NOR_AMASK,
		CONFIG_SYS_NOR_CSOR,
		{
			CONFIG_SYS_NOR_FTIM0,
			CONFIG_SYS_NOR_FTIM1,
			CONFIG_SYS_NOR_FTIM2,
			CONFIG_SYS_NOR_FTIM3
		},

	},
	{
		"nand",
		CONFIG_SYS_NAND_CSPR,
		CONFIG_SYS_NAND_CSPR_EXT,
		CONFIG_SYS_NAND_AMASK,
		CONFIG_SYS_NAND_CSOR,
		{
			CONFIG_SYS_NAND_FTIM0,
			CONFIG_SYS_NAND_FTIM1,
			CONFIG_SYS_NAND_FTIM2,
			CONFIG_SYS_NAND_FTIM3
		},
	},
	{
		"cpld",
		CONFIG_SYS_CPLD_CSPR,
		CONFIG_SYS_CPLD_CSPR_EXT,
		CONFIG_SYS_CPLD_AMASK,
		CONFIG_SYS_CPLD_CSOR,
		{
			CONFIG_SYS_CPLD_FTIM0,
			CONFIG_SYS_CPLD_FTIM1,
			CONFIG_SYS_CPLD_FTIM2,
			CONFIG_SYS_CPLD_FTIM3
		},
	}
};

struct ifc_regs ifc_cfg_nand_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nand",
		CONFIG_SYS_NAND_CSPR,
		CONFIG_SYS_NAND_CSPR_EXT,
		CONFIG_SYS_NAND_AMASK,
		CONFIG_SYS_NAND_CSOR,
		{
			CONFIG_SYS_NAND_FTIM0,
			CONFIG_SYS_NAND_FTIM1,
			CONFIG_SYS_NAND_FTIM2,
			CONFIG_SYS_NAND_FTIM3
		},
	},
	{
		"nor",
		CONFIG_SYS_NOR_CSPR,
		CONFIG_SYS_NOR_CSPR_EXT,
		CONFIG_SYS_NOR_AMASK,
		CONFIG_SYS_NOR_CSOR,
		{
			CONFIG_SYS_NOR_FTIM0,
			CONFIG_SYS_NOR_FTIM1,
			CONFIG_SYS_NOR_FTIM2,
			CONFIG_SYS_NOR_FTIM3
		},
	},
	{
		"cpld",
		CONFIG_SYS_CPLD_CSPR,
		CONFIG_SYS_CPLD_CSPR_EXT,
		CONFIG_SYS_CPLD_AMASK,
		CONFIG_SYS_CPLD_CSOR,
		{
			CONFIG_SYS_CPLD_FTIM0,
			CONFIG_SYS_CPLD_FTIM1,
			CONFIG_SYS_CPLD_FTIM2,
			CONFIG_SYS_CPLD_FTIM3
		},
	}
};

void ifc_cfg_boot_info(struct ifc_regs_info *regs_info)
{
	enum boot_src src = get_boot_src();

	if (src == BOOT_SOURCE_IFC_NAND)
		regs_info->regs = ifc_cfg_nand_boot;
	else
		regs_info->regs = ifc_cfg_nor_boot;
	regs_info->cs_size = CONFIG_SYS_FSL_IFC_BANK_COUNT;
}

#endif
int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

#ifndef CONFIG_SPL_BUILD

int checkboard(void)
{
#ifdef CONFIG_TFABOOT
	enum boot_src src = get_boot_src();
#endif
	static const char *freq[2] = {"100.00MHZ", "156.25MHZ"};
#ifndef CONFIG_SD_BOOT
	u8 cfg_rcw_src1, cfg_rcw_src2;
	u16 cfg_rcw_src;
#endif
	u8 sd1refclk_sel;

	printf("Board: LS1043ARDB, boot from ");

#ifdef CONFIG_TFABOOT
	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD\n");
	else {
#endif

#ifdef CONFIG_SD_BOOT
	puts("SD\n");
#else
	cfg_rcw_src1 = CPLD_READ(cfg_rcw_src1);
	cfg_rcw_src2 = CPLD_READ(cfg_rcw_src2);
	cpld_rev_bit(&cfg_rcw_src1);
	cfg_rcw_src = cfg_rcw_src1;
	cfg_rcw_src = (cfg_rcw_src << 1) | cfg_rcw_src2;

	if (cfg_rcw_src == 0x25)
		printf("vBank %d\n", CPLD_READ(vbank));
	else if (cfg_rcw_src == 0x106)
		puts("NAND\n");
	else
		printf("Invalid setting of SW4\n");
#endif

#ifdef CONFIG_TFABOOT
	}
#endif
	printf("CPLD:  V%x.%x\nPCBA:  V%x.0\n", CPLD_READ(cpld_ver),
	       CPLD_READ(cpld_ver_sub), CPLD_READ(pcba_ver));

	puts("SERDES Reference Clocks:\n");
	sd1refclk_sel = CPLD_READ(sd1refclk_sel);
	printf("SD1_CLK1 = %s, SD1_CLK2 = %s\n", freq[sd1refclk_sel], freq[0]);

	return 0;
}

int board_init(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
	erratum_a010315();
#endif

#ifdef CONFIG_FSL_IFC
	init_final_memctl_regs();
#endif

#ifdef CONFIG_SECURE_BOOT
	/* In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	 */
	u32 val;
	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

#ifdef CONFIG_U_QE
	u_qe_init();
#endif
	/* invert AQR105 IRQ pins polarity */
	out_be32(&scfg->intpcr, AQR105_IRQ_MASK);

	return 0;
}

int config_board_mux(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;

	if (hwconfig("qe-hdlc")) {
		out_be32(&scfg->rcwpmuxcr0,
			 (in_be32(&scfg->rcwpmuxcr0) & ~0xff00) | 0x6600);
		printf("Assign to qe-hdlc clk, rcwpmuxcr0=%x\n",
		       in_be32(&scfg->rcwpmuxcr0));
	} else {
#ifdef CONFIG_HAS_FSL_XHCI_USB
		out_be32(&scfg->rcwpmuxcr0, 0x3333);
		out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);
		usb_pwrfault = (SCFG_USBPWRFAULT_DEDICATED <<
				SCFG_USBPWRFAULT_USB3_SHIFT) |
				(SCFG_USBPWRFAULT_DEDICATED <<
				SCFG_USBPWRFAULT_USB2_SHIFT) |
				(SCFG_USBPWRFAULT_SHARED <<
				 SCFG_USBPWRFAULT_USB1_SHIFT);
		out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif
	}
	return 0;
}

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
	config_board_mux();
	return 0;
}
#endif

void fdt_del_qe(void *blob)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
				"fsl,qe")) >= 0) {
		fdt_del_node(blob, nodeoff);
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	/* fixup DT for the two DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

	fdt_fixup_memory_banks(blob, base, size, 2);
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

	fdt_fixup_icid(blob);

	/*
	 * qe-hdlc and usb multi-use the pins,
	 * when set hwconfig to qe-hdlc, delete usb node.
	 */
	if (hwconfig("qe-hdlc"))
#ifdef CONFIG_HAS_FSL_XHCI_USB
		fdt_del_node_and_alias(blob, "usb1");
#endif
	/*
	 * qe just support qe-uart and qe-hdlc,
	 * if qe-uart and qe-hdlc are not set in hwconfig,
	 * delete qe node.
	 */
	if (!hwconfig("qe-uart") && !hwconfig("qe-hdlc"))
		fdt_del_qe(blob);

	return 0;
}

u8 flash_read8(void *addr)
{
	return __raw_readb(addr + 1);
}

void flash_write16(u16 val, void *addr)
{
	u16 shftval = (((val >> 8) & 0xff) | ((val << 8) & 0xff00));

	__raw_writew(shftval, addr);
}

u16 flash_read16(void *addr)
{
	u16 val = __raw_readw(addr);

	return (((val) >> 8) & 0x00ff) | (((val) << 8) & 0xff00);
}

#endif
