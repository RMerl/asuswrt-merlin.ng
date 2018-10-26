#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:GPL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 * Generic board routine for Broadcom 963xx ARM boards
 */
#include <linux/types.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clkdev.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/clkdev.h>

#include <mach/hardware.h>
#include <mach/memory.h>
#include <mach/smp.h>

#include <plat/bsp.h>
#if defined(CONFIG_BCM963138)
#include <plat/ca9mpcore.h>
#elif defined(CONFIG_BCM963148)
#include <plat/b15core.h>
#endif
#include <bcm_map_part.h>
#include <board.h>
#include <bcm_extirq.h>
#include <asm/mach/map.h>
#include <linux/memblock.h>

#ifdef CONFIG_OF
extern int __init bcm_scan_fdt(void);
extern int __init bcm_reserve_memory(void);
extern int __init bcm_dt_postinit(void);
#endif

#define BOOT_ATAGS 0
#define BOOT_FDT 1

static int boot_param_status = BOOT_ATAGS;

#ifdef CONFIG_ATAGS

#define MB_ALIGNED(__val)	ALIGN(__val, 0x100000)

#if IS_ENABLED(CONFIG_BCM_ADSL)
#include "softdsl/AdslCoreDefs.h"
/* Reserve memory for DSL */
#define ADSL_SDRAM_RESERVE_SIZE		MB_ALIGNED(ADSL_SDRAM_IMAGE_SIZE)
#endif


#define SO_MEMORY_SIZE_BYTES SECTION_SIZE
#if IS_ENABLED(CONFIG_BCM_RDPA)
unsigned long tm_size = 0, mc_size = 0;
#endif

#if  IS_ENABLED(CONFIG_BCM_DHD_RUNNER)
unsigned long dhd_pool_size[DHD_RESERVE_MEM_NUM];
#endif

#ifdef CONFIG_BCM_CFE_XARGS_EARLY
extern void __init bl_xparms_setup(const unsigned char* blparms, unsigned int size);
#endif

extern void check_if_rootfs_is_set(char *cmdline);
extern unsigned long memsize;
extern bool is_rootfs_set;
/* Pointers to memory buffers allocated for the DSP module */
void *dsp_core;
void *dsp_init;
EXPORT_SYMBOL(dsp_core);
EXPORT_SYMBOL(dsp_init);

#endif /* CONFIG_ATAGS */

extern unsigned long getMemorySize(void);
extern bool is_memory_reserved;
extern int rsvd_mem_cnt;
extern reserve_mem_t reserve_mem[TOTAL_RESERVE_MEM_NUM];

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
static uint32_t preSOmemScratch;
static void *so_memory_virt_addr=&preSOmemScratch;
#endif

		
/*
*****************************************************************************
** FUNCTION:   allocDspModBuffers
**
** PURPOSE:    Allocates buffers for the init and core sections of the DSP
**             module. This module is special since it has to be allocated
**             in the 0x800.. memory range which is not mapped by the TLB.
**
** PARAMETERS: None
** RETURNS:    Nothing
*****************************************************************************
*/
void __init allocDspModBuffers(void)
{
}

#ifdef CONFIG_BCM_B15_MEGA_BARRIER
void BcmMegaBarrier(void) 
{
	__asm__("dsb");
	writel_relaxed(so_memory_virt_addr, so_memory_virt_addr);
	__asm__("dsb");
}
EXPORT_SYMBOL(BcmMegaBarrier);
#endif /*CONFIG_BCM_B15_MEGA_BARRIER*/

void __init board_map_io(void)
{
	struct map_desc desc[TOTAL_RESERVE_MEM_NUM+1];
	int i = 0;

	/* Map SoC specific I/O */
	soc_map_io();

	for(i = 0; i < rsvd_mem_cnt; i++ ) {
		desc[i].virtual = (unsigned long)phys_to_virt(reserve_mem[i].phys_addr);
		desc[i].pfn = __phys_to_pfn(reserve_mem[i].phys_addr);
		desc[i].length = reserve_mem[i].size;
#if defined (CONFIG_BCM_B15_MEGA_BARRIER)
		if (strcmp(B15_MEGA_BARRIER, reserve_mem[i].name)==0) {
			so_memory_virt_addr = (void*)desc[i].virtual;
			desc[i].type = MT_MEMORY_RW_SO;
		} else {
			desc[i].type = MT_MEMORY_RWX_NONCACHED;
		}	
#else
		desc[i].type = MT_MEMORY_RWX_NONCACHED;
#endif
		printk(KERN_INFO "creating a %s device at physical "
				"address of 0x%08lx to virtual address at "
				"0x%08lx with size of 0x%lx byte for %s\n",
				(desc[i].type == MT_MEMORY_RWX_NONCACHED)?"MT_MEMORY_NONCACHED":"MT_MEMORY", 
				(unsigned long)reserve_mem[i].phys_addr,
				desc[i].virtual, desc[i].length,
				reserve_mem[i].name);
	}

	if( i > 0 )
		iotable_init(desc, i);

	if (getMemorySize() <= SZ_32M)
		printk("WARNING! System is with 0x%0lx memory, might not "
				"boot successfully.\n"
				"\tcheck ATAG or CMDLINE\n", getMemorySize());

	soc_init_clock();
}


void __init board_init_early(void)
{
	soc_init_early();
}


void __init board_init_irq(void)
{
	soc_init_irq();

#if !defined (CONFIG_OF)
#ifdef CONFIG_CACHE_L2X0
	/* cache initialization */
	soc_l2_cache_init();
#endif
#endif
}

void __init board_init_timer(void)
{
	soc_init_timer();
}

static void __init dt_fixup(void)
{
	boot_param_status = BOOT_FDT;
}

static void __init bcm_setup(void)
{
#if !defined(CONFIG_BCM_KF_IKOS) || !defined(CONFIG_BRCM_IKOS)
	kerSysEarlyFlashInit();
	kerSysFlashInit();
	bcm_extirq_init();
#endif
}

void __init board_init_machine(void)
{
	/*
	 * Add common platform devices that do not have board dependent HW
	 * configurations
	 */
	soc_add_devices();

	bcm_setup();
#if defined (CONFIG_OF)
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
#endif

	return;
}

#ifdef CONFIG_ATAGS

static void __init set_memsize_from_cmdline(char *cmdline)
{
	char *cmd_ptr, *end_ptr;

	cmd_ptr = strstr(cmdline, "mem=");
	if (cmd_ptr != NULL) {
		cmd_ptr += 4;
		memsize = (unsigned long)memparse(cmd_ptr, &end_ptr);
	}
}

static void __init reserve_system_mem(char* name, unsigned long addr, unsigned long size)
{
	if (rsvd_mem_cnt >= TOTAL_RESERVE_MEM_NUM) {
		printk("reserved memory count %d reached the total memory reserve count %d!!!", rsvd_mem_cnt, TOTAL_RESERVE_MEM_NUM);
		return;
	}
	memblock_remove(addr, size);
	strcpy(reserve_mem[rsvd_mem_cnt].name, name);
	reserve_mem[rsvd_mem_cnt].size = size;
	reserve_mem[rsvd_mem_cnt].phys_addr = (uint32_t)addr;
	rsvd_mem_cnt++;

	return;
}


/* in ARM, there are two ways of passing in memory size.
 * one is by setting it in ATAG_MEM, and the other one is by setting the
 * size in CMDLINE.  The first appearance of mem=nn[KMG] in CMDLINE is the
 * value that has the highest priority. And if there is no memory size set
 * in CMDLINE, then it will use the value in ATAG_MEM.  If there is no ATAG
 * given from boot loader, then a default ATAG with memory size set to 16MB
 * will be taken effect.
 * Assuming CONFIG_CMDLINE_EXTEND is set. The logic doesn't work if
 * CONFIG_CMDLINE_FROM_BOOTLOADER is set. */
static void __init board_fixup(struct tag *t, char **cmdline)
{
	soc_fixup();
	/* obtaining info passing down from boot loader */
	for (; t->hdr.size; t = tag_next(t)) {
		if ((t->hdr.tag == ATAG_CORE) && (t->u.core.rootdev != 0xff))
			is_rootfs_set = true;

		if (t->hdr.tag == ATAG_MEM)
			memsize = t->u.mem.size;

#if IS_ENABLED(CONFIG_BCM_RDPA)
		if (t->hdr.tag == ATAG_RDPSIZE) {
			tm_size = t->u.rdpsize.tm_size * SZ_1M;
			mc_size = t->u.rdpsize.mc_size * SZ_1M;
		}
#endif

#if IS_ENABLED(CONFIG_BCM_DHD_RUNNER)
		dhd_pool_size[0] = 0;
		dhd_pool_size[1] = 0;
		dhd_pool_size[2] = 0;
		if (t->hdr.tag == ATAG_DHDSIZE) {
			/* if the kernel is still running on old cfe */
			if (t->u.dhdparm.dhd_size[0] != 0xff &&
				t->u.dhdparm.dhd_size[1] != 0xff &&
				t->u.dhdparm.dhd_size[2] != 0xff ) {
				dhd_pool_size[0] = t->u.dhdparm.dhd_size[0] * SZ_1M;
				dhd_pool_size[1] = t->u.dhdparm.dhd_size[1] * SZ_1M;
				dhd_pool_size[2] = t->u.dhdparm.dhd_size[2] * SZ_1M;
				}
		}
#endif

		if (t->hdr.tag == ATAG_BLPARM ){
			const unsigned char* bcm_blparms_buf = bcm_get_blparms();
			if (bcm_blparms_buf) {
				memcpy((unsigned char*)bcm_blparms_buf, t->u.blparm.blparm, bcm_get_blparms_size());
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
				bl_xparms_setup(bcm_blparms_buf, bcm_get_blparms_size());
#endif
			} else {
				printk(KERN_ERR "%s:%d Unable to get BCM blparms buffer\n",__func__,__LINE__);
			}
		}

		if (t->hdr.tag == ATAG_CMDLINE) {
			set_memsize_from_cmdline(t->u.cmdline.cmdline);
			check_if_rootfs_is_set(t->u.cmdline.cmdline);
		}
		if ((t->hdr.tag == ATAG_INITRD2) || (t->hdr.tag == ATAG_INITRD))
			is_rootfs_set = true;
	}
	set_memsize_from_cmdline(*cmdline);
	check_if_rootfs_is_set(*cmdline);
}

static void __init board_reserve_atag(void)
{
	/* used for reserve mem blocks */
	unsigned long mem_end = getMemorySize();
	unsigned long rsrv_mem_required = SZ_8M;
#if IS_ENABLED(CONFIG_BCM_DHD_RUNNER)
	int j = 0;
#endif
	rsvd_mem_cnt = 0;
	/* both reserved memory for RDP and DSL have to be within first
	 * 256MB */
	if (mem_end > SZ_256M)
		mem_end = SZ_256M;

#if IS_ENABLED(CONFIG_BCM_RDPA)
	/* Make sure the input values are larger than minimum required */
	if (tm_size < TM_DEF_DDR_SIZE)
		tm_size = TM_DEF_DDR_SIZE;

	if (mc_size < TM_MC_DEF_DDR_SIZE)
		mc_size = TM_MC_DEF_DDR_SIZE;

	/* both TM and MC reserved memory size has to be multiple of 2MB */
	if (tm_size & SZ_1M)
		tm_size += SZ_1M;
	if (mc_size & SZ_1M)
		mc_size += SZ_1M;

	rsrv_mem_required += tm_size + mc_size;
#endif

#if IS_ENABLED(CONFIG_BCM_DHD_RUNNER)
        /* Make sure the input values are larger than minimum required */
        rsrv_mem_required += dhd_pool_size[0];
        rsrv_mem_required += dhd_pool_size[1];
        rsrv_mem_required += dhd_pool_size[2];
#endif

#if IS_ENABLED(CONFIG_BCM_ADSL)
	rsrv_mem_required += ADSL_SDRAM_RESERVE_SIZE;
#endif

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
	rsrv_mem_required += SO_MEMORY_SIZE_BYTES;
#endif

#if IS_ENABLED(CONFIG_BCM_ADSL) || IS_ENABLED(CONFIG_BCM_RDPA) || IS_ENABLED(CONFIG_BCM_DHD_RUNNER) || defined(CONFIG_BCM_B15_MEGA_BARRIER)
	/* check if those configured memory sizes are over what
	 * system has */

	if (getMemorySize() < rsrv_mem_required) {

#if IS_ENABLED(CONFIG_BCM_DHD_RUNNER)
		rsrv_mem_required -= (dhd_pool_size[0] + dhd_pool_size[1] + dhd_pool_size[2]);
		dhd_pool_size[0] = dhd_pool_size[1] = dhd_pool_size[2] = 0x0;
#endif

#if IS_ENABLED(CONFIG_BCM_RDPA)
		/* If RDP is enabled, try to use the default
		 * TM and MC reserved memory size and try again */
		rsrv_mem_required -= tm_size + mc_size;
		tm_size = TM_DEF_DDR_SIZE;
		mc_size = TM_MC_DEF_DDR_SIZE;
		rsrv_mem_required += tm_size + mc_size;
#endif

		if (getMemorySize() < rsrv_mem_required)
			return;
	}
#endif

#if IS_ENABLED(CONFIG_BCM_ADSL)
	/* reserve memory for DSL.  We use memblock_remove + IO_MAP the removed
	 * memory block to MT_MEMORY_NONCACHED here because ADSL driver code
	 * will need to access the memory.  Another option is to use
	 * memblock_reserve where the kernel still sees the memory, but I could
	 * not find a function to make the reserved memory noncacheable. */
	mem_end -= ADSL_SDRAM_RESERVE_SIZE;
	reserve_system_mem(ADSL_BASE_ADDR_STR, mem_end, ADSL_SDRAM_RESERVE_SIZE);
#endif

#if IS_ENABLED(CONFIG_BCM_RDPA)
	mem_end -= tm_size;
	/* TM reserved memory has to be 2MB-aligned */
	if (mem_end & SZ_1M)
		mem_end -= SZ_1M;
	reserve_system_mem(TM_BASE_ADDR_STR, mem_end, tm_size);

	mem_end -= mc_size;
	/* MC reserved memory has to be 2MB-aligned */
	if (unlikely(mem_end & SZ_1M))
		mem_end -= SZ_1M;
	reserve_system_mem(TM_MC_BASE_ADDR_STR, mem_end, mc_size);
#endif

#if IS_ENABLED(CONFIG_BCM_DHD_RUNNER)
	for( j = 0; j < 3; j++ ) {
		if(dhd_pool_size[j] != 0) {
			char name[16];
			mem_end -= dhd_pool_size[j];
			/* DHD reserved memory has to be 2MB-aligned */
			if (unlikely(mem_end & SZ_1M))
				mem_end -= SZ_1M;
			sprintf(name, "%s%d", "dhd", j);
			reserve_system_mem(name, mem_end, dhd_pool_size[j]);
		}
	}
#endif

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
	mem_end -= SO_MEMORY_SIZE_BYTES;
	reserve_system_mem(B15_MEGA_BARRIER, mem_end, SO_MEMORY_SIZE_BYTES);
#endif

	if( rsvd_mem_cnt )
		is_memory_reserved = true;
}

#endif /*CONFIG_ATAGS*/

static void board_restart(enum reboot_mode mode, const char *cmd)
{
#ifndef CONFIG_BRCM_IKOS
	kerSysSoftReset();
#endif
}


static void __init board_reserve(void)
{
	if (boot_param_status == BOOT_FDT){
		bcm_scan_fdt();
		bcm_reserve_memory();
                bcm_dt_postinit();
	} else {
		board_reserve_atag();
	}
}


#if defined (CONFIG_BCM963138)

#if defined (CONFIG_OF)
static const char * const bcm63xx_dt_compat[] = {
        "brcm,bcm963138",
        NULL
};
#endif

MACHINE_START(BCM963138, "BCM963138")
	/* Maintainer: Broadcom */
	.reserve	= board_reserve,
	.map_io		= board_map_io,	
	.init_early	= board_init_early,
#if !defined (CONFIG_OF)
	.init_irq	= board_init_irq,
#endif
	.init_time	= board_init_timer,
	.init_machine	= board_init_machine,
	.restart	= board_restart,
#ifdef CONFIG_ZONE_DMA
	/* If enable CONFIG_ZONE_DMA, it will reserve the given size of
	 * memory from SDRAM and use it exclusively for DMA purpose.
	 * This ensures the device driver can allocate enough memory. */
	.dma_zone_size	= SZ_16M,	/* must be multiple of 2MB */
#endif

#if defined (CONFIG_ATAGS)
	.fixup          = board_fixup,
#endif

#if defined (CONFIG_OF)
	.dt_compat      = bcm63xx_dt_compat,
	.dt_fixup       = dt_fixup,
	.l2c_aux_val	= BCM_L2C_AUX_VAL,
	.l2c_aux_mask	= BCM_L2C_AUX_MSK,
#endif
MACHINE_END

#endif


#if defined (CONFIG_BCM963148)

#ifdef CONFIG_OF
static const char * const bcm63xx_dt_compat[] = {
        "brcm,bcm963148",
        NULL
};
#endif


MACHINE_START(BCM963148, "BCM963148")
	/* Maintainer: Broadcom */
	.reserve	= board_reserve,
	.map_io		= board_map_io,	
	.init_early	= board_init_early,
#if !defined (CONFIG_OF)
	.init_irq	= board_init_irq,
#endif
	.init_time	= board_init_timer,
	.init_machine	= board_init_machine,
	.restart	= board_restart,
#ifdef CONFIG_ZONE_DMA
	/* If enable CONFIG_ZONE_DMA, it will reserve the given size of
	 * memory from SDRAM and use it exclusively for DMA purpose.
	 * This ensures the device driver can allocate enough memory. */
	.dma_zone_size	= SZ_16M,	/* must be multiple of 2MB and within 16MB for DSL PHY */
#endif
#if defined (CONFIG_ATAGS)
	.fixup          = board_fixup,
#endif
#if defined (CONFIG_OF)
	.dt_compat      = bcm63xx_dt_compat,
	.dt_fixup       = dt_fixup, 
#endif
MACHINE_END

#endif

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
