#include <config.h>
#include <mpc86xx.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#ifndef CACHE_LINE_SIZE
# define CACHE_LINE_SIZE L1_CACHE_BYTES
#endif

#if CACHE_LINE_SIZE == 128
#define LG_CACHE_LINE_SIZE 7
#elif CACHE_LINE_SIZE == 32
#define LG_CACHE_LINE_SIZE 5
#elif CACHE_LINE_SIZE == 16
#define LG_CACHE_LINE_SIZE 4
#elif CACHE_LINE_SIZE == 8
#define LG_CACHE_LINE_SIZE 3
#else
# error "Invalid cache line size!"
#endif

/*
 * Most of this code is taken from 74xx_7xx/cache.S
 * and then cleaned up a bit
 */

/*
 * Invalidate L1 instruction cache.
 */
_GLOBAL(invalidate_l1_instruction_cache)
	/* use invalidate-all bit in HID0 */
	mfspr	r3,HID0
	ori	r3,r3,HID0_ICFI
	mtspr	HID0,r3
	isync
	blr

/*
 * Invalidate L1 data cache.
 */
_GLOBAL(invalidate_l1_data_cache)
	mfspr	r3,HID0
	ori	r3,r3,HID0_DCFI
	mtspr	HID0,r3
	isync
	blr

/*
 * Flush data cache.
 */
_GLOBAL(flush_dcache)
	lis	r3,0
	lis	r5,CACHE_LINE_SIZE
flush:
	cmp	0,1,r3,r5
	bge	done
	lwz	r5,0(r3)
	lis	r5,CACHE_LINE_SIZE
	addi	r3,r3,0x4
	b	flush
done:
	blr
/*
 * Write any modified data cache blocks out to memory
 * and invalidate the corresponding instruction cache blocks.
 * This is a no-op on the 601.
 *
 * flush_icache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(flush_icache_range)
	li	r5,CACHE_LINE_SIZE-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,LG_CACHE_LINE_SIZE
	beqlr
	mtctr	r4
	mr	r6,r3
1:	dcbst	0,r3
	addi	r3,r3,CACHE_LINE_SIZE
	bdnz	1b
	sync				/* wait for dcbst's to get to ram */
	mtctr	r4
2:	icbi	0,r6
	addi	r6,r6,CACHE_LINE_SIZE
	bdnz	2b
	sync				/* additional sync needed on g4 */
	isync
	blr
/*
 * Write any modified data cache blocks out to memory.
 * Does not invalidate the corresponding cache lines (especially for
 * any corresponding instruction cache).
 *
 * clean_dcache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(clean_dcache_range)
	li	r5,CACHE_LINE_SIZE-1
	andc	r3,r3,r5	/* align r3 down to cache line */
	subf	r4,r3,r4	/* r4 = offset of stop from start of cache line */
	add	r4,r4,r5	/* r4 += cache_line_size-1 */
	srwi.	r4,r4,LG_CACHE_LINE_SIZE  /* r4 = number of cache lines to flush */
	beqlr				  /* if r4 == 0 return */
	mtctr	r4			  /* ctr = r4 */

	sync
1:	dcbst	0,r3
	addi	r3,r3,CACHE_LINE_SIZE
	bdnz	1b
	sync				/* wait for dcbst's to get to ram */
	blr

/*
 * Flush a particular page from the data cache to RAM.
 * Note: this is necessary because the instruction cache does *not*
 * snoop from the data cache.
 *
 *	void __flush_page_to_ram(void *page)
 */
_GLOBAL(__flush_page_to_ram)
	rlwinm	r3,r3,0,0,19		/* Get page base address */
	li	r4,4096/CACHE_LINE_SIZE	/* Number of lines in a page */
	mtctr	r4
	mr	r6,r3
0:	dcbst	0,r3			/* Write line to ram */
	addi	r3,r3,CACHE_LINE_SIZE
	bdnz	0b
	sync
	mtctr	r4
1:	icbi	0,r6
	addi	r6,r6,CACHE_LINE_SIZE
	bdnz	1b
	sync
	isync
	blr

/*
 * Flush a particular page from the instruction cache.
 * Note: this is necessary because the instruction cache does *not*
 * snoop from the data cache.
 *
 *	void __flush_icache_page(void *page)
 */
_GLOBAL(__flush_icache_page)
	li	r4,4096/CACHE_LINE_SIZE	/* Number of lines in a page */
	mtctr	r4
1:	icbi	0,r3
	addi	r3,r3,CACHE_LINE_SIZE
	bdnz	1b
	sync
	isync
	blr

/*
 * Clear a page using the dcbz instruction, which doesn't cause any
 * memory traffic (except to write out any cache lines which get
 * displaced).  This only works on cacheable memory.
 */
_GLOBAL(clear_page)
	li	r0,4096/CACHE_LINE_SIZE
	mtctr	r0
1:	dcbz	0,r3
	addi	r3,r3,CACHE_LINE_SIZE
	bdnz	1b
	blr

/*
 * Enable L1 Instruction cache
 */
_GLOBAL(icache_enable)
	mfspr	r3, HID0
	li	r5, HID0_ICFI|HID0_ILOCK
	andc	r3, r3, r5
	ori	r3, r3, HID0_ICE
	ori	r5, r3, HID0_ICFI
	mtspr	HID0, r5
	mtspr	HID0, r3
	isync
	blr

/*
 * Disable L1 Instruction cache
 */
_GLOBAL(icache_disable)
	mflr	r4
	bl	invalidate_l1_instruction_cache		/* uses r3 */
	sync
	mtlr	r4
	mfspr	r3, HID0
	li	r5, 0
	ori	r5, r5, HID0_ICE
	andc	r3, r3, r5
	mtspr	HID0, r3
	isync
	blr

/*
 * Is instruction cache enabled?
 */
_GLOBAL(icache_status)
	mfspr	r3, HID0
	andi.	r3, r3, HID0_ICE
	blr


_GLOBAL(l1dcache_enable)
	mfspr	r3, HID0
	li	r5, HID0_DCFI|HID0_DLOCK
	andc	r3, r3, r5
	mtspr	HID0, r3		/* no invalidate, unlock */
	ori	r3, r3, HID0_DCE
	ori	r5, r3, HID0_DCFI
	mtspr	HID0, r5		/* enable + invalidate */
	mtspr	HID0, r3		/* enable */
	sync
	blr

/*
 * Enable data cache(s) - L1 and optionally L2
 * Calls l2cache_enable. LR saved in r5
 */
_GLOBAL(dcache_enable)
	mfspr	r3, HID0
	li	r5, HID0_DCFI|HID0_DLOCK
	andc	r3, r3, r5
	mtspr	HID0, r3		/* no invalidate, unlock */
	ori	r3, r3, HID0_DCE
	ori	r5, r3, HID0_DCFI
	mtspr	HID0, r5		/* enable + invalidate */
	mtspr	HID0, r3		/* enable */
	sync
#ifdef CONFIG_SYS_L2
	mflr	r5
	bl	l2cache_enable		/* uses r3 and r4 */
	sync
	mtlr	r5
#endif
	blr


/*
 * Disable data cache(s) - L1 and optionally L2
 * Calls flush_dcache and l2cache_disable_no_flush.
 * LR saved in r4
 */
_GLOBAL(dcache_disable)
	mflr	r4			/* save link register */
	bl	flush_dcache	/* uses r3 and r5 */
	sync
	mfspr	r3, HID0
	li	r5, HID0_DCFI|HID0_DLOCK
	andc	r3, r3, r5
	mtspr	HID0, r3		/* no invalidate, unlock */
	li	r5, HID0_DCE|HID0_DCFI
	andc	r3, r3, r5		/* no enable, no invalidate */
	mtspr	HID0, r3
	sync
#ifdef CONFIG_SYS_L2
	bl	l2cache_disable_no_flush /* uses r3 */
#endif
	mtlr	r4			/* restore link register */
	blr

/*
 * Is data cache enabled?
 */
_GLOBAL(dcache_status)
	mfspr	r3, HID0
	andi.	r3, r3, HID0_DCE
	blr

/*
 * Invalidate L2 cache using L2I, assume L2 is enabled
 */
_GLOBAL(l2cache_invalidate)
	mfspr	r3, l2cr
	rlwinm.	r3, r3, 0, 0, 0
	beq	1f

	mfspr	r3, l2cr
	rlwinm	r3, r3, 0, 1, 31

#ifdef	CONFIG_ALTIVEC
	dssall
#endif
	sync
	mtspr	l2cr, r3
	sync
1:	mfspr	r3, l2cr
	oris	r3, r3, L2CR_L2I@h
	mtspr	l2cr, r3

invl2:
	mfspr	r3, l2cr
	andis.	r3, r3, L2CR_L2I@h
	bne	invl2
	blr

/*
 * Enable L2 cache
 * Calls l2cache_invalidate. LR is saved in r4
 */
_GLOBAL(l2cache_enable)
	mflr	r4			/* save link register */
	bl	l2cache_invalidate	/* uses r3 */
	sync
	lis	r3, L2_ENABLE@h
	ori	r3, r3, L2_ENABLE@l
	mtspr	l2cr, r3
	isync
	mtlr	r4			/* restore link register */
	blr

/*
 * Disable L2 cache
 * Calls flush_dcache. LR is saved in r4
 */
_GLOBAL(l2cache_disable)
	mflr	r4			/* save link register */
	bl	flush_dcache		/* uses r3 and r5 */
	sync
	mtlr	r4			/* restore link register */
l2cache_disable_no_flush:		/* provide way to disable L2 w/o flushing */
	lis	r3, L2_INIT@h
	ori	r3, r3, L2_INIT@l
	mtspr	l2cr, r3
	isync
	blr
