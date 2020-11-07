#if defined(CONFIG_BCM_KF_NBUFF)

#ifndef __NBUFF_H_INCLUDED__
#define __NBUFF_H_INCLUDED__


/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

/*
 *******************************************************************************
 *
 * File Name  : nbuff.h
 * Description: Definition of a network buffer to support various forms of
 *      network buffer, to include Linux socket buff (SKB), lightweight
 *      fast kernel buff (FKB), BRCM Free Pool buffer (FPB), and traffic
 *      generator support buffer (TGB)
 *
 *      nbuff.h may also be used to provide an interface to common APIs 
 *      available on other OS (in particular BSD style mbuf).
 *
 * Common APIs provided: pushing, pulling, reading, writing, cloning, freeing
 *
 * Implementation Note:
 *
 * One may view NBuff as a base class from which other buff types are derived.
 * Examples of derived network buffer types are sk_buff, fkbuff, fpbuff, tgbuff
 *
 * A pointer to a buffer is converted to a pointer to a special (derived) 
 * network buffer type by encoding the type into the least significant 2 bits
 * of a word aligned buffer pointer. pBuf points to the real network 
 * buffer and pNBuff refers to pBuf ANDed with the Network Buffer Type.
 * C++ this pointer to a virtual class (vtable based virtual function thunks).
 *
 * Thunk functions to redirect the calls to the appropriate buffer type, e.g.
 * SKB or FKB uses the Network Buffer Pointer type information.
 *
 * This file also implements the Fast Kernel Buffer API. The fast kernel buffer
 * carries a minimal context of the received buffer and associated buffer
 * recycling information.
 *
 ******************************************************************************* */

#include <linux/version.h>
#include <generated/autoconf.h>
#include <linux/types.h>            /* include ISO C99 inttypes.h             */
#include <linux/skbuff.h>           /* include corresponding BSD style mbuf   */
#include <linux/blog.h>
#include <linux/blog_net.h> /*TODO rename this file as bcm_net.h as it's not specific to blog */
#include <bcm_pkt_lengths.h>
#include <linux/netdevice.h>

#define NBUFF_VERSION              "v1.0"

/* Engineering Constants for Fast Kernel Buffer Global Pool (used for clones) */
#define SUPPORT_FKB_EXTEND
#if defined(CONFIG_BCM_KF_WL)
#define FKBC_POOL_SIZE_ENGG         (2080)  /*1280 more to be allocated for wireless*/
#else
#define FKBC_POOL_SIZE_ENGG         800
#endif
#define FKBC_EXTEND_SIZE_ENGG       32      /* Number of FkBuf_t per extension*/
#define FKBC_EXTEND_MAX_ENGG        16      /* Maximum extensions allowed     */

#define FKBM_POOL_SIZE_ENGG         128
#define FKBM_EXTEND_SIZE_ENGG       2
#define FKBM_EXTEND_MAX_ENGG        200     /* Assuming one unshare           */

/*
 * Network device drivers ported to NBUFF must ensure that the headroom is at
 * least 186 bytes in size. Remove this dependancy (TBD).
 */
// #define CC_FKB_HEADROOM_AUDIT

/* Conditional compile of FKB functional APIs as inlined or non-inlined */
#define CC_CONFIG_FKB_FN_INLINE
#ifdef CC_CONFIG_FKB_FN_INLINE
#define FKB_FN(fn_name, fn_signature, body)                                    \
static inline fn_signature { body; }    /* APIs inlined in header file */
#else
#ifdef FKB_IMPLEMENTATION_FILE
#define FKB_FN(fn_name, fn_signature, body)                                    \
fn_signature { body; }                                                         \
EXPORT_SYMBOL(fn_name);                 /* APIs declared in implementation */
#else
#define FKB_FN(fn_name, fn_signature, body)                                    \
extern fn_signature;
#endif  /* !defined(FKB_IMPLEMENTATION_FILE) */
#endif  /* !defined(FKB_FN) */

/* LAB ONLY: Design development */
// #define CC_CONFIG_FKB_STATS
// #define CC_CONFIG_FKB_COLOR
// #define CC_CONFIG_FKB_DEBUG
// #define CC_CONFIG_FKB_AUDIT
// #define CC_CONFIG_FKB_STACK

// #include <linux/smp.h>       /* smp_processor_id() CC_CONFIG_FKB_AUDIT */

#if defined(CC_CONFIG_FKB_STATS)
#define FKB_STATS(stats_code)   do { stats_code } while(0)
#else
#define FKB_STATS(stats_code)   NULL_STMT
#endif

#if defined(CC_CONFIG_FKB_STACK)
extern void dump_stack(void);
#define DUMP_STACK()            dump_stack()
#else
#define DUMP_STACK()            NULL_STMT
#endif

#if defined(CC_CONFIG_FKB_AUDIT)
#define FKB_AUDIT(audit_code)   do { audit_code } while(0)
#else
#define FKB_AUDIT(audit_code)   NULL_STMT
#endif

extern int nbuff_dbg;
#if defined(CC_CONFIG_FKB_DEBUG)
#define fkb_dbg(lvl, fmt, arg...) \
    if (nbuff_dbg >= lvl) printk( "FKB %s :" fmt "[<%p>]\n", \
        __FUNCTION__, ##arg, __builtin_return_address(0) )
#define FKB_DBG(debug_code)     do { debug_code } while(0)
#else
#define fkb_dbg(lvl, fmt, arg...)      do {} while(0)
#define FKB_DBG(debug_code)     NULL_STMT
#endif

#define CC_NBUFF_FLUSH_OPTIMIZATION

/* CACHE OPERATIONS */
#define FKB_CACHE_FLUSH         0
#define FKB_CACHE_INV           1

/* OS Specific Section Begin */
#if defined(__KERNEL__)     /* Linux MIPS Cache Specific */
/*
 *------------------------------------------------------------------------------
 * common cache operations:
 *
 * - addr is rounded down to the cache line
 * - end is rounded up to cache line.
 *
 * - if ((addr == end) and (addr was cache aligned before rounding))
 *       no operation is performed.
 *   else
 *       flush data cache line UPTO but NOT INCLUDING rounded up end.
 *
 * Note:
 * if before rounding, (addr == end)  AND addr was not cache aligned,
 *      we would flush at least one line.
 *
 * Uses: L1_CACHE_BYTES
 *------------------------------------------------------------------------------
 */
#include <asm/cache.h>
#ifdef CONFIG_MIPS
#include <asm/r4kcache.h>
#endif  /* CONFIG_MIPS */

extern void cache_flush_data_len(void *addr, int len);

/*
 * Macros to round down and up, an address to a cachealigned address
 */
#define ADDR_ALIGN_DN(addr, align)  ( (addr) & ~((align) - 1) )
#define ADDR_ALIGN_UP(addr, align)  ( ((addr) + (align) - 1) & ~((align) - 1) )

#ifdef CONFIG_MIPS
/*
 *------------------------------------------------------------------------------
 * Function   : cache_flush_region
 * Description: 
 * Writeback flush, then invalidate a region demarcated by addr to end.
 * Cache line following rounded up end is not flushed.
 *------------------------------------------------------------------------------
 */
static inline void cache_flush_region(void *addr, void *end)
{
    uintptr_t a = ADDR_ALIGN_DN( (uintptr_t)addr, L1_CACHE_BYTES );
    uintptr_t e = ADDR_ALIGN_UP( (uintptr_t)end, L1_CACHE_BYTES );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += L1_CACHE_BYTES;    /* next cache line base */
    }
}

/*
 *------------------------------------------------------------------------------
 * Function   : cache_flush_len
 * Description: 
 * Writeback flush, then invalidate a region given an address and a length.
 * The demarcation end is computed by applying length to address before
 * rounding down address. End is rounded up.
 * Cache line following rounded up end is not flushed.
 *------------------------------------------------------------------------------
 */
static inline void cache_flush_len(void *addr, int len)
{
    uintptr_t a = ADDR_ALIGN_DN( (uintptr_t)addr, L1_CACHE_BYTES );
    uintptr_t e = ADDR_ALIGN_UP( ((uintptr_t)addr + len),
                                     L1_CACHE_BYTES );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += L1_CACHE_BYTES;    /* next cache line base */
    }
}

static inline void fpm_cache_flush_len(void *addr, int len)
{
    /* No global coherency support for MIPS, always perform flush */
    cache_flush_len(addr, len);
}

/*
 *------------------------------------------------------------------------------
 * Function   : cache_invalidate_region
 * Description: 
 * invalidate a region demarcated by addr to end.
 * Cache line following rounded up end is not invalidateed.
 *------------------------------------------------------------------------------
 */
static inline void cache_invalidate_region(void *addr, void *end)
{
    uintptr_t a = ADDR_ALIGN_DN( (uintptr_t)addr, L1_CACHE_BYTES );
    uintptr_t e = ADDR_ALIGN_UP( (uintptr_t)end, L1_CACHE_BYTES );
    while ( a < e )
    {
        invalidate_dcache_line(a);   /* Hit_Invalidate_D */
        a += L1_CACHE_BYTES;    /* next cache line base */
    }
}

/*
 *------------------------------------------------------------------------------
 * Function   : cache_invalidate_len
 * Description: 
 * invalidate a region given an address and a length.
 * The demarcation end is computed by applying length to address before
 * rounding down address. End is rounded up.
 * Cache line following rounded up end is not invalidateed.
 *------------------------------------------------------------------------------
 */
static inline void cache_invalidate_len(void *addr, int len)
{
    uintptr_t a = ADDR_ALIGN_DN( (uintptr_t)addr, L1_CACHE_BYTES );
    uintptr_t e = ADDR_ALIGN_UP( ((uintptr_t)addr + len),
                                     L1_CACHE_BYTES );
    while ( a < e )
    {
        invalidate_dcache_line(a);   /* Hit_Invalidate_D */
        a += L1_CACHE_BYTES;    /* next cache line base */
    }
}

/*
 *------------------------------------------------------------------------------
 * Function   : _is_kptr_
 * Description: Test whether a variable can be a pointer to a kernel space.
 *              This form of variable overloading may only be used for denoting
 *              pointers to kernel space or as a variable where the most
 *              significant nibble is unused.
 *              In 32bit Linux kernel, a pointer to a KSEG0, KSEG1, KSEG2 will
 *              have 0x8, 0xA or 0xC in the most significant nibble.
 *------------------------------------------------------------------------------
 */
static inline uint32_t _is_kptr_(const void * vptr)
{
	return ( (uintptr_t)vptr > 0x0FFFFFFF );
}

#define cache_invalidate_region_outer_first(a, b)	cache_invalidate_region(a, b)
#define cache_invalidate_len_outer_first(a, b)		cache_invalidate_len(a, b)

#elif defined(CONFIG_ARM)


#include <asm/cacheflush.h>
#if defined(CONFIG_ARM_L1_CACHE_SHIFT)
#define L1_CACHE_LINE_SIZE	(0x1 << CONFIG_ARM_L1_CACHE_SHIFT)
#else
#warning There is no L1 cache line size defined!
#endif

#if defined(CONFIG_OUTER_CACHE)

#if defined(CONFIG_CACHE_L2X0)
#define L2_CACHE_LINE_SIZE	32
#endif

#if defined(L2_CACHE_LINE_SIZE) && (L1_CACHE_LINE_SIZE != L2_CACHE_LINE_SIZE)
#warning  L1 Cache line size is different from L2 cache line size!
#endif

#define CONFIG_OPTIMIZED_CACHE_FLUSH	1
#endif

static inline void cache_flush_len(void *addr, int len);
static inline void _cache_flush_len(void *addr, int len);

#ifdef CONFIG_BCM_GLB_COHERENCY
#define cache_invalidate_len_outer_first(virt_addr, len)
#define cache_invalidate_region_outer_first(virt_addr, end)
#define cache_invalidate_len(virt_addr, len)
#define cache_invalidate_region(virt_addr, end)
#define cache_flush_region(addr, end)
#else

/* the following functions are optimized that it does NOT support
 * HIGHMEM in 32-bit system, please make sure buffer allocated
 * are in memory zone 'Normal' or before */
static inline void cache_invalidate_len_outer_first(void *virt_addr, int len)
{
	uintptr_t start_vaddr = (uintptr_t)virt_addr;
	uintptr_t end_vaddr = start_vaddr + len;
#if defined(CONFIG_OUTER_CACHE)
	uintptr_t start_paddr = virt_to_phys(virt_addr);
	uintptr_t end_paddr = start_paddr + len;
#endif

#if defined(CONFIG_OUTER_CACHE)
	outer_spin_lock_irqsave();
#endif
	/* 1st, flush & invalidate if start addr and / or end addr are not
	 * cache line aligned */
	if (start_vaddr & (L1_CACHE_LINE_SIZE - 1)) {
		start_vaddr &= ~(L1_CACHE_LINE_SIZE - 1);
		__cpuc_flush_line(start_vaddr);
#if defined(CONFIG_OUTER_CACHE)
		dsb();
#endif
		start_vaddr += L1_CACHE_LINE_SIZE;
	}

#if defined(CONFIG_OUTER_CACHE)
	if (start_paddr & (L2_CACHE_LINE_SIZE - 1)) {
		start_paddr &= ~(L2_CACHE_LINE_SIZE - 1);
		outer_flush_line_no_lock(start_paddr);
		outer_sync_no_lock();
		start_paddr += L2_CACHE_LINE_SIZE;
	}
#endif

	if (end_vaddr & (L1_CACHE_LINE_SIZE - 1)) {
		end_vaddr &= ~(L1_CACHE_LINE_SIZE - 1);
		__cpuc_flush_line(end_vaddr);
#if defined(CONFIG_OUTER_CACHE)
		dsb();
#endif
	}

#if defined(CONFIG_OUTER_CACHE)
	if (end_paddr & (L2_CACHE_LINE_SIZE - 1)) {
		end_paddr &= ~(L2_CACHE_LINE_SIZE - 1);
		outer_flush_line_no_lock(end_paddr);
		outer_sync_no_lock();
	}
#endif

#if defined(CONFIG_OUTER_CACHE)
	/* now do the real invalidation jobs */
	while (start_paddr < end_paddr) {
		outer_inv_line_no_lock(start_paddr);
		start_paddr += L2_CACHE_LINE_SIZE;
	}
	outer_sync_no_lock();
#endif

	/* now do the real invalidation jobs */
	while (start_vaddr < end_vaddr) {
		__cpuc_inv_line(start_vaddr);
		start_vaddr += L1_CACHE_LINE_SIZE;
	}

	dsb();
#if defined(CONFIG_OUTER_CACHE)
	outer_spin_unlock_irqrestore();
#endif

	if ((len >= PAGE_SIZE) && (((uintptr_t)virt_addr & ~PAGE_MASK) == 0))
		set_bit(PG_dcache_clean, &phys_to_page(virt_to_phys(virt_addr))->flags);
}

static inline void cache_invalidate_region_outer_first(void *virt_addr, void *end)
{
	cache_invalidate_len_outer_first(virt_addr,
			(uintptr_t)end - (uintptr_t)virt_addr);
}

static inline void cache_invalidate_len(void *virt_addr, int len)
{
	uintptr_t start_vaddr = (uintptr_t)virt_addr;
	uintptr_t end_vaddr = start_vaddr + len;
#if defined(CONFIG_OUTER_CACHE)
	uintptr_t start_paddr = virt_to_phys(virt_addr);
	uintptr_t end_paddr = start_paddr + len;
#endif

#if defined(CONFIG_OUTER_CACHE)
	outer_spin_lock_irqsave();
#endif
	/* 1st, flush & invalidate if start addr and / or end addr are not
	 * cache line aligned */
	if (start_vaddr & (L1_CACHE_LINE_SIZE - 1)) {
		start_vaddr &= ~(L1_CACHE_LINE_SIZE - 1);
		__cpuc_flush_line(start_vaddr);
#if defined(CONFIG_OUTER_CACHE)
		dsb();
#endif
		start_vaddr += L1_CACHE_LINE_SIZE;
	}

#if defined(CONFIG_OUTER_CACHE)
	if (start_paddr & (L2_CACHE_LINE_SIZE - 1)) {
		start_paddr &= ~(L2_CACHE_LINE_SIZE - 1);
		outer_flush_line_no_lock(start_paddr);
		start_paddr += L2_CACHE_LINE_SIZE;
	}
#endif

	if (end_vaddr & (L1_CACHE_LINE_SIZE - 1)) {
		end_vaddr &= ~(L1_CACHE_LINE_SIZE - 1);
		__cpuc_flush_line(end_vaddr);
#if defined(CONFIG_OUTER_CACHE)
		dsb();
#endif
	}

#if defined(CONFIG_OUTER_CACHE)
	if (end_paddr & (L2_CACHE_LINE_SIZE - 1)) {
		end_paddr &= ~(L2_CACHE_LINE_SIZE - 1);
		outer_flush_line_no_lock(end_paddr);
	}
#endif

	/* now do the real invalidation jobs */
	while (start_vaddr < end_vaddr) {
		__cpuc_inv_line(start_vaddr);
#if defined(CONFIG_OUTER_CACHE)
		dsb();
		outer_inv_line_no_lock(start_paddr);
		start_paddr += L2_CACHE_LINE_SIZE;
#endif
		start_vaddr += L1_CACHE_LINE_SIZE;
	}
#if defined(CONFIG_OUTER_CACHE)
	outer_sync_no_lock();
	outer_spin_unlock_irqrestore();
#else
	dsb();
#endif

	if ((len >= PAGE_SIZE) && (((uintptr_t)virt_addr & ~PAGE_MASK) == 0))
		set_bit(PG_dcache_clean, &phys_to_page(virt_to_phys(virt_addr))->flags);
}

static inline void cache_invalidate_region(void *virt_addr, void *end)
{
	cache_invalidate_len(virt_addr,
			(uintptr_t)end - (uintptr_t)virt_addr);
}

static inline void cache_flush_region(void *addr, void *end)
{
	cache_flush_len(addr, (uintptr_t)end - (uintptr_t)addr);
}

#endif /* CONFIG_BCM_GLB_COHERENCY */

static inline void cache_flush_len(void *addr, int len)
{
#ifndef CONFIG_BCM_GLB_COHERENCY
    _cache_flush_len(addr, len);
#endif
}
static inline void fpm_cache_flush_len(void *addr, int len)
{
#if !defined(CONFIG_BCM_GLB_COHERENCY) || defined(CONFIG_BCM_FPM_COHERENCY_EXCLUDE)
    _cache_flush_len(addr, len);
#endif
}

static inline void _cache_flush_len(void *addr, int len)
{
	uintptr_t start_vaddr = (uintptr_t)addr & ~(L1_CACHE_LINE_SIZE - 1);
	uintptr_t end_vaddr = (uintptr_t)addr + len;
#if defined(CONFIG_OUTER_CACHE)
	uintptr_t start_paddr = (uintptr_t)virt_to_phys((void *)start_vaddr);
#endif

#if defined(CONFIG_OUTER_CACHE)
	outer_spin_lock_irqsave();
#endif
#if defined(CONFIG_OPTIMIZED_CACHE_FLUSH)
	/* this function has been optimized in a non-recommended way, if any
	 * type of packet error occurs, please try undefine
	 * CONFIG_OPTIMIZED_CACHE_FLUSH to use the recommended algorithm
	 * provided by ARM cache document.
	 * Usually, when we have multiple levels of cache, in a cache_flush
	 * case, we do L1_clean -> L2_clean -> L2_invalidate -> L1_clean
	 * -> L1_invalidate, we can optimize this sequence to L1_clean ->
	 * L2_flush -> L1_flush.  This is our original approach.  However,
	 * this will introduce 3 loops of cache operation.
	 * This optimized method will do L1_flush -> L2_flush.  This will only
	 * introduce 2 loops of cache operation, but it also puts us into
	 * danger that L2 cache might update L1 cache on the cache line
	 * that should have been invalidated. */

	while (start_vaddr < end_vaddr) {
		__cpuc_flush_line(start_vaddr);
		start_vaddr += L1_CACHE_LINE_SIZE;
#if defined(CONFIG_OUTER_CACHE)
		dsb();
		outer_flush_line_no_lock(start_paddr);
		start_paddr += L2_CACHE_LINE_SIZE;
#endif
	}
#if defined(CONFIG_OUTER_CACHE)
	outer_sync_no_lock();
#else
	wmb();
#endif
#else	/* the non-optimized cache_flush */
	while (start_vaddr < end_vaddr) {
#if defined(CONFIG_OUTER_CACHE)
		__cpuc_clean_line(start_vaddr);
		dsb();
		outer_flush_line_no_lock(start_paddr);
		start_paddr += L2_CACHE_LINE_SIZE;
		outer_sync_no_lock();
#endif
		__cpuc_flush_line(start_vaddr);
		start_vaddr += L1_CACHE_LINE_SIZE;
	}
	wmb();
#endif
#if defined(CONFIG_OUTER_CACHE)
	outer_spin_unlock_irqrestore();
#endif
}


static inline uint32_t _is_kptr_(const void * vptr)
{
	return ( (uintptr_t)vptr > 0x0FFFFFFF );
}

#elif defined(CONFIG_ARM64)

#define nbuff_flush_dcache_area(addr, len)		\
	__asm__ __volatile__ (				\
	"mov	x0, %0 \n"				\
	"mov	x1, %1 \n"				\
	"mrs	x3, ctr_el0 \n"				\
	"ubfm	x3, x3, #16, #19 \n"			\
	"mov	x2, #4 \n"				\
	"lsl	x2, x2, x3  \n"				\
	"add	x1, x0, x1 \n"				\
	"sub	x3, x2, #1 \n"				\
	"bic	x0, x0, x3 \n"				\
	"1:	dc	civac, x0 \n"			\
	"add	x0, x0, x2 \n"				\
	"cmp	x0, x1 \n"				\
	"b.lo	1b \n"					\
	"dsb	sy \n"					\
	: : "r" ((uintptr_t)addr), "r" ((uintptr_t)len) \
	: "x0", "x1", "x2", "x3", "cc")

#define nbuff_inval_dcache_range(start, end)		\
	__asm__ __volatile__ (				\
	"mov	x0, %0 \n"				\
	"mov	x1, %1 \n"				\
	"mrs	x3, ctr_el0 \n"				\
	"ubfm	x3, x3, #16, #19 \n"			\
	"mov	x2, #4 \n"				\
	"lsl	x2, x2, x3 \n"				\
	"sub	x3, x2, #1 \n"				\
	"tst	x1, x3 \n"				\
	"bic	x1, x1, x3 \n"				\
	"b.eq	1f \n"					\
	"dc	civac, x1 \n"				\
	"1:	tst	x0, x3 \n"			\
	"bic	x0, x0, x3 \n"				\
	"b.eq	2f \n"					\
	"dc	civac, x0 \n"				\
	"b	3f \n"					\
	"2:	dc	ivac, x0 \n"			\
	"3:	add	x0, x0, x2 \n"			\
	"cmp	x0, x1 \n"				\
	"b.lo	2b \n"					\
	"dsb	sy \n"					\
	: : "r" ((uintptr_t)start), "r" ((uintptr_t)end)\
	: "x0", "x1", "x2", "x3", "cc")

static inline void cache_flush_region(void *addr, void *end)
{
#ifndef CONFIG_BCM_GLB_COHERENCY
	nbuff_flush_dcache_area(addr, (uintptr_t)end - (uintptr_t)addr);
#endif
}

static inline void cache_flush_len(void *addr, int len)
{
#ifndef CONFIG_BCM_GLB_COHERENCY
	nbuff_flush_dcache_area(addr, len);
#endif
}

static inline void fpm_cache_flush_len(void *addr, int len)
{
#if !defined(CONFIG_BCM_GLB_COHERENCY) || defined(CONFIG_BCM_FPM_COHERENCY_EXCLUDE)
	nbuff_flush_dcache_area(addr, len);
#endif
}

static inline void cache_invalidate_region(void *addr, void *end)
{
#ifndef CONFIG_BCM_GLB_COHERENCY
	nbuff_inval_dcache_range(addr, end);
#endif
}

static inline void cache_invalidate_len(void *addr, int len)
{
#ifndef CONFIG_BCM_GLB_COHERENCY
	nbuff_inval_dcache_range(addr, (void*)((uintptr_t)addr+len));
#endif
}

#define cache_invalidate_region_outer_first(a, b)	cache_invalidate_region(a, b)
#define cache_invalidate_len_outer_first(a, b)		cache_invalidate_len(a, b)

static inline uint32_t _is_kptr_(const void * vptr)
{
	return ( (uintptr_t)vptr > 0xFFFFFF8000000000 );
}
#endif

#endif  /* defined(__KERNEL__) Linux MIPS Cache Specific */
/* OS Specific Section End */


/*
 * For BSD style mbuf with FKB : 
 * generate nbuff.h by replacing "SKBUFF" to "BCMMBUF", and,
 * use custom arg1 and arg2 instead of mark and priority, respectively.
 */
 
#ifdef TRACE_COMPILE
#pragma message "got here 4"
#endif

struct sk_buff;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
struct blog_t;
#endif
struct net_device;
typedef int (*HardStartXmitFuncP) (struct sk_buff *skb,
                                   struct net_device *dev);

struct fkbuff;
typedef struct fkbuff FkBuff_t;

#define FKB_NULL                    ((FkBuff_t *)NULL)

#include <linux/nbuff_types.h>

/*
 *------------------------------------------------------------------------------
 *
 * Pointer conversion between pBuf and pNBuff encoded buffer pointers
 * uint8_t * pBuf;
 * pNBuff_t  pNBuff;
 * ...
 * // overlays FKBUFF_PTR into pointer to build a virtual pNBuff_t
 * pNBuff = PBUF_2_PNBUFF(pBuf,FKBUFF_PTR);
 * ...
 * // extracts a real uint8_t * from a virtual pNBuff_t
 * pBuf = PNBUFF_2_PBUF(pNBuff);
 *
 *------------------------------------------------------------------------------
 */
#define PBUF_2_PNBUFF(pBuf,realType) \
            ( (pNBuff_t) ((uintptr_t)(pBuf)   | (uintptr_t)(realType)) )
#define PNBUFF_2_PBUF(pNBuff)       \
            ( (uint8_t*) ((uintptr_t)(pNBuff) & (uintptr_t)NBUFF_PTR_MASK) )

#if (MUST_BE_ZERO != 0)
#error  "Design assumption SKBUFF_PTR == 0"
#endif
#define PNBUFF_2_SKBUFF(pNBuff)     ((struct sk_buff *)(pNBuff))

#define SKBUFF_2_PNBUFF(skb)        ((pNBuff_t)(skb)) /* see MUST_BE_ZERO */
#define FKBUFF_2_PNBUFF(fkb)        PBUF_2_PNBUFF(fkb,FKBUFF_PTR)

/*
 *------------------------------------------------------------------------------
 *
 * Cast from/to virtual "pNBuff_t" to/from real typed pointers
 *
 *  pNBuff_t pNBuff2Skb, pNBuff2Fkb;    // "void *" with NBuffPtrType_t
 *  struct sk_buff * skb_p;
 *  struct fkbuff  * fkb_p;
 *  ...
 *  pNBuff2Skb = CAST_REAL_TO_VIRT_PNBUFF(skb_p,SKBUFF_PTR);
 *  pNBuff2Fkb = CAST_REAL_TO_VIRT_PNBUFF(fkb_p,FKBUFF_PTR);
 *  ...
 *  skb_p = CAST_VIRT_TO_REAL_PNBUFF(pNBuff2Skb, struct sk_buff *);
 *  fkb_p = CAST_VIRT_TO_REAL_PNBUFF(pNBuff2Fkb, struct fkbuff  *);
 * or,
 *  fkb_p = PNBUFF_2_FKBUFF(pNBuff2Fkb);  
 *------------------------------------------------------------------------------
 */

#define CAST_REAL_TO_VIRT_PNBUFF(pRealNBuff,realType) \
            ( (pNBuff_t) (PBUF_2_PNBUFF((pRealNBuff),(realType))) )

#define CAST_VIRT_TO_REAL_PNBUFF(pVirtNBuff,realType) \
            ( (realType) PNBUFF_2_PBUF(pVirtNBuff) )

#define PNBUFF_2_FKBUFF(pNBuff) CAST_VIRT_TO_REAL_PNBUFF((pNBuff), struct fkbuff*)

/*
 *------------------------------------------------------------------------------
 *  FKB: Fast Kernel Buffers placed directly into Rx DMA Buffer
 *  May be used ONLY for common APIs such as those available in BSD-Style mbuf
 *------------------------------------------------------------------------------
 */

struct fkbuff
{
    /* List pointer must be the first field */
    union {
        FkBuff_t  * list;           /* SLL of free FKBs for cloning           */
        FkBuff_t  * master_p;       /* Clone FKB to point to master FKB       */
        atomic_long_t  users;       /* (private) # of references to FKB       */
    };
    union {                         /* Use _is_kptr_ to determine if ptr      */
        union {
            void          *ptr;
            struct blog_t *blog_p;  /* Pointer to a blog                      */
            uint8_t       *dirty_p; /* Pointer to packet payload dirty incache*/
            uint32_t       flags;   /* Access all flags                       */
        };
        /*
         * First nibble denotes a pointer or flag usage.
         * Lowest two significant bits denote the type of pinter
         * Remaining 22 bits may be used as flags
         */
        struct {
            uint32_t   ptr_type : 8;/* Identifies whether pointer             */
            uint32_t   unused   :21;/* Future use for flags                   */
            uint32_t   in_skb   : 1;/* flag: FKB passed inside a SKB          */
            uint32_t   other_ptr: 1;/* future use, to override another pointer*/
            uint32_t   dptr_tag : 1;/* Pointer type is a dirty pointer        */
        };
    };
    uint8_t       * data;           /* Pointer to packet data                 */

    union {
        /* here the bits 31-24 are valid only for native fkbs's
         * these bits bits will be cleared when using fkbInSkb 
         * Note that it is critical to have the Little Endian/Big endian 
         * declaration since FKB will use length as bit field and SKB will use  
         * length as a word  Need to maintain the same bit positions across MIPS 
         * and ARM.
         */
        struct{
            BE_DECL(
                uint32_t  rx_csum_verified:1;
                uint32_t  spdtst:1;
                uint32_t  reserved:6;
                uint32_t  len:24;              /* Packet length               */
            )
            LE_DECL(
                uint32_t  len:24;
                uint32_t  reserved:6;
                uint32_t  spdtst:1;
                uint32_t  rx_csum_verified:1;
            )
        };
        uint32_t len_word;
    };

    union {
        /* only the lower 32 bit in mark is used in 64 bit system,
         * but we delcare it as unsigned long for the ease for fcache
         * to handle it in different architecture, since it is part
         * of union with a dst_entry pointer */
        unsigned long mark;             /* Custom arg1, e.g. tag or mark field    */
        void      *queue;          /* Single link list queue of FKB | SKB    */
        void      *dst_entry;       /* rtcache entry for locally termiated pkts */
        uint32_t       fc_ctxt; /* hybrid flow cache context              */
    };
    union {
        uint32_t    priority;       /* Custom arg2, packet priority, tx info  */
        wlFlowInf_t wl;             /* WLAN Flow Info */
        uint32_t    flowid;           /* used for locally terminated pkts */
    };

    RecycleFuncP  recycle_hook;   /* Nbuff recycle handler   */
    union {
             /* recycle hook for Clone FKB is used in DHD pointing to extra info
	      * BE CAREFULL when using this recyle_context for free etc....  
	      */ 
	    void *dhd_pkttag_info_p;		  
	    unsigned long recycle_context;     /* Rx network device/channel or pool */
        uint32_t    fpm_num;
    };

} ____cacheline_aligned;   /* 2 cache lines wide */

#define FKB_CLEAR_LEN_WORD_FLAGS(len_word) (len_word &= 0x00FFFFFF)


/*
 *------------------------------------------------------------------------------
 * An fkbuff may be referred to as a:
 *  master - a pre-allocated rxBuffer, inplaced ahead of the headroom.
 *  cloned - allocated from a free pool of fkbuff and points to a master.
 *
 *  in_skb - when a FKB is passed as a member of a SKB structure.
 *------------------------------------------------------------------------------
 */
#define FKB_IN_SKB                  (1 << 2)    /* Bit#2 is in_skb */

/* Return flags with the in_skb tag set */
static inline uint32_t _set_in_skb_tag_(uint32_t flags)
{
    return (flags | FKB_IN_SKB);
}

/* Fetch the in_skb tag in flags */
static inline uint32_t _get_in_skb_tag_(void *ptr, uint32_t flags)
{
    if (_is_kptr_(ptr))
        return 0;
    return (flags & FKB_IN_SKB);
}

/* Determine whether the in_skb tag is set in flags */
static inline uint32_t _is_in_skb_tag_(void *ptr, uint32_t flags)
{
    return ( _get_in_skb_tag_(ptr, flags) ? 1 : 0 );
}

#define CHK_IQ_PRIO                  (1 << 3)    /* Bit#3 is check IQ Prio */

/* Return flags with the in_skb_tag and chk_iq_prio set */
static inline uint32_t _set_in_skb_n_chk_iq_prio_tag_(uint32_t flags)
{
    return (flags | FKB_IN_SKB | CHK_IQ_PRIO);
}

/* Return flags with the chk_iq_prio set */
static inline uint32_t _set_chk_iq_prio_tag_(uint32_t flags)
{
    return (flags | CHK_IQ_PRIO);
}

/* Fetch the chk_iq_prio tag in flags */
static inline uint32_t _get_chk_iq_prio_tag_(uint32_t flags)
{
    return (flags & CHK_IQ_PRIO);
}

/* Determine whether the chk_iq_prio tag is set in flags */
static inline uint32_t _is_chk_iq_prio_tag_(uint32_t flags)
{
    return ( _get_chk_iq_prio_tag_(flags) ? 1 : 0 );
}


/*
 *------------------------------------------------------------------------------
 * APIs to convert between a real kernel pointer and a dirty pointer.
 *------------------------------------------------------------------------------
 */

#define FKB_DPTR_TAG                (1 << 0)    /* Bit#0 is dptr_tag */

/* Test whether a pointer is a dirty pointer type */
static inline uint32_t is_dptr_tag_(uint8_t * ptr)
{
    return ( ( (uint32_t) ((uintptr_t)ptr & FKB_DPTR_TAG) ) ? 1 : 0);
}

/* Encode a real kernel pointer to a dirty pointer type */
static inline uint8_t * _to_dptr_from_kptr_(uint8_t * kernel_ptr)
{
    if((uintptr_t)(kernel_ptr) & FKB_DPTR_TAG)
        kernel_ptr++;
    /* Tag a kernel pointer's dirty_ptr bit, to denote a FKB dirty pointer */
    return ( (uint8_t*) ((uintptr_t)(kernel_ptr) | FKB_DPTR_TAG) );
}

/* Decode a dirty pointer type into a real kernel pointer */
static inline uint8_t * _to_kptr_from_dptr_(uint8_t * dirty_ptr)
{
    FKB_AUDIT(
        if ( dirty_ptr && !is_dptr_tag_(dirty_ptr) )
            printk("FKB ASSERT %s !is_dptr_tag_(0x%08x)\n",
                   __FUNCTION__, (uintptr_t)dirty_ptr); );

    /* Fetch kernel pointer from encoded FKB dirty_ptr,
       by clearing dirty_ptr bit */
    return ( (uint8_t*) ((uintptr_t)(dirty_ptr) & (~FKB_DPTR_TAG)) );
}

#define FKB_OPTR_TAG                (1<<1)      /* Bit#1 other_ptr tag */

#define FKB_BLOG_TAG_MASK           (FKB_DPTR_TAG | FKB_OPTR_TAG)

/* Verify whether a FKB pointer is pointing to a Blog */
#define _IS_BPTR_(fkb_ptr) \
         ( _is_kptr_(fkb_ptr) && ! ((uintptr_t)(fkb_ptr) & FKB_BLOG_TAG_MASK) )


/*
 *------------------------------------------------------------------------------
 *
 *                  Types of preallocated FKB pools
 * 
 *  - A Master FKB object contains memory for the rx buffer, with a FkBuff_t
 *    placed at the head of the buffer. A Master FKB object may serve to
 *    replenish a network devices receive ring, when packet buffers are not
 *    promptly recycled. A Master FKB may also be used for packet replication
 *    where in one of the transmitted packet replicas may need a unique
 *    modification distinct from other replicas. In such a case, the FKB must
 *    be first "unshared" by a deep packet buffer copy into a Master Fkb.
 *    A Free Pool of Master FKB objects is maintained. Master FKB may be
 *    alocated and recycled from this Master FKB Pool.
 *    The Master FKB Pool may also be used for replinishing a network device
 *    driver's rx buffer ring.
 *
 *  - A Cloned FKB object does not contain memory for the rx buffer.
 *    Used by fkb_clone, to create multiple references to a packet buffer.
 *    Multiple references to a packet buffer may be used for packet replication.
 *    A FKB allocated from the FKB Cloned Pool will have master_p pointing to
 *    a Master FKB and the recycle_hook member set to NULL.
 *
 *------------------------------------------------------------------------------
 */
typedef enum {
    FkbMasterPool_e = 0,
    FkbClonedPool_e = 1,
    FkbMaxPools_e
} FkbObject_t;

/*
 * Function   : _get_master_users_
 * Description: Given a pointer to a Master FKB, fetch the users count
 * Caution    : Does not check whether the FKB is a Master or not!
 */
static inline uint32_t _get_master_users_(FkBuff_t * fkbM_p)
{
    uint32_t users;
    users = atomic_read(&fkbM_p->users);

    FKB_AUDIT(
        if ( users == 0 )
            printk("FKB ASSERT cpu<%u> %s(0x%08x) users == 0, recycle<%pS>\n",
                   smp_processor_id(), __FUNCTION__,
                   (int)fkbM_p, fkbM_p->recycle_hook); );
    return users;
}

/*
 * Function   : _is_fkb_cloned_pool_
 * Description: Test whether an "allocated" FKB is from the FKB Cloned Pool.
 */
static inline uint32_t _is_fkb_cloned_pool_(FkBuff_t * fkb_p)
{
    if ( _is_kptr_(fkb_p->master_p)
         && (fkb_p->recycle_hook == (RecycleFuncP)NULL) )
    {
        FKB_AUDIT(
            /* ASSERT if the FKB is actually linked in a FKB pool */
            if ( _is_kptr_(fkb_p->master_p->list) )
            {
                printk("FKB ASSERT cpu<%u> %s :"
                       " _is_kptr_((0x%08x)->0x%08x->0x%08x)"
                       " master<0x%08x>.recycle<%pS>\n",
                       smp_processor_id(), __FUNCTION__, (int)fkb_p,
                       (int)fkb_p->master_p, (int)fkb_p->master_p->list,
                       (int)fkb_p->master_p,
                       fkb_p->master_p->recycle_hook);
            }
            /* ASSERT that Master FKB users count is greater than 0 */
            if ( _get_master_users_(fkb_p->master_p) == 0 )
            {
                printk("FKB ASSERT cpu<%u> %s :"
                       " _get_master_users_(0x%08x->0x%08x) == 0\n",
                       smp_processor_id(), __FUNCTION__,
                       (int)fkb_p, (int)fkb_p->master_p);
                return 0;
            } );

        return 1;   /* Allocated FKB is from the FKB Cloned Pool */
    }
    else
        return 0;
}

/*
 * Function   : _get_fkb_users_
 * Description: Given a pointer to a FKB (Master or Cloned), fetch users count
 */
static inline uint32_t _get_fkb_users_(FkBuff_t * fkb_p)
{
    if ( _is_kptr_(fkb_p->master_p) )       /* Cloned FKB */
    {
        FKB_AUDIT(
            if ( !_is_fkb_cloned_pool_(fkb_p) ) /* double check Cloned FKB */
            {
                printk("FKB ASSERT cpu<%u> %s :"
                       " !_is_fkb_cloned_pool_(0x%08x)"
                       " master<0x%08x>.recycle<%pS>\n",
                       smp_processor_id(), __FUNCTION__,
                       (int)fkb_p, (int)fkb_p->master_p,
                       fkb_p->master_p->recycle_hook);
                return 0;
            } );

        return _get_master_users_(fkb_p->master_p);
    }
    else                                    /* Master FKB */
        return _get_master_users_(fkb_p);
}

/*
 * Function   : _get_fkb_master_ptr_
 * Description: Fetch the pointer to the Master FKB.
 */
static inline FkBuff_t * _get_fkb_master_ptr_(FkBuff_t * fkb_p)
{
    if ( _is_kptr_(fkb_p->master_p) )       /* Cloned FKB */
    {
        FKB_AUDIT( 
            if ( !_is_fkb_cloned_pool_(fkb_p) ) /* double check Cloned FKB */
            {
                printk("FKB ASSERT cpu<%u> %s "
                       " !_is_fkb_cloned_pool_(0x%08x)"
                       " master<0x%08x>.recycle<%pS>\n",
                       smp_processor_id(), __FUNCTION__,
                       (int)fkb_p, (int)fkb_p->master_p,
                       fkb_p->master_p->recycle_hook);
                return FKB_NULL;
            } );

        return fkb_p->master_p;
    }
    else                                    /* Master FKB */
    {
        FKB_AUDIT( 
            if ( _get_master_users_(fkb_p) == 0 )  /* assert Master FKB users */
            {
                printk("FKB ASSERT cpu<%u> %s "
                       " _get_master_users_(0x%08x) == 0\n",
                       smp_processor_id(), __FUNCTION__, (int)fkb_p);
                return FKB_NULL;
            } );

        return fkb_p;
    }
}

/*
 *------------------------------------------------------------------------------
 * Placement of a FKB object in the Rx DMA buffer:
 *
 * RX DMA Buffer:   |----- FKB ----|--- reserve headroom ---|---...... 
 *                  ^              ^                        ^
 *                pFkb           pHead                    pData
 *                pBuf
 *------------------------------------------------------------------------------
 */
#define PFKBUFF_PHEAD_OFFSET        sizeof(FkBuff_t)
#define PFKBUFF_TO_PHEAD(pFkb)      ((uint8_t*)((FkBuff_t*)(pFkb) + 1))
#define PHEAD_TO_PFKBUFF(pHead)    \
            (FkBuff_t *)((uint8_t*)(pHead)-PFKBUFF_PHEAD_OFFSET)

#define PDATA_TO_PFKBUFF(pData,headroom)    \
            (FkBuff_t *)((uint8_t*)(pData)-(headroom)-PFKBUFF_PHEAD_OFFSET)
#define PFKBUFF_TO_PDATA(pFkb,headroom)     \
            (uint8_t*)((uint8_t*)(pFkb) + PFKBUFF_PHEAD_OFFSET + (headroom))


#define NBUFF_ALIGN_MASK_8   0x07
pNBuff_t nbuff_align_data(pNBuff_t pNBuff, uint8_t **data_pp,
                          uint32_t len, unsigned long alignMask);

/*
 *------------------------------------------------------------------------------
 *  FKB Functional Interfaces
 *------------------------------------------------------------------------------
 */

/*
 * Function   : fkb_in_skb_test
 * Description: Verifies that the layout of SKB member fields corresponding to
 *              a FKB have the same layout. This allows a FKB to be passed via
 *              a SKB.
 */

extern int fkb_in_skb_test( int fkb_in_skb_offset,
                            int list_offset, int blog_p_offset,
                            int data_offset, int len_offset, int mark_offset,
                            int priority_offset, int recycle_hook_offset,
                            int recycle_context_offset );

/*
 * Global FKB Subsystem Constructor
 * fkb_construct() validates that the layout of fkbuff members in sk_buff
 * is the same. An sk_buff contains an fkbuff and permits a quick translation
 * to and from a fkbuff. It also preallocates the pools of FKBs.
 */
extern int fkb_construct(int fkb_in_skb_offset);

/*
 * Function   : fkb_stats
 * Description: Report FKB Pool statistics, see CC_CONFIG_FKB_STATS
 */
extern void fkb_stats(void);

/*
 * Function   : fkb_alloc
 * Description: Allocate a Cloned/Master FKB object from preallocated pool
 */
extern FkBuff_t * fkb_alloc( FkbObject_t object );

/*
 * Function   : fkb_free
 * Description: Free a FKB object to its respective preallocated pool.
 */
extern void fkb_free(FkBuff_t * fkb_p);

/*
 * Function   : fkb_unshare
 * Description: If a FKB is pointing to a buffer with multiple references
 * to this buffer, then create a copy of the buffer and return a FKB with a
 * single reference to this buffer.
 */
extern FkBuff_t * fkb_unshare(FkBuff_t * fkb_p);

/*
 * Function   : fkbM_borrow
 * Description: Allocate a Master FKB object from the pre-allocated pool.
 */
extern FkBuff_t * fkbM_borrow(void);

/*
 * Function   : fkbM_return
 * Description: Return a Master FKB object to a pre-allocated pool.
 */
extern void fkbM_return(FkBuff_t * fkbM_p);

/*
 * Function   : fkb_set_ref
 * Description: Set reference count to an FKB.
 */
static inline void _fkb_set_ref(FkBuff_t * fkb_p, const int count)
{
    atomic_long_set(&fkb_p->users, count);
}
FKB_FN( fkb_set_ref,
        void fkb_set_ref(FkBuff_t * fkb_p, const int count),
        _fkb_set_ref(fkb_p, count) )

/*
 * Function   : fkb_inc_ref
 * Description: Increment reference count to an FKB.
 */
static inline void _fkb_inc_ref(FkBuff_t * fkb_p)
{
    atomic_long_inc(&fkb_p->users);
}
FKB_FN( fkb_inc_ref,
        void fkb_inc_ref(FkBuff_t * fkb_p),
        _fkb_inc_ref(fkb_p) )

/*
 * Function   : fkb_dec_ref
 * Description: Decrement reference count to an FKB.
 */
static inline void _fkb_dec_ref(FkBuff_t * fkb_p)
{
    atomic_long_dec(&fkb_p->users);
    /* For debug, may want to assert that users does not become negative */
}
FKB_FN( fkb_dec_ref,
        void fkb_dec_ref(FkBuff_t * fkb_p),
        _fkb_dec_ref(fkb_p) )


/*
 * Function   : fkb_preinit
 * Description: A network device driver may use this function to place a
 * FKB object into rx buffers, when they are created. FKB objects preceeds
 * the reserved headroom.
 */
static inline void fkb_preinit(uint8_t * pBuf, RecycleFuncP recycle_hook,
                               unsigned long recycle_context)
{
    FkBuff_t *fkb_p = (FkBuff_t *)pBuf;
    fkb_p->recycle_hook = recycle_hook;         /* never modified */
    fkb_p->recycle_context = recycle_context;   /* never modified */

    fkb_p->ptr = NULL;                  /* resets dirty_p, blog_p */
    fkb_p->data = NULL;
    fkb_p->len_word = 0;
    fkb_p->mark = 0;
    fkb_p->priority = 0;
    fkb_set_ref(fkb_p, 0);
}

/*
 * Function   : fkb_init
 * Description: Initialize the FKB context for a received packet. Invoked by a
 * network device on extract the packet from a buffer descriptor and associating
 * a FKB context to the received packet.
 */
static inline FkBuff_t * _fkb_init(uint8_t * pBuf, uint32_t headroom,
                                   uint8_t * pData, uint32_t len)
{
    FkBuff_t * fkb_p = PDATA_TO_PFKBUFF(pBuf, headroom);
    fkb_dbg( 1, "fkb_p<%p> pBuf<%p> headroom<%u> pData<%p> len<%d>",
              fkb_p, pBuf, (int)headroom, pData, len );

#if defined(CC_FKB_HEADROOM_AUDIT)
    if ( headroom < BCM_PKT_HEADROOM )
        printk("NBUFF: Insufficient headroom <%u>, need <%u> %-10s\n",
               headroom, BCM_PKT_HEADROOM, __FUNCTION__);
#endif

    fkb_p->data = pData;
    fkb_p->len_word = 0;/*clear flags */
    fkb_p->len  = len;
    fkb_p->ptr  = (void*)NULL;   /* resets dirty_p, blog_p */
    fkb_p->mark = 0;
    fkb_p->priority = 0;
    
    fkb_set_ref( fkb_p, 1 );

    return fkb_p;
}
FKB_FN( fkb_init,
        FkBuff_t * fkb_init(uint8_t * pBuf, uint32_t headroom,
                            uint8_t * pData, uint32_t len),
        return _fkb_init(pBuf, headroom, pData, len) )

/*
 * Function   : fkb_qinit
 * Description: Same as fkb_init, with the exception that a recycle queue
 * context is associated with the FKB, each time the packet is receieved.
 */
static inline FkBuff_t * _fkb_qinit(uint8_t * pBuf, uint32_t headroom,
                    uint8_t * pData, uint32_t len, unsigned long qcontext)
{
    FkBuff_t * fkb_p = PDATA_TO_PFKBUFF(pBuf, headroom);
    fkb_dbg(1, "fkb_p<%p> qcontext<%lx>", fkb_p, qcontext );
    fkb_p->recycle_context = qcontext;

    return _fkb_init(pBuf, headroom, pData, len);
}
FKB_FN( fkb_qinit,
        FkBuff_t * fkb_qinit(uint8_t * pBuf, uint32_t headroom,
                             uint8_t * pData, uint32_t len, unsigned long qcontext),
        return _fkb_qinit(pBuf, headroom, pData, len, qcontext) )

/*
 * Function   : fkb_release
 * Description: Release any associated blog and set ref count to 0. A fkb
 * may be released multiple times (not decrement reference count).
 */
static inline void _fkb_release(FkBuff_t * fkb_p)
{
    fkb_dbg(1, "fkb_p<%p> fkb_p->blog_p<%p>", fkb_p, fkb_p->blog_p );
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
    if ( _IS_BPTR_( fkb_p->blog_p ) )
        blog_put(fkb_p->blog_p);
#endif
    fkb_p->ptr = (void*)NULL;   /* reset dirty_p, blog_p */

    fkb_set_ref( fkb_p, 0 );    /* fkb_release may be invoked multiple times */
}
FKB_FN( fkb_release,
        void fkb_release(FkBuff_t * fkb_p),
        _fkb_release(fkb_p) )

/*
 * Function   : fkb_headroom
 * Description: Determine available headroom for the packet in the buffer.
 */
static inline int _fkb_headroom(const FkBuff_t *fkb_p)
{
    return (int)( (uintptr_t)(fkb_p->data) - (uintptr_t)(fkb_p+1) );
}
FKB_FN( fkb_headroom,
        int fkb_headroom(const FkBuff_t *fkb_p),
        return _fkb_headroom(fkb_p) )

/*
 * Function   : fkb_init_headroom
 * Description: The available headroom the packet in the buffer at fkb_init time.
 */
static inline int _fkb_init_headroom(void)
{
    return BCM_PKT_HEADROOM;
}
FKB_FN( fkb_init_headroom,
        int fkb_init_headroom(void),
        return _fkb_init_headroom() )


/*
 * Function   : fkb_push
 * Description: Prepare space for data at head of the packet buffer.
 */
static inline uint8_t * _fkb_push(FkBuff_t * fkb_p, uint32_t len)
{
    fkb_p->len  += len;
    fkb_p->data -= len;
    return fkb_p->data;
}
FKB_FN( fkb_push,
        uint8_t * fkb_push(FkBuff_t * fkb_p, uint32_t len),
        return _fkb_push(fkb_p, len) )

/*
 * Function   : fkb_pull
 * Description: Delete data from the head of packet buffer.
 */
static inline uint8_t * _fkb_pull(FkBuff_t * fkb_p, uint32_t len)
{
    fkb_p->len  -= len;
    fkb_p->data += len;
    return fkb_p->data;
}
FKB_FN( fkb_pull,
        uint8_t * fkb_pull(FkBuff_t * fkb_p, uint32_t len),
        return _fkb_pull(fkb_p, len) )

/*
 * Function   : fkb_put
 * Description: Prepare space for data at tail of the packet buffer.
 */
static inline uint8_t * _fkb_put(FkBuff_t * fkb_p, uint32_t len)
{
    uint8_t * tail_p = fkb_p->data + fkb_p->len; 
    fkb_p->len  += len;
    return tail_p;
}
FKB_FN( fkb_put,
        uint8_t * fkb_put(FkBuff_t * fkb_p, uint32_t len),
        return _fkb_put(fkb_p, len) )

/*
 * Function   : fkb_pad
 * Description: Pad the packet by requested number of bytes.
 */
static inline uint32_t _fkb_pad(FkBuff_t * fkb_p, uint32_t padding)
{
    memset((uint8_t *)(fkb_p->data + fkb_p->len), 0, padding);
    fkb_p->len  += padding;
    return fkb_p->len;
}
FKB_FN( fkb_pad,
        uint32_t fkb_pad(FkBuff_t * fkb_p, uint32_t padding),
        return _fkb_pad(fkb_p, padding) )

/*
 * Function   : fkb_len
 * Description: Determine the length of the packet.
 */
static inline uint32_t _fkb_len(FkBuff_t * fkb_p)
{
    return fkb_p->len;
}
FKB_FN( fkb_len,
        uint32_t fkb_len(FkBuff_t * fkb_p),
        return _fkb_len(fkb_p) )

/*
 * Function   : fkb_data
 * Description: Fetch the start of the packet.
 */
static inline uint8_t * _fkb_data(FkBuff_t * fkb_p)
{
    return fkb_p->data;
}
FKB_FN( fkb_data,
        uint8_t * fkb_data(FkBuff_t * fkb_p),
        return _fkb_data(fkb_p) )

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
/*
 * Function   : fkb_blog
 * Description: Fetch the associated blog.
 */
static inline struct blog_t * _fkb_blog(FkBuff_t * fkb_p)
{
    return fkb_p->blog_p;
}
FKB_FN( fkb_blog,
        struct blog_t * fkb_blog(FkBuff_t * fkb_p),
        return _fkb_blog(fkb_p) )
#endif

/*
 * Function   : fkb_clone
 * Description: Allocate a FKB from the Cloned Pool and make it reference the
 * same packet.
 */
static inline FkBuff_t * _fkb_clone(FkBuff_t * fkbM_p)
{
    FkBuff_t * fkbC_p;

    FKB_AUDIT( 
        if ( smp_processor_id() )
            printk("FKB ASSERT %s not supported on CP 1\n", __FUNCTION__); );

    /* Fetch a pointer to the Master FKB */
    fkbM_p = _get_fkb_master_ptr_( fkbM_p );

    fkbC_p = fkb_alloc( FkbClonedPool_e );  /* Allocate FKB from Cloned pool */

    if ( unlikely(fkbC_p != FKB_NULL) )
    {
        fkb_inc_ref( fkbM_p );
        fkbC_p->master_p   = fkbM_p;
        fkbC_p->ptr   = fkbM_p->ptr;

        fkbC_p->data       = fkbM_p->data;
        fkbC_p->len_word   = fkbM_p->len_word;
        fkbC_p->mark       = fkbM_p->mark;
        fkbC_p->priority   = fkbM_p->priority;
    }

    fkb_dbg(1, "fkbC_p<%p> ---> fkbM_p<%p>", fkbC_p, fkbM_p );

    return fkbC_p;       /* May be null */
}
FKB_FN( fkb_clone,
        FkBuff_t * fkb_clone(FkBuff_t * fkbM_p),
        return _fkb_clone(fkbM_p) )

extern void fkb_flush(FkBuff_t * fkb_p, uint8_t * data_p, int len, int cache_op);

/*
 *------------------------------------------------------------------------------
 * Virtual accessors to common members of network kernel buffer
 *------------------------------------------------------------------------------
 */

/* __BUILD_NBUFF_SET_ACCESSOR: generates function nbuff_set_MEMBER() */
#define __BUILD_NBUFF_SET_ACCESSOR( TYPE, MEMBER )                             \
static inline void nbuff_set_##MEMBER(pNBuff_t pNBuff, TYPE MEMBER) \
{                                                                              \
    void * pBuf = PNBUFF_2_PBUF(pNBuff);                                       \
    if ( IS_SKBUFF_PTR(pNBuff) )                                               \
        ((struct sk_buff *)pBuf)->MEMBER = MEMBER;                             \
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */                         \
    else                                                                       \
        ((FkBuff_t *)pBuf)->MEMBER = MEMBER;                                   \
}

/* __BUILD_NBUFF_GET_ACCESSOR: generates function nbuff_get_MEMBER() */
#define __BUILD_NBUFF_GET_ACCESSOR( TYPE, MEMBER )                             \
static inline TYPE nbuff_get_##MEMBER(pNBuff_t pNBuff)                         \
{                                                                              \
    void * pBuf = PNBUFF_2_PBUF(pNBuff);                                       \
    if ( IS_SKBUFF_PTR(pNBuff) )                                               \
        return (TYPE)(((struct sk_buff *)pBuf)->MEMBER);                       \
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */                         \
    else                                                                       \
        return (TYPE)(((FkBuff_t *)pBuf)->MEMBER);                             \
}

/*
 * Common set/get accessor of base network buffer fields:
 * nbuff_set_data(), nbuff_set_len(), nbuff_set_mark(), nbuff_set_priority()
 * nbuff_get_data(), nbuff_get_len(), nbuff_get_mark(), nbuff_get_priority()
 */
__BUILD_NBUFF_SET_ACCESSOR(uint8_t *, data) 
__BUILD_NBUFF_SET_ACCESSOR(uint32_t, len) 
__BUILD_NBUFF_SET_ACCESSOR(uint32_t, mark)      /* Custom network buffer arg1 */
__BUILD_NBUFF_SET_ACCESSOR(void *, queue)     /* Custom network buffer arg1 */
__BUILD_NBUFF_SET_ACCESSOR(uint32_t, priority)  /* Custom network buffer arg2 */

__BUILD_NBUFF_GET_ACCESSOR(uint8_t *, data)
__BUILD_NBUFF_GET_ACCESSOR(uint32_t, len)
__BUILD_NBUFF_GET_ACCESSOR(uint32_t, mark)      /* Custom network buffer arg1 */
__BUILD_NBUFF_GET_ACCESSOR(void *, queue)     /* Custom network buffer arg1 */
__BUILD_NBUFF_GET_ACCESSOR(uint32_t, priority)  /* Custom network buffer arg2 */

/*
 * Function   : nbuff_get_context
 * Description: Extracts the data and len fields from a pNBuff_t.
 */
static inline void * nbuff_get_context(pNBuff_t pNBuff,
                                     uint8_t ** data_p, uint32_t *len_p)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( pBuf == (void*) NULL )
        return pBuf;
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        *data_p     = ((struct sk_buff *)pBuf)->data;
        *len_p      = ((struct sk_buff *)pBuf)->len;
    }
    else
    {
        *data_p     = ((FkBuff_t *)pBuf)->data;
        *len_p      = ((FkBuff_t *)pBuf)->len;
    }
    fkb_dbg(1, "pNBuff<%p> pBuf<%p> data_p<%p>",
           pNBuff, pBuf, *data_p );
    return pBuf;
}

/*
 * Function   : nbuff_get_params
 * Description: Extracts the data, len, mark and priority field from a network
 * buffer.
 */
static inline void * nbuff_get_params(pNBuff_t pNBuff,
                                     uint8_t ** data_p, uint32_t *len_p,
                                     uint32_t * mark_p, uint32_t *priority_p)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( pBuf == (void*) NULL )
        return pBuf;
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        *data_p     = ((struct sk_buff *)pBuf)->data;
        *len_p      = ((struct sk_buff *)pBuf)->len;
        *mark_p     = ((struct sk_buff *)pBuf)->mark;
        *priority_p = ((struct sk_buff *)pBuf)->priority;
    }
    else
    {
        *data_p     = ((FkBuff_t *)pBuf)->data;
        *len_p      = ((FkBuff_t *)pBuf)->len;
        *mark_p     = ((FkBuff_t *)pBuf)->mark;
        *priority_p = ((FkBuff_t *)pBuf)->priority;
    }
    fkb_dbg(1, "pNBuff<%p> pBuf<%p> data_p<%p>",
            pNBuff, pBuf, *data_p );
    return pBuf;
}
    
/* adds recycle flags/context to nbuff_get_params used in impl4 enet */
/*
 * Function   : nbuff_get_params_ext
 * Description: Extracts the data, len, mark, priority and 
 * recycle flags/context field from a network buffer.
 */
static inline void * nbuff_get_params_ext(pNBuff_t pNBuff, uint8_t **data_p, 
                                          uint32_t *len_p, uint32_t *mark_p, 
                                          uint32_t *priority_p, 
                                          uint32_t *rflags_p)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( pBuf == (void*) NULL )
        return pBuf;
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        *data_p     = ((struct sk_buff *)pBuf)->data;
        *len_p      = ((struct sk_buff *)pBuf)->len;
        *mark_p     = ((struct sk_buff *)pBuf)->mark;
        *priority_p = ((struct sk_buff *)pBuf)->priority;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
        *rflags_p   = ((struct sk_buff *)pBuf)->recycle_flags;
#endif
    }
    else
    {
        *data_p     = ((FkBuff_t *)pBuf)->data;
        *len_p      = ((FkBuff_t *)pBuf)->len;
        *mark_p     = ((FkBuff_t *)pBuf)->mark;
        *priority_p = ((FkBuff_t *)pBuf)->priority;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
        *rflags_p   = ((FkBuff_t *)pBuf)->recycle_context;
#endif
    }
    fkb_dbg(1, "pNBuff<%p> pBuf<%p> data_p<%p>",
            pNBuff, pBuf, *data_p );
    return pBuf;
}

/*
 *------------------------------------------------------------------------------
 * Virtual common functional apis of a network kernel buffer
 *------------------------------------------------------------------------------
 */

/*
 * Function   : nbuff_push
 * Description: Make space at the start of a network buffer.
 * CAUTION    : In the case of a FKB, no check for headroom is done.
 */
static inline uint8_t * nbuff_push(pNBuff_t pNBuff, uint32_t len)
{
    uint8_t * data;
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( IS_SKBUFF_PTR(pNBuff) )
        data = skb_push(((struct sk_buff *)pBuf), len);
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        data = fkb_push((FkBuff_t*)pBuf, len);
    fkb_dbg(1, "pNBuff<%p> pBuf<%p> data<%p> len<%u>",
            pNBuff, pBuf, data, len );
    return data;
}

/*
 * Function   : nbuff_pull
 * Description: Delete data from start of a network buffer.
 */
static inline uint8_t * nbuff_pull(pNBuff_t pNBuff, uint32_t len)
{
    uint8_t * data;
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( IS_SKBUFF_PTR(pNBuff) )
        data = skb_pull(((struct sk_buff *)pBuf), len);
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        data = fkb_pull((FkBuff_t *)pBuf, len);
    fkb_dbg(1, "pNBuff<%p> pBuf<%p> data<%p> len<%u>",
            pNBuff, pBuf, data, len );
    return data;
}

/*
 * Function   : nbuff_put
 * Description: Make space at the tail of a network buffer.
 * CAUTION: In the case of a FKB, no check for tailroom is done.
 */
static inline uint8_t * nbuff_put(pNBuff_t pNBuff, uint32_t len)
{
    uint8_t * tail;
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( IS_SKBUFF_PTR(pNBuff) )
        tail = skb_put(((struct sk_buff *)pBuf), len);
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        tail = fkb_put((FkBuff_t *)pBuf, len);
    fkb_dbg(1, "pNBuff<%p> pBuf<%p> tail<%p> len<%u>",
            pNBuff, pBuf, tail, len );
    return tail;
}

extern void dev_kfree_skb_thread(struct sk_buff *skb);
extern void nbuff_free_ex(pNBuff_t pNBuff, int in_thread);
extern void nbuff_free(pNBuff_t pNBuff);

/*
 * Function   : nbuff_unshare
 * Description: If there are more than one references to the data buffer
 * associated with the network buffer, create a deep copy of the data buffer
 * and return a network buffer context to it. The returned network buffer
 * may be then used to modify the data packet without impacting the original
 * network buffer and its data buffer.
 *
 * If the data packet had a single network buffer referencing it, then the
 * original network buffer is returned.
 */
static inline pNBuff_t nbuff_unshare(pNBuff_t pNBuff)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<%p> pBuf<%p>", pNBuff, pBuf);
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        struct sk_buff *skb_p;
        skb_p = skb_unshare( (struct sk_buff *)pBuf, GFP_ATOMIC);
        pNBuff = SKBUFF_2_PNBUFF(skb_p);
    }
    else
    {
        FkBuff_t * fkb_p;
        fkb_p = fkb_unshare( (FkBuff_t *)pBuf );
        pNBuff = FKBUFF_2_PNBUFF(fkb_p);
    }

    fkb_dbg(2, "<<");
    return pNBuff;
}

/*
 * Function   : nbuff_invalidate_headroom
 * Description: invalidate datacache lines of memory prefixing "data" pointer.
 * Invalidation does not include the dcache line "data" is in. This dcache line
 * must be flushed, not invalidated.
 */
static inline void nbuff_invalidate_headroom(pNBuff_t pNBuff, uint8_t * data)
{

    /* Invalidate functions used here will round up end pointer to cache line 
     * boundry. That's the reason for L1_CACHE_BYTES substruction.
     */
    int32_t inv_len = 0;
    fkb_dbg(1, "pNBuff<0x%08x> data<0x%08x>", (int)pNBuff, (int)data);

    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        inv_len = skb_avail_headroom( PNBUFF_2_SKBUFF(pNBuff) ) - L1_CACHE_BYTES;
        cache_invalidate_region(PNBUFF_2_SKBUFF(pNBuff)->head, data - L1_CACHE_BYTES);
    }
    else
    {
        FkBuff_t * fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);

        if ( _is_fkb_cloned_pool_(fkb_p) )
            fkb_p = fkb_p->master_p;

        inv_len =  data - PFKBUFF_TO_PHEAD(fkb_p) - L1_CACHE_BYTES;
        fkb_flush(fkb_p, PFKBUFF_TO_PHEAD(fkb_p), inv_len, FKB_CACHE_INV); 
    }
    fkb_dbg(1, " len<%d>", inv_len);
    fkb_dbg(2, "<<");
}

extern void nbuff_flush(pNBuff_t pNBuff, uint8_t * data, int len);
extern void nbuff_flushfree(pNBuff_t pNBuff);

/*
 * Function   : nbuff_xlate
 * Description: Convert a FKB to a SKB. The SKB is data filled with the
 * data, len, mark, priority, and recycle hook and context. 
 *
 * Other SKB fields for SKB API manipulation are also initialized.
 * SKB fields for network stack manipulation are NOT initialized.
 *
 * This function is typically used only in a network device drivers' hard
 * start xmit function handler. A hard start xmit function handler may receive
 * a network buffer of a FKB type and may not wish to rework the implementation
 * to use nbuff APIs. In such an event, a nbuff may be translated to a skbuff.
 */
struct sk_buff * fkb_xlate(FkBuff_t * fkb_p);
static inline struct sk_buff * nbuff_xlate( pNBuff_t pNBuff )
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<%p> pBuf<%p>", pNBuff, pBuf);

    if ( IS_SKBUFF_PTR(pNBuff) )
        return (struct sk_buff *)pBuf;
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        return fkb_xlate( (FkBuff_t *)pBuf );
}


/* Miscellaneous helper routines */
static inline void u16cpy( void * dst_p, const void * src_p, uint32_t bytes )
{
    uint16_t * dst16_p = (uint16_t*)dst_p;
    uint16_t * src16_p = (uint16_t*)src_p;
    do { // assuming: (bytes % sizeof(uint16_t) == 0 !!!
        *dst16_p++ = *src16_p++;
    } while ( bytes -= sizeof(uint16_t) );
}

static inline void u16datacpy( void * dst_p, const void * src_p, uint32_t bytes )
{
    uint16_t * dst16_p = (uint16_t*)dst_p;
    uint16_t * src16_p = (uint16_t*)src_p;
    do { // assuming: (bytes % sizeof(uint16_t) == 0 !!!
        *dst16_p++ = htons (*src16_p++);
    } while ( bytes -= sizeof(uint16_t) );
}

static inline int u16cmp( void * dst_p, const void * src_p,
                          uint32_t bytes )
{
    uint16_t * dst16_p = (uint16_t*)dst_p;
    uint16_t * src16_p = (uint16_t*)src_p;
    do { // assuming: (bytes % sizeof(uint16_t) == 0 !!!
        if ( *dst16_p++ != *src16_p++ )
            return -1;
    } while ( bytes -= sizeof(uint16_t) );

    return 0;
}

static inline int nbuff_pad(pNBuff_t pNBuff, int padLen)
{
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        skb_pad((struct sk_buff *)pNBuff, padLen);
    }
    else
    {
        fkb_pad(PNBUFF_2_FKBUFF(pNBuff), padLen);
    }
    return 0;
}

#ifdef DUMP_DATA
/* dumpHexData dump out the hex base binary data */
static inline void dumpHexData1(uint8_t *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = pHead;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

static inline void dump_pkt(const char * fname, uint8_t * pBuf, uint32_t len)
{
    //int dump_len = ( len < 64) ? len : 64;
    int dump_len = len ;
    printk("%s: data<0x%lu len<%u>", fname, (uintptr_t)pBuf, len);
    dumpHexData1(pBuf, dump_len);
    cache_flush_len((void*)pBuf, dump_len);
}
#define DUMP_PKT(pBuf,len)      dump_pkt(__FUNCTION__, (pBuf), (len))
#else   /* !defined(DUMP_DATA) */
#define DUMP_PKT(pBuf,len)      do {} while(0)
#endif

#endif  /* defined(__NBUFF_H_INCLUDED__) */

#endif
