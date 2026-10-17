// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 */
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/smp.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int spl_board_init_f(void)
{
	return 0;
}

__weak void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed: %d\n", ret);

	riscv_cpu_setup();

	preloader_console_init();

	ret = spl_board_init_f();
	if (ret)
		panic("spl_board_init_f() failed: %d\n", ret);
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_riscv_t)(ulong hart, void *dtb);
	void *fdt_blob;
	__maybe_unused int ret;

#if CONFIG_IS_ENABLED(LOAD_FIT) || CONFIG_IS_ENABLED(LOAD_FIT_FULL)
	fdt_blob = spl_image->fdt_addr;
#else
	fdt_blob = (void *)gd->fdt_blob;
#endif

	image_entry_riscv_t image_entry =
		(image_entry_riscv_t)spl_image->entry_point;
	invalidate_icache_all();

	debug("image entry point: 0x%lX\n", spl_image->entry_point);
#ifdef CONFIG_SPL_SMP
	ret = smp_call_function(spl_image->entry_point, (ulong)fdt_blob, 0, 0);
	if (ret)
		hang();
#endif
	image_entry(gd->arch.boot_hart, fdt_blob);
}
