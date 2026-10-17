// SPDX-License-Identifier: BSD-3-Clause
/*
 * Reference to the ARM TF Project,
 * plat/arm/common/arm_bl2_setup.c
 * Portions copyright (c) 2013-2016, ARM Limited and Contributors. All rights
 * reserved.
 * Copyright (C) 2016 Rockchip Electronic Co.,Ltd
 * Written by Kever Yang <kever.yang@rock-chips.com>
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <atf_common.h>
#include <cpu_func.h>
#include <errno.h>
#include <image.h>
#include <log.h>
#include <spl.h>
#include <asm/cache.h>
#include <asm-generic/global_data.h>

DECLARE_GLOBAL_DATA_PTR;
void dcache_sanitize_disable(uintptr_t sanitize_base_addr, unsigned long sanitize_size);

/* Holds all the structures we need for bl31 parameter passing */
struct bl2_to_bl31_params_mem {
	struct bl31_params bl31_params;
	struct atf_image_info bl31_image_info;
	struct atf_image_info bl32_image_info;
	struct atf_image_info bl33_image_info;
	struct entry_point_info bl33_ep_info;
	struct entry_point_info bl32_ep_info;
	struct entry_point_info bl31_ep_info;
};

struct bl2_to_bl31_params_mem_v2 {
	struct bl_params bl_params;
	struct bl_params_node bl31_params_node;
	struct bl_params_node bl32_params_node;
	struct bl_params_node bl33_params_node;
	struct atf_image_info bl31_image_info;
	struct atf_image_info bl32_image_info;
	struct atf_image_info bl33_image_info;
	struct entry_point_info bl33_ep_info;
	struct entry_point_info bl32_ep_info;
	struct entry_point_info bl31_ep_info;
};

struct bl31_params *bl2_plat_get_bl31_params_default(uintptr_t bl32_entry,
						     uintptr_t bl33_entry,
						     uintptr_t fdt_addr)
{
	static struct bl2_to_bl31_params_mem bl31_params_mem;
	struct bl31_params *bl2_to_bl31_params;
	struct entry_point_info *bl32_ep_info;
	struct entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, sizeof(struct bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

	/* Fill BL32 related information */
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_HEAD(bl32_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->args.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl32_ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				     DISABLE_ALL_EXECPTIONS);

	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

	/* Fill BL33 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_HEAD(bl33_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->args.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_entry;
	bl33_ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
				     DISABLE_ALL_EXECPTIONS);

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

	return bl2_to_bl31_params;
}

__weak struct bl31_params *bl2_plat_get_bl31_params(uintptr_t bl32_entry,
						    uintptr_t bl33_entry,
						    uintptr_t fdt_addr)
{
	return bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

struct bl_params *bl2_plat_get_bl31_params_v2_default(uintptr_t bl32_entry,
						      uintptr_t bl33_entry,
						      uintptr_t fdt_addr)
{
	static struct bl2_to_bl31_params_mem_v2 bl31_params_mem;
	struct bl_params *bl_params;
	struct bl_params_node *bl_params_node;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, sizeof(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_node;

	/* Fill BL31 related information */
	bl_params_node = &bl31_params_mem.bl31_params_node;
	bl_params_node->image_id = ATF_BL31_IMAGE_ID;
	bl_params_node->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_node->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_node->next_params_info = &bl31_params_mem.bl32_params_node;
	SET_PARAM_HEAD(bl_params_node->image_info, ATF_PARAM_IMAGE_BINARY,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_node = &bl31_params_mem.bl32_params_node;
	bl_params_node->image_id = ATF_BL32_IMAGE_ID;
	bl_params_node->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_node->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_node->next_params_info = &bl31_params_mem.bl33_params_node;
	SET_PARAM_HEAD(bl_params_node->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_node->ep_info->args.arg3 = fdt_addr;
	bl_params_node->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_node->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXECPTIONS);
	SET_PARAM_HEAD(bl_params_node->image_info, ATF_PARAM_IMAGE_BINARY,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_node = &bl31_params_mem.bl33_params_node;
	bl_params_node->image_id = ATF_BL33_IMAGE_ID;
	bl_params_node->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_node->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_node->next_params_info = NULL;
	SET_PARAM_HEAD(bl_params_node->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_node->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_node->ep_info->pc = bl33_entry;
	bl_params_node->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXECPTIONS);
	SET_PARAM_HEAD(bl_params_node->image_info, ATF_PARAM_IMAGE_BINARY,
		       ATF_VERSION_2, 0);

	return bl_params;
}

__weak struct bl_params *bl2_plat_get_bl31_params_v2(uintptr_t bl32_entry,
						     uintptr_t bl33_entry,
						     uintptr_t fdt_addr)
{
	return bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

static inline void raw_write_daif(unsigned int daif)
{
#ifdef CONFIG_ARM64
	__asm__ __volatile__("msr DAIF, %0\n\t" : : "r" (daif) : "memory");
#endif
}

typedef void __noreturn (*atf_entry_t)(struct bl31_params *params, void *plat_params);

static void __noreturn bl31_entry(uintptr_t bl31_entry, uintptr_t bl32_entry,
				  uintptr_t bl33_entry, uintptr_t fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	void *bl31_params;

	if (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	else
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);

	/* Sanitize the heap and dcache, disable dcache before ATF handoff */ 
	dcache_sanitize_disable(gd->malloc_base, gd->malloc_limit);

	/* WARNING: NO HEAP ACCESS AFTER THIS POINT */

	atf_entry(bl31_params, (void *)fdt_addr);
}

static int spl_fit_images_find(void *blob, int os)
{
	int parent, node, ndepth = 0;
	const void *data;

	if (!blob)
		return -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	if (parent < 0)
		return -FDT_ERR_NOTFOUND;

	for (node = fdt_next_node(blob, parent, &ndepth);
	     (node >= 0) && (ndepth > 0);
	     node = fdt_next_node(blob, node, &ndepth)) {
		if (ndepth != 1)
			continue;

		data = fdt_getprop(blob, node, FIT_OS_PROP, NULL);
		if (!data)
			continue;

		if (genimg_get_os_id(data) == os)
			return node;
	};

	return -FDT_ERR_NOTFOUND;
}

uintptr_t spl_fit_images_get_entry(void *blob, int node)
{
	ulong  val;
	int ret;

	ret = fit_image_get_entry(blob, node, &val);
	if (ret)
		ret = fit_image_get_load(blob, node, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	return val;
}

void __noreturn spl_invoke_atf(struct spl_image_info *spl_image)
{
	uintptr_t  bl32_entry = 0;
	uintptr_t  bl33_entry = CONFIG_TEXT_BASE;
	void *blob = spl_image->fdt_addr;
	uintptr_t platform_param = (uintptr_t)blob;
	int node;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	node = spl_fit_images_find(blob, IH_OS_TEE);
	if (node >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, node);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	node = spl_fit_images_find(blob, IH_OS_U_BOOT);
	if (node >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, node);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	if (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
