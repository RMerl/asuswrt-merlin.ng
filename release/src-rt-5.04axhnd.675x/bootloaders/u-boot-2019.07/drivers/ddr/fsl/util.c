// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#ifdef CONFIG_PPC
#include <asm/fsl_law.h>
#endif
#include <div64.h>

#include <fsl_ddr.h>
#include <fsl_immap.h>
#include <asm/io.h>
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3) || \
	defined(CONFIG_ARM)
#include <asm/arch/clock.h>
#endif

/* To avoid 64-bit full-divides, we factor this here */
#define ULL_2E12 2000000000000ULL
#define UL_5POW12 244140625UL
#define UL_2POW13 (1UL << 13)

#define ULL_8FS 0xFFFFFFFFULL

u32 fsl_ddr_get_version(unsigned int ctrl_num)
{
	struct ccsr_ddr __iomem *ddr;
	u32 ver_major_minor_errata;

	switch (ctrl_num) {
	case 0:
		ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;
		break;
#if defined(CONFIG_SYS_FSL_DDR2_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 1)
	case 1:
		ddr = (void *)CONFIG_SYS_FSL_DDR2_ADDR;
		break;
#endif
#if defined(CONFIG_SYS_FSL_DDR3_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 2)
	case 2:
		ddr = (void *)CONFIG_SYS_FSL_DDR3_ADDR;
		break;
#endif
#if defined(CONFIG_SYS_FSL_DDR4_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 3)
	case 3:
		ddr = (void *)CONFIG_SYS_FSL_DDR4_ADDR;
		break;
#endif
	default:
		printf("%s unexpected ctrl_num = %u\n", __func__, ctrl_num);
		return 0;
	}
	ver_major_minor_errata = (ddr_in32(&ddr->ip_rev1) & 0xFFFF) << 8;
	ver_major_minor_errata |= (ddr_in32(&ddr->ip_rev2) & 0xFF00) >> 8;

	return ver_major_minor_errata;
}

/*
 * Round up mclk_ps to nearest 1 ps in memory controller code
 * if the error is 0.5ps or more.
 *
 * If an imprecise data rate is too high due to rounding error
 * propagation, compute a suitably rounded mclk_ps to compute
 * a working memory controller configuration.
 */
unsigned int get_memory_clk_period_ps(const unsigned int ctrl_num)
{
	unsigned int data_rate = get_ddr_freq(ctrl_num);
	unsigned int result;

	/* Round to nearest 10ps, being careful about 64-bit multiply/divide */
	unsigned long long rem, mclk_ps = ULL_2E12;

	/* Now perform the big divide, the result fits in 32-bits */
	rem = do_div(mclk_ps, data_rate);
	result = (rem >= (data_rate >> 1)) ? mclk_ps + 1 : mclk_ps;

	return result;
}

/* Convert picoseconds into DRAM clock cycles (rounding up if needed). */
unsigned int picos_to_mclk(const unsigned int ctrl_num, unsigned int picos)
{
	unsigned long long clks, clks_rem;
	unsigned long data_rate = get_ddr_freq(ctrl_num);

	/* Short circuit for zero picos */
	if (!picos)
		return 0;

	/* First multiply the time by the data rate (32x32 => 64) */
	clks = picos * (unsigned long long)data_rate;
	/*
	 * Now divide by 5^12 and track the 32-bit remainder, then divide
	 * by 2*(2^12) using shifts (and updating the remainder).
	 */
	clks_rem = do_div(clks, UL_5POW12);
	clks_rem += (clks & (UL_2POW13-1)) * UL_5POW12;
	clks >>= 13;

	/* If we had a remainder greater than the 1ps error, then round up */
	if (clks_rem > data_rate)
		clks++;

	/* Clamp to the maximum representable value */
	if (clks > ULL_8FS)
		clks = ULL_8FS;
	return (unsigned int) clks;
}

unsigned int mclk_to_picos(const unsigned int ctrl_num, unsigned int mclk)
{
	return get_memory_clk_period_ps(ctrl_num) * mclk;
}

#ifdef CONFIG_PPC
void
__fsl_ddr_set_lawbar(const common_timing_params_t *memctl_common_params,
			   unsigned int law_memctl,
			   unsigned int ctrl_num)
{
	unsigned long long base = memctl_common_params->base_address;
	unsigned long long size = memctl_common_params->total_mem;

	/*
	 * If no DIMMs on this controller, do not proceed any further.
	 */
	if (!memctl_common_params->ndimms_present) {
		return;
	}

#if !defined(CONFIG_PHYS_64BIT)
	if (base >= CONFIG_MAX_MEM_MAPPED)
		return;
	if ((base + size) >= CONFIG_MAX_MEM_MAPPED)
		size = CONFIG_MAX_MEM_MAPPED - base;
#endif
	if (set_ddr_laws(base, size, law_memctl) < 0) {
		printf("%s: ERROR (ctrl #%d, TRGT ID=%x)\n", __func__, ctrl_num,
			law_memctl);
		return ;
	}
	debug("setup ddr law base = 0x%llx, size 0x%llx, TRGT_ID 0x%x\n",
		base, size, law_memctl);
}

__attribute__((weak, alias("__fsl_ddr_set_lawbar"))) void
fsl_ddr_set_lawbar(const common_timing_params_t *memctl_common_params,
			 unsigned int memctl_interleaved,
			 unsigned int ctrl_num);
#endif

void fsl_ddr_set_intl3r(const unsigned int granule_size)
{
#ifdef CONFIG_E6500
	u32 *mcintl3r = (void *) (CONFIG_SYS_IMMR + 0x18004);
	*mcintl3r = 0x80000000 | (granule_size & 0x1f);
	debug("Enable MCINTL3R with granule size 0x%x\n", granule_size);
#endif
}

u32 fsl_ddr_get_intl3r(void)
{
	u32 val = 0;
#ifdef CONFIG_E6500
	u32 *mcintl3r = (void *) (CONFIG_SYS_IMMR + 0x18004);
	val = *mcintl3r;
#endif
	return val;
}

void print_ddr_info(unsigned int start_ctrl)
{
	struct ccsr_ddr __iomem *ddr =
		(struct ccsr_ddr __iomem *)(CONFIG_SYS_FSL_DDR_ADDR);

#if	defined(CONFIG_E6500) && (CONFIG_SYS_NUM_DDR_CTLRS == 3)
	u32 *mcintl3r = (void *) (CONFIG_SYS_IMMR + 0x18004);
#endif
#if (CONFIG_SYS_NUM_DDR_CTLRS > 1)
	uint32_t cs0_config = ddr_in32(&ddr->cs0_config);
#endif
	uint32_t sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	int cas_lat;

#if CONFIG_SYS_NUM_DDR_CTLRS >= 2
	if ((!(sdram_cfg & SDRAM_CFG_MEM_EN)) ||
	    (start_ctrl == 1)) {
		ddr = (void __iomem *)CONFIG_SYS_FSL_DDR2_ADDR;
		sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	}
#endif
#if CONFIG_SYS_NUM_DDR_CTLRS >= 3
	if ((!(sdram_cfg & SDRAM_CFG_MEM_EN)) ||
	    (start_ctrl == 2)) {
		ddr = (void __iomem *)CONFIG_SYS_FSL_DDR3_ADDR;
		sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	}
#endif

	if (!(sdram_cfg & SDRAM_CFG_MEM_EN)) {
		puts(" (DDR not enabled)\n");
		return;
	}

	puts(" (DDR");
	switch ((sdram_cfg & SDRAM_CFG_SDRAM_TYPE_MASK) >>
		SDRAM_CFG_SDRAM_TYPE_SHIFT) {
	case SDRAM_TYPE_DDR1:
		puts("1");
		break;
	case SDRAM_TYPE_DDR2:
		puts("2");
		break;
	case SDRAM_TYPE_DDR3:
		puts("3");
		break;
	case SDRAM_TYPE_DDR4:
		puts("4");
		break;
	default:
		puts("?");
		break;
	}

	if (sdram_cfg & SDRAM_CFG_32_BE)
		puts(", 32-bit");
	else if (sdram_cfg & SDRAM_CFG_16_BE)
		puts(", 16-bit");
	else
		puts(", 64-bit");

	/* Calculate CAS latency based on timing cfg values */
	cas_lat = ((ddr_in32(&ddr->timing_cfg_1) >> 16) & 0xf);
	if (fsl_ddr_get_version(0) <= 0x40400)
		cas_lat += 1;
	else
		cas_lat += 2;
	cas_lat += ((ddr_in32(&ddr->timing_cfg_3) >> 12) & 3) << 4;
	printf(", CL=%d", cas_lat >> 1);
	if (cas_lat & 0x1)
		puts(".5");

	if (sdram_cfg & SDRAM_CFG_ECC_EN)
		puts(", ECC on)");
	else
		puts(", ECC off)");

#if (CONFIG_SYS_NUM_DDR_CTLRS == 3)
#ifdef CONFIG_E6500
	if (*mcintl3r & 0x80000000) {
		puts("\n");
		puts("       DDR Controller Interleaving Mode: ");
		switch (*mcintl3r & 0x1f) {
		case FSL_DDR_3WAY_1KB_INTERLEAVING:
			puts("3-way 1KB");
			break;
		case FSL_DDR_3WAY_4KB_INTERLEAVING:
			puts("3-way 4KB");
			break;
		case FSL_DDR_3WAY_8KB_INTERLEAVING:
			puts("3-way 8KB");
			break;
		default:
			puts("3-way UNKNOWN");
			break;
		}
	}
#endif
#endif
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 2)
	if ((cs0_config & 0x20000000) && (start_ctrl == 0)) {
		puts("\n");
		puts("       DDR Controller Interleaving Mode: ");

		switch ((cs0_config >> 24) & 0xf) {
		case FSL_DDR_256B_INTERLEAVING:
			puts("256B");
			break;
		case FSL_DDR_CACHE_LINE_INTERLEAVING:
			puts("cache line");
			break;
		case FSL_DDR_PAGE_INTERLEAVING:
			puts("page");
			break;
		case FSL_DDR_BANK_INTERLEAVING:
			puts("bank");
			break;
		case FSL_DDR_SUPERBANK_INTERLEAVING:
			puts("super-bank");
			break;
		default:
			puts("invalid");
			break;
		}
	}
#endif

	if ((sdram_cfg >> 8) & 0x7f) {
		puts("\n");
		puts("       DDR Chip-Select Interleaving Mode: ");
		switch(sdram_cfg >> 8 & 0x7f) {
		case FSL_DDR_CS0_CS1_CS2_CS3:
			puts("CS0+CS1+CS2+CS3");
			break;
		case FSL_DDR_CS0_CS1:
			puts("CS0+CS1");
			break;
		case FSL_DDR_CS2_CS3:
			puts("CS2+CS3");
			break;
		case FSL_DDR_CS0_CS1_AND_CS2_CS3:
			puts("CS0+CS1 and CS2+CS3");
			break;
		default:
			puts("invalid");
			break;
		}
	}
}

void __weak detail_board_ddr_info(void)
{
	print_ddr_info(0);
}

void board_add_ram_info(int use_default)
{
	detail_board_ddr_info();
}

#ifdef CONFIG_FSL_DDR_SYNC_REFRESH
#define DDRC_DEBUG20_INIT_DONE	0x80000000
#define DDRC_DEBUG2_RF		0x00000040
void fsl_ddr_sync_memctl_refresh(unsigned int first_ctrl,
				 unsigned int last_ctrl)
{
	unsigned int i;
	u32 ddrc_debug20;
	u32 ddrc_debug2[CONFIG_SYS_NUM_DDR_CTLRS] = {};
	u32 *ddrc_debug2_p[CONFIG_SYS_NUM_DDR_CTLRS] = {};
	struct ccsr_ddr __iomem *ddr;

	for (i = first_ctrl; i <= last_ctrl; i++) {
		switch (i) {
		case 0:
			ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;
			break;
#if defined(CONFIG_SYS_FSL_DDR2_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 1)
		case 1:
			ddr = (void *)CONFIG_SYS_FSL_DDR2_ADDR;
			break;
#endif
#if defined(CONFIG_SYS_FSL_DDR3_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 2)
		case 2:
			ddr = (void *)CONFIG_SYS_FSL_DDR3_ADDR;
			break;
#endif
#if defined(CONFIG_SYS_FSL_DDR4_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 3)
		case 3:
			ddr = (void *)CONFIG_SYS_FSL_DDR4_ADDR;
			break;
#endif
		default:
			printf("%s unexpected ctrl = %u\n", __func__, i);
			return;
		}
		ddrc_debug20 = ddr_in32(&ddr->debug[19]);
		ddrc_debug2_p[i] = &ddr->debug[1];
		while (!(ddrc_debug20 & DDRC_DEBUG20_INIT_DONE)) {
			/* keep polling until DDRC init is done */
			udelay(100);
			ddrc_debug20 = ddr_in32(&ddr->debug[19]);
		}
		ddrc_debug2[i] = ddr_in32(&ddr->debug[1]) | DDRC_DEBUG2_RF;
	}
	/*
	 * Sync refresh
	 * This is put together to make sure the refresh reqeusts are sent
	 * closely to each other.
	 */
	for (i = first_ctrl; i <= last_ctrl; i++)
		ddr_out32(ddrc_debug2_p[i], ddrc_debug2[i]);
}
#endif /* CONFIG_FSL_DDR_SYNC_REFRESH */

void remove_unused_controllers(fsl_ddr_info_t *info)
{
#ifdef CONFIG_SYS_FSL_HAS_CCN504
	int i;
	u64 nodeid;
	void *hnf_sam_ctrl = (void *)(CCI_HN_F_0_BASE + CCN_HN_F_SAM_CTL);
	bool ddr0_used = false;
	bool ddr1_used = false;

	for (i = 0; i < 8; i++) {
		nodeid = in_le64(hnf_sam_ctrl) & CCN_HN_F_SAM_NODEID_MASK;
		if (nodeid == CCN_HN_F_SAM_NODEID_DDR0) {
			ddr0_used = true;
		} else if (nodeid == CCN_HN_F_SAM_NODEID_DDR1) {
			ddr1_used = true;
		} else {
			printf("Unknown nodeid in HN-F SAM control: 0x%llx\n",
			       nodeid);
		}
		hnf_sam_ctrl += (CCI_HN_F_1_BASE - CCI_HN_F_0_BASE);
	}
	if (!ddr0_used && !ddr1_used) {
		printf("Invalid configuration in HN-F SAM control\n");
		return;
	}

	if (!ddr0_used && info->first_ctrl == 0) {
		info->first_ctrl = 1;
		info->num_ctrls = 1;
		debug("First DDR controller disabled\n");
		return;
	}

	if (!ddr1_used && info->first_ctrl + info->num_ctrls > 1) {
		info->num_ctrls = 1;
		debug("Second DDR controller disabled\n");
	}
#endif
}
