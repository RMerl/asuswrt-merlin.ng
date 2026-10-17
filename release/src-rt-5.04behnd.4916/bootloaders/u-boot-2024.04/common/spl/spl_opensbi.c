// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#include <common.h>
#include <cpu_func.h>
#include <errno.h>
#include <hang.h>
#include <image.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/smp.h>
#include <opensbi.h>
#include <linux/libfdt.h>
#include <linux/printk.h>
#include <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

struct fw_dynamic_info opensbi_info;

static int spl_opensbi_find_os_node(void *blob, int *uboot_node, int os_type)
{
	int fit_images_node, node;
	const char *fit_os;

	fit_images_node = fdt_path_offset(blob, "/fit-images");
	if (fit_images_node < 0)
		return -ENODEV;

	fdt_for_each_subnode(node, blob, fit_images_node) {
		fit_os = fdt_getprop(blob, node, FIT_OS_PROP, NULL);
		if (!fit_os)
			continue;

		if (genimg_get_os_id(fit_os) == os_type) {
			*uboot_node = node;
			return 0;
		}
	}

	return -ENODEV;
}

void __noreturn spl_invoke_opensbi(struct spl_image_info *spl_image)
{
	int ret, os_node;
	ulong os_entry;
	int os_type;
	typedef void __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	if (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	/*
	 * Originally, u-boot-spl will place DTB directly after the kernel,
	 * but the size of the kernel did not include the BSS section, which
	 * means u-boot-spl will place the DTB in the kernel BSS section
	 * causing the DTB to be cleared by kernel BSS initializtion.
	 * Moving DTB in front of the kernel can avoid the error.
	 */
#if CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	memcpy((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	if (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	else
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_node(spl_image->fdt_addr, &os_node, os_type);
	if (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_node, &os_entry);
	if (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_node, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	opensbi_info.version = FW_DYNAMIC_INFO_VERSION;
	opensbi_info.next_addr = os_entry;
	opensbi_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	opensbi_info.options = CONFIG_SPL_OPENSBI_SCRATCH_OPTIONS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_entry = (opensbi_entry_t)spl_image->entry_point;
	invalidate_icache_all();

#ifdef CONFIG_SPL_SMP
	/*
	 * Start OpenSBI on all secondary harts and wait for acknowledgment.
	 *
	 * OpenSBI first relocates itself to its link address. This is done by
	 * the main hart. To make sure no hart is still running U-Boot SPL
	 * during relocation, we wait for all secondary harts to acknowledge
	 * the call-function request before entering OpenSBI on the main hart.
	 * Otherwise, code corruption can occur if the link address ranges of
	 * U-Boot SPL and OpenSBI overlap.
	 */
	ret = smp_call_function((ulong)spl_image->entry_point,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	if (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
