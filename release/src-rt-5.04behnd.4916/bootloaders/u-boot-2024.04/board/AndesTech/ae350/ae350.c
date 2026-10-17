// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <config.h>
#include <cpu_func.h>
#include <flash.h>
#include <image.h>
#include <init.h>
#include <net.h>
#if defined(CONFIG_FTMAC100) && !defined(CONFIG_DM_ETH)
#include <netdev.h>
#endif
#include <asm/csr.h>
#include <asm/global_data.h>
#include <asm/sbi.h>
#include <linux/io.h>
#include <faraday/ftsmc020.h>
#include <fdtdec.h>
#include <dm.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initializations
 */
#if IS_ENABLED(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
    long csr_marchid = 0;
    const long mask_64 = 0x8000;
    const long mask_cpu = 0xff;
    char cpu_name[10] = {};

#if CONFIG_IS_ENABLED(RISCV_SMODE)
    sbi_get_marchid(&csr_marchid);
#elif CONFIG_IS_ENABLED(RISCV_MMODE)
    csr_marchid = csr_read(CSR_MARCHID);
#endif
    if (mask_64 & csr_marchid)
        snprintf(cpu_name, sizeof(cpu_name), "ax%lx", (mask_cpu & csr_marchid));
    else
        snprintf(cpu_name, sizeof(cpu_name), "a%lx", (mask_cpu & csr_marchid));

    return env_set("cpu", cpu_name);
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_0 + 0x400;

	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#if defined(CONFIG_FTMAC100) && !defined(CONFIG_DM_ETH)
int board_eth_init(struct bd_info *bd)
{
	return ftmac100_initialize(bd);
}
#endif

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	return 0;
}

#define ANDES_HW_DTB_ADDRESS	0xF2000000
void *board_fdt_blob_setup(int *err)
{
	*err = 0;

	if (IS_ENABLED(CONFIG_OF_SEPARATE) || IS_ENABLED(CONFIG_OF_BOARD)) {
		if (fdt_magic((uintptr_t)gd->arch.firmware_fdt_addr) == FDT_MAGIC)
			return (void *)(ulong)gd->arch.firmware_fdt_addr;
	}

	if (fdt_magic(CONFIG_SYS_FDT_BASE) == FDT_MAGIC)
		return (void *)CONFIG_SYS_FDT_BASE;
	return (void *)ANDES_HW_DTB_ADDRESS;

	*err = -EINVAL;
	return NULL;
}

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init()
{
	/* enable v5l2 cache */
	if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		enable_caches();
}
#endif

int smc_init(void)
{
	int node = -1;
	const char *compat = "andestech,atfsmc020";
	void *blob = (void *)gd->fdt_blob;
	fdt_addr_t addr;
	struct ftsmc020_bank *regs;

	node = fdt_node_offset_by_compatible(blob, -1, compat);
	if (node < 0)
		return -FDT_ERR_NOTFOUND;

	addr = fdtdec_get_addr_size_auto_noparent(blob, node,
		"reg", 0, NULL, false);

	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	regs = (struct ftsmc020_bank *)(uintptr_t)addr;
	regs->cr &= ~FTSMC020_BANK_WPROT;

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	smc_init();

	return 0;
}
#endif

#ifdef CONFIG_SPL
void board_boot_order(u32 *spl_boot_list)
{
	u8 i;
	u32 boot_devices[] = {
#ifdef CONFIG_SPL_RAM_SUPPORT
		BOOT_DEVICE_RAM,
#endif
#ifdef CONFIG_SPL_MMC
		BOOT_DEVICE_MMC1,
#endif
	};

	for (i = 0; i < ARRAY_SIZE(boot_devices); i++)
		spl_boot_list[i] = boot_devices[i];
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* boot using first FIT config */
	return 0;
}
#endif
