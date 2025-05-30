/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/ddr.h>

#include "spl_ddrinit.h"
#include "ddrinit_dpfe.h"
#include "boot_blob.h"
#include "tpl_params.h"
#ifdef CONFIG_ARMV7_LPAE
#include "mmu_map_v7.h"

/* virtual testing address for the upper SDRAM */
#define VIRT_SDRAM_2_ADDR 0xc0000000
#endif

typedef int (*load_dpfe_seg_fun)(dpfe_seg_param* param);

/*
This is a stub function for jtag load.
cmm script will have a breakpoint, stop here and load the next stage of the dpfe
*/
static int load_dpfe_segment_stub(dpfe_seg_param* param)
{
static int last_seg=0;

	/* cmm script sets this to 1 when it reaches the last stage of dpfe */
	return last_seg;
}
static int load_dpfe_segment(dpfe_seg_param* param)
{
	int last_seg = -1, rc;
	int size = param->buf_size;
	uint32_t magic = 0;

	if (BP_DDR_IS_DDR3(param->mcb_sel))
		magic = DPFE_DDR3_TABLE_MAGIC|param->seg_id;
#if defined(CONFIG_BCMBCA_DDR4)
	else if (BP_DDR_IS_DDR4(param->mcb_sel))
		magic = DPFE_DDR4_TABLE_MAGIC|param->seg_id;
#endif
#if defined(CONFIG_BCMBCA_LPDDR4)
	else if (BP1_DDR_IS_LPDDR4(param->mcb_sel) || BP1_DDR_IS_LPDDR4X(param->mcb_sel))
		magic = DPFE_LPDDR4_TABLE_MAGIC|param->seg_id;
#endif
#if defined(CONFIG_BCMBCA_LPDDR5)
	else if (BP1_DDR_IS_LPDDR5(param->mcb_sel))
		magic = DPFE_LPDDR5_TABLE_MAGIC|param->seg_id;
#endif

	if (param->seg_buf && param->buf_size) {
		rc = load_boot_blob(magic, param->seg_id, param->seg_buf, &size);
		if (rc == BOOT_BLOB_NOT_IN_HASTTBL) { /* seg id not in has table. try with last seg id flag */
			size = param->buf_size;
			rc = load_boot_blob(magic|0x80, param->seg_id|0x80, param->seg_buf, &size);
			if (rc == BOOT_BLOB_SUCCESS)
				last_seg = 1;
			else
				last_seg = rc;
		} else
			last_seg = rc;
	}
	debug("load_dpfe_segment %d return %d\n", param->seg_id, last_seg);
	return last_seg;
}


/* return 1 if there is alias, 0 no alias.  memsize in MB */
static int memc_alias_test(uint32_t memsize)
{
	volatile uint32_t *base_addr;
	volatile uint32_t *test_addr;
	uint64_t test_size, total_size;
	uint32_t data;
#ifdef CONFIG_ARMV7_LPAE
	phys_addr_t pa;
#endif
	int ret = 0;

	total_size = ((uint64_t) (memsize)) << 20;
	base_addr = (volatile uint32_t *)((uintptr_t) CONFIG_SYS_SDRAM_BASE);

	for (test_size = 256; test_size < total_size;
	     test_size = test_size << 1) {
		test_addr =
		    (volatile uint32_t *)((uintptr_t) base_addr +
					  (uintptr_t) test_size);
#if defined(PHYS_SDRAM_2)
		/* if we are over the lower memory region from 0 to 2GB, we shift to the upper memory region */
		if (test_size >= PHYS_SDRAM_1_SIZE) {
#ifdef CONFIG_ARMV7_LPAE
			pa = (phys_addr_t)(uintptr_t)base_addr + test_size + PHYS_SDRAM_1_SIZE;
			test_addr = (volatile uint32_t *)VIRT_SDRAM_2_ADDR;
			map_section((u32)test_addr, pa, MMU_SECTION_SIZE, SECTION_ATTR_DEVICE);
#else
			test_addr =
			    (volatile uint32_t *)((uintptr_t) test_addr +
						  PHYS_SDRAM_1_SIZE);
#endif
		}
#endif

		data = *test_addr;
		*base_addr = 0;
		*test_addr = 0xaa55beef;
		if (*base_addr == *test_addr) {
			printf("alias detected at 0x%p\n", test_addr);
			ret = 1;
		}

		/* check base addr and make sure it does not get overriden */
		if (*base_addr != 0x0) {
			printf("alias test at 0x%p corrupted base 0x%x 0x%x\n",
			       test_addr, *base_addr, *test_addr);
			ret = 1;
		}
		*test_addr = data;

#if defined(PHYS_SDRAM_2)
		if (test_size >= PHYS_SDRAM_1_SIZE) {
#ifdef CONFIG_ARMV7_LPAE
			unmap_section((u32)test_addr, pa, MMU_SECTION_SIZE);
#endif
		}
#endif
		if (ret)
			break;
	}

	return ret;
}

#define IS_ADDR_IN_PAGETBL_RANGE(addr) \
	(((uintptr_t)(addr) >= CONFIG_SYS_PAGETBL_BASE) && \
	 ((uintptr_t)(addr) < CONFIG_SYS_PAGETBL_BASE+CONFIG_SYS_PAGETBL_SIZE))

static int memory_test_range(uint32_t *addr, uint32_t size)
{
	uint32_t *temp;
	int i, ret = 0;
 	uint32_t data = 0;

	for (temp = addr, i = 0; i < size/sizeof(uint32_t); i++) {
		if(IS_ADDR_IN_PAGETBL_RANGE(temp))
			temp++;
		else
			*temp++ = i;
	}

	for (temp = addr, i = 0; i < size/sizeof(uint32_t); i++) {
		if(IS_ADDR_IN_PAGETBL_RANGE(temp))
			temp++;
		else {
			data = *temp++;
			if (data != i)
				break;
		}
	}

	if (i != size/sizeof(uint32_t)) {
		printf("DDR test failed at 0x%p read back 0x%x expected 0x%x\n",
			addr+i, data, i);
		ret = -1;
	}

	return ret;
}

/* return 1 if there is error, 0 no error.  memsize in MB */
static int memc_memory_test(uint32_t memsize)
{
	uint32_t *addr;
	int ret = 0;
	uint64_t total_size;
#ifdef CONFIG_ARMV7_LPAE
	phys_addr_t pa;
#endif

	total_size = ((uint64_t) (memsize)) << 20;

	addr = (uint32_t *)CONFIG_SYS_SDRAM_BASE;
	/* coverity[var_deref_model: Passing null pointer addr to memory_test_range,
	   which dereferences it] */
	ret = memory_test_range(addr, SZ_4K);
#if defined(PHYS_SDRAM_2)
	if (total_size > PHYS_SDRAM_1_SIZE) {
#ifdef CONFIG_ARMV7_LPAE
		pa = PHYS_SDRAM_2;
		addr = (uint32_t *)VIRT_SDRAM_2_ADDR;
#else
		addr = (uint32_t *)PHYS_SDRAM_2;
#endif
		total_size -= PHYS_SDRAM_1_SIZE;
		while ((int64_t)total_size) {
#ifdef CONFIG_ARMV7_LPAE
			map_section((u32)addr, pa, MMU_SECTION_SIZE, SECTION_ATTR_DEVICE);
#endif
			ret |= memory_test_range(addr, SZ_4K);
			total_size -= SZ_1G;
#ifdef CONFIG_ARMV7_LPAE
			unmap_section((u32)addr, pa, MMU_SECTION_SIZE);
			pa += SZ_1G;
#else
			addr = (uint32_t*)((uintptr_t)addr + SZ_1G);
#endif
		}
	}
#endif
	if (!ret)
		printf("DDR test done successfully\n");
	else {
		ret = -2;
	}

	return ret;
}


int ddr_init_dpfe(ddr_init_param * ddrinit_params)
{
	uint32_t memcfg, ddr_size;
	int is_safemode, ret;
	uint8_t *seg_buf = NULL;
	int seg_id, last_seg;
	dpfe_seg_param seg_params;
	dpfe_param dpfe_params;
	dpfe_func run_dpfe;
	load_dpfe_seg_fun load_dpfe_segment_p=load_dpfe_segment;

	memset(&dpfe_params, 0x0, sizeof(dpfe_param));
	if (IS_JTAG_LOADED(boot_params))
	{
		load_dpfe_segment_p=load_dpfe_segment_stub;
	}

	is_safemode = ddrinit_params->safe_mode;
	memcfg = ddrinit_params->mcb_sel;

	seg_buf = (void *)ddrinit_params->dpfe_segbuf;
	run_dpfe = (dpfe_func)ddrinit_params->dpfe_stdalone;

	dpfe_params.mcb = (uint32_t *) ddrinit_params->mcb;
#ifdef CONFIG_BCMBCA_DDRC_SCRAMBLER
	dpfe_params.seed = ddrinit_params->seed;
	dpfe_params.scramble_enable = ddrinit_params->scramble_enable;
	dpfe_params.unscram_addr = ddrinit_params->unscram_addr;
	dpfe_params.unscram_size = ddrinit_params->unscram_size;
#endif
	dpfe_params.ddr_size = &ddr_size;
	dpfe_params.seg_param = &seg_params;
	dpfe_params.dpfe_option = 0;

	if (is_safemode)
		dpfe_params.dpfe_option |= DPFE_OPTION_SAFEMODE;

	seg_id = last_seg = 0;
	dpfe_params.dpfe_option |= DPFE_OPTION_SEG_FIRST;

	seg_params.seg_buf = seg_buf;
	seg_params.mcb_sel = memcfg;
	seg_params.buf_size = CONFIG_DPFE_SEGSIZE;;
  
	while (!last_seg) {
		seg_params.seg_id = seg_id;
		last_seg = load_dpfe_segment_p(&seg_params);
		if (last_seg < 0) {
			printf("failed to load dpfe segment %d!\n", seg_id);
			return -1;
		}
		if (last_seg)
			dpfe_params.dpfe_option |= DPFE_OPTION_SEG_LAST;

		ret = run_dpfe(&dpfe_params);
		if (ret < 0) {
			return -1;
		}

		if (ret > 0) {
			printf("shmoo finish early at segment %d!\n", seg_id);
			break;
		}

		dpfe_params.dpfe_option &= ~DPFE_OPTION_SEG_MASK;
		seg_id++;
	}

	/* make sure configure register write are really carried over to target block */
	__asm__ __volatile__("dsb	sy");
	__asm__ __volatile__("isb");


	/* get the ddr size in mega bytes */
	if (ddrinit_params->ddr_size != NULL)
		*ddrinit_params->ddr_size = ddr_size;


	/* Make sure it is configured size is not larger the actual size */
	if (is_safemode == 0) {
		if (memc_alias_test(ddr_size)) {
			printf
			    ("\nMemory alias detected. Probably wrong memory size is specified or memory subsystem not working\n");
			return -3;
		}
	}

	if ((ret = memc_memory_test(ddr_size)))
		return ret;

	return ret;
}
