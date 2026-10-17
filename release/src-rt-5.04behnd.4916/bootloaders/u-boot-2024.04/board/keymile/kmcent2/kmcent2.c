// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Keymile AG
 * Rainer Boschung <rainer.boschung@keymile.com>
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <event.h>
#include <asm/cache.h>
#include <asm/fsl_fdt.h>
#include <asm/fsl_law.h>
#include <asm/fsl_liodn.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_serdes.h>
#include <asm/immap_85xx.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <fdt_support.h>
#include <fm_eth.h>
#include <hwconfig.h>
#include <image.h>
#include <linux/compiler.h>
#include <net.h>
#include <netdev.h>
#include <vsc9953.h>

#include "../common/common.h"
#include "../common/qrio.h"

DECLARE_GLOBAL_DATA_PTR;

static uchar ivm_content[CONFIG_SYS_IVM_EEPROM_MAX_LEN];

int checkboard(void)
{
	printf("Board: Hitachi Power Grids kmcent2\n");

	return 0;
}

#define RSTRQSR1_WDT_RR	0x00200000
#define RSTRQSR1_SW_RR	0x00100000

int board_early_init_f(void)
{
	struct fsl_ifc ifc = {(void *)CFG_SYS_IFC_ADDR, (void *)NULL};
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	bool cpuwd_flag = false;

	/* board specific IFC configuration: increased bus turnaround time */
	setbits_be32(&ifc.gregs->ifc_gcr, 8 << IFC_GCR_TBCTL_TRN_TIME_SHIFT);

	/* configure mode for uP reset request */
	qrio_uprstreq(UPREQ_CORE_RST);

	/* board only uses the DDR_MCK0, so disable the DDR_MCK1 */
	setbits_be32(&gur->ddrclkdr, 0x40000000);

	/* set reset reason according CPU register */
	if ((gur->rstrqsr1 & (RSTRQSR1_WDT_RR | RSTRQSR1_SW_RR)) ==
	    RSTRQSR1_WDT_RR)
		cpuwd_flag = true;

	qrio_cpuwd_flag(cpuwd_flag);
	/* clear CPU bits by writing 1 */
	setbits_be32(&gur->rstrqsr1, RSTRQSR1_WDT_RR | RSTRQSR1_SW_RR);

	/* configure PRST lines for the application: */
	/*
	 * ETHSW_DDR_RST:
	 * reset at power-up and unit reset only and enable WD on it
	 */
	qrio_prstcfg(KM_ETHSW_DDR_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_ETHSW_DDR_RST, true);
	/*
	 * XES_PHY_RST:
	 * reset at power-up and unit reset only and enable WD on it
	 */
	qrio_prstcfg(KM_XES_PHY_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_XES_PHY_RST, true);
	/*
	 * ES_PHY_RST:
	 * reset at power-up and unit reset only and enable WD on it
	 */
	qrio_prstcfg(KM_ES_PHY_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_ES_PHY_RST, true);
	/*
	 * EFE_RST:
	 * reset at power-up and unit reset only and enable WD on it
	 */
	qrio_prstcfg(KM_EFE_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_EFE_RST, true);
	/*
	 * BFTIC4_RST:
	 * reset at power-up and unit reset only and enable WD on it
	 */
	qrio_prstcfg(KM_BFTIC4_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_BFTIC4_RST, true);
	/*
	 * DPAXE_RST:
	 * reset at power-up and unit reset only and enable WD on it
	 */
	qrio_prstcfg(KM_DPAXE_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_DPAXE_RST, true);
	/*
	 * PEXSW_RST:
	 * reset at power-up and unit reset only, deassert reset w/o WD
	 */
	qrio_prstcfg(KM_PEXSW_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_prst(KM_PEXSW_RST, false, false);
	/*
	 * PEXSW_NT_RST:
	 * reset at power-up and unit reset only, deassert reset w/o WD
	 */
	qrio_prstcfg(KM_PEXSW_NT_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_prst(KM_PEXSW_NT_RST, false, false);
	/*
	 * BOBCAT_RST:
	 * reset at power-up and unit reset only, deassert reset w/o WD
	 */
	qrio_prstcfg(KM_BOBCAT_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_prst(KM_BOBCAT_RST, false, false);
	/*
	 * FEMT_RST:
	 * reset at power-up and unit reset only and enable WD
	 */
	qrio_prstcfg(KM_FEMT_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_FEMT_RST, true);
	/*
	 * FOAM_RST:
	 * reset at power-up and unit reset only and enable WD
	 */
	qrio_prstcfg(KM_FOAM_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_FOAM_RST, true);

	return 0;
}

int board_early_init_r(void)
{
	int ret = 0;

	const unsigned int flashbase = CFG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CFG_SYS_FLASH_BASE_PHYS,
		MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);

	set_liodns();
	setup_qbman_portals();

	qrio_set_leds();

	/* enable Application Buffer */
	qrio_enable_app_buffer();

	return ret;
}

unsigned long get_serial_clock(unsigned long dummy)
{
	return (gd->bus_clk / 2);
}

static int kmcent2_misc_init_f(void)
{
	/* configure QRIO pis for i2c deblocking */
	i2c_deblock_gpio_cfg();

	/*
	 * CFE_RST (front phy):
	 * reset at power-up, unit and core reset, deasset reset w/o WD
	 */
	qrio_prstcfg(KM_CFE_RST, PRSTCFG_POWUP_UNIT_CORE_RST);
	qrio_prst(KM_CFE_RST, false, false);

	/*
	 * ZL30158_RST (PTP clock generator):
	 * reset at power-up only, deassert reset and enable WD on it
	 */
	qrio_prstcfg(KM_ZL30158_RST, PRSTCFG_POWUP_RST);
	qrio_prst(KM_ZL30158_RST, false, false);

	/*
	 * ZL30364_RST (EEC generator):
	 * reset at power-up only, deassert reset and enable WD on it
	 */
	qrio_prstcfg(KM_ZL30364_RST, PRSTCFG_POWUP_RST);
	qrio_prst(KM_ZL30364_RST, false, false);

	return 0;
}
EVENT_SPY_SIMPLE(EVT_MISC_INIT_F, kmcent2_misc_init_f);

#define USED_SRDS_BANK 0
#define EXPECTED_SRDS_RFCK SRDS_PLLCR0_RFCK_SEL_100

#define BRG01_IOCLK12	0x02000000
#define EC2_GTX_CLK125	0x08000000

int misc_init_r(void)
{
	serdes_corenet_t *regs = (void *)CFG_SYS_FSL_CORENET_SERDES_ADDR;
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CFG_SYS_MPC85xx_SCFG;
	ccsr_gur_t __iomem *gur = (ccsr_gur_t __iomem *)CFG_SYS_MPC85xx_GUTS_ADDR;

	/* check SERDES bank 0 reference clock */
	u32 actual = in_be32(&regs->bank[USED_SRDS_BANK].pllcr0);

	if (actual & SRDS_PLLCR0_POFF)
		printf("Warning: SERDES bank %u pll is off\n", USED_SRDS_BANK);
	if ((actual & SRDS_PLLCR0_RFCK_SEL_MASK) != EXPECTED_SRDS_RFCK) {
		printf("Warning: SERDES bank %u expects %sMHz clock, is %sMHz\n",
		       USED_SRDS_BANK,
		       serdes_clock_to_string(EXPECTED_SRDS_RFCK),
		       serdes_clock_to_string(actual));
	}

	/* QE IO clk : BRG01 is used over clk12 for HDLC clk (20 MhZ) */
	out_be32(&scfg->qeioclkcr,
		 in_be32(&scfg->qeioclkcr) | BRG01_IOCLK12);

	ivm_read_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN,
			CONFIG_PIGGY_MAC_ADDRESS_OFFSET);

	/* Fix polarity of Card Detect and Write Protect */
	out_be32(&gur->sdhcpcr, 0xFFFFFFFF);

	/*
	 * EC1 is disabled in our design, so we must explicitly set GTXCLKSEL
	 * to EC2
	 */
	out_be32(&scfg->emiiocr, in_be32(&scfg->emiiocr) | EC2_GTX_CLK125);

	return 0;
}

int hush_init_var(void)
{
	ivm_analyze_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}

static int last_stage_init(void)
{
	const char *kmem;
	/* DIP switch support on BFTIC */
	struct bfticu_iomap *bftic4 =
		(struct bfticu_iomap *)SYS_BFTIC_BASE;
	u8 dip_switch = in_8((u8 *)&bftic4->mswitch) & BFTICU_DIPSWITCH_MASK;

	if (dip_switch != 0) {
		/* start bootloader */
		puts("DIP:   Enabled\n");
		env_set("actual_bank", "0");
	}

	set_km_env();

	/*
	 * bootm_size is used to fixup the FDT memory node
	 * set it to kernelmem that has the same value
	 */
	kmem = env_get("kernelmem");
	if (kmem)
		env_set("bootm_size", kmem);

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

void fdt_fixup_fman_mac_addresses(void *blob)
{
	int node, ret;
	char path[25];
	unsigned char mac_addr[6];

	/*
	 * Just the fm1-mac5 must be set by us, u-boot handle the 2 others,
	 * get the mac addr from env
	 */
	if (!eth_env_get_enetaddr_by_index("eth", 4, mac_addr)) {
		printf("eth4addr env variable not defined\n");
		return;
	}

	/* local management port */
	strcpy(path, "/soc/fman/ethernet@e8000");
	node = fdt_path_offset(blob, path);
	if (node < 0) {
		printf("no %s\n", path);
		return;
	}

	ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
	if (ret) {
		printf("%s\n\terror setting local-mac-address property\n",
		       path);
	}
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	fdt_fixup_liodn(blob);

	fdt_fixup_fman_mac_addresses(blob);

	if (hwconfig("qe-tdm"))
		fdt_del_diu(blob);
	return 0;
}

/* DIC26_SELFTEST GPIO used to start factory test sw */
#define SELFTEST_PORT	QRIO_GPIO_A
#define SELFTEST_PIN	0

int post_hotkeys_pressed(void)
{
	qrio_gpio_direction_input(SELFTEST_PORT, SELFTEST_PIN);
	return qrio_get_gpio(SELFTEST_PORT, SELFTEST_PIN);
}
