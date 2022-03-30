/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
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

#define MAP_BL_SP_MIN_TOTAL	MAP_REGION_FLAT(			\
					BL32_BASE,			\
					BL32_END - BL32_BASE,		\
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


#define BL32_END     (uintptr_t)(&__BL32_END__)
#define DEVICE_BASE  0x80000000
#define DEVICE_SIZE  0x00010000


#define DESC_DOMAIN(x)          ((x << 5) & 0x000001E0)
// section descriptor definitions
#define SECTION_AP              0xc00
#define SECTION_XN              0x10
#define SECTION_PXN             0x1
/* A9 does not support PXN */
#define SECTION_XN_ALL          (SECTION_XN)
#define SECTION_SHAREABLE       (1 << 16)
#define SECTION_SUPER_DESC      (1 << 18)
#define SECTION_DESC_NS         (1 << 19) 
// TEX[2] = 1
#define SECTION_OUTER_NC_INNER_WBWA         0x00004006
#define SECTION_OUTER_WBNWA_INNER_WBWA      0x00007006
#define SECTION_OUTER_WTNWA_INNER_WBWA      0x00006006
#define SECTION_OUTER_WBWA_INNER_NC         0x00005002
// TEX[2] = 0, OUTER & INNER are same all the time
#define SECTION_OUTER_WBWA_INNER_WBWA       0x0000100E
#define SECTION_OUTER_NSD_INNER_NSD         0x00002002
#define SECTION_OUTER_NC_INNER_NC           0x00001002
#define SECTION_OUTER_WTNWA_INNER_WTNWA     0x0000000A
#define SECTION_OUTER_WBNWA_INNER_WBNWA     0x0000000E
#define SECTION_OUTER_SO_INNER_SO           0x00000002
#define SECTION_OUTER_SD_INNER_SD           0x00000006

// definition for common section attribute 
#define SECTION_ATTR_INVALID       0x0  
#define SECTION_ATTR_CACHED_MEM    \
	(SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0)|SECTION_SHAREABLE)
#define SECTION_ATTR_NONCACHED_MEM \
	(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
#define SECTION_ATTR_DEVICE        \
	(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|SECTION_XN_ALL|DESC_DOMAIN(0))
#define SECTION_ATTR_DEVICE_EXEC   \
	(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))

#define SCU_INV_CTRL_INIT              0xFFFF
#define SCU_SAC_CTRL_INIT              0xF
#define SCU_NSAC_CTRL_INIT             0xFFF
#define SCU_INV_SEC                    0x0C
#define SCU_SAC                        0x50
#define SCU_NSAC                       0x54
#define SCU_CTRL                       0x0

extern void a9_mmu_invalidate_tlb(void);
extern void a9_mmu_enable(void);
extern void a9_l1cache_enable_i(void);
extern void a9_l1cache_enable_d(void);
extern void a9_mmu_set_ttbr(char* table_base);
extern void a9_gic_secure_init(void);
extern void arm_cl2_invbyway(uint32_t);
/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
static uintptr_t ns_entry_point;

#if defined (PLATFORM_FLAVOR_63138)
char mmu_table[1024*16] __attribute__((aligned(4096*4)));
#endif

const mmap_region_t plat_arm_mmap[] = {
  MAP_REGION_FLAT(DEVICE_BASE,  DEVICE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
  MAP_REGION_FLAT(GICD_BASE,    DEVICE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#if UART0_BASE
  MAP_REGION_FLAT(UART0_BASE,   UART0_SIZE,  MT_DEVICE | MT_RW | MT_SECURE),
#endif
#if defined(BIUCFG_BASE)
  MAP_REGION_FLAT(BIUCFG_BASE,  BIUCFG_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#elif defined(BIUCTRL_BASE)
  MAP_REGION_FLAT(BIUCTRL_BASE, BIUCTRL_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#endif
#if defined(BOOTLUT_BASE)
  MAP_REGION_FLAT(BOOTLUT_BASE, BOOTLUT_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#endif
#if defined(PL310_BASE)
  MAP_REGION_FLAT(PL310_BASE, PL310_MAP_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#endif
#if defined(B15_CTRL_BASE)
  MAP_REGION_FLAT(B15_CTRL_BASE, B15_CTRL_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#endif
#if defined(TIMR_BASE)
  MAP_REGION_FLAT(TIMR_BASE, TIMR_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#endif
  MAP_REGION_FLAT(PMC_BASE,     PMC_SIZE,    MT_DEVICE | MT_RW | MT_SECURE),
  {0}
};


#if defined(PL310_BASE)
void a9_cl2_config(void)
{
	/* Disable PL310 */
	uint32_t  pl310_base = PL310_BASE;
	mmio_write_32(pl310_base + PL310_CTRL, 0);
	mmio_write_32(pl310_base + PL310_DATA_RAM_CTRL, PL310_DATA_RAM_CTRL_INIT);
	mmio_write_32(pl310_base + PL310_AUX_CTRL, PL310_AUX_CTRL_INIT);

	/* invalidate all cache ways */
	arm_cl2_invbyway(pl310_base);
}
#endif


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

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info;

	next_image_info = &bl33_image_ep_info;

	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}


void arm_sp_min_early_platform_setup ( void *plat_params, uintptr_t tos_fw_config,
                                       uintptr_t hw_config, void *plat_params_from_bl2 )
{
	bl31_params_t *from_bl2 = (bl31_params_t*)plat_params;

	if (plat_my_core_pos() != 0){
		plat_panic_handler();
	}

#if defined(SCU_BASE)
	mmio_write_32(SCU_BASE + SCU_INV_SEC, SCU_INV_CTRL_INIT);
	mmio_write_32(SCU_BASE + SCU_SAC, SCU_SAC_CTRL_INIT);
	mmio_write_32(SCU_BASE + SCU_NSAC, SCU_NSAC_CTRL_INIT);
	/* SCU enable */
	mmio_write_32(SCU_BASE + SCU_CTRL, 0x1);
#endif

#if defined(PL310_BASE)
    a9_cl2_config();
#endif

	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL322_BASE;
	bl32_image_ep_info.spsr = MODE32_svc << MODE32_SHIFT;
	/* Provide boot parameters to OPTEE */
	memset(&bl32_image_ep_info.args, 0, sizeof(aapcs32_params_t));
	bl32_image_ep_info.args.arg0 = (u_register_t)plat_params_from_bl2;

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);

	/* Configure ns entry point */
	plat_set_ns_image_entrypoint(from_bl2->bl33_ep_info->pc);

	/*
	 * Tell SP_MIN where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry() & ~(MODE32_MASK << MODE32_SHIFT);
	/* Always enter in non-secure SVC mode */
	bl33_image_ep_info.spsr |= MODE32_svc << MODE32_SHIFT;
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}

void sp_min_early_platform_setup(void *from_bl2,
		void *plat_params_from_bl2)
{
	arm_sp_min_early_platform_setup(from_bl2, 0, 0, plat_params_from_bl2);
}


void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	sp_min_early_platform_setup((void *)arg0, (void *)arg1);
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

const struct gicv2_driver_data plat_gicv2_driver_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
	.interrupt_props = irq_sec_array,
	.interrupt_props_num = ARRAY_SIZE(irq_sec_array),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/*******************************************************************************
 * Perform any BL32 platform setup common to ARM standard platforms
 ******************************************************************************/
void arm_bl32_platform_setup(void)
{
#if defined (PLATFORM_FLAVOR_63138)
	a9_gic_secure_init();
#else
	/* Initialize the gic cpu and distributor interfaces */
	gicv2_driver_init(&plat_gicv2_driver_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
#endif
}

/*******************************************************************************
 * Perform any BL32 platform runtime setup prior to ARM standard platforms
 ******************************************************************************/
void arm_bl32_plat_runtime_setup(void)
{
	/* Initialize the runtime functions */
}

void sp_min_platform_setup(void)
{
	arm_bl32_platform_setup();
}

void sp_min_runtime_setup(void)
{
	arm_bl32_plat_runtime_setup();
}

#if defined (PLATFORM_FLAVOR_63138)
void a9_mmu_setup_region(uint32_t mem_addr, uint32_t mem_size, uint32_t mem_attr)
{
#define size_mb(s) ((s + 0xFFFFF) / 0x100000)
	register uint32_t table_base = (uint32_t)mmu_table;
	register uint32_t table_size = sizeof(mmu_table);

	/* Align the address to MB and then adjust the size */
	mem_size += mem_addr & 0xFFFFF;
	mem_addr &= ~0xFFFFF;
	/* Convert the mem size in MB */
	mem_size = size_mb(mem_size);

	__asm__ volatile ("mov r0, %0" : : "r" (table_size) : "r0");
	__asm__ volatile ("push {r0}");
	__asm__ volatile ("mov r0, %0" : : "r" (mem_addr)   : "r0");
	__asm__ volatile ("push {r0}");
	__asm__ volatile ("mov r0, %0" : : "r" (mem_attr)   : "r0");
	__asm__ volatile ("push {r0}");
	__asm__ volatile ("mov r0, %0" : : "r" (mem_size)   : "r0");
	__asm__ volatile ("push {r0}");
	__asm__ volatile ("mov r0, %0" : : "r" (mem_addr)   : "r0");
	__asm__ volatile ("push {r0}");
	__asm__ volatile ("mov r0, %0" : : "r" (table_base) : "r0");
	__asm__ volatile ("push {r0}");
	__asm__ volatile ("pop {r0-r4,r6}");

	__asm__ volatile ("bl a9_mmu_set_scn" ::: "lr");
}

void a9_bl32_plat_map_mmu(void)
{
	int i = 0;
	int region_cnt = sizeof(plat_arm_mmap) / sizeof(mmap_region_t);
    /* Initialize the mmu table */
	memset(mmu_table, 0, sizeof(mmu_table));
    /* Map the code/data section of ATF */
	a9_mmu_setup_region(BL32_BASE, BL32_END - BL32_BASE, SECTION_ATTR_CACHED_MEM);
	for (i = 0; i <  region_cnt - 1; i++ ){
		a9_mmu_setup_region((uint32_t)plat_arm_mmap[i].base_pa,  plat_arm_mmap[i].size, SECTION_ATTR_DEVICE);
	}
}

void a9_bl32_plat_enable_mmu(void)
{
	a9_mmu_invalidate_tlb();
	a9_mmu_set_ttbr (mmu_table);
	a9_mmu_enable();
	a9_l1cache_enable_i();
	a9_l1cache_enable_d();
	if (plat_my_core_pos() != 0){
		mmio_write_32(PL310_BASE + PL310_CTRL, 1);
	}
}
#endif

/*******************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 ******************************************************************************/
void arm_bl32_plat_arch_setup(void)
{
#if defined (PLATFORM_FLAVOR_63138)
	a9_bl32_plat_map_mmu();
	a9_bl32_plat_enable_mmu();
#else
	const mmap_region_t bl_regions[] = {
		MAP_BL_SP_MIN_TOTAL,
		ARM_MAP_BL_RO,
#if USE_COHERENT_MEM
		ARM_MAP_BL_COHERENT_RAM,
#endif
		{0}
	};

	generic_delay_timer_init();

	setup_page_tables(bl_regions, plat_arm_get_mmap());

	enable_mmu_svc_mon(0);
#endif

#if defined(BIUCFG_BASE) && !(defined (PLATFORM_FLAVOR_63138) || defined (PLATFORM_FLAVOR_63148))
	/* Enable BUS access control for non-secure Linux */
	*(unsigned int*)(BIUCFG_BASE + 0x300) = 0xFF;
#endif
}

void sp_min_plat_arch_setup(void)
{
	arm_bl32_plat_arch_setup();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

int plat_bcm_calc_core_pos(int mpidr)
{
	return plat_arm_calc_core_pos(mpidr);
}
