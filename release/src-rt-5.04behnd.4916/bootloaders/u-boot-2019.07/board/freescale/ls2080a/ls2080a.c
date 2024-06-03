// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor
 */
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <fsl-mc/fsl_mc.h>
#include <environment.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	init_final_memctl_regs();

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();
	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	if (soc_has_dp_ddr() && gd->bd->bi_dram[2].size) {
		puts("\nDP-DDR ");
		print_size(gd->bd->bi_dram[2].size, "");
		print_ddr_info(CONFIG_DP_DDR_CTRL);
	}
#endif
}

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int error = 0;

#ifdef CONFIG_SMC91111
	error = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	error = cpu_eth_init(bis);
#endif
	return error;
}

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/soc/fsl-mc");

	/*
	 * TODO: Remove this when backward compatibility
	 * with old DT node (/fsl-mc) is no longer needed.
	 */
	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl-mc");

	if (offset < 0) {
		printf("%s: ERROR: fsl-mc node not found in device tree (error %d)\n",
		       __func__, offset);
		return;
	}

	if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0))
		fdt_status_okay(fdt, offset);
	else
		fdt_status_fail(fdt, offset);
}

void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}
#endif

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

	fdt_fsl_mc_fixup_iommu_map_entry(blob);

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}
#endif

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
}
#endif

#ifdef CONFIG_TFABOOT
void *env_sf_get_env_addr(void)
{
	return (void *)(CONFIG_SYS_FSL_QSPI_BASE1 + CONFIG_ENV_OFFSET);
}
#endif
