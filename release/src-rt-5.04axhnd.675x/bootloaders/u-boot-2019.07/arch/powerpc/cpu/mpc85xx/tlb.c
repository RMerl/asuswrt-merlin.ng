// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#ifdef CONFIG_ADDR_MAP
#include <addr_map.h>
#endif

#include <linux/log2.h>

DECLARE_GLOBAL_DATA_PTR;

void invalidate_tlb(u8 tlb)
{
	if (tlb == 0)
		mtspr(MMUCSR0, 0x4);
	if (tlb == 1)
		mtspr(MMUCSR0, 0x2);
}

__weak void init_tlbs(void)
{
	int i;

	for (i = 0; i < num_tlb_entries; i++) {
		write_tlb(tlb_table[i].mas0,
			  tlb_table[i].mas1,
			  tlb_table[i].mas2,
			  tlb_table[i].mas3,
			  tlb_table[i].mas7);
	}

	return ;
}

#if !defined(CONFIG_NAND_SPL) && \
	(!defined(CONFIG_SPL_BUILD) || !defined(CONFIG_SPL_INIT_MINIMAL))
void read_tlbcam_entry(int idx, u32 *valid, u32 *tsize, unsigned long *epn,
		       phys_addr_t *rpn)
{
	u32 _mas1;

	mtspr(MAS0, FSL_BOOKE_MAS0(1, idx, 0));
	asm volatile("tlbre;isync");
	_mas1 = mfspr(MAS1);

	*valid = (_mas1 & MAS1_VALID);
	*tsize = (_mas1 >> 7) & 0x1f;
	*epn = mfspr(MAS2) & MAS2_EPN;
	*rpn = mfspr(MAS3) & MAS3_RPN;
#ifdef CONFIG_ENABLE_36BIT_PHYS
	*rpn |= ((u64)mfspr(MAS7)) << 32;
#endif
}

void print_tlbcam(void)
{
	int i;
	unsigned int num_cam = mfspr(SPRN_TLB1CFG) & 0xfff;

	/* walk all the entries */
	printf("TLBCAM entries\n");
	for (i = 0; i < num_cam; i++) {
		unsigned long epn;
		u32 tsize, valid;
		phys_addr_t rpn;

		read_tlbcam_entry(i, &valid, &tsize, &epn, &rpn);
		printf("entry %02d: V: %d EPN 0x%08x RPN 0x%08llx size:",
			i, (valid == 0) ? 0 : 1, (unsigned int)epn,
			(unsigned long long)rpn);
		print_size(TSIZE_TO_BYTES(tsize), "\n");
	}
}

static inline void use_tlb_cam(u8 idx)
{
	int i = idx / 32;
	int bit = idx % 32;

	gd->arch.used_tlb_cams[i] |= (1 << bit);
}

static inline void free_tlb_cam(u8 idx)
{
	int i = idx / 32;
	int bit = idx % 32;

	gd->arch.used_tlb_cams[i] &= ~(1 << bit);
}

void init_used_tlb_cams(void)
{
	int i;
	unsigned int num_cam = mfspr(SPRN_TLB1CFG) & 0xfff;

	for (i = 0; i < ((CONFIG_SYS_NUM_TLBCAMS+31)/32); i++)
		gd->arch.used_tlb_cams[i] = 0;

	/* walk all the entries */
	for (i = 0; i < num_cam; i++) {
		mtspr(MAS0, FSL_BOOKE_MAS0(1, i, 0));
		asm volatile("tlbre;isync");
		if (mfspr(MAS1) & MAS1_VALID)
			use_tlb_cam(i);
	}
}

int find_free_tlbcam(void)
{
	int i;
	u32 idx;

	for (i = 0; i < ((CONFIG_SYS_NUM_TLBCAMS+31)/32); i++) {
		idx = ffz(gd->arch.used_tlb_cams[i]);

		if (idx != 32)
			break;
	}

	idx += i * 32;

	if (idx >= CONFIG_SYS_NUM_TLBCAMS)
		return -1;

	return idx;
}

void set_tlb(u8 tlb, u32 epn, u64 rpn,
	     u8 perms, u8 wimge,
	     u8 ts, u8 esel, u8 tsize, u8 iprot)
{
	u32 _mas0, _mas1, _mas2, _mas3, _mas7;

	if (tlb == 1)
		use_tlb_cam(esel);

	if ((mfspr(SPRN_MMUCFG) & MMUCFG_MAVN) == MMUCFG_MAVN_V1 &&
	    tsize & 1) {
		printf("%s: bad tsize %d on entry %d at 0x%08x\n",
			__func__, tsize, tlb, epn);
		return;
	}

	_mas0 = FSL_BOOKE_MAS0(tlb, esel, 0);
	_mas1 = FSL_BOOKE_MAS1(1, iprot, 0, ts, tsize);
	_mas2 = FSL_BOOKE_MAS2(epn, wimge);
	_mas3 = FSL_BOOKE_MAS3(rpn, 0, perms);
	_mas7 = FSL_BOOKE_MAS7(rpn);

	write_tlb(_mas0, _mas1, _mas2, _mas3, _mas7);

#ifdef CONFIG_ADDR_MAP
	if ((tlb == 1) && (gd->flags & GD_FLG_RELOC))
		addrmap_set_entry(epn, rpn, TSIZE_TO_BYTES(tsize), esel);
#endif
}

void disable_tlb(u8 esel)
{
	u32 _mas0, _mas1, _mas2, _mas3;

	free_tlb_cam(esel);

	_mas0 = FSL_BOOKE_MAS0(1, esel, 0);
	_mas1 = 0;
	_mas2 = 0;
	_mas3 = 0;

	mtspr(MAS0, _mas0);
	mtspr(MAS1, _mas1);
	mtspr(MAS2, _mas2);
	mtspr(MAS3, _mas3);
#ifdef CONFIG_ENABLE_36BIT_PHYS
	mtspr(MAS7, 0);
#endif
	asm volatile("isync;msync;tlbwe;isync");

#ifdef CONFIG_ADDR_MAP
	if (gd->flags & GD_FLG_RELOC)
		addrmap_set_entry(0, 0, 0, esel);
#endif
}

static void tlbsx (const volatile unsigned *addr)
{
	__asm__ __volatile__ ("tlbsx 0,%0" : : "r" (addr), "m" (*addr));
}

/* return -1 if we didn't find anything */
int find_tlb_idx(void *addr, u8 tlbsel)
{
	u32 _mas0, _mas1;

	/* zero out Search PID, AS */
	mtspr(MAS6, 0);

	tlbsx(addr);

	_mas0 = mfspr(MAS0);
	_mas1 = mfspr(MAS1);

	/* we found something, and its in the TLB we expect */
	if ((MAS1_VALID & _mas1) &&
		(MAS0_TLBSEL(tlbsel) == (_mas0 & MAS0_TLBSEL_MSK))) {
		return ((_mas0 & MAS0_ESEL_MSK) >> 16);
	}

	return -1;
}

#ifdef CONFIG_ADDR_MAP
void init_addr_map(void)
{
	int i;
	unsigned int num_cam = mfspr(SPRN_TLB1CFG) & 0xfff;

	/* walk all the entries */
	for (i = 0; i < num_cam; i++) {
		unsigned long epn;
		u32 tsize, valid;
		phys_addr_t rpn;

		read_tlbcam_entry(i, &valid, &tsize, &epn, &rpn);
		if (valid & MAS1_VALID)
			addrmap_set_entry(epn, rpn, TSIZE_TO_BYTES(tsize), i);
	}

	return ;
}
#endif

uint64_t tlb_map_range(ulong v_addr, phys_addr_t p_addr, uint64_t size,
		       enum tlb_map_type map_type)
{
	int i;
	unsigned int tlb_size;
	unsigned int wimge;
	unsigned int perm;
	unsigned int max_cam, tsize_mask;

	if (map_type == TLB_MAP_RAM) {
		perm = MAS3_SX|MAS3_SW|MAS3_SR;
		wimge = MAS2_M;
#ifdef CONFIG_SYS_PPC_DDR_WIMGE
		wimge = CONFIG_SYS_PPC_DDR_WIMGE;
#endif
	} else {
		perm = MAS3_SW|MAS3_SR;
		wimge = MAS2_I|MAS2_G;
	}

	if ((mfspr(SPRN_MMUCFG) & MMUCFG_MAVN) == MMUCFG_MAVN_V1) {
		/* Convert (4^max) kB to (2^max) bytes */
		max_cam = ((mfspr(SPRN_TLB1CFG) >> 16) & 0xf) * 2 + 10;
		tsize_mask = ~1U;
	} else {
		/* Convert (2^max) kB to (2^max) bytes */
		max_cam = __ilog2(mfspr(SPRN_TLB1PS)) + 10;
		tsize_mask = ~0U;
	}

	for (i = 0; size && i < 8; i++) {
		int tlb_index = find_free_tlbcam();
		u32 camsize = __ilog2_u64(size) & tsize_mask;
		u32 align = __ilog2(v_addr) & tsize_mask;

		if (tlb_index == -1)
			break;

		if (align == -2) align = max_cam;
		if (camsize > align)
			camsize = align;

		if (camsize > max_cam)
			camsize = max_cam;

		tlb_size = camsize - 10;

		set_tlb(1, v_addr, p_addr, perm, wimge,
			0, tlb_index, tlb_size, 1);

		size -= 1ULL << camsize;
		v_addr += 1UL << camsize;
		p_addr += 1UL << camsize;
	}

	return size;
}

unsigned int setup_ddr_tlbs_phys(phys_addr_t p_addr,
				 unsigned int memsize_in_meg)
{
	unsigned int ram_tlb_address = (unsigned int)CONFIG_SYS_DDR_SDRAM_BASE;
	u64 memsize = (u64)memsize_in_meg << 20;
	u64 size;

	size = min(memsize, (u64)CONFIG_MAX_MEM_MAPPED);
	size = tlb_map_range(ram_tlb_address, p_addr, size, TLB_MAP_RAM);

	if (size || memsize > CONFIG_MAX_MEM_MAPPED) {
		print_size(memsize > CONFIG_MAX_MEM_MAPPED ?
			   memsize - CONFIG_MAX_MEM_MAPPED + size : size,
			   " left unmapped\n");
	}

	return memsize_in_meg;
}

unsigned int setup_ddr_tlbs(unsigned int memsize_in_meg)
{
	return
		setup_ddr_tlbs_phys(CONFIG_SYS_DDR_SDRAM_BASE, memsize_in_meg);
}

/* Invalidate the DDR TLBs for the requested size */
void clear_ddr_tlbs_phys(phys_addr_t p_addr, unsigned int memsize_in_meg)
{
	u32 vstart = CONFIG_SYS_DDR_SDRAM_BASE;
	unsigned long epn;
	u32 tsize, valid, ptr;
	phys_addr_t rpn = 0;
	int ddr_esel;
	u64 memsize = (u64)memsize_in_meg << 20;

	ptr = vstart;

	while (ptr < (vstart + memsize)) {
		ddr_esel = find_tlb_idx((void *)ptr, 1);
		if (ddr_esel != -1) {
			read_tlbcam_entry(ddr_esel, &valid, &tsize, &epn, &rpn);
			disable_tlb(ddr_esel);
		}
		ptr += TSIZE_TO_BYTES(tsize);
	}
}

void clear_ddr_tlbs(unsigned int memsize_in_meg)
{
	clear_ddr_tlbs_phys(CONFIG_SYS_DDR_SDRAM_BASE, memsize_in_meg);
}


#endif /* not SPL */
