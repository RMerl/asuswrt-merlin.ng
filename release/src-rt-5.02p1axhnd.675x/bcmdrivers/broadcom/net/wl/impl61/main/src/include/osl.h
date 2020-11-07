/*
 * OS Abstraction Layer
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: osl.h 774649 2019-05-01 13:45:54Z $
 */

#ifndef _osl_h_
#define _osl_h_

#include <osl_decl.h>

enum {
	TAIL_BYTES_TYPE_FCS = 1,
	TAIL_BYTES_TYPE_ICV = 2,
	TAIL_BYTES_TYPE_MIC = 3
};

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#ifdef container_of
#undef container_of
#define container_of(ptr, type, member) ({ \
	typeof(((type *)0)->member) *__mptr = (ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); \
})
#endif /* container_of */
#endif /* STRICT_GCC_WARNINGS */

#ifdef MACOSX
#define OSL_PKTTAG_SZ 56
#elif defined DONGLEBUILD
#define OSL_PKTTAG_SZ 32 /* Size of PktTag */
#else
#define OSL_PKTTAG_SZ 48 /* Size of PktTag */
#endif // endif

/* Drivers use PKTFREESETCB to register a callback function when a packet is freed by OSL */

typedef void (*pktfree_cb_fn_t)(void *ctx, void *pkt, unsigned int status);

/* Drivers use REGOPSSET() to register register read/write funcitons */
typedef unsigned int (*osl_rreg_fn_t)(void *ctx, volatile void *reg, unsigned int size);
typedef void  (*osl_wreg_fn_t)(void *ctx, volatile void *reg, unsigned int val, unsigned int size);

#ifdef __mips__
#define PREF_LOAD		0
#define PREF_STORE		1
#define PREF_LOAD_STREAMED	4
#define PREF_STORE_STREAMED	5
#define PREF_LOAD_RETAINED	6
#define PREF_STORE_RETAINED	7
#define PREF_WBACK_INV		25
#define PREF_PREPARE4STORE	30

#define MAKE_PREFETCH_FN(hint) \
static inline void prefetch_##hint(const void *addr) \
{ \
	__asm__ __volatile__(\
	"       .set    mips4           \n" \
	"       pref    %0, (%1)        \n" \
	"       .set    mips0           \n" \
	: \
	: "i" (hint), "r" (addr)); \
}

#define MAKE_PREFETCH_RANGE_FN(hint) \
static inline void prefetch_range_##hint(const void *addr, int len) \
{ \
	int size = len; \
	while (size > 0) { \
		prefetch_##hint(addr); \
		size -= 32; \
	} \
}

MAKE_PREFETCH_FN(PREF_LOAD)
MAKE_PREFETCH_RANGE_FN(PREF_LOAD)
MAKE_PREFETCH_FN(PREF_STORE)
MAKE_PREFETCH_RANGE_FN(PREF_STORE)
MAKE_PREFETCH_FN(PREF_LOAD_STREAMED)
MAKE_PREFETCH_RANGE_FN(PREF_LOAD_STREAMED)
MAKE_PREFETCH_FN(PREF_STORE_STREAMED)
MAKE_PREFETCH_RANGE_FN(PREF_STORE_STREAMED)
MAKE_PREFETCH_FN(PREF_LOAD_RETAINED)
MAKE_PREFETCH_RANGE_FN(PREF_LOAD_RETAINED)
MAKE_PREFETCH_FN(PREF_STORE_RETAINED)
MAKE_PREFETCH_RANGE_FN(PREF_STORE_RETAINED)
#endif /* __mips__ */

#if  defined(DOS)
#include <dos_osl.h>
#elif defined(PCBIOS)
#include <pcbios_osl.h>
#elif defined(EFI)
#include <efi_osl.h>
#elif defined(WL_UNITTEST)
#include <utest_osl.h>
#elif defined(linux)
#include <linux_osl.h>
#include <linux_pkt.h>
#elif defined(_CFE_) /* router boot loader */
#include <cfe_osl.h>
#elif defined(_RTE_)
#include <rte_osl.h>
#include <hnd_pkt.h>
#elif defined(_MINOSL_)
#include <min_osl.h>
#elif defined(MACOSX)
#include <macosx_osl.h>
#elif defined(TARGETOS_nucleus)
#include <nucleus_osl.h>
#else
#error "Unsupported OSL requested"
#endif /* defined(DOS) */

#ifndef PKTDBG_TRACE
#define PKTDBG_TRACE(osh, pkt, bit)	BCM_REFERENCE(osh)
#endif // endif

#ifndef PKTCTFMAP
#define PKTCTFMAP(osh, p)		BCM_REFERENCE(osh)
#endif /* PKTCTFMAP */

/* --------------------------------------------------------------------------
** Register manipulation macros.
*/

#define	SET_REG(osh, r, mask, val)	W_REG((osh), (r), ((R_REG((osh), r) & ~(mask)) | (val)))

#ifndef AND_REG
#define AND_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) & (v))
#endif   /* !AND_REG */

#ifndef OR_REG
#define OR_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) | (v))
#endif   /* !OR_REG */

#if !defined(OSL_SYSUPTIME)
#define OSL_SYSUPTIME() (0)
#define OSL_SYSUPTIME_NOT_DEFINED 1
#endif /* !defined(OSL_SYSUPTIME) */

#if !defined(OSL_SYSUPTIME_US)
#define OSL_SYSUPTIME_US() (0)
#define OSL_SYSUPTIME_US_NOT_DEFINED 1
#endif /* !defined(OSL_SYSUPTIME) */

#if defined(OSL_SYSUPTIME_NOT_DEFINED) && defined(OSL_SYSUPTIME_US_NOT_DEFINED)
#define OSL_SYSUPTIME_SUPPORT FALSE
#else
#define OSL_SYSUPTIME_SUPPORT TRUE
#endif /* OSL_SYSUPTIME */

#ifndef OSL_SYS_HALT
#define OSL_SYS_HALT()	do {} while (0)
#endif // endif

#ifndef DMB
#if defined(STB)
#define DMB()	mb();
#else /* STB */
#define DMB()	do {} while (0)
#endif /* STB */
#endif /* DMB */

#ifndef OSL_MEM_AVAIL
#define OSL_MEM_AVAIL()	(0xffffffff)
#endif // endif

#ifndef OSL_OBFUSCATE_BUF
/* For security reasons printing pointers is not allowed.
 * Some OSLs implement OSL_OBFUSCATE_BUF to OS specific obfuscate API.
 * If OSL_OBFUSCATE_BUF() is not implemented in OSL, then default to
 * printing the input pointer
 */
#define OSL_OBFUSCATE_BUF(x) (x)
#endif /* OSL_OBFUSCATE_BUF */

#if !((defined(linux) && defined(PKTC)) || defined(PKTC_DONGLE))

#define	PKTCGETATTR(skb)	(0)
#define	PKTCSETATTR(skb, f, p, b) BCM_REFERENCE(skb)
#define	PKTCCLRATTR(skb)	BCM_REFERENCE(skb)
#define	PKTCCNT(skb)		(1)
#define	PKTCLEN(skb)		PKTLEN(NULL, skb)
#define	PKTCGETFLAGS(skb)	(0)
#define	PKTCSETFLAGS(skb, f)	BCM_REFERENCE(skb)
#define	PKTCCLRFLAGS(skb)	BCM_REFERENCE(skb)
#define	PKTCFLAGS(skb)		(0)
#define	PKTCSETCNT(skb, c)	BCM_REFERENCE(skb)
#define	PKTCINCRCNT(skb)	BCM_REFERENCE(skb)
#define	PKTCADDCNT(skb, c)	BCM_REFERENCE(skb)
#define	PKTCSETLEN(skb, l)	BCM_REFERENCE(skb)
#define	PKTCADDLEN(skb, l)	BCM_REFERENCE(skb)
#define	PKTCSETFLAG(skb, fb)	BCM_REFERENCE(skb)
#define	PKTCCLRFLAG(skb, fb)	BCM_REFERENCE(skb)
#define	PKTCLINK(skb)		NULL
#define	PKTSETCLINK(skb, x)	BCM_REFERENCE(skb)
#define FOREACH_CHAINED_PKT(skb, nskb) \
	for ((nskb) = NULL; (skb) != NULL; (skb) = (nskb))
#define	PKTCFREE		PKTFREE
#define PKTCENQTAIL(h, t, p) \
do { \
	if ((t) == NULL) { \
		(h) = (t) = (p); \
	} \
} while (0)
#endif /* !linux || !PKTC */

#ifndef PKTSETCHAINED
#define PKTSETCHAINED(osh, skb)		BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTCLRCHAINED
#define PKTCLRCHAINED(osh, skb)		BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTISCHAINED
#define PKTISCHAINED(skb)		FALSE
#endif // endif

#ifndef _RTE_
/* Lbuf with fraglist */
#ifndef PKTFRAGPKTID
#define PKTFRAGPKTID(osh, lb)		(0)
#endif // endif
#ifndef PKTSETFRAGPKTID
#define PKTSETFRAGPKTID(osh, lb, id)	BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTFRAGTOTNUM
#define PKTFRAGTOTNUM(osh, lb)		(0)
#endif // endif
#ifndef PKTSETFRAGTOTNUM
#define PKTSETFRAGTOTNUM(osh, lb, tot)	BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTFRAGTOTLEN
#define PKTFRAGTOTLEN(osh, lb)		(0)
#endif // endif
#ifndef PKTSETFRAGTOTLEN
#define PKTSETFRAGTOTLEN(osh, lb, len)	BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTIFINDEX
#define PKTIFINDEX(osh, lb)		(0)
#endif // endif
#ifndef PKTSETIFINDEX
#define PKTSETIFINDEX(osh, lb, idx)	BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTGETLF
#define	PKTGETLF(osh, len, send, lbuf_type)	(0)
#endif // endif

/* in rx path, reuse totlen as used len */
#ifndef PKTFRAGUSEDLEN
#define PKTFRAGUSEDLEN(osh, lb)			(0)
#endif // endif
#ifndef PKTSETFRAGUSEDLEN
#define PKTSETFRAGUSEDLEN(osh, lb, len)		BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTFRAGLEN
#define PKTFRAGLEN(osh, lb, ix)			(0)
#endif // endif
#ifndef PKTSETFRAGLEN
#define PKTSETFRAGLEN(osh, lb, ix, len)		BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTFRAGDATA_LO
#define PKTFRAGDATA_LO(osh, lb, ix)		(0)
#endif // endif
#ifndef PKTSETFRAGDATA_LO
#define PKTSETFRAGDATA_LO(osh, lb, ix, addr)	BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTFRAGDATA_HI
#define PKTFRAGDATA_HI(osh, lb, ix)		(0)
#endif // endif
#ifndef PKTSETFRAGDATA_HI
#define PKTSETFRAGDATA_HI(osh, lb, ix, addr)	BCM_REFERENCE(osh)
#endif // endif

/* RX FRAG */
#ifndef PKTISRXFRAG
#define PKTISRXFRAG(osh, lb)    	(0)
#endif // endif
#ifndef PKTSETRXFRAG
#define PKTSETRXFRAG(osh, lb)		BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTRESETRXFRAG
#define PKTRESETRXFRAG(osh, lb)		BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTISPKTFETCHED
#define PKTISPKTFETCHED(osh, lb)	(0)
#endif // endif

/* TX FRAG */
#ifndef PKTISTXFRAG
#define PKTISTXFRAG(osh, lb)		(0)
#endif // endif
#ifndef PKTSETTXFRAG
#define PKTSETTXFRAG(osh, lb)		BCM_REFERENCE(osh)
#endif // endif

/** Cache Flow processing Packet Macros */
#ifndef PKTISCFP
#define PKTISCFP(lb)				(0)
#endif // endif
#ifndef PKTGETCFPFLOWID
#define PKTGETCFPFLOWID(lb)			((uint16)(~0))
#endif // endif
#ifndef PKTSETCFPFLOWID
#define PKTSETCFPFLOWID(lb, flowid)	BCM_REFERENCE(lb)
#endif // endif
#ifndef PKTCLRCFPFLOWID
#define PKTCLRCFPFLOWID(lb, flowid)	BCM_REFERENCE(lb)
#endif // endif

/* Need Rx completion used for AMPDU reordering */
#ifndef PKTNEEDRXCPL
#define PKTNEEDRXCPL(osh, lb)           (TRUE)
#endif // endif
#ifndef PKTSETNORXCPL
#define PKTSETNORXCPL(osh, lb)          BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTRESETNORXCPL
#define PKTRESETNORXCPL(osh, lb)        BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTISFRAG
#define PKTISFRAG(osh, lb)		(0)
#endif // endif
#ifndef PKTFRAGISCHAINED
#define PKTFRAGISCHAINED(osh, i)	(0)
#endif // endif
/* TRIM Tail bytes from lfrag */
#ifndef PKTFRAG_TRIM_TAILBYTES
#define PKTFRAG_TRIM_TAILBYTES(osh, p, len, type)	PKTSETLEN(osh, p, PKTLEN(osh, p) - len)
#endif // endif
#ifndef PKTISHDRCONVTD
#define PKTISHDRCONVTD(osh, lb)		(0)
#endif // endif

/* Forwarded pkt indication */
#ifndef PKTISFRWDPKT
#define PKTISFRWDPKT(osh, lb)		0
#endif // endif
#ifndef PKTSETFRWDPKT
#define PKTSETFRWDPKT(osh, lb)		BCM_REFERENCE(osh)
#endif // endif
#ifndef PKTRESETFRWDPKT
#define PKTRESETFRWDPKT(osh, lb)	BCM_REFERENCE(osh)
#endif // endif

#endif	/* _RTE_ */

#ifdef BCM_SECURE_DMA
#define SECURE_DMA_ENAB(osh) (1)
#else

#define SECURE_DMA_ENAB(osh) (0)
#ifndef BCMDMA64OSL
#define	SECURE_DMA_MAP(osh, va, size, direction, p, dmah, pcma, offset, buftype) \
	((dma_addr_t) ((0)))
#else
#define	SECURE_DMA_MAP(osh, va, size, direction, p, dmah, pcma, offset, buftype) \
	((dma_addr_t) ((0)))
#endif // endif
#define	SECURE_DMA_DD_MAP(osh, va, size, direction, p, dmah) 0
#ifndef BCMDMA64OSL
#define	SECURE_DMA_MAP_TXMETA(osh, va, size, direction, p, dmah, pcma) ((dma_addr_t) ((0)))
#else
#define	SECURE_DMA_MAP_TXMETA(osh, va, size, direction, p, dmah, pcma) ((dma_addr_t) ((0)))
#endif // endif
#define	SECURE_DMA_UNMAP(osh, pa, size, direction, p, dmah, pcma, offset)
#define	SECURE_DMA_UNMAP_ALL(osh, pcma)

#endif /* BCMDMA64OSL */

#if !defined(linux)
#define PKTLIST_INIT(x)			BCM_REFERENCE(x)
#define PKTLIST_ENQ(x, y)		BCM_REFERENCE(x)
#define PKTLIST_DEQ(x)			BCM_REFERENCE(x)
#define PKTLIST_UNLINK(x, y)		BCM_REFERENCE(x)
#define PKTLIST_FINI(osh, x)		({ BCM_REFERENCE(osh); BCM_REFERENCE(x); }
#endif /* ! linux */

#ifndef ROMMABLE_ASSERT
#define ROMMABLE_ASSERT(exp) ASSERT(exp)
#endif /* ROMMABLE_ASSERT */

#ifndef MALLOC_NOPERSIST
	#define MALLOC_NOPERSIST MALLOC
#endif /* !MALLOC_NOPERSIST */

#ifndef MALLOC_PERSIST
	#define MALLOC_PERSIST MALLOC
#endif /* !MALLOC_PERSIST */

#ifndef MALLOC_NOPERSIST
	#define MALLOC_NOPERSIST MALLOC
#endif /* !MALLOC_NOPERSIST */

#ifndef MALLOC_PERSIST_ATTACH
	#define MALLOC_PERSIST_ATTACH MALLOC
#endif /* !MALLOC_PERSIST_ATTACH */

#ifndef MALLOCZ_PERSIST_ATTACH
	#define MALLOCZ_PERSIST_ATTACH MALLOCZ
#endif /* !MALLOCZ_PERSIST_ATTACH */

#ifndef MALLOCZ_NOPERSIST
	#define MALLOCZ_NOPERSIST MALLOCZ
#endif /* !MALLOCZ_NOPERSIST */

#ifndef MALLOCZ_PERSIST
	#define MALLOCZ_PERSIST MALLOCZ
#endif /* !MALLOCZ_PERSIST */

#ifndef MFREE_PERSIST
	#define MFREE_PERSIST MFREE
#endif /* !MFREE_PERSIST */

#ifndef MALLOC_SET_NOPERSIST
	#define MALLOC_SET_NOPERSIST(osh)	do { } while (0)
#endif /* !MALLOC_SET_NOPERSIST */

#ifndef MALLOC_CLEAR_NOPERSIST
	#define MALLOC_CLEAR_NOPERSIST(osh)	do { } while (0)
#endif /* !MALLOC_CLEAR_NOPERSIST */

#if defined(OSL_MEMCHECK)
#define MEMCHECK(f, l)	osl_memcheck(f, l)
#else
#define MEMCHECK(f, l)
#endif /* OSL_MEMCHECK */

#endif	/* _osl_h_ */
