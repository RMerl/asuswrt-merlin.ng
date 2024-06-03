/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Cache-handling routined for MIPS CPUs
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>
#include <asm/cm.h>

	.macro	f_fill64 dst, offset, val
	LONG_S	\val, (\offset +  0 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  1 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  2 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  3 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  4 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  5 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  6 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  7 * LONGSIZE)(\dst)
#if LONGSIZE == 4
	LONG_S	\val, (\offset +  8 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  9 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 10 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 11 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 12 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 13 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 14 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 15 * LONGSIZE)(\dst)
#endif
	.endm

	.macro cache_loop	curr, end, line_sz, op
10:	cache		\op, 0(\curr)
	PTR_ADDU	\curr, \curr, \line_sz
	bne		\curr, \end, 10b
	.endm

	.macro	l1_info		sz, line_sz, off
	.set	push
	.set	noat

	mfc0	$1, CP0_CONFIG, 1

	/* detect line size */
	srl	\line_sz, $1, \off + MIPS_CONF1_DL_SHF - MIPS_CONF1_DA_SHF
	andi	\line_sz, \line_sz, (MIPS_CONF1_DL >> MIPS_CONF1_DL_SHF)
	move	\sz, zero
	beqz	\line_sz, 10f
	li	\sz, 2
	sllv	\line_sz, \sz, \line_sz

	/* detect associativity */
	srl	\sz, $1, \off + MIPS_CONF1_DA_SHF - MIPS_CONF1_DA_SHF
	andi	\sz, \sz, (MIPS_CONF1_DA >> MIPS_CONF1_DA_SHF)
	addiu	\sz, \sz, 1

	/* sz *= line_sz */
	mul	\sz, \sz, \line_sz

	/* detect log32(sets) */
	srl	$1, $1, \off + MIPS_CONF1_DS_SHF - MIPS_CONF1_DA_SHF
	andi	$1, $1, (MIPS_CONF1_DS >> MIPS_CONF1_DS_SHF)
	addiu	$1, $1, 1
	andi	$1, $1, 0x7

	/* sz <<= log32(sets) */
	sllv	\sz, \sz, $1

	/* sz *= 32 */
	li	$1, 32
	mul	\sz, \sz, $1
10:
	.set	pop
	.endm

/*
 * mips_cache_reset - low level initialisation of the primary caches
 *
 * This routine initialises the primary caches to ensure that they have good
 * parity.  It must be called by the ROM before any cached locations are used
 * to prevent the possibility of data with bad parity being written to memory.
 *
 * To initialise the instruction cache it is essential that a source of data
 * with good parity is available. This routine will initialise an area of
 * memory starting at location zero to be used as a source of parity.
 *
 * Note that this function does not follow the standard calling convention &
 * may clobber typically callee-saved registers.
 *
 * RETURNS: N/A
 *
 */
#define R_RETURN	s0
#define R_IC_SIZE	s1
#define R_IC_LINE	s2
#define R_DC_SIZE	s3
#define R_DC_LINE	s4
#define R_L2_SIZE	s5
#define R_L2_LINE	s6
#define R_L2_BYPASSED	s7
#define R_L2_L2C	t8
LEAF(mips_cache_reset)
	move	R_RETURN, ra

#ifdef CONFIG_MIPS_L2_CACHE
	/*
	 * For there to be an L2 present, Config2 must be present. If it isn't
	 * then we proceed knowing there's no L2 cache.
	 */
	move	R_L2_SIZE, zero
	move	R_L2_LINE, zero
	move	R_L2_BYPASSED, zero
	move	R_L2_L2C, zero
	mfc0	t0, CP0_CONFIG, 1
	bgez	t0, l2_probe_done

	/*
	 * From MIPSr6 onwards the L2 cache configuration might not be reported
	 * by Config2. The Config5.L2C bit indicates whether this is the case,
	 * and if it is then we need knowledge of where else to look. For cores
	 * from Imagination Technologies this is a CM GCR.
	 */
# if __mips_isa_rev >= 6
	/* Check that Config5 exists */
	mfc0	t0, CP0_CONFIG, 2
	bgez	t0, l2_probe_cop0
	mfc0	t0, CP0_CONFIG, 3
	bgez	t0, l2_probe_cop0
	mfc0	t0, CP0_CONFIG, 4
	bgez	t0, l2_probe_cop0

	/* Check Config5.L2C is set */
	mfc0	t0, CP0_CONFIG, 5
	and	R_L2_L2C, t0, MIPS_CONF5_L2C
	beqz	R_L2_L2C, l2_probe_cop0

	/* Config5.L2C is set */
#  ifdef CONFIG_MIPS_CM
	/* The CM will provide L2 configuration */
	PTR_LI	t0, CKSEG1ADDR(CONFIG_MIPS_CM_BASE)
	lw	t1, GCR_L2_CONFIG(t0)
	bgez	t1, l2_probe_done

	ext	R_L2_LINE, t1, \
		GCR_L2_CONFIG_LINESZ_SHIFT, GCR_L2_CONFIG_LINESZ_BITS
	beqz	R_L2_LINE, l2_probe_done
	li	t2, 2
	sllv	R_L2_LINE, t2, R_L2_LINE

	ext	t2, t1, GCR_L2_CONFIG_ASSOC_SHIFT, GCR_L2_CONFIG_ASSOC_BITS
	addiu	t2, t2, 1
	mul	R_L2_SIZE, R_L2_LINE, t2

	ext	t2, t1, GCR_L2_CONFIG_SETSZ_SHIFT, GCR_L2_CONFIG_SETSZ_BITS
	sllv	R_L2_SIZE, R_L2_SIZE, t2
	li	t2, 64
	mul	R_L2_SIZE, R_L2_SIZE, t2

	/* Bypass the L2 cache so that we can init the L1s early */
	or	t1, t1, GCR_L2_CONFIG_BYPASS
	sw	t1, GCR_L2_CONFIG(t0)
	sync
	li	R_L2_BYPASSED, 1

	/* Zero the L2 tag registers */
	sw	zero, GCR_L2_TAG_ADDR(t0)
	sw	zero, GCR_L2_TAG_ADDR_UPPER(t0)
	sw	zero, GCR_L2_TAG_STATE(t0)
	sw	zero, GCR_L2_TAG_STATE_UPPER(t0)
	sw	zero, GCR_L2_DATA(t0)
	sw	zero, GCR_L2_DATA_UPPER(t0)
	sync
#  else
	/* We don't know how to retrieve L2 configuration on this system */
#  endif
	b	l2_probe_done
# endif

	/*
	 * For pre-r6 systems, or r6 systems with Config5.L2C==0, probe the L2
	 * cache configuration from the cop0 Config2 register.
	 */
l2_probe_cop0:
	mfc0	t0, CP0_CONFIG, 2

	srl	R_L2_LINE, t0, MIPS_CONF2_SL_SHF
	andi	R_L2_LINE, R_L2_LINE, MIPS_CONF2_SL >> MIPS_CONF2_SL_SHF
	beqz	R_L2_LINE, l2_probe_done
	li	t1, 2
	sllv	R_L2_LINE, t1, R_L2_LINE

	srl	t1, t0, MIPS_CONF2_SA_SHF
	andi	t1, t1, MIPS_CONF2_SA >> MIPS_CONF2_SA_SHF
	addiu	t1, t1, 1
	mul	R_L2_SIZE, R_L2_LINE, t1

	srl	t1, t0, MIPS_CONF2_SS_SHF
	andi	t1, t1, MIPS_CONF2_SS >> MIPS_CONF2_SS_SHF
	sllv	R_L2_SIZE, R_L2_SIZE, t1
	li	t1, 64
	mul	R_L2_SIZE, R_L2_SIZE, t1

	/* Attempt to bypass the L2 so that we can init the L1s early */
	or	t0, t0, MIPS_CONF2_L2B
	mtc0	t0, CP0_CONFIG, 2
	ehb
	mfc0	t0, CP0_CONFIG, 2
	and	R_L2_BYPASSED, t0, MIPS_CONF2_L2B

	/* Zero the L2 tag registers */
	mtc0	zero, CP0_TAGLO, 4
	ehb
l2_probe_done:
#endif

#ifndef CONFIG_SYS_CACHE_SIZE_AUTO
	li	R_IC_SIZE, CONFIG_SYS_ICACHE_SIZE
	li	R_IC_LINE, CONFIG_SYS_ICACHE_LINE_SIZE
#else
	l1_info	R_IC_SIZE, R_IC_LINE, MIPS_CONF1_IA_SHF
#endif

#ifndef CONFIG_SYS_CACHE_SIZE_AUTO
	li	R_DC_SIZE, CONFIG_SYS_DCACHE_SIZE
	li	R_DC_LINE, CONFIG_SYS_DCACHE_LINE_SIZE
#else
	l1_info	R_DC_SIZE, R_DC_LINE, MIPS_CONF1_DA_SHF
#endif

#ifdef CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD

	/* Determine the largest L1 cache size */
#ifndef CONFIG_SYS_CACHE_SIZE_AUTO
#if CONFIG_SYS_ICACHE_SIZE > CONFIG_SYS_DCACHE_SIZE
	li	v0, CONFIG_SYS_ICACHE_SIZE
#else
	li	v0, CONFIG_SYS_DCACHE_SIZE
#endif
#else
	move	v0, R_IC_SIZE
	sltu	t1, R_IC_SIZE, R_DC_SIZE
	movn	v0, R_DC_SIZE, t1
#endif
	/*
	 * Now clear that much memory starting from zero.
	 */
	PTR_LI		a0, CKSEG1ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
	PTR_ADDU	a1, a0, v0
2:	PTR_ADDIU	a0, 64
	f_fill64	a0, -64, zero
	bne		a0, a1, 2b

#endif /* CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD */

#ifdef CONFIG_MIPS_L2_CACHE
	/*
	 * If the L2 is bypassed, init the L1 first so that we can execute the
	 * rest of the cache initialisation using the L1 instruction cache.
	 */
	bnez		R_L2_BYPASSED, l1_init

l2_init:
	PTR_LI		t0, CKSEG0ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
	PTR_ADDU	t1, t0, R_L2_SIZE
1:	cache		INDEX_STORE_TAG_SD, 0(t0)
	PTR_ADDU	t0, t0, R_L2_LINE
	bne		t0, t1, 1b

	/*
	 * If the L2 was bypassed then we already initialised the L1s before
	 * the L2, so we are now done.
	 */
	bnez		R_L2_BYPASSED, l2_unbypass
#endif

	/*
	 * The TagLo registers used depend upon the CPU implementation, but the
	 * architecture requires that it is safe for software to write to both
	 * TagLo selects 0 & 2 covering supported cases.
	 */
l1_init:
	mtc0		zero, CP0_TAGLO
	mtc0		zero, CP0_TAGLO, 2
	ehb

	/*
	 * The caches are probably in an indeterminate state, so we force good
	 * parity into them by doing an invalidate for each line. If
	 * CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD is set then we'll proceed to
	 * perform a load/fill & a further invalidate for each line, assuming
	 * that the bottom of RAM (having just been cleared) will generate good
	 * parity for the cache.
	 */

	/*
	 * Initialize the I-cache first,
	 */
	blez		R_IC_SIZE, 1f
	PTR_LI		t0, CKSEG0ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
	PTR_ADDU	t1, t0, R_IC_SIZE
	/* clear tag to invalidate */
	cache_loop	t0, t1, R_IC_LINE, INDEX_STORE_TAG_I
#ifdef CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD
	/* fill once, so data field parity is correct */
	PTR_LI		t0, CKSEG0ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
	cache_loop	t0, t1, R_IC_LINE, FILL
	/* invalidate again - prudent but not strictly neccessary */
	PTR_LI		t0, CKSEG0ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
	cache_loop	t0, t1, R_IC_LINE, INDEX_STORE_TAG_I
#endif
	sync

	/*
	 * Enable use of the I-cache by setting Config.K0. The code for this
	 * must be executed from KSEG1. Jump from KSEG0 to KSEG1 to do this.
	 * Jump back to KSEG0 after caches are enabled and insert an
	 * instruction hazard barrier.
	 */
	PTR_LA		t0, change_k0_cca
	li		t1, CPHYSADDR(~0)
	and		t0, t0, t1
	PTR_LI		t1, CKSEG1
	or		t0, t0, t1
	li		a0, CONF_CM_CACHABLE_NONCOHERENT
	jalr.hb		t0

	/*
	 * then initialize D-cache.
	 */
1:	blez		R_DC_SIZE, 3f
	PTR_LI		t0, CKSEG0ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
	PTR_ADDU	t1, t0, R_DC_SIZE
	/* clear all tags */
	cache_loop	t0, t1, R_DC_LINE, INDEX_STORE_TAG_D
#ifdef CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD
	/* load from each line (in cached space) */
	PTR_LI		t0, CKSEG0ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
2:	LONG_L		zero, 0(t0)
	PTR_ADDU	t0, R_DC_LINE
	bne		t0, t1, 2b
	/* clear all tags */
	PTR_LI		t0, CKSEG0ADDR(CONFIG_MIPS_CACHE_INDEX_BASE)
	cache_loop	t0, t1, R_DC_LINE, INDEX_STORE_TAG_D
#endif
3:

#ifdef CONFIG_MIPS_L2_CACHE
	/* If the L2 isn't bypassed then we're done */
	beqz		R_L2_BYPASSED, return

	/* The L2 is bypassed - go initialise it */
	b		l2_init

l2_unbypass:
# if __mips_isa_rev >= 6
	beqz		R_L2_L2C, 1f

	li		t0, CKSEG1ADDR(CONFIG_MIPS_CM_BASE)
	lw		t1, GCR_L2_CONFIG(t0)
	xor		t1, t1, GCR_L2_CONFIG_BYPASS
	sw		t1, GCR_L2_CONFIG(t0)
	sync
	ehb
	b		2f
# endif
1:	mfc0		t0, CP0_CONFIG, 2
	xor		t0, t0, MIPS_CONF2_L2B
	mtc0		t0, CP0_CONFIG, 2
	ehb

2:
# ifdef CONFIG_MIPS_CM
	/* Config3 must exist for a CM to be present */
	mfc0		t0, CP0_CONFIG, 1
	bgez		t0, 2f
	mfc0		t0, CP0_CONFIG, 2
	bgez		t0, 2f

	/* Check Config3.CMGCR to determine CM presence */
	mfc0		t0, CP0_CONFIG, 3
	and		t0, t0, MIPS_CONF3_CMGCR
	beqz		t0, 2f

	/* Change Config.K0 to a coherent CCA */
	PTR_LA		t0, change_k0_cca
	li		a0, CONF_CM_CACHABLE_COW
	jalr		t0

	/*
	 * Join the coherent domain such that the caches of this core are kept
	 * coherent with those of other cores.
	 */
	PTR_LI		t0, CKSEG1ADDR(CONFIG_MIPS_CM_BASE)
	lw		t1, GCR_REV(t0)
	li		t2, GCR_REV_CM3
	li		t3, GCR_Cx_COHERENCE_EN
	bge		t1, t2, 1f
	li		t3, GCR_Cx_COHERENCE_DOM_EN
1:	sw		t3, GCR_Cx_COHERENCE(t0)
	ehb
2:
# endif
#endif

return:
	/* Ensure all cache operations complete before returning */
	sync
	jr	R_RETURN
	END(mips_cache_reset)

LEAF(change_k0_cca)
	mfc0		t0, CP0_CONFIG
#if __mips_isa_rev >= 2
	ins		t0, a0, 0, 3
#else
	xor		a0, a0, t0
	andi		a0, a0, CONF_CM_CMASK
	xor		a0, a0, t0
#endif
	mtc0		a0, CP0_CONFIG

	jr.hb		ra
	END(change_k0_cca)
