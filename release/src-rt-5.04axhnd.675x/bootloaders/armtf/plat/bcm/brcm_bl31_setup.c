/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <plat_arm.h>
#include <platform.h>
#include <gicv2.h>
#include <string.h>
#include <generic_delay_timer.h>
#include <delay_timer.h>

#define BL31_END (uintptr_t)(&__BL31_END__)

#define MAP_BL31_TOTAL	MAP_REGION_FLAT(			\
					BL31_BASE,			\
					BL31_END - BL31_BASE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define ARM_MAP_BL_RO			MAP_REGION_FLAT(			\
						BL_CODE_BASE,			\
						BL_CODE_END - BL_CODE_BASE,	\
						MT_CODE | MT_SECURE)

#define ARM_MAP_BL_COHERENT_RAM		MAP_REGION_FLAT(			\
						BL_COHERENT_RAM_BASE,		\
						BL_COHERENT_RAM_END		\
							- BL_COHERENT_RAM_BASE, \
						MT_DEVICE | MT_RW | MT_SECURE)

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static uintptr_t ns_entry_point;

#define DEVICE_BASE  0x80000000
#define DEVICE_SIZE  0x00010000

const mmap_region_t plat_arm_mmap[] = {
  MAP_REGION_FLAT(DEVICE_BASE,  DEVICE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
  MAP_REGION_FLAT(GICD_BASE,    DEVICE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
  MAP_REGION_FLAT(UART0_BASE,   UART0_SIZE,  MT_DEVICE | MT_RW | MT_SECURE),
#if defined(BIUCFG_BASE)
  MAP_REGION_FLAT(BIUCFG_BASE,  BIUCFG_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#elif defined(BIUCTRL_BASE)
  MAP_REGION_FLAT(BIUCTRL_BASE, BIUCTRL_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#endif
  MAP_REGION_FLAT(BOOTLUT_BASE, BOOTLUT_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#if defined(WDTIMR0_BASE)
  MAP_REGION_FLAT(WDTIMR_BASE,  WDTIMR_SIZE,  MT_DEVICE | MT_RW | MT_SECURE),
#endif
  MAP_REGION_FLAT(PMC_BASE,     PMC_SIZE,    MT_DEVICE | MT_RW | MT_SECURE),
  {0}
};


/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;
	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Obtain ns entry point 
 ******************************************************************************/
uintptr_t plat_get_ns_image_entrypoint(void)
{
	return ns_entry_point;
}

/*******************************************************************************
 * Set ns-entry point
 ******************************************************************************/
void plat_set_ns_image_entrypoint(uintptr_t entry_point)
{
	ns_entry_point = entry_point;
}


struct atf_image_info {
	struct param_header h;
	uintptr_t image_base;   /* physical address of base of image */
	uint32_t image_size;    /* bytes read from image file */
};

struct atf_entry_point_info {
	struct param_header h;
	uintptr_t pc;
	uint32_t spsr;
	struct aapcs64_params args;
};

typedef struct bl31_params {
	param_header_t h;
	struct atf_image_info *bl31_image_info;
	struct atf_entry_point_info *bl32_ep_info;
	struct atf_image_info *bl32_image_info;
	struct atf_entry_point_info *bl33_ep_info;
	struct atf_image_info *bl33_image_info;
} bl31_params_t;

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & S-EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void arm_bl31_early_platform_setup(void *param0, uintptr_t plat_params_from_bl2, uintptr_t arg2, void * arg43)
{
	bl31_params_t *from_bl2 = (bl31_params_t *)param0;

#ifdef BL32_BASE
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = arm_get_spsr_for_bl32_entry();
	/* Provide boot parameters to OPTEE */
	memset(&bl32_image_ep_info.args, 0, sizeof(aapcs64_params_t));
	bl32_image_ep_info.args.arg0 = MODE_RW_64;
	bl32_image_ep_info.args.arg3 = (u_register_t)plat_params_from_bl2;
#endif /* BL32_BASE */

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);

	/* Configure ns entry point */
	plat_set_ns_image_entrypoint(from_bl2->bl33_ep_info->pc);

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();

	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/*
	* Provide Linux DTB address
	*/
	bl33_image_ep_info.args.arg0 = (u_register_t)from_bl2;
	bl33_image_ep_info.args.arg1 = (u_register_t)plat_get_ns_image_entrypoint();
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2,
                                u_register_t arg3)
{
	arm_bl31_early_platform_setup((void*)arg0, (uintptr_t)arg1, arg2, (void*)arg3);
}

#define PLATFORM_G1S_PROPS(grp)						\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_LEVEL)


static const interrupt_prop_t irq_sec_array[] = {
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

static const struct gicv2_driver_data plat_gicv2_driver_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
	.interrupt_props = irq_sec_array,
	.interrupt_props_num = ARRAY_SIZE(irq_sec_array),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void arm_bl31_platform_setup(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	gicv2_driver_init(&plat_gicv2_driver_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/*******************************************************************************
 * Perform any BL31 platform runtime setup prior to BL31 exit common to ARM
 * standard platforms
 ******************************************************************************/
void arm_bl31_plat_runtime_setup(void)
{
	/* Initialize the runtime functions */
}

void bl31_platform_setup(void)
{
	arm_bl31_platform_setup();
}

void bl31_plat_runtime_setup(void)
{
	arm_bl31_plat_runtime_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 ******************************************************************************/
void arm_bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
		ARM_MAP_BL_RO,
#if USE_COHERENT_MEM
		ARM_MAP_BL_COHERENT_RAM,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());

	generic_delay_timer_init();

	enable_mmu_el3(0);
}

void bl31_plat_arch_setup(void)
{
	arm_bl31_plat_arch_setup();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}
