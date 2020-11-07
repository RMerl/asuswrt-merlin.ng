/*
 * Linux OS Independent Layer
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
 * $Id: linux_osl.h 782101 2019-12-10 22:09:09Z $
 */

#ifndef _linux_osl_h_
#define _linux_osl_h_

#include <typedefs.h>
#define DECLSPEC_ALIGN(x)	__attribute__ ((aligned(x)))

/* Linux Kernel: File Operations: start */
extern void * osl_os_open_image(char * filename);
extern int osl_os_get_image_block(char * buf, int len, void * image);
extern void osl_os_close_image(void * image);
extern int osl_os_image_size(void *image);
/* Linux Kernel: File Operations: end */

#ifdef BCMDRIVER

/* OSL initialization */
#ifdef SHARED_OSL_CMN
extern osl_t *osl_attach(void *pdev, uint bustype, bool pkttag, void **osh_cmn);
#else
extern osl_t *osl_attach(void *pdev, uint bustype, bool pkttag);
#endif /* SHARED_OSL_CMN */

extern void osl_detach(osl_t *osh);
extern int osl_static_mem_init(osl_t *osh, void *adapter);
extern int osl_static_mem_deinit(osl_t *osh, void *adapter);
extern void osl_set_bus_handle(osl_t *osh, void *bus_handle);
extern void* osl_get_bus_handle(osl_t *osh);

/* Global ASSERT type */
extern uint32 g_assert_type;

#ifdef CONFIG_PHYS_ADDR_T_64BIT
#define PRI_FMT_x       "llx"
#define PRI_FMT_X       "llX"
#define PRI_FMT_o       "llo"
#define PRI_FMT_d       "lld"
#else
#define PRI_FMT_x       "x"
#define PRI_FMT_X       "X"
#define PRI_FMT_o       "o"
#define PRI_FMT_d       "d"
#endif /* CONFIG_PHYS_ADDR_T_64BIT */
/* ASSERT */
#ifndef ASSERT
	#ifdef __GNUC__
		#define GCC_VERSION \
			(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
		#if GCC_VERSION > 30100
			#define ASSERT(exp)	do {} while (0)
		#else
			/* ASSERT could cause segmentation fault on GCC3.1, use empty instead */
			#define ASSERT(exp)
		#endif /* GCC_VERSION > 30100 */
	#endif /* __GNUC__ */
#endif /* ASSERT */

/* bcm_prefetch_32B */
static inline void bcm_prefetch_32B(const uint8 *addr, const int cachelines_32B)
{
#if (defined(BCM47XX_CA9) || (defined(STB) && defined(__arm__))) && (__LINUX_ARM_ARCH__ \
	>= 5)
	switch (cachelines_32B) {
		case 4: __asm__ __volatile__("pld\t%a0" :: "p"(addr + 96) : "cc");
		case 3: __asm__ __volatile__("pld\t%a0" :: "p"(addr + 64) : "cc");
		case 2: __asm__ __volatile__("pld\t%a0" :: "p"(addr + 32) : "cc");
		case 1: __asm__ __volatile__("pld\t%a0" :: "p"(addr +  0) : "cc");
	}
#elif defined(__mips__)
	/* Hint Pref_Load = 0 */
	switch (cachelines_32B) {
		case 4: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr + 96));
		case 3: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr + 64));
		case 2: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr + 32));
		case 1: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr +  0));
	}
#endif /* BCM47XX_CA9, __mips__ */
}

/* microsecond delay */
#define	OSL_DELAY(usec)		osl_delay(usec)
extern void osl_delay(uint usec);

#define OSL_SLEEP(ms)			osl_sleep(ms)
extern void osl_sleep(uint ms);

#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) \
	osl_pcmcia_read_attr((osh), (offset), (buf), (size))
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) \
	osl_pcmcia_write_attr((osh), (offset), (buf), (size))
extern void osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size);
extern void osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size);

/* PCI configuration space access macros */
#define	OSL_PCI_READ_CONFIG(osh, offset, size) \
	osl_pci_read_config((osh), (offset), (size))
#define	OSL_PCI_WRITE_CONFIG(osh, offset, size, val) \
	osl_pci_write_config((osh), (offset), (size), (val))
extern uint32 osl_pci_read_config(osl_t *osh, uint offset, uint size);
extern void osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val);

/* PCI device bus # and slot # */
#define OSL_PCI_BUS(osh)	osl_pci_bus(osh)
#define OSL_PCI_SLOT(osh)	osl_pci_slot(osh)
#define OSL_PCIE_DOMAIN(osh)	osl_pcie_domain(osh)
#define OSL_PCIE_BUS(osh)	osl_pcie_bus(osh)
extern uint osl_pci_bus(osl_t *osh);
extern uint osl_pci_slot(osl_t *osh);
extern uint osl_pcie_domain(osl_t *osh);
extern uint osl_pcie_bus(osl_t *osh);
extern struct pci_dev *osl_pci_device(osl_t *osh);

#define OSL_PCIE_MPS_LIMIT(osh, devctl_offset, mps)	osl_pcie_mps_limit(osh, devctl_offset, mps)
extern void osl_pcie_mps_limit(osl_t *osh, uint devctl_offset, uint mps);

#define OSL_ACP_COHERENCE		(1<<1L)
#define OSL_FWDERBUF			(1<<2L)

/* Pkttag flag should be part of public information */
typedef struct {
	bool pkttag;
	bool mmbus;		/**< Bus supports memory-mapped register accesses */
	pktfree_cb_fn_t tx_fn;  /**< Callback function for PKTFREE */
	void *tx_ctx;		/**< Context to the callback function */
#ifdef OSLREGOPS
	osl_rreg_fn_t rreg_fn;	/**< Read Register function */
	osl_wreg_fn_t wreg_fn;	/**< Write Register function */
	void *reg_ctx;		/**< Context to the reg callback functions */
#else
	void	*unused[3];
#endif // endif
	void (*rx_fn)(void *rx_ctx, void *p);
	void *rx_ctx;
	void (*stsbuf_free_cb_fn)(void *stsbuf_free_cb_ctx, void *p);
	void *stsbuf_free_cb_ctx;
#ifdef BCM_SKB_FREE_OFFLOAD
    bool skb_free_offload;
#endif // endif
} osl_pubinfo_t;

#ifdef BCM_NBUFF_WLMCAST
#include <dhd_nic_common.h>
#endif /* BCM_NBUFF_WLMCAST */

extern void osl_flag_set(osl_t *osh, uint32 mask);
extern void osl_flag_clr(osl_t *osh, uint32 mask);
extern bool osl_is_flag_set(osl_t *osh, uint32 mask);

#define PKTFREESETCB(osh, _tx_fn, _tx_ctx)		\
	do {						\
	   ((osl_pubinfo_t*)osh)->tx_fn = _tx_fn;	\
	   ((osl_pubinfo_t*)osh)->tx_ctx = _tx_ctx;	\
	} while (0)

#define PKTFREESETRXCB(osh, _rx_fn, _rx_ctx)		\
	do {						\
	   ((osl_pubinfo_t*)osh)->rx_fn = _rx_fn;	\
	   ((osl_pubinfo_t*)osh)->rx_ctx = _rx_ctx;	\
	} while (0)

#define PKTFREESETRXSTSCB(osh, _stsbuf_free_cb_fn, _stsbuf_free_cb_ctx)   \
	do {            \
		((osl_pubinfo_t*)osh)->stsbuf_free_cb_fn = _stsbuf_free_cb_fn;  \
		((osl_pubinfo_t*)osh)->stsbuf_free_cb_ctx = _stsbuf_free_cb_ctx;  \
	} while (0)

#ifdef OSLREGOPS
#define REGOPSSET(osh, rreg, wreg, ctx)			\
	do {						\
	   ((osl_pubinfo_t*)osh)->rreg_fn = rreg;	\
	   ((osl_pubinfo_t*)osh)->wreg_fn = wreg;	\
	   ((osl_pubinfo_t*)osh)->reg_ctx = ctx;	\
	} while (0)
#endif /* OSLREGOPS */

/* host/bus architecture-specific byte swap */
#define BUS_SWAP32(v)		(v)
	#define MALLOC(osh, size)	osl_malloc((osh), (size))
	#define MALLOCZ(osh, size)	osl_mallocz((osh), (size))
	#define MFREE(osh, addr, size)	osl_mfree((osh), (addr), (size))
	#define VMALLOC(osh, size)	osl_vmalloc((osh), (size))
	#define VMALLOCZ(osh, size)	osl_vmallocz((osh), (size))
	#define VMFREE(osh, addr, size)	osl_vmfree((osh), (addr), (size))
	#define MALLOCED(osh)		osl_malloced((osh))
	#define MEMORY_LEFTOVER(osh) osl_check_memleak(osh)
	extern void *osl_malloc(osl_t *osh, uint size);
	extern void *osl_mallocz(osl_t *osh, uint size);
	extern void osl_mfree(osl_t *osh, void *addr, uint size);
	extern void *osl_vmalloc(osl_t *osh, uint size);
	extern void *osl_vmallocz(osl_t *osh, uint size);
	extern void osl_vmfree(osl_t *osh, void *addr, uint size);
	extern uint osl_malloced(osl_t *osh);
	extern uint osl_check_memleak(osl_t *osh);

#define	MALLOC_FAILED(osh)	osl_malloc_failed((osh))
extern uint osl_malloc_failed(osl_t *osh);

/* allocate/free shared (dma-able) consistent memory */
#define	DMA_CONSISTENT_ALIGN	osl_dma_consistent_align()
#define	DMA_ALLOC_CONSISTENT(osh, size, align, tot, pap, dmah) \
	osl_dma_alloc_consistent((osh), (size), (align), (tot), (pap))
#define	DMA_FREE_CONSISTENT(osh, va, size, pa, dmah) \
	osl_dma_free_consistent((osh), (void*)(va), (size), (pa))

#define	DMA_ALLOC_CONSISTENT_FORCE32(osh, size, align, tot, pap, dmah) \
	osl_dma_alloc_consistent((osh), (size), (align), (tot), (pap))
#define	DMA_FREE_CONSISTENT_FORCE32(osh, va, size, pa, dmah) \
	osl_dma_free_consistent((osh), (void*)(va), (size), (pa))

extern uint osl_dma_consistent_align(void);
extern void *osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align,
	uint *tot, dmaaddr_t *pap);
extern void osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa);

/* map/unmap direction */
#define DMA_NO	0	/* Used to skip cache op */
#define	DMA_TX	1	/* TX direction for DMA */
#define	DMA_RX	2	/* RX direction for DMA */

/* map/unmap shared (dma-able) memory */
#define	DMA_UNMAP(osh, pa, size, direction, p, dmah) \
	osl_dma_unmap((osh), (pa), (size), (direction))
extern void osl_dma_flush(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *txp_dmah);
extern void osl_dma_sync(osl_t *osh, dmaaddr_t pa, uint size, int direction);
extern dmaaddr_t osl_dma_map(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *txp_dmah);
extern void osl_dma_unmap(osl_t *osh, dmaaddr_t pa, uint size, int direction);

#ifndef PHYS_TO_VIRT
#define	PHYS_TO_VIRT(pa)	osl_phys_to_virt(pa)
#endif // endif
#ifndef VIRT_TO_PHYS
#define	VIRT_TO_PHYS(va)	osl_virt_to_phys(va)
#endif // endif
extern void * osl_phys_to_virt(void * pa);
extern void * osl_virt_to_phys(void * va);

/* API for DMA addressing capability */
#define OSL_DMADDRWIDTH(osh, addrwidth) ({BCM_REFERENCE(osh); BCM_REFERENCE(addrwidth);})

#define OSL_SMP_WMB()	smp_wmb()

#if defined(CONFIG_BCM_GLB_COHERENCY)
/* Compile time macro OSL_CACHE_COHERENT, instead of OSL_ARCH_IS_COHERENT() */
#define OSL_CACHE_COHERENT    1
#define ACP_WAR_ENAB()        0
#define ACP_WIN_LIMIT          (~0ULL)    /* entire memory */
#define arch_is_coherent()     1
#else /* !CONFIG_BCM_GLB_COHERENCY */
#if defined(BCA_HNDROUTER) && defined(__ARM_ARCH_7A__)
/* ACP definitions for 47189 + Linux-4.1 */
#define ACP_WAR_ENAB()	0		/* Do not need this WAR for 47189 b1 */
#define ACP_WIN_LIMIT	0xff	/* Added to passthrough the compilation */
#define arch_is_coherent()	0
#endif /* BCA_HNDROUTER && __ARM_ARCH_7A__ */
#endif /* !CONFIG_BCM_GLB_COHERENCY */

/* API for CPU relax */
extern void osl_cpu_relax(void);
#define OSL_CPU_RELAX() osl_cpu_relax()

extern void osl_preempt_disable(osl_t *osh);
extern void osl_preempt_enable(osl_t *osh);
#define OSL_DISABLE_PREEMPTION(osh)	osl_preempt_disable(osh)
#define OSL_ENABLE_PREEMPTION(osh)	osl_preempt_enable(osh)

#if defined(__mips__) || (!defined(DHD_USE_COHERENT_MEM_FOR_RING) && \
	defined(__ARM_ARCH_7A__)) || defined(STB_SOC_WIFI) || (defined(BCA_HNDROUTER) && \
	defined(__aarch64__))
	extern void osl_cache_flush(void *va, uint size);
	extern void osl_cache_inv(void *va, uint size);
	extern void osl_prefetch(const void *ptr);
	#define OSL_CACHE_FLUSH(va, len)	osl_cache_flush((void *)(va), len)
	#define OSL_CACHE_INV(va, len)		osl_cache_inv((void *)(va), len)
	#define OSL_PREFETCH(ptr)			osl_prefetch(ptr)
#if defined(__ARM_ARCH_7A__) || defined(STB_SOC_WIFI)
	extern int osl_arch_is_coherent(void);
	#define OSL_ARCH_IS_COHERENT()		osl_arch_is_coherent()
	extern int osl_acp_war_enab(void);
	#define OSL_ACP_WAR_ENAB()			osl_acp_war_enab()
#if defined(BCM47XX_CA9) && defined(BCM_GMAC3)
	/* Partial ACP mode is ONLY supported on Atlas */
	extern void cache_flush_line(void *va);
	extern void cache_inv_line(void *va);
	extern void osl_cache_flush_noacp(void *va, uint len);
	extern void osl_cache_inv_noacp(void *va, uint len);
	#define OSL_CACHE_FLUSH_LINE(va)    cache_flush_line((void*)(va))
	#define OSL_CACHE_INV_LINE(va)      cache_inv_line((void*)(va))
	#define OSL_CACHE_FLUSH_NOACP(va, len) cache_flush_len((void*)(va), (len))
	#define OSL_CACHE_INV_NOACP(va, len) cache_inv_len((void*)(va), (len))
#endif /* BCM47XX_CA9 && BCM_GMAC3 */
#else  /* !__ARM_ARCH_7A__ */
	#define OSL_ARCH_IS_COHERENT()		NULL
	#define OSL_ACP_WAR_ENAB()			NULL
#endif /* !__ARM_ARCH_7A__ */
#else  /* !__mips__ && !__ARM_ARCH_7A__ */
	#define OSL_CACHE_FLUSH(va, len)	BCM_REFERENCE(va)
	#define OSL_CACHE_INV(va, len)		BCM_REFERENCE(va)
	#define OSL_PREFETCH(ptr)		BCM_REFERENCE(ptr)

	#define OSL_ARCH_IS_COHERENT()		NULL
	#define OSL_ACP_WAR_ENAB()			NULL
#endif /* !__mips__ && !__ARM_ARCH_7A__ */

#ifdef BCM_BACKPLANE_TIMEOUT
extern void osl_set_bpt_cb(osl_t *osh, void *bpt_cb, void *bpt_ctx);
extern void osl_bpt_rreg(osl_t *osh, ulong addr, volatile void *v, uint size);
#endif /* BCM_BACKPLANE_TIMEOUT */

#if defined(BCM47XX_CA9) || (defined(STB) && defined(__arm__))
extern void osl_pcie_rreg(osl_t *osh, ulong addr, volatile void *v, uint size);
#endif	/* BCM47XX_CA9 || (STB && __arm__) */

/* register access macros */
#if defined(BCMJTAG)
	#include <bcmjtag.h>
	#define OSL_WRITE_REG(osh, r, v) \
		({ \
		 BCM_REFERENCE(osh); \
		 bcmjtag_write(NULL, (uintptr)(r), (v), sizeof(*(r))); \
		 })
	#define OSL_READ_REG(osh, r) \
		({ \
		 BCM_REFERENCE(osh); \
		 bcmjtag_read(NULL, (uintptr)(r), sizeof(*(r)));
		 })
#elif defined(BCM_BACKPLANE_TIMEOUT)
#define OSL_READ_REG(osh, r) \
	({\
		__typeof(*(r)) __osl_v; \
		osl_bpt_rreg(osh, (uintptr)(r), &__osl_v, sizeof(*(r))); \
		__osl_v; \
	})
#elif (defined(BCM47XX_CA9) || (defined(STB) && defined(__arm__)))
#define OSL_READ_REG(osh, r) \
	({\
		__typeof(*(r)) __osl_v; \
		osl_pcie_rreg(osh, (uintptr)(r), &__osl_v, sizeof(*(r))); \
		__osl_v; \
	})
#endif // endif

#if defined(BCM47XX_CA9) || defined(BCM_BACKPLANE_TIMEOUT) || (defined(STB) && \
	defined(__arm__))
	#define SELECT_BUS_WRITE(osh, mmap_op, bus_op) ({BCM_REFERENCE(osh); mmap_op;})
	#define SELECT_BUS_READ(osh, mmap_op, bus_op) ({BCM_REFERENCE(osh); bus_op;})
#else /* !BCM47XX_CA9 && !BCM_BACKPLANE_TIMEOUT && !(STB && __arm__) */
#if defined(BCMJTAG)
	#define SELECT_BUS_WRITE(osh, mmap_op, bus_op) if (((osl_pubinfo_t*)(osh))->mmbus) \
		mmap_op else bus_op
	#define SELECT_BUS_READ(osh, mmap_op, bus_op) (((osl_pubinfo_t*)(osh))->mmbus) ? \
		mmap_op : bus_op
#else
	#define SELECT_BUS_WRITE(osh, mmap_op, bus_op) ({BCM_REFERENCE(osh); mmap_op;})
	#define SELECT_BUS_READ(osh, mmap_op, bus_op) ({BCM_REFERENCE(osh); mmap_op;})
#endif // endif
#endif /* BCM47XX_CA9 || BCM_BACKPLANE_TIMEOUT || (STB && __arm__) */

#define OSL_ERROR(bcmerror)	osl_error(bcmerror)
extern int osl_error(int bcmerror);

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#if defined(WL_EAP_AMSDU_CRYPTO_OFFLD)
/* When this conditional is turned on and run-time configuration is enabled,
 * host can receive frames larger than MTU size. Right now this is set to default size.
 * This should be set to higher value for larger A-MSDU frame sizes.
 */
#define	PKTBUFSZ	2048
#else
#define	PKTBUFSZ	2048   /* largest reasonable packet buffer, driver uses for ethernet MTU */
#endif // endif

#define OSH_NULL   NULL

/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 * Macros expand to calls to functions defined in linux_osl.c .
 */
#ifndef BINOSL
#include <linuxver.h>           /* use current 2.4.x calling conventions */
#include <linux/kernel.h>       /* for vsn/printf's */
#include <linux/string.h>       /* for mem*, str* */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 29)
extern uint64 osl_sysuptime_us(void);
#define OSL_SYSUPTIME()		((uint32)jiffies_to_msecs(jiffies))
#define OSL_SYSUPTIME_US()	osl_sysuptime_us()
#else
#define OSL_SYSUPTIME()		((uint32)jiffies * (1000 / HZ))
#error "OSL_SYSUPTIME_US() may need to be defined"
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 29) */
#define	printf(fmt, args...)	printk(fmt , ## args)
#include <linux/kernel.h>	/* for vsn/printf's */
#include <linux/string.h>	/* for mem*, str* */
/* bcopy's: Linux kernel doesn't provide these (anymore) */
#define	bcopy_hw(src, dst, len)		memcpy((dst), (src), (len))
#define	bcopy_hw_async(src, dst, len)	memcpy((dst), (src), (len))
#define	bcopy_hw_poll_for_completion()
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), '\0', (len))

/* register access macros */
#if defined(OSLREGOPS)
#define R_REG(osh, r) (\
	sizeof(*(r)) == sizeof(uint8) ? osl_readb((osh), (volatile uint8*)(r)) : \
	sizeof(*(r)) == sizeof(uint16) ? osl_readw((osh), (volatile uint16*)(r)) : \
	osl_readl((osh), (volatile uint32*)(r)) \
)
#define W_REG(osh, r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	osl_writeb((osh), (volatile uint8*)(r), (uint8)(v)); break; \
	case sizeof(uint16):	osl_writew((osh), (volatile uint16*)(r), (uint16)(v)); break; \
	case sizeof(uint32):	osl_writel((osh), (volatile uint32*)(r), (uint32)(v)); break; \
	} \
} while (0)

extern uint8 osl_readb(osl_t *osh, volatile uint8 *r);
extern uint16 osl_readw(osl_t *osh, volatile uint16 *r);
extern uint32 osl_readl(osl_t *osh, volatile uint32 *r);
extern void osl_writeb(osl_t *osh, volatile uint8 *r, uint8 v);
extern void osl_writew(osl_t *osh, volatile uint16 *r, uint16 v);
extern void osl_writel(osl_t *osh, volatile uint32 *r, uint32 v);

#else /* OSLREGOPS */

#ifndef IL_BIGENDIAN
#ifndef __mips__
#ifdef CONFIG_64BIT
/* readq is defined only for 64 bit platform */
#define R_REG(osh, r) (\
	SELECT_BUS_READ(osh, \
		({ \
			__typeof(*(r)) __osl_v = 0; \
			BCM_REFERENCE(osh);	\
			switch (sizeof(*(r))) { \
				case sizeof(uint8):	__osl_v = \
					readb((volatile uint8*)(r)); break; \
				case sizeof(uint16):	__osl_v = \
					readw((volatile uint16*)(r)); break; \
				case sizeof(uint32):	__osl_v = \
					readl((volatile uint32*)(r)); break; \
				case sizeof(uint64):	__osl_v = \
					readq((volatile uint64*)(r)); break; \
			} \
			__osl_v; \
		}), \
		OSL_READ_REG(osh, r)) \
)
#else /* !CONFIG_64BIT */
#define R_REG(osh, r) (\
	SELECT_BUS_READ(osh, \
		({ \
			__typeof(*(r)) __osl_v = 0; \
			switch (sizeof(*(r))) { \
				case sizeof(uint8):	__osl_v = \
					readb((volatile uint8*)(r)); break; \
				case sizeof(uint16):	__osl_v = \
					readw((volatile uint16*)(r)); break; \
				case sizeof(uint32):	__osl_v = \
					readl((volatile uint32*)(r)); break; \
			} \
			__osl_v; \
		}), \
		OSL_READ_REG(osh, r)) \
)
#endif /* CONFIG_64BIT */

#else /* __mips__ */

#ifdef CONFIG_64BIT
/* readq is defined only for 64 bit platform */
#define R_REG(osh, r) (\
	SELECT_BUS_READ(osh, \
		({ \
			__typeof(*(r)) __osl_v = 0; \
			__asm__ __volatile__("sync"); \
			switch (sizeof(*(r))) { \
				case sizeof(uint8):	__osl_v = \
					readb((volatile uint8*)(r)); break; \
				case sizeof(uint16):	__osl_v = \
					readw((volatile uint16*)(r)); break; \
				case sizeof(uint32):	__osl_v = \
					readl((volatile uint32*)(r)); break; \
				case sizeof(uint64):	__osl_v = \
					readq((volatile uint64*)(r)); break; \
			} \
			__asm__ __volatile__("sync"); \
			__osl_v; \
		}), \
		({ \
			__typeof(*(r)) __osl_v; \
			__asm__ __volatile__("sync"); \
			__osl_v = OSL_READ_REG(osh, r); \
			__asm__ __volatile__("sync"); \
			__osl_v; \
		})) \
)
#else /* !CONFIG_64BIT */
#define R_REG(osh, r) (\
	SELECT_BUS_READ(osh, \
		({ \
			__typeof(*(r)) __osl_v = 0; \
			__asm__ __volatile__("sync"); \
			switch (sizeof(*(r))) { \
				case sizeof(uint8):	__osl_v = \
					readb((volatile uint8*)(r)); break; \
				case sizeof(uint16):	__osl_v = \
					readw((volatile uint16*)(r)); break; \
				case sizeof(uint32):	__osl_v = \
					readl((volatile uint32*)(r)); break; \
			} \
			__asm__ __volatile__("sync"); \
			__osl_v; \
		}), \
		({ \
			__typeof(*(r)) __osl_v__; \
			__asm__ __volatile__("sync"); \
			__osl_v__ = OSL_READ_REG(osh, r); \
			__asm__ __volatile__("sync"); \
			__osl_v__; \
		})) \
)
#endif /* CONFIG_64BIT */
#endif /* __mips__ */

#ifdef CONFIG_64BIT
/* writeq is defined only for 64 bit platform */
#define W_REG(osh, r, v) do { \
	SELECT_BUS_WRITE(osh, \
		switch (sizeof(*(r))) { \
			case sizeof(uint8):	writeb((uint8)(v), (volatile uint8*)(r)); break; \
			case sizeof(uint16):	writew((uint16)(v), (volatile uint16*)(r)); break; \
			case sizeof(uint32):	writel((uint32)(v), (volatile uint32*)(r)); break; \
			case sizeof(uint64):	writeq((uint64)(v), (volatile uint64*)(r)); break; \
		}, \
		(OSL_WRITE_REG(osh, r, v))); \
	} while (0)

#else /* !CONFIG_64BIT */
#define W_REG(osh, r, v) do { \
	SELECT_BUS_WRITE(osh, \
		switch (sizeof(*(r))) { \
			case sizeof(uint8):	writeb((uint8)(v), (volatile uint8*)(r)); break; \
			case sizeof(uint16):	writew((uint16)(v), (volatile uint16*)(r)); break; \
			case sizeof(uint32):	writel((uint32)(v), (volatile uint32*)(r)); break; \
		}, \
		(OSL_WRITE_REG(osh, r, v))); \
	} while (0)
#endif /* CONFIG_64BIT */

#else	/* IL_BIGENDIAN */

#ifdef CONFIG_64BIT
/* readq and writeq is defined only for 64 bit platform */
#define R_REG(osh, r) (\
	SELECT_BUS_READ(osh, \
		({ \
			__typeof(*(r)) __osl_v = 0; \
			switch (sizeof(*(r))) { \
				case sizeof(uint8):	__osl_v = \
					readb((volatile uint8*)((uintptr)(r)^3)); break; \
				case sizeof(uint16):	__osl_v = \
					readw((volatile uint16*)((uintptr)(r)^2)); break; \
				case sizeof(uint32):	__osl_v = \
					readl((volatile uint32*)(r)); break; \
				case sizeof(uint64):    __osl_v = \
					readq((volatile uint64*)(r)); break; \
			} \
			__osl_v; \
		}), \
		OSL_READ_REG(osh, r)) \
)
#define W_REG(osh, r, v) do { \
	SELECT_BUS_WRITE(osh, \
		switch (sizeof(*(r))) { \
			case sizeof(uint8):	writeb((uint8)(v), \
					(volatile uint8*)((uintptr)(r)^3)); break; \
			case sizeof(uint16):	writew((uint16)(v), \
					(volatile uint16*)((uintptr)(r)^2)); break; \
			case sizeof(uint32):	writel((uint32)(v), \
					(volatile uint32*)(r)); break; \
			case sizeof(uint64):	writeq((uint64)(v), \
					(volatile uint64*)(r)); break; \
		}, \
		(OSL_WRITE_REG(osh, r, v))); \
	} while (0)

#else /* !CONFIG_64BIT */
#define R_REG(osh, r) (\
	SELECT_BUS_READ(osh, \
		({ \
			__typeof(*(r)) __osl_v = 0; \
			switch (sizeof(*(r))) { \
				case sizeof(uint8):	__osl_v = \
					readb((volatile uint8*)((uintptr)(r)^3)); break; \
				case sizeof(uint16):	__osl_v = \
					readw((volatile uint16*)((uintptr)(r)^2)); break; \
				case sizeof(uint32):	__osl_v = \
					readl((volatile uint32*)(r)); break; \
			} \
			__osl_v; \
		}), \
		OSL_READ_REG(osh, r)) \
)
#define W_REG(osh, r, v) do { \
	SELECT_BUS_WRITE(osh, \
		switch (sizeof(*(r))) { \
			case sizeof(uint8):	writeb((uint8)(v), \
					(volatile uint8*)((uintptr)(r)^3)); break; \
			case sizeof(uint16):	writew((uint16)(v), \
					(volatile uint16*)((uintptr)(r)^2)); break; \
			case sizeof(uint32):	writel((uint32)(v), \
					(volatile uint32*)(r)); break; \
		}, \
		(OSL_WRITE_REG(osh, r, v))); \
	} while (0)
#endif /* CONFIG_64BIT */
#endif /* IL_BIGENDIAN */

#endif /* OSLREGOPS */

#define	AND_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) & (v))
#define	OR_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) | (v))

/* bcopy, bcmp, and bzero functions */
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), '\0', (len))

/* uncached/cached virtual address */
#ifdef __mips__
#include <asm/addrspace.h>
#define OSL_UNCACHED(va)	((void *)KSEG1ADDR((va)))
#define OSL_CACHED(va)		((void *)KSEG0ADDR((va)))
#else
#define OSL_UNCACHED(va)	((void *)va)
#define OSL_CACHED(va)		((void *)va)
#endif /* mips */

#ifdef __mips__
#define OSL_PREF_RANGE_LD(va, sz) prefetch_range_PREF_LOAD_RETAINED(va, sz)
#define OSL_PREF_RANGE_ST(va, sz) prefetch_range_PREF_STORE_RETAINED(va, sz)
#else /* __mips__ */
#define OSL_PREF_RANGE_LD(va, sz) BCM_REFERENCE(va)
#define OSL_PREF_RANGE_ST(va, sz) BCM_REFERENCE(va)
#endif /* __mips__ */

/* get processor cycle count */
#if defined(mips)
#define	OSL_GETCYCLES(x)	((x) = read_c0_count() * 2)
#elif defined(__i386__)
#define	OSL_GETCYCLES(x)	rdtscl((x))
#else
#define OSL_GETCYCLES(x)	((x) = 0)
#endif /* defined(mips) */

/* dereference an address that may cause a bus exception */
#ifdef mips
#if defined(MODULE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 17))
#define BUSPROBE(val, addr)	panic("get_dbe() will not fixup a bus exception when compiled into"\
					" a module")
#else
#define	BUSPROBE(val, addr)	get_dbe((val), (addr))
#include <asm/paccess.h>
#endif /* defined(MODULE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 17)) */
#else
#define	BUSPROBE(val, addr)	({ (val) = R_REG(NULL, (addr)); 0; })
#endif /* mips */

/* map/unmap physical to virtual I/O */
#if !defined(CONFIG_MMC_MSM7X00A)
#define	REG_MAP(pa, size)	ioremap_nocache((unsigned long)(pa), (unsigned long)(size))
#else
#define REG_MAP(pa, size)       (void *)(0)
#endif /* !defined(CONFIG_MMC_MSM7X00A */
#define	REG_UNMAP(va)		iounmap((va))

/* shared (dma-able) memory access macros */
#define	R_SM(r)			*(r)
#define	W_SM(r, v)		(*(r) = (v))
#define	BZERO_SM(r, len)	memset((r), '\0', (len))

/* Because the non BINOSL implemenation of the PKT OSL routines are macros (for
 * performance reasons),  we need the Linux headers.
 */
/* XXX REVISIT  Is there a more specific header file we should be including for the
 * struct/definitions we need? johnvb
 */
#include <linuxver.h>		/* use current 2.4.x calling conventions */

#else	/* BINOSL */

/* Where to get the declarations for mem, str, printf, bcopy's? Two basic approaches.
 *
 * First, use the Linux header files and the C standard library replacmenent versions
 * built-in to the kernel.  Use this approach when compiling non hybrid code or compling
 * the OS port files.  The second approach is to use our own defines/prototypes and
 * functions we have provided in the Linux OSL, i.e. linux_osl.c.  Use this approach when
 * compiling the files that make up the hybrid binary.  We are ensuring we
 * don't directly link to the kernel replacement routines from the hybrid binary.
 *
 * NOTE: The issue we are trying to avoid is any questioning of whether the
 * hybrid binary is derived from Linux.  The wireless common code (wlc) is designed
 * to be OS independent through the use of the OSL API and thus the hybrid binary doesn't
 * derive from the Linux kernel at all.  But since we defined our OSL API to include
 * a small collection of standard C library routines and these routines are provided in
 * the kernel we want to avoid even the appearance of deriving at all even though clearly
 * usage of a C standard library API doesn't represent a derivation from Linux.  Lastly
 * note at the time of this checkin 4 references to memcpy/memset could not be eliminated
 * from the binary because they are created internally by GCC as part of things like
 * structure assignment.  I don't think the compiler should be doing this but there is
 * no options to disable it on Intel architectures (there is for MIPS so somebody must
 * agree with me).  I may be able to even remove these references eventually with
 * a GNU binutil such as objcopy via a symbol rename (i.e. memcpy to osl_memcpy).
 */
	#define	printf(fmt, args...)	printk(fmt , ## args)
	#include <linux/kernel.h>	/* for vsn/printf's */
	#include <linux/string.h>	/* for mem*, str* */
	/* bcopy's: Linux kernel doesn't provide these (anymore) */
	#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
	#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
	#define	bzero(b, len)		memset((b), '\0', (len))

	/* These are provided only because when compiling linux_osl.c there
	 * must be an explicit prototype (separate from the definition) because
	 * we are compiling with GCC option -Wstrict-prototypes.  Conversely
	 * these could be placed directly in linux_osl.c.
	 */
	extern int osl_printf(const char *format, ...);
	extern int osl_sprintf(char *buf, const char *format, ...);
	extern int osl_snprintf(char *buf, size_t n, const char *format, ...);
	extern int osl_vsprintf(char *buf, const char *format, va_list ap);
	extern int osl_vsnprintf(char *buf, size_t n, const char *format, va_list ap);
	extern int osl_strcmp(const char *s1, const char *s2);
	extern int osl_strncmp(const char *s1, const char *s2, uint n);
	extern int osl_strlen(const char *s);
	extern char* osl_strcpy(char *d, const char *s);
	extern char* osl_strncpy(char *d, const char *s, uint n);
	extern char* osl_strchr(const char *s, int c);
	extern char* osl_strrchr(const char *s, int c);
	extern void *osl_memset(void *d, int c, size_t n);
	extern void *osl_memcpy(void *d, const void *s, size_t n);
	extern void *osl_memmove(void *d, const void *s, size_t n);
	extern int osl_memcmp(const void *s1, const void *s2, size_t n);

/* register access macros */
#if !defined(BCMJTAG)
#define R_REG(osh, r) \
	({ \
	 BCM_REFERENCE(osh); \
	 sizeof(*(r)) == sizeof(uint8) ? osl_readb((volatile uint8*)(r)) : \
	 sizeof(*(r)) == sizeof(uint16) ? osl_readw((volatile uint16*)(r)) : \
	 osl_readl((volatile uint32*)(r)); \
	 })
#define W_REG(osh, r, v) do { \
	BCM_REFERENCE(osh); \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	osl_writeb((uint8)(v), (volatile uint8*)(r)); break; \
	case sizeof(uint16):	osl_writew((uint16)(v), (volatile uint16*)(r)); break; \
	case sizeof(uint32):	osl_writel((uint32)(v), (volatile uint32*)(r)); break; \
	} \
} while (0)

/* else added by johnvb to make sdio and jtag work with BINOSL, at least compile ... UNTESTED */
#else
#define R_REG(osh, r) OSL_READ_REG(osh, r)
#define W_REG(osh, r, v) do { OSL_WRITE_REG(osh, r, v); } while (0)
#endif // endif

#define	AND_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) & (v))
#define	OR_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) | (v))
extern uint8 osl_readb(volatile uint8 *r);
extern uint16 osl_readw(volatile uint16 *r);
extern uint32 osl_readl(volatile uint32 *r);
extern void osl_writeb(uint8 v, volatile uint8 *r);
extern void osl_writew(uint16 v, volatile uint16 *r);
extern void osl_writel(uint32 v, volatile uint32 *r);

/* system up time in ms */
#define OSL_SYSUPTIME()		osl_sysuptime()
extern uint32 osl_sysuptime(void);

/* uncached/cached virtual address */
#define OSL_UNCACHED(va)	osl_uncached((va))
extern void *osl_uncached(void *va);
#define OSL_CACHED(va)		osl_cached((va))
extern void *osl_cached(void *va);

#define OSL_PREF_RANGE_LD(va, sz)
#define OSL_PREF_RANGE_ST(va, sz)

/* get processor cycle count */
#define OSL_GETCYCLES(x)	((x) = osl_getcycles())
extern uint osl_getcycles(void);

/* dereference an address that may target abort */
#define	BUSPROBE(val, addr)	osl_busprobe(&(val), (addr))
extern int osl_busprobe(uint32 *val, uint32 addr);

/* map/unmap physical to virtual */
#define	REG_MAP(pa, size)	osl_reg_map((pa), (size))
#define	REG_UNMAP(va)		osl_reg_unmap((va))
extern void *osl_reg_map(uint32 pa, uint size);
extern void osl_reg_unmap(void *va);

/* shared (dma-able) memory access macros */
#define	R_SM(r)			*(r)
#define	W_SM(r, v)		(*(r) = (v))
#define	BZERO_SM(r, len)	bzero((r), (len))

#endif	/* BINOSL */

#define OSL_RAND()		osl_rand()
extern uint32 osl_rand(void);

#ifdef CTFMAP
/* XXX hndctf.h includes bcmutils.h so it needs to be after BCMDBG_PKT stuffs,
 * which has the definition of BCMDBG_PTRACE in it. Otherwise bcmutuils.h won't
 * see BCMDBG_PTRACE it wants.
 */
#include <ctf/hndctf.h>
#define	CTFMAPSZ	320
#define	DMA_MAP(osh, va, size, direction, p, dmah) \
({ \
	typeof(size) sz = (size); \
	if (p && PKTISCTF((osh), (p))) { \
		sz = (((size) < CTFMAPSZ) ? (size) : CTFMAPSZ); \
		CTFMAPPTR((osh), (p)) = (void *)(((uint8 *)(va)) + sz); \
	} \
	osl_dma_map((osh), (va), sz, (direction), (p), (dmah)); \
})
#if defined(__mips__)
#define	_DMA_MAP(osh, va, size, direction, p, dmah) \
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(direction); \
	 BCM_REFERENCE(p); \
	 BCM_REFERENCE(dmah); \
	 dma_cache_inv((uint)(va), (size)); \
	 })
#elif defined(__ARM_ARCH_7A__) && !defined(BCM_SECURE_DMA)
#include <asm/cacheflush.h>
#define	_DMA_MAP(osh, va, size, direction, p, dmah) \
	osl_dma_map((osh), (va), (size), (direction), (p), (dmah))
#else
#define	_DMA_MAP(osh, va, size, direction, p, dmah)	BCM_REFERENCE(osh)
#endif // endif

#define	DMA_SYNC(osh, pa, size, direction)		BCM_REFERENCE(osh)

#else /* CTFMAP */
#define	DMA_FLUSH(osh, va, size, direction, p, dmah) \
	osl_dma_flush((osh), (va), (size), (direction), (p), (dmah))
#define	DMA_SYNC(osh, pa, size, direction) \
	osl_dma_sync((osh), (pa), (size), (direction))
#if !defined(BCM_SECURE_DMA)
#define DMA_MAP(osh, va, size, direction, p, dmah) \
	osl_dma_map((osh), (va), (size), (direction), (p), (dmah))

#define BULK_DMA_MAP(osh, dmah, map_start, map_end) \
	({\
		BCM_REFERENCE(dmah); \
		BCM_REFERENCE(osh); \
		BCM_REFERENCE(map_start); \
		BCM_REFERENCE(map_end); \
	})

#define BULK_DMA_UNMAP(osh, dmah, map_start, map_end) \
	({\
		BCM_REFERENCE(dmah); \
		BCM_REFERENCE(osh); \
		BCM_REFERENCE(map_start); \
		BCM_REFERENCE(map_end); \
	})

#endif /* !(defined(BCM_SECURE_DMA)) */
#endif /* CTFMAP */

#else /* ! BCMDRIVER */

/* XXX  Non BCMDRIVER code "OSL".
 *   There are only a very limited number of OSL API's made available here:
 *     mem*'s, str*'s, b*'s, *printf's, MALLOC/MFREE and ASSERT.  All others are
 *   missing.  This doesn't really seem like an OSL implementation.  I am wondering
 *   if non BCMDRIVER code should be using a different header file defined for that
 *   purpose.  johnvb.
 */

/* ASSERT */
	#define ASSERT(exp)	do {} while (0)

/* MALLOC and MFREE */
#define MALLOC(o, l) malloc(l)
#define MFREE(o, p, l) free(p)
#include <stdlib.h>

/* str* and mem* functions */
#include <string.h>

/* *printf functions */
#include <stdio.h>

/* bcopy, bcmp, and bzero */
extern void bcopy(const void *src, void *dst, size_t len);
extern int bcmp(const void *b1, const void *b2, size_t len);
extern void bzero(void *b, size_t len);
#endif /* ! BCMDRIVER */

/* Current STB 7445D1 doesn't use ACP and it is non-coherrent.
 * Adding these dummy values for build apss only
 * When we revisit need to change these.
 */
#if defined(STBLINUX)

#if defined(__ARM_ARCH_7A__) || defined(STB_SOC_WIFI)
#define ACP_WAR_ENAB() 0
#define ACP_WIN_LIMIT 1
#define arch_is_coherent() 0
#endif /* __ARM_ARCH_7A__ */

#endif /* STBLINUX */

#ifdef BCM_SECURE_DMA

#define	SECURE_DMA_MAP(osh, va, size, direction, p, dmah, pcma, offset, buftype) \
	osl_sec_dma_map((osh), (va), (size), (direction), (p), (dmah), (pcma), (offset), (buftype))
#define	SECURE_DMA_DD_MAP(osh, va, size, direction, p, dmah) \
	osl_sec_dma_dd_map((osh), (va), (size), (direction), (p), (dmah))
#define	SECURE_DMA_MAP_TXMETA(osh, va, size, direction, p, dmah, pcma) \
	osl_sec_dma_map_txmeta((osh), (va), (size), (direction), (p), (dmah), (pcma))
#define	SECURE_DMA_UNMAP(osh, pa, size, direction, p, dmah, pcma, offset) \
	osl_sec_dma_unmap((osh), (pa), (size), (direction), (p), (dmah), (pcma), (offset))
#define	SECURE_DMA_UNMAP_ALL(osh, pcma) \
	osl_sec_dma_unmap_all((osh), (pcma))

#define DMA_MAP(osh, va, size, direction, p, dmah)

#define	SECURE_DMA_BUFFS_IS_AVAIL(osh) \
	osl_sec_dma_buffs_is_avail((osh))

#define	SECURE_DMA_RX_BUFFS_IS_AVAIL(osh) \
	osl_sec_dma_rx_buffs_is_avail((osh))

#define	SECURE_DMA_RXCTL_BUFFS_IS_AVAIL(osh) \
	osl_sec_dma_rxctl_buffs_is_avail((osh))

typedef struct sec_cma_info {
	struct sec_mem_elem *sec_alloc_list;
	struct sec_mem_elem *sec_alloc_list_tail;
} sec_cma_info_t;

/*
 * Total SECDMA memory Reserved is 40M.
 * This secdma memory will be used by both CMA_DMA_DATA_MEMBLOCK and CMA_DMA_DESC_MEMBLOCK
 * CMA_DMA_DESC_MEMBLOCK size	= (0x6000 * 340) + 0x100000 = Apprximately 9M
 * CMA_DMA_DATA_MEMBLOCK	= 20M - CMA_DMA_DESC_MEMBLOCK = 31M
 * Total 4K buffers		= CMA_DMA_DATA_MEMBLOCK/CMA_BUFSIZE_4K = 7936.
 *
 * Now All Avaibale 4K buffers are divided in to 3 pools as below.
 * RXBUF POST Pool		= CMA_RXBUF_BUFNUM  4K Buffers = 2048
 * RXCTR_BUF_POST POST Pool	= RXCTR_BUF_POST  8K Buffers = 64
 * TXBUF Pool (CMA_TXBUF_BUFNUM)= 7936 - (64*2) - 2048 = 5760
 */

#define CMA_BUFSIZE_8K	8192
#define CMA_BUFSIZE_4K	4096
#define CMA_BUFSIZE_2K	2048
#define CMA_BUFSIZE_512	512

#define CMA_RXBUF_POST		0
#define CMA_TXBUF_POST		1
#define CMA_RXCTR_BUF_POST	2

#define	CMA_RXCTRL_BUFNUM	64	/* 8K buffer count(pool) for RXCTRL Buffers */
#define	CMA_RXBUF_BUFNUM	2048	/* 4K buffer count(pool) RXBUF Post */

#define	CMA_TXBUF_BUFNUM	5760 /* 4K buffer count(pool) for TXBUF_POST */
#define SEC_CMA_COHERENT_BLK	0x6000 /* 24576 */

#ifdef HTXHDR
#define SEC_CMA_COHERENT_BLK_EX 0x100000 /* 1MB, for HTXHDR scratch_buf (772 KB) */
#define SEC_CMA_COHERENT_MAX  (340+1) /* 1 last node is for HTXHDR scratch_buf (772 KB) */
#else
#define SEC_CMA_COHERENT_BLK_EX 0
/*
 * This includes all the rings and other allocations as well :
 * Max txflows(MAX_DHD_TX_FLOWS)	= 320
 * h2dctrl				= 1
 * d2hctrl				= 1
 * h2drxp				= 4 (with 43684, it can grow up to 4)
 * d2htxctrl				= 1
 * d2hrxcpl				= 4 (with 43684, it can be upto 4)
 * dma read/write indices allocations	= 5
 * IOCTL response buffer		= 1
 * IOCTL request buffer			= 1
 * DHDHDR				= 1
 * host_bus_throughput_buf		= 1
 * Total				= 340
 */
#define SEC_CMA_COHERENT_MAX (340)
#endif /* HTXHDR */

#define CMA_DMA_DESC_MEMBLOCK	((SEC_CMA_COHERENT_BLK * SEC_CMA_COHERENT_MAX) \
							+ (SEC_CMA_COHERENT_BLK_EX))
#define CMA_DMA_DATA_MEMBLOCK	(CMA_BUFSIZE_4K*CMA_TXBUF_BUFNUM)
#define CMA_DMA_RXCTRL_MEMBLOCK	(CMA_BUFSIZE_8K*CMA_RXCTRL_BUFNUM)
#define CMA_DMA_RXBUF_POST_MEMBLOCK	(CMA_BUFSIZE_4K*CMA_RXBUF_BUFNUM)
#define	CMA_MEMBLOCK	((CMA_DMA_DESC_MEMBLOCK + CMA_DMA_DATA_MEMBLOCK+CMA_DMA_RXCTRL_MEMBLOCK)\
								+ (CMA_DMA_RXBUF_POST_MEMBLOCK))

#define SEC_DMA_ALIGN	(1<<16)
typedef struct sec_mem_elem {
	size_t			size;
	int				direction;
	int				buftype;
	phys_addr_t		pa_cma;     /**< physical  address */
	void			*va;        /**< virtual address of driver pkt */
	dma_addr_t		dma_handle; /**< bus address assign by linux */
	void			*vac;       /**< virtual address of cma buffer */
	struct page *pa_cma_page;	/* phys to page address */
	struct	sec_mem_elem	*next;
} sec_mem_elem_t;

extern bool osl_sec_dma_buffs_is_avail(osl_t *osh);
extern bool osl_sec_dma_rx_buffs_is_avail(osl_t *osh);
extern bool osl_sec_dma_rxctl_buffs_is_avail(osl_t *osh);

extern dma_addr_t osl_sec_dma_map(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *dmah, void *ptr_cma_info, uint offset, uint buftype);
extern dma_addr_t osl_sec_dma_dd_map(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *dmah);
extern dma_addr_t osl_sec_dma_map_txmeta(osl_t *osh, void *va, uint size,
  int direction, void *p, hnddma_seg_map_t *dmah, void *ptr_cma_info);
extern void osl_sec_dma_unmap(osl_t *osh, dma_addr_t dma_handle, uint size, int direction,
	void *p, hnddma_seg_map_t *map, void *ptr_cma_info, uint offset);
extern void osl_sec_dma_unmap_all(osl_t *osh, void *ptr_cma_info);

#endif /* BCM_SECURE_DMA */

#if defined(BCM_NBUFF)
/*
 *  NBUFF (fkb) type packet does not have prev or next pointers,
 *  and can't be made into double linked list.
 *  We add prev and next pointers in the dhd_pkttag_fd
 *  to help make double linked list for packets
 */
typedef struct dll PKT_LIST;
#define PKTLIST_INIT(x)		dhd_pkt_queue_head_init((x))
#define PKTLIST_ENQ(x, y)	dhd_pkt_queue_head((PKT_LIST *)(x), (void *)(y))
#define PKTLIST_DEQ(x)		dhd_pkt_dequeue((PKT_LIST *)(x))
#define PKTLIST_UNLINK(x, y)	dhd_pkt_unlink((PKT_LIST *)(x), (void *)(y))
#define PKTLIST_FINI(osh, x)	dhd_pkt_queue_purge((osl_t *)(osh), (PKT_LIST *)(x))

#else /* !BCM_NBUFF */

typedef struct sk_buff_head PKT_LIST;
#define PKTLIST_INIT(x)		skb_queue_head_init((x))
#define PKTLIST_ENQ(x, y)	skb_queue_head((struct sk_buff_head *)(x), (struct sk_buff *)(y))
#define PKTLIST_DEQ(x)		skb_dequeue((struct sk_buff_head *)(x))
#define PKTLIST_UNLINK(x, y)	skb_unlink((struct sk_buff *)(y), (struct sk_buff_head *)(x))
#define PKTLIST_FINI(osh, x)	({BCM_REFERENCE(osh); skb_queue_purge((struct sk_buff_head *)(x));})
#endif /* BCM_NBUFF */

#ifdef BCA_HNDROUTER
extern void osl_adjust_mac(unsigned int instance_id,char *mac); /* for NIC */
extern int osl_nvram_vars_adjust_mac(unsigned int instance_id, char *memblock, uint* len);
#endif // endif

#ifdef BCM_SKB_FREE_OFFLOAD
#define	BCM_SKB_FREE_OFFLOAD_ENAB(osh)	OSH_PUB(osh).skb_free_offload
#else
#define	BCM_SKB_FREE_OFFLOAD_ENAB(osh)	(0)
#endif // endif

#endif	/* _linux_osl_h_ */
