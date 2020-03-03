/*
 * native hashtable management.
 *
 * SMP scalability work:
 *    Copyright (C) 2001 Anton Blanchard <anton@au.ibm.com>, IBM
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#undef DEBUG_LOW

#include <linux/spinlock.h>
#include <linux/bitops.h>
#include <linux/of.h>
#include <linux/threads.h>
#include <linux/smp.h>

#include <asm/machdep.h>
#include <asm/mmu.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/tlb.h>
#include <asm/cputable.h>
#include <asm/udbg.h>
#include <asm/kexec.h>
#include <asm/ppc-opcode.h>

#include <misc/cxl.h>

#ifdef DEBUG_LOW
#define DBG_LOW(fmt...) udbg_printf(fmt)
#else
#define DBG_LOW(fmt...)
#endif

#ifdef __BIG_ENDIAN__
#define HPTE_LOCK_BIT 3
#else
#define HPTE_LOCK_BIT (56+3)
#endif

DEFINE_RAW_SPINLOCK(native_tlbie_lock);

static inline void __tlbie(unsigned long vpn, int psize, int apsize, int ssize)
{
	unsigned long va;
	unsigned int penc;
	unsigned long sllp;

	/*
	 * We need 14 to 65 bits of va for a tlibe of 4K page
	 * With vpn we ignore the lower VPN_SHIFT bits already.
	 * And top two bits are already ignored because we can
	 * only accomadate 76 bits in a 64 bit vpn with a VPN_SHIFT
	 * of 12.
	 */
	va = vpn << VPN_SHIFT;
	/*
	 * clear top 16 bits of 64bit va, non SLS segment
	 * Older versions of the architecture (2.02 and earler) require the
	 * masking of the top 16 bits.
	 */
	va &= ~(0xffffULL << 48);

	switch (psize) {
	case MMU_PAGE_4K:
		/* clear out bits after (52) [0....52.....63] */
		va &= ~((1ul << (64 - 52)) - 1);
		va |= ssize << 8;
		sllp = ((mmu_psize_defs[apsize].sllp & SLB_VSID_L) >> 6) |
			((mmu_psize_defs[apsize].sllp & SLB_VSID_LP) >> 4);
		va |= sllp << 5;
		asm volatile(ASM_FTR_IFCLR("tlbie %0,0", PPC_TLBIE(%1,%0), %2)
			     : : "r" (va), "r"(0), "i" (CPU_FTR_ARCH_206)
			     : "memory");
		break;
	default:
		/* We need 14 to 14 + i bits of va */
		penc = mmu_psize_defs[psize].penc[apsize];
		va &= ~((1ul << mmu_psize_defs[apsize].shift) - 1);
		va |= penc << 12;
		va |= ssize << 8;
		/*
		 * AVAL bits:
		 * We don't need all the bits, but rest of the bits
		 * must be ignored by the processor.
		 * vpn cover upto 65 bits of va. (0...65) and we need
		 * 58..64 bits of va.
		 */
		va |= (vpn & 0xfe); /* AVAL */
		va |= 1; /* L */
		asm volatile(ASM_FTR_IFCLR("tlbie %0,1", PPC_TLBIE(%1,%0), %2)
			     : : "r" (va), "r"(0), "i" (CPU_FTR_ARCH_206)
			     : "memory");
		break;
	}
}

static inline void __tlbiel(unsigned long vpn, int psize, int apsize, int ssize)
{
	unsigned long va;
	unsigned int penc;
	unsigned long sllp;

	/* VPN_SHIFT can be atmost 12 */
	va = vpn << VPN_SHIFT;
	/*
	 * clear top 16 bits of 64 bit va, non SLS segment
	 * Older versions of the architecture (2.02 and earler) require the
	 * masking of the top 16 bits.
	 */
	va &= ~(0xffffULL << 48);

	switch (psize) {
	case MMU_PAGE_4K:
		/* clear out bits after(52) [0....52.....63] */
		va &= ~((1ul << (64 - 52)) - 1);
		va |= ssize << 8;
		sllp = ((mmu_psize_defs[apsize].sllp & SLB_VSID_L) >> 6) |
			((mmu_psize_defs[apsize].sllp & SLB_VSID_LP) >> 4);
		va |= sllp << 5;
		asm volatile(".long 0x7c000224 | (%0 << 11) | (0 << 21)"
			     : : "r"(va) : "memory");
		break;
	default:
		/* We need 14 to 14 + i bits of va */
		penc = mmu_psize_defs[psize].penc[apsize];
		va &= ~((1ul << mmu_psize_defs[apsize].shift) - 1);
		va |= penc << 12;
		va |= ssize << 8;
		/*
		 * AVAL bits:
		 * We don't need all the bits, but rest of the bits
		 * must be ignored by the processor.
		 * vpn cover upto 65 bits of va. (0...65) and we need
		 * 58..64 bits of va.
		 */
		va |= (vpn & 0xfe);
		va |= 1; /* L */
		asm volatile(".long 0x7c000224 | (%0 << 11) | (1 << 21)"
			     : : "r"(va) : "memory");
		break;
	}

}

static inline void tlbie(unsigned long vpn, int psize, int apsize,
			 int ssize, int local)
{
	unsigned int use_local;
	int lock_tlbie = !mmu_has_feature(MMU_FTR_LOCKLESS_TLBIE);

	use_local = local && mmu_has_feature(MMU_FTR_TLBIEL) && !cxl_ctx_in_use();

	if (use_local)
		use_local = mmu_psize_defs[psize].tlbiel;
	if (lock_tlbie && !use_local)
		raw_spin_lock(&native_tlbie_lock);
	asm volatile("ptesync": : :"memory");
	if (use_local) {
		__tlbiel(vpn, psize, apsize, ssize);
		asm volatile("ptesync": : :"memory");
	} else {
		__tlbie(vpn, psize, apsize, ssize);
		asm volatile("eieio; tlbsync; ptesync": : :"memory");
	}
	if (lock_tlbie && !use_local)
		raw_spin_unlock(&native_tlbie_lock);
}

static inline void native_lock_hpte(struct hash_pte *hptep)
{
	unsigned long *word = (unsigned long *)&hptep->v;

	while (1) {
		if (!test_and_set_bit_lock(HPTE_LOCK_BIT, word))
			break;
		while(test_bit(HPTE_LOCK_BIT, word))
			cpu_relax();
	}
}

static inline void native_unlock_hpte(struct hash_pte *hptep)
{
	unsigned long *word = (unsigned long *)&hptep->v;

	clear_bit_unlock(HPTE_LOCK_BIT, word);
}

static long native_hpte_insert(unsigned long hpte_group, unsigned long vpn,
			unsigned long pa, unsigned long rflags,
			unsigned long vflags, int psize, int apsize, int ssize)
{
	struct hash_pte *hptep = htab_address + hpte_group;
	unsigned long hpte_v, hpte_r;
	int i;

	if (!(vflags & HPTE_V_BOLTED)) {
		DBG_LOW("    insert(group=%lx, vpn=%016lx, pa=%016lx,"
			" rflags=%lx, vflags=%lx, psize=%d)\n",
			hpte_group, vpn, pa, rflags, vflags, psize);
	}

	for (i = 0; i < HPTES_PER_GROUP; i++) {
		if (! (be64_to_cpu(hptep->v) & HPTE_V_VALID)) {
			/* retry with lock held */
			native_lock_hpte(hptep);
			if (! (be64_to_cpu(hptep->v) & HPTE_V_VALID))
				break;
			native_unlock_hpte(hptep);
		}

		hptep++;
	}

	if (i == HPTES_PER_GROUP)
		return -1;

	hpte_v = hpte_encode_v(vpn, psize, apsize, ssize) | vflags | HPTE_V_VALID;
	hpte_r = hpte_encode_r(pa, psize, apsize) | rflags;

	if (!(vflags & HPTE_V_BOLTED)) {
		DBG_LOW(" i=%x hpte_v=%016lx, hpte_r=%016lx\n",
			i, hpte_v, hpte_r);
	}

	hptep->r = cpu_to_be64(hpte_r);
	/* Guarantee the second dword is visible before the valid bit */
	eieio();
	/*
	 * Now set the first dword including the valid bit
	 * NOTE: this also unlocks the hpte
	 */
	hptep->v = cpu_to_be64(hpte_v);

	__asm__ __volatile__ ("ptesync" : : : "memory");

	return i | (!!(vflags & HPTE_V_SECONDARY) << 3);
}

static long native_hpte_remove(unsigned long hpte_group)
{
	struct hash_pte *hptep;
	int i;
	int slot_offset;
	unsigned long hpte_v;

	DBG_LOW("    remove(group=%lx)\n", hpte_group);

	/* pick a random entry to start at */
	slot_offset = mftb() & 0x7;

	for (i = 0; i < HPTES_PER_GROUP; i++) {
		hptep = htab_address + hpte_group + slot_offset;
		hpte_v = be64_to_cpu(hptep->v);

		if ((hpte_v & HPTE_V_VALID) && !(hpte_v & HPTE_V_BOLTED)) {
			/* retry with lock held */
			native_lock_hpte(hptep);
			hpte_v = be64_to_cpu(hptep->v);
			if ((hpte_v & HPTE_V_VALID)
			    && !(hpte_v & HPTE_V_BOLTED))
				break;
			native_unlock_hpte(hptep);
		}

		slot_offset++;
		slot_offset &= 0x7;
	}

	if (i == HPTES_PER_GROUP)
		return -1;

	/* Invalidate the hpte. NOTE: this also unlocks it */
	hptep->v = 0;

	return i;
}

static long native_hpte_updatepp(unsigned long slot, unsigned long newpp,
				 unsigned long vpn, int bpsize,
				 int apsize, int ssize, unsigned long flags)
{
	struct hash_pte *hptep = htab_address + slot;
	unsigned long hpte_v, want_v;
	int ret = 0, local = 0;

	want_v = hpte_encode_avpn(vpn, bpsize, ssize);

	DBG_LOW("    update(vpn=%016lx, avpnv=%016lx, group=%lx, newpp=%lx)",
		vpn, want_v & HPTE_V_AVPN, slot, newpp);

	hpte_v = be64_to_cpu(hptep->v);
	/*
	 * We need to invalidate the TLB always because hpte_remove doesn't do
	 * a tlb invalidate. If a hash bucket gets full, we "evict" a more/less
	 * random entry from it. When we do that we don't invalidate the TLB
	 * (hpte_remove) because we assume the old translation is still
	 * technically "valid".
	 */
	if (!HPTE_V_COMPARE(hpte_v, want_v) || !(hpte_v & HPTE_V_VALID)) {
		DBG_LOW(" -> miss\n");
		ret = -1;
	} else {
		native_lock_hpte(hptep);
		/* recheck with locks held */
		hpte_v = be64_to_cpu(hptep->v);
		if (unlikely(!HPTE_V_COMPARE(hpte_v, want_v) ||
			     !(hpte_v & HPTE_V_VALID))) {
			ret = -1;
		} else {
			DBG_LOW(" -> hit\n");
			/* Update the HPTE */
			hptep->r = cpu_to_be64((be64_to_cpu(hptep->r) &
						~(HPTE_R_PP | HPTE_R_N)) |
					       (newpp & (HPTE_R_PP | HPTE_R_N |
							 HPTE_R_C)));
		}
		native_unlock_hpte(hptep);
	}

	if (flags & HPTE_LOCAL_UPDATE)
		local = 1;
	/*
	 * Ensure it is out of the tlb too if it is not a nohpte fault
	 */
	if (!(flags & HPTE_NOHPTE_UPDATE))
		tlbie(vpn, bpsize, apsize, ssize, local);

	return ret;
}

static long native_hpte_find(unsigned long vpn, int psize, int ssize)
{
	struct hash_pte *hptep;
	unsigned long hash;
	unsigned long i;
	long slot;
	unsigned long want_v, hpte_v;

	hash = hpt_hash(vpn, mmu_psize_defs[psize].shift, ssize);
	want_v = hpte_encode_avpn(vpn, psize, ssize);

	/* Bolted mappings are only ever in the primary group */
	slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
	for (i = 0; i < HPTES_PER_GROUP; i++) {
		hptep = htab_address + slot;
		hpte_v = be64_to_cpu(hptep->v);

		if (HPTE_V_COMPARE(hpte_v, want_v) && (hpte_v & HPTE_V_VALID))
			/* HPTE matches */
			return slot;
		++slot;
	}

	return -1;
}

/*
 * Update the page protection bits. Intended to be used to create
 * guard pages for kernel data structures on pages which are bolted
 * in the HPT. Assumes pages being operated on will not be stolen.
 *
 * No need to lock here because we should be the only user.
 */
static void native_hpte_updateboltedpp(unsigned long newpp, unsigned long ea,
				       int psize, int ssize)
{
	unsigned long vpn;
	unsigned long vsid;
	long slot;
	struct hash_pte *hptep;

	vsid = get_kernel_vsid(ea, ssize);
	vpn = hpt_vpn(ea, vsid, ssize);

	slot = native_hpte_find(vpn, psize, ssize);
	if (slot == -1)
		panic("could not find page to bolt\n");
	hptep = htab_address + slot;

	/* Update the HPTE */
	hptep->r = cpu_to_be64((be64_to_cpu(hptep->r) &
			~(HPTE_R_PP | HPTE_R_N)) |
		(newpp & (HPTE_R_PP | HPTE_R_N)));
	/*
	 * Ensure it is out of the tlb too. Bolted entries base and
	 * actual page size will be same.
	 */
	tlbie(vpn, psize, psize, ssize, 0);
}

static void native_hpte_invalidate(unsigned long slot, unsigned long vpn,
				   int bpsize, int apsize, int ssize, int local)
{
	struct hash_pte *hptep = htab_address + slot;
	unsigned long hpte_v;
	unsigned long want_v;
	unsigned long flags;

	local_irq_save(flags);

	DBG_LOW("    invalidate(vpn=%016lx, hash: %lx)\n", vpn, slot);

	want_v = hpte_encode_avpn(vpn, bpsize, ssize);
	native_lock_hpte(hptep);
	hpte_v = be64_to_cpu(hptep->v);

	/*
	 * We need to invalidate the TLB always because hpte_remove doesn't do
	 * a tlb invalidate. If a hash bucket gets full, we "evict" a more/less
	 * random entry from it. When we do that we don't invalidate the TLB
	 * (hpte_remove) because we assume the old translation is still
	 * technically "valid".
	 */
	if (!HPTE_V_COMPARE(hpte_v, want_v) || !(hpte_v & HPTE_V_VALID))
		native_unlock_hpte(hptep);
	else
		/* Invalidate the hpte. NOTE: this also unlocks it */
		hptep->v = 0;

	/* Invalidate the TLB */
	tlbie(vpn, bpsize, apsize, ssize, local);

	local_irq_restore(flags);
}

static void native_hugepage_invalidate(unsigned long vsid,
				       unsigned long addr,
				       unsigned char *hpte_slot_array,
				       int psize, int ssize, int local)
{
	int i;
	struct hash_pte *hptep;
	int actual_psize = MMU_PAGE_16M;
	unsigned int max_hpte_count, valid;
	unsigned long flags, s_addr = addr;
	unsigned long hpte_v, want_v, shift;
	unsigned long hidx, vpn = 0, hash, slot;

	shift = mmu_psize_defs[psize].shift;
	max_hpte_count = 1U << (PMD_SHIFT - shift);

	local_irq_save(flags);
	for (i = 0; i < max_hpte_count; i++) {
		valid = hpte_valid(hpte_slot_array, i);
		if (!valid)
			continue;
		hidx =  hpte_hash_index(hpte_slot_array, i);

		/* get the vpn */
		addr = s_addr + (i * (1ul << shift));
		vpn = hpt_vpn(addr, vsid, ssize);
		hash = hpt_hash(vpn, shift, ssize);
		if (hidx & _PTEIDX_SECONDARY)
			hash = ~hash;

		slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
		slot += hidx & _PTEIDX_GROUP_IX;

		hptep = htab_address + slot;
		want_v = hpte_encode_avpn(vpn, psize, ssize);
		native_lock_hpte(hptep);
		hpte_v = be64_to_cpu(hptep->v);

		/* Even if we miss, we need to invalidate the TLB */
		if (!HPTE_V_COMPARE(hpte_v, want_v) || !(hpte_v & HPTE_V_VALID))
			native_unlock_hpte(hptep);
		else
			/* Invalidate the hpte. NOTE: this also unlocks it */
			hptep->v = 0;
		/*
		 * We need to do tlb invalidate for all the address, tlbie
		 * instruction compares entry_VA in tlb with the VA specified
		 * here
		 */
		tlbie(vpn, psize, actual_psize, ssize, local);
	}
	local_irq_restore(flags);
}

static inline int __hpte_actual_psize(unsigned int lp, int psize)
{
	int i, shift;
	unsigned int mask;

	/* start from 1 ignoring MMU_PAGE_4K */
	for (i = 1; i < MMU_PAGE_COUNT; i++) {

		/* invalid penc */
		if (mmu_psize_defs[psize].penc[i] == -1)
			continue;
		/*
		 * encoding bits per actual page size
		 *        PTE LP     actual page size
		 *    rrrr rrrz		>=8KB
		 *    rrrr rrzz		>=16KB
		 *    rrrr rzzz		>=32KB
		 *    rrrr zzzz		>=64KB
		 * .......
		 */
		shift = mmu_psize_defs[i].shift - LP_SHIFT;
		if (shift > LP_BITS)
			shift = LP_BITS;
		mask = (1 << shift) - 1;
		if ((lp & mask) == mmu_psize_defs[psize].penc[i])
			return i;
	}
	return -1;
}

static void hpte_decode(struct hash_pte *hpte, unsigned long slot,
			int *psize, int *apsize, int *ssize, unsigned long *vpn)
{
	unsigned long avpn, pteg, vpi;
	unsigned long hpte_v = be64_to_cpu(hpte->v);
	unsigned long hpte_r = be64_to_cpu(hpte->r);
	unsigned long vsid, seg_off;
	int size, a_size, shift;
	/* Look at the 8 bit LP value */
	unsigned int lp = (hpte_r >> LP_SHIFT) & ((1 << LP_BITS) - 1);

	if (!(hpte_v & HPTE_V_LARGE)) {
		size   = MMU_PAGE_4K;
		a_size = MMU_PAGE_4K;
	} else {
		for (size = 0; size < MMU_PAGE_COUNT; size++) {

			/* valid entries have a shift value */
			if (!mmu_psize_defs[size].shift)
				continue;

			a_size = __hpte_actual_psize(lp, size);
			if (a_size != -1)
				break;
		}
	}
	/* This works for all page sizes, and for 256M and 1T segments */
	*ssize = hpte_v >> HPTE_V_SSIZE_SHIFT;
	shift = mmu_psize_defs[size].shift;

	avpn = (HPTE_V_AVPN_VAL(hpte_v) & ~mmu_psize_defs[size].avpnm);
	pteg = slot / HPTES_PER_GROUP;
	if (hpte_v & HPTE_V_SECONDARY)
		pteg = ~pteg;

	switch (*ssize) {
	case MMU_SEGSIZE_256M:
		/* We only have 28 - 23 bits of seg_off in avpn */
		seg_off = (avpn & 0x1f) << 23;
		vsid    =  avpn >> 5;
		/* We can find more bits from the pteg value */
		if (shift < 23) {
			vpi = (vsid ^ pteg) & htab_hash_mask;
			seg_off |= vpi << shift;
		}
		*vpn = vsid << (SID_SHIFT - VPN_SHIFT) | seg_off >> VPN_SHIFT;
		break;
	case MMU_SEGSIZE_1T:
		/* We only have 40 - 23 bits of seg_off in avpn */
		seg_off = (avpn & 0x1ffff) << 23;
		vsid    = avpn >> 17;
		if (shift < 23) {
			vpi = (vsid ^ (vsid << 25) ^ pteg) & htab_hash_mask;
			seg_off |= vpi << shift;
		}
		*vpn = vsid << (SID_SHIFT_1T - VPN_SHIFT) | seg_off >> VPN_SHIFT;
		break;
	default:
		*vpn = size = 0;
	}
	*psize  = size;
	*apsize = a_size;
}

/*
 * clear all mappings on kexec.  All cpus are in real mode (or they will
 * be when they isi), and we are the only one left.  We rely on our kernel
 * mapping being 0xC0's and the hardware ignoring those two real bits.
 *
 * TODO: add batching support when enabled.  remember, no dynamic memory here,
 * athough there is the control page available...
 */
static void native_hpte_clear(void)
{
	unsigned long vpn = 0;
	unsigned long slot, slots, flags;
	struct hash_pte *hptep = htab_address;
	unsigned long hpte_v;
	unsigned long pteg_count;
	int psize, apsize, ssize;

	pteg_count = htab_hash_mask + 1;

	local_irq_save(flags);

	/* we take the tlbie lock and hold it.  Some hardware will
	 * deadlock if we try to tlbie from two processors at once.
	 */
	raw_spin_lock(&native_tlbie_lock);

	slots = pteg_count * HPTES_PER_GROUP;

	for (slot = 0; slot < slots; slot++, hptep++) {
		/*
		 * we could lock the pte here, but we are the only cpu
		 * running,  right?  and for crash dump, we probably
		 * don't want to wait for a maybe bad cpu.
		 */
		hpte_v = be64_to_cpu(hptep->v);

		/*
		 * Call __tlbie() here rather than tlbie() since we
		 * already hold the native_tlbie_lock.
		 */
		if (hpte_v & HPTE_V_VALID) {
			hpte_decode(hptep, slot, &psize, &apsize, &ssize, &vpn);
			hptep->v = 0;
			__tlbie(vpn, psize, apsize, ssize);
		}
	}

	asm volatile("eieio; tlbsync; ptesync":::"memory");
	raw_spin_unlock(&native_tlbie_lock);
	local_irq_restore(flags);
}

/*
 * Batched hash table flush, we batch the tlbie's to avoid taking/releasing
 * the lock all the time
 */
static void native_flush_hash_range(unsigned long number, int local)
{
	unsigned long vpn;
	unsigned long hash, index, hidx, shift, slot;
	struct hash_pte *hptep;
	unsigned long hpte_v;
	unsigned long want_v;
	unsigned long flags;
	real_pte_t pte;
	struct ppc64_tlb_batch *batch = this_cpu_ptr(&ppc64_tlb_batch);
	unsigned long psize = batch->psize;
	int ssize = batch->ssize;
	int i;
	unsigned int use_local;

	use_local = local && mmu_has_feature(MMU_FTR_TLBIEL) &&
		mmu_psize_defs[psize].tlbiel && !cxl_ctx_in_use();

	local_irq_save(flags);

	for (i = 0; i < number; i++) {
		vpn = batch->vpn[i];
		pte = batch->pte[i];

		pte_iterate_hashed_subpages(pte, psize, vpn, index, shift) {
			hash = hpt_hash(vpn, shift, ssize);
			hidx = __rpte_to_hidx(pte, index);
			if (hidx & _PTEIDX_SECONDARY)
				hash = ~hash;
			slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
			slot += hidx & _PTEIDX_GROUP_IX;
			hptep = htab_address + slot;
			want_v = hpte_encode_avpn(vpn, psize, ssize);
			native_lock_hpte(hptep);
			hpte_v = be64_to_cpu(hptep->v);
			if (!HPTE_V_COMPARE(hpte_v, want_v) ||
			    !(hpte_v & HPTE_V_VALID))
				native_unlock_hpte(hptep);
			else
				hptep->v = 0;
		} pte_iterate_hashed_end();
	}

	if (use_local) {
		asm volatile("ptesync":::"memory");
		for (i = 0; i < number; i++) {
			vpn = batch->vpn[i];
			pte = batch->pte[i];

			pte_iterate_hashed_subpages(pte, psize,
						    vpn, index, shift) {
				__tlbiel(vpn, psize, psize, ssize);
			} pte_iterate_hashed_end();
		}
		asm volatile("ptesync":::"memory");
	} else {
		int lock_tlbie = !mmu_has_feature(MMU_FTR_LOCKLESS_TLBIE);

		if (lock_tlbie)
			raw_spin_lock(&native_tlbie_lock);

		asm volatile("ptesync":::"memory");
		for (i = 0; i < number; i++) {
			vpn = batch->vpn[i];
			pte = batch->pte[i];

			pte_iterate_hashed_subpages(pte, psize,
						    vpn, index, shift) {
				__tlbie(vpn, psize, psize, ssize);
			} pte_iterate_hashed_end();
		}
		asm volatile("eieio; tlbsync; ptesync":::"memory");

		if (lock_tlbie)
			raw_spin_unlock(&native_tlbie_lock);
	}

	local_irq_restore(flags);
}

void __init hpte_init_native(void)
{
	ppc_md.hpte_invalidate	= native_hpte_invalidate;
	ppc_md.hpte_updatepp	= native_hpte_updatepp;
	ppc_md.hpte_updateboltedpp = native_hpte_updateboltedpp;
	ppc_md.hpte_insert	= native_hpte_insert;
	ppc_md.hpte_remove	= native_hpte_remove;
	ppc_md.hpte_clear_all	= native_hpte_clear;
	ppc_md.flush_hash_range = native_flush_hash_range;
	ppc_md.hugepage_invalidate   = native_hugepage_invalidate;
}
