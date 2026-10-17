/*
 * Linux OS Independent Layer
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id: linux_osl.c 855002 2025-04-25 22:13:49Z $
 */

#define LINUX_PORT

#include <typedefs.h>
#include <bcmendian.h>
#include <linuxver.h>
#include <bcmdefs.h>
#include <linux/security.h>
#include <linux/namei.h>
#include <bcm_tkcore.h>

#ifdef mips
#include <asm/paccess.h>
#include <asm/cache.h>
#include <asm/r4kcache.h>
#undef ABS
#endif /* mips */

#if !(defined(STBLINUX) || defined(CMWIFI))
#if defined(__ARM_ARCH_7A__) && !defined(DHD_USE_COHERENT_MEM_FOR_RING)
#include <asm/cacheflush.h>
#endif /* __ARM_ARCH_7A__ && !DHD_USE_COHERENT_MEM_FOR_RING */
#endif /* !(STBLINUX || CMWIFI) */

#include <linux/random.h>

#include <osl.h>
#include <bcmutils.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <pcicfg.h>
#include <pcie_core.h>

#if defined(BCM_BTRACE)
#include <bcm_btrace.h>
#else /* BCM_BTRACE */
#define BTRACE_NOINSTR_FUNC
#endif /* BCM_BTRACE */

#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
#include <wl_erouter.h>
#include <linux/workqueue.h>
#endif /* CMWIFI && CMWIFI_EROUTER */

#include <linux/fs.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/namei.h>
#endif /* KERNEL >= 5.10 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
#include <linux/time.h>
#else
#include <linux/timekeeping.h>
#endif /* LINUX_VERSION_CODE */
#if defined(STB)
#include <linux/spinlock.h>
extern spinlock_t l2x0_reg_lock;
#endif /* STB */

#ifdef BCM_OBJECT_TRACE
#include <bcmutils.h>
#endif /* BCM_OBJECT_TRACE */
#include "linux_osl_priv.h"

#ifdef MLO_IPC
#include <mlo_ipc.h>
#endif /* MLO_IPC */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
#define osl_vfs_mkdir(_path, _dentry, _mode) \
	vfs_mkdir(&nop_mnt_idmap, (_path).dentry->d_inode, (_dentry), (_mode))
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#define osl_vfs_mkdir(_path, _dentry, _mode) \
	vfs_mkdir(mnt_user_ns((_path).mnt), (_path).dentry->d_inode, (_dentry), (_mode))
#else
#define osl_vfs_mkdir(_path, _dentry, _mode) \
	vfs_mkdir((_path).dentry->d_inode, (_dentry), (_mode))
#endif

#define PCI_CFG_RETRY		10

#define DUMPBUFSZ 1024

#if defined(BCM_ROUTER) && !defined(CMWIFI)
#include <board.h>
#endif
#if defined(BCM_CMA_RSV_MEM)
#include <wlan_rsvmem.h>
#endif /* BCM_CMA_RSV_MEM */

#if (defined(BCMDBG) && !defined(BCMDONGLEHOST))
uint32 g_assert_type = ASSERT_T5_DMP_AND_HANG; /* Debug dump collection for NIC drivers */
#else
uint32 g_assert_type = ASSERT_T0_PANIC; /* By Default Kernel Panic */
#endif

module_param(g_assert_type, int, ASSERT_T0_PANIC);

#if defined(CMWIFI)
#include <dma-direct.h>
#endif /* CMWIFI */

#if defined(BCM_ROUTER) && defined(__aarch64__) && !defined(OSL_CACHE_COHERENT) && \
	!defined(CMWIFI)
#define OSL_GLB_DMA_OPS_OBJ
#endif /* BCM_ROUTER && __aarch64__ && !OSL_CACHE_COHERENT && !CMWIFI */

#if defined(OSL_GLB_DMA_OPS_OBJ)
#include <linux/of_device.h>
/* Global device handle for dma_sync_xxx operations */
typedef struct osl_dma_ops_obj {
	struct device dev;
	refcount_t refcnt;
	u64 dma_mask;
	struct device_dma_parameters dma_parms;
} osl_dma_ops_obj_t;
static struct osl_dma_ops_obj *g_dmaops_obj = OSH_NULL;
#define DMA_OPS_DEV(_osh) \
	(_osh) ? &((struct pci_dev *)((_osh)->pdev))->dev : \
	(g_dmaops_obj) ? &(g_dmaops_obj->dev) : NULL
#else
#define DMA_OPS_DEV(_osh)                 &((struct pci_dev *)((_osh)->pdev))->dev
#endif /* !OSL_GLB_DMA_OPS_OBJ */

#ifdef CONFIG_WIFI_RETAIN_ALLOC
/* reuse large memory allocations for runtime unbind/bind operation */
typedef enum {
	OSL_ALLOC_TYPE_KMALLOC = 0,
	OSL_ALLOC_TYPE_DMA_MEM = 1,
	OSL_ALLOC_TYPE_LAST
} osl_alloc_type_t;

#define OSL_REUSE_ALLOC_TABLE_SIZE			64
#define OSL_ALLOC_REUSE_MIN_KMALLOC_SIZE		(0x100000)
/* allocation < 4MB is possible with with kmalloc, MAX_ORDER=11 */
#define OSL_ALLOC_REUSE_MIN_DMAMEM_SIZE			(0x400000)

typedef struct osl_retain_alloc_info {
	bool valid;
	uint unit;
	uint size;
	dma_addr_t pa;
	void *va;
	osl_alloc_type_t  mem_type;
} osl_retain_alloc_info_t;

static bool osl_retain_alloc_save(osl_t *osh, void *va, dma_addr_t *pa,
		uint size, osl_alloc_type_t type);
static void *osl_retain_alloc_get(osl_t *osh, dma_addr_t *pa,
		uint size, osl_alloc_type_t type);
static void osl_retain_alloc_stats(osl_retain_alloc_info_t *tbl, int tbl_sz);
static void osl_retain_alloc_release(osl_retain_alloc_info_t *tbl, int tbl_sz);

static struct device *osl_retain_alloc_dev = NULL;
static spinlock_t osl_retain_alloc_lock;
static osl_retain_alloc_info_t *osl_retain_alloc_info_tbl = NULL;
static uint32 osl_retain_alloc_min_size[OSL_ALLOC_TYPE_LAST] = {
	OSL_ALLOC_REUSE_MIN_KMALLOC_SIZE,
	OSL_ALLOC_REUSE_MIN_DMAMEM_SIZE
};
#endif /* CONFIG_WIFI_RETAIN_ALLOC */

#if defined(BCM_DMA_MALLOC)
static void _dma_msort_insert(dma_mem_t *meta, dll_t *list);
#endif /* BCM_DMA_MALLOC */

static void osl_register_alloc_iter(osl_t *osh);

static int16 linuxbcmerrormap[] =
{	0,				/* 0 */
	-EINVAL,		/* BCME_ERROR */
	-EINVAL,		/* BCME_BADARG */
	-EINVAL,		/* BCME_BADOPTION */
	-EINVAL,		/* BCME_NOTUP */
	-EINVAL,		/* BCME_NOTDOWN */
	-EINVAL,		/* BCME_NOTAP */
	-EINVAL,		/* BCME_NOTSTA */
	-EINVAL,		/* BCME_BADKEYIDX */
	-EINVAL,		/* BCME_RADIOOFF */
	-EINVAL,		/* BCME_NOTBANDLOCKED */
	-EINVAL, 		/* BCME_NOCLK */
	-EINVAL, 		/* BCME_BADRATESET */
	-EINVAL, 		/* BCME_BADBAND */
	-E2BIG,			/* BCME_BUFTOOSHORT */
	-E2BIG,			/* BCME_BUFTOOLONG */
	-EBUSY, 		/* BCME_BUSY */
	-EINVAL, 		/* BCME_NOTASSOCIATED */
	-EINVAL, 		/* BCME_BADSSIDLEN */
	-EINVAL, 		/* BCME_OUTOFRANGECHAN */
	-EINVAL, 		/* BCME_BADCHAN */
	-EFAULT, 		/* BCME_BADADDR */
	-ENOMEM, 		/* BCME_NORESOURCE */
	-EOPNOTSUPP,		/* BCME_UNSUPPORTED */
	-EMSGSIZE,		/* BCME_BADLENGTH */
	-EINVAL,		/* BCME_NOTREADY */
	-EPERM,			/* BCME_EPERM */
	-ENOMEM, 		/* BCME_NOMEM */
	-EINVAL, 		/* BCME_ASSOCIATED */
	-ERANGE, 		/* BCME_RANGE */
	-EINVAL, 		/* BCME_NOTFOUND */
	-EINVAL, 		/* BCME_WME_NOT_ENABLED */
	-EINVAL, 		/* BCME_TSPEC_NOTFOUND */
	-EINVAL, 		/* BCME_ACM_NOTSUPPORTED */
	-EINVAL,		/* BCME_NOT_WME_ASSOCIATION */
	-EIO,			/* BCME_SDIO_ERROR */
	-ENODEV,		/* BCME_DONGLE_DOWN */
	-EINVAL,		/* BCME_VERSION */
	-EIO,			/* BCME_TXFAIL */
	-EIO,			/* BCME_RXFAIL */
	-ENODEV,		/* BCME_NODEVICE */
	-EINVAL,		/* BCME_NMODE_DISABLED */
	-ENODATA,		/* BCME_NONRESIDENT */
	-EBUSY,			/* BCME_SCANREJECT */
	-EINVAL,		/* BCME_USAGE_ERROR */
	-EIO,     		/* BCME_IOCTL_ERROR */
	-EIO,			/* BCME_SERIAL_PORT_ERR */
	-EOPNOTSUPP,	/* BCME_DISABLED, BCME_NOTENABLED */
	-EIO,			/* BCME_DECERR */
	-EIO,			/* BCME_ENCERR */
	-EIO,			/* BCME_MICERR */
	-ERANGE,		/* BCME_REPLAY */
	-EINVAL,		/* BCME_IE_NOTFOUND */
	-EINVAL,		/* BCME_DATA_NOTFOUND */
	-EINVAL,        /* BCME_NOT_GC */
	-EINVAL,        /* BCME_PRS_REQ_FAILED */
	-EINVAL,        /* BCME_NO_P2P_SE */
	-EINVAL,        /* BCME_NOA_PND */
	-EINVAL,        /* BCME_FRAG_Q_FAILED */
	-EINVAL,        /* BCME_GET_AF_FAILED */
	-EINVAL,	/* BCME_MSCH_NOTREADY */
	-EINVAL,	/* BCME_IOV_LAST_CMD */
	-EINVAL,	/* BCME_MINIPMU_CAL_FAIL */
	-EINVAL,	/* BCME_RCAL_FAIL */
	-EINVAL,	/* BCME_LPF_RCCAL_FAIL */
	-EINVAL,	/* BCME_DACBUF_RCCAL_FAIL */
	-EINVAL,	/* BCME_VCOCAL_FAIL */
	-EINVAL,	/* BCME_BANDLOCKED */
	-EINVAL,	/* BCME_BAD_IE_DATA */
	-EINVAL,	/* BCME_NOT_ADMITTED */
	-EINVAL,        /* BCME_TRY_AGAIN */

/* When an new error code is added to bcmutils.h, add os
 * specific error translation here as well
 */
/* check if BCME_LAST changed since the last time this function was updated */
#if BCME_LAST != -70
#error "You need to add a OS error translation in the linuxbcmerrormap \
	for new error code defined in bcmutils.h"
#endif
};
uint lmtest = FALSE;

/* translate bcmerrors into linux errors */
int
osl_error(int bcmerror)
{
	if (bcmerror > 0)
		bcmerror = 0;
	else if (bcmerror < BCME_LAST)
		bcmerror = BCME_ERROR;

	/* Array bounds covered by ASSERT in osl_attach */
	return linuxbcmerrormap[-bcmerror];
}

#if defined(OSL_GLB_DMA_OPS_OBJ)
void
osl_dma_ops_dev_release(struct device *dev)
{
	struct osl_dma_ops_obj *opsobj;

	if (dev) {
	    opsobj = container_of(dev, struct osl_dma_ops_obj, dev);
	    kfree(opsobj);
	    printf("%s Freed dma_ops_dev 0x%px\n", __FUNCTION__, opsobj);
	}
}

int
osl_dma_ops_dev_attach(osl_t *osh)
{
	struct device *dev;
	struct osl_dma_ops_obj *opsobj;

	if (!(osh) || !(osh->pdev)) {
	    return BCME_OK;
	}

	/*
	 * OSL_CACHE_FLUSH/INV api's doesn't have osh/dev handle and call
	 * dma_sync_single_for_device()/cpu() kernel API's with NULL device handle.
	 * Linux 4.19 kernel/arm64 architecture mandates device handle to be passed
	 * for all dma_sync_single_xxx api's.
	 *
	 * If Device Tree or "dma-ranges" DT entry doesn't exist, all wlan pci
	 * devices are configured with same DMA operations over the entire range of
	 * DDR memory. Create a dummy device for dma operations and use it globally
	 * for all osl instances
	 */
	if (g_dmaops_obj) {
	    refcount_inc(&g_dmaops_obj->refcnt);
	    return BCME_OK;
	}

	opsobj = kzalloc(sizeof(struct osl_dma_ops_obj), GFP_KERNEL);
	if (!opsobj) {
	    printf("%s: failed to allocate memory for dma device\n", __FUNCTION__);
	    return BCME_ERROR;
	}
	printf("%s allocated dma_ops_dev 0x%px\n", __FUNCTION__, opsobj);
	dev = &opsobj->dev;

	if (dev_set_name(dev, "%s-dma_dev", KBUILD_MODNAME) < 0) {
	    printf("%s: failed to set name for dma device\n", __FUNCTION__);
	    kfree(opsobj);
	    return BCME_ERROR;
	}

	/* Initialze parameters similar to pci device */
	dev->release = osl_dma_ops_dev_release;
	dev->coherent_dma_mask = 0xffffffffull;
	opsobj->dma_mask = 0xffffffff;
	dev->dma_mask = &opsobj->dma_mask;
	dev->dma_parms = &opsobj->dma_parms;
	dma_set_max_seg_size(dev, 65536);
	dma_set_seg_boundary(dev, 0xffffffff);

	if (device_register(dev) < 0) {
	    printf("%s: failed to register dma device\n", __FUNCTION__);
	    put_device(dev);
	    kfree(opsobj);
	    return BCME_ERROR;
	}

	if (IS_ENABLED(CONFIG_OF)) {
		/* of_node = NULL, force_dma = true */
		if (of_dma_configure(dev, NULL, true) < 0) {
			printf("%s: failed to configure dma device\n", __FUNCTION__);
			device_unregister(dev);
			return BCME_ERROR;
		}
	} else {
		printf("%s: DT support not enabled in the BSP\n", __FUNCTION__);
		arch_setup_dma_ops(dev, 0, (1ULL << 32), NULL, false);
	}

	g_dmaops_obj = opsobj;
	refcount_set(&g_dmaops_obj->refcnt, 1);
	return BCME_OK;
}

void
osl_dma_ops_dev_detach(osl_t *osh)
{
	if (!(osh) || !(osh->pdev)) {
	    return;
	}

	if (g_dmaops_obj) {
	    if (refcount_dec_and_test(&g_dmaops_obj->refcnt) == true) {
	        device_unregister(&g_dmaops_obj->dev);
	        g_dmaops_obj = NULL;
	    }
	}

	return;
}
#endif /* OSL_GLB_DMA_OPS_OBJ */

osl_t *
osl_attach(void *pdev, enum bustype_e bustype, bool pkttag)
{
	osl_t *osh;
	gfp_t flags;
#if defined(STB) || defined(STBAP) || defined(CMWIFI_33940)
	struct pci_dev * pcidev;
	pcidev = pdev;
#endif /* STB || STBAP || CMWIFI_33940 */

	flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
	if (!(osh = kmalloc(sizeof(osl_t), flags)))
		return osh;

	ASSERT(osh);

	bzero(osh, sizeof(osl_t));
	if (!(osh->cmn = kmalloc(sizeof(osl_cmn_t), flags))) {
		kfree(osh);
		return NULL;
	}
	bzero(osh->cmn, sizeof(osl_cmn_t));
	atomic_set(&osh->cmn->malloced, 0);
	spin_lock_init(&(osh->cmn->dbgmem_lock));

	spin_lock_init(&(osh->cmn->pktalloc_lock));

	bcm_object_trace_init();

	/* Check that error map has the right number of entries in it */
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(linuxbcmerrormap) - 1));

	osh->failed = 0;
	osh->pdev = pdev;
	OSH_PUB(osh).pkttag = pkttag;
	osh->bustype = bustype;
	osh->magic = OS_HANDLE_MAGIC;

#ifdef OSL_CACHE_COHERENT
	/* Set ACP coherence flag */
	osl_flag_set(osh, OSL_ACP_COHERENCE);
#endif

#if !defined(BCMDBUS)
#if (defined(STB) || defined(STBAP)) || defined(CMWIFI_33940)
	if (pcidev && dma_set_mask(&pcidev->dev, DMA_BIT_MASK(64))) {
		if (dma_set_mask(&pcidev->dev, DMA_BIT_MASK(32))) {
			printk("%s: DMA set 32bit and 64 bit mask failed.\n", __FUNCTION__);
			kfree(osh->cmn);
			kfree(osh);
			return NULL;
		}
		else {
			printk("%s: DMA set 64bit mask failed,32 bit succeed\n", __FUNCTION__);
		}
	}
	if (pcidev && dma_set_coherent_mask(&pcidev->dev, DMA_BIT_MASK(64))) {
		if (dma_set_coherent_mask(&pcidev->dev, DMA_BIT_MASK(32))) {
			printk("%s: DMA set coherant 32bit & 64 bit mask failed.\n", __FUNCTION__);
			kfree(osh->cmn);
			kfree(osh);
			return NULL;
		}
		else {
			printk("%s: DMA set coherant 64bit mask failed,32 bit succeed\n",
				__FUNCTION__);
		}
	}
#endif /* (STB || STBAP || CMWIFI_33940) */
#endif /* BCMDBUS */

#if defined(OSL_GLB_DMA_OPS_OBJ)
	if (osl_dma_ops_dev_attach(osh) != BCME_OK) {
		if (osh->cmn) {
			kfree(osh->cmn);
		}
		kfree(osh);
		return NULL;
	}
#endif /*  OSL_GLB_DMA_OPS_OBJ */

	switch (bustype) {
		case PCI_BUS:
		case SI_BUS:
			OSH_PUB(osh).mmbus = TRUE;
			break;
		case JTAG_BUS:
		case SDIO_BUS:
		case USB_BUS:
		case SPI_BUS:
		case RPC_BUS:
			OSH_PUB(osh).mmbus = FALSE;
			break;
		default:
			ASSERT(FALSE);
			break;
	}

#ifdef BCMDBG_CTRACE
	spin_lock_init(&osh->ctrace_lock);
	INIT_LIST_HEAD(&osh->ctrace_list);
	osh->ctrace_num = 0;
#endif /* BCMDBG_CTRACE */

#ifdef BCMDBG_ASSERT
	if (pkttag) {
		struct sk_buff *skb;
		BCM_REFERENCE(skb);
		ASSERT(OSL_PKTTAG_SZ <= sizeof(skb->cb));
	}
#endif

#ifdef BCM_SKB_FREE_OFFLOAD
	/* Enable SKB_FREE_OFFLOAD to another core */
	OSH_PUB(osh).skb_free_offload = 1;
	OSH_PUB(osh).skbfreelist.len = 0;
	OSH_PUB(osh).skbfreelist.head = NULL;
	OSH_PUB(osh).skbfreelist.tail = NULL;
#endif /* BCM_SKB_FREE_OFFLOAD */

	osl_register_alloc_iter(osh);
	return osh;
} /* osl_attach */

void osl_set_bus_handle(osl_t *osh, void *bus_handle)
{
	osh->bus_handle = bus_handle;
}

void* osl_get_bus_handle(osl_t *osh)
{
	return osh->bus_handle;
}

#if defined(BCM_BACKPLANE_TIMEOUT)
void osl_set_bpt_cb(osl_t *osh, void *bpt_cb, void *bpt_ctx)
{
	if (osh) {
		osh->bpt_cb = (bpt_cb_fn)bpt_cb;
		osh->sih = bpt_ctx;
	}
}
#endif	/* BCM_BACKPLANE_TIMEOUT */

void
osl_detach(osl_t *osh)
{
#ifdef BCM_NBUFF_PKTPOOL_CACHE
	struct sk_buff *skb;
#endif /* BCM_NBUFF_PKTPOOL_CACHE */

	if (osh == NULL)
		return;

	ASSERT(osh->cmn != NULL);

	tkcore_remove_alloc_iterator(osh);

#if defined(OSL_GLB_DMA_OPS_OBJ)
	osl_dma_ops_dev_detach(osh);
#endif /*  OSL_GLB_DMA_OPS_OBJ */

#ifdef BCMDBG_MEM
	if (MEMORY_LEFTOVER(osh)) {
		static char dumpbuf[DUMPBUFSZ];
		struct bcmstrbuf b;

		printf("osh%d: %s: MEMORY LEAK %d bytes\n", osh->unit, __FUNCTION__, MALLOCED(osh));
		bcm_binit(&b, dumpbuf, DUMPBUFSZ);
		MALLOC_DUMP(osh, &b);
		printf("%s", b.origbuf);
	}

#if !defined(BCMDONGLEHOST)
	if (osh->cmn->dma_alloc_consistent_bytes != 0) {
		printf("osh%d: %s: MEMORY LEAK dma_consist: %d bytes\n",
			osh->unit, __FUNCTION__, osh->cmn->dma_alloc_consistent_bytes);
	}
#endif /* !BCMDONGLEHOST */
	if (osh->cmn->dma_alloc_contiguous_bytes != 0) {
		printf("osh%d: %s: MEMORY LEAK dma_contig: %d bytes\n",
			osh->unit, __FUNCTION__, osh->cmn->dma_alloc_contiguous_bytes);
	}
#endif /* BCMDBG_MEM */

#ifdef BCM_NBUFF_PKTPOOL_CACHE
	/* Free up BPM local pool cache */
	while (OSH_PUB(osh).skb_cache) {

		skb = OSH_PUB(osh).skb_cache;
		OSH_PUB(osh).skb_cache = skb->next;
		skb->next = NULL;

		/* Packets in skb_cache are accounted in OSH but are not added to debug pktlist
		 * so decreament the packet account and then convert the native before freeing
		 */
		PKTACCOUNT(osh, 1, FALSE);
		skb = PKTFRMNATIVE(osh, skb);
		PKTFREE(osh, skb, FALSE);
	}
#endif /* BCM_NBUFF_PKTPOOL_CACHE */

	bcm_object_trace_deinit();

	ASSERT(osh->magic == OS_HANDLE_MAGIC);
	kfree(osh->cmn);
	kfree(osh);
} /* osl_detach */

/* APIs to set/get specific quirks in OSL layer */
void BCMFASTPATH
osl_flag_set(osl_t *osh, uint32 mask)
{
	osh->flags |= mask;
}

void
osl_flag_clr(osl_t *osh, uint32 mask)
{
	osh->flags &= ~mask;
}

#if defined(STB)
inline bool BCMFASTPATH
#else
bool
#endif /* STB */
osl_is_flag_set(osl_t *osh, uint32 mask)
{
	return (osh->flags & mask);
}

#if ((defined STB) || (defined STBAP)) || (defined(CMWIFI) && (LINUX_VERSION_CODE >= \
	KERNEL_VERSION(5, 4, 0)))
#ifdef CMWIFI
/* use linux defines instead of module defs for dma directions */
#undef DMA_TX
#undef DMA_RX
#define DMA_TX DMA_TO_DEVICE
#define DMA_RX DMA_FROM_DEVICE
#endif /* CMWIFI */

inline void BCMFASTPATH
osl_cache_flush(osl_t* osh, void *va, uint size)
{
	dma_addr_t dma_handle;
	struct pci_dev * pdev;
	pdev = osh->pdev;

	BCM_REFERENCE(dma_handle);
	BCM_REFERENCE(pdev);

	if (size > 0) {
#ifdef CMWIFI_33940
	/* note CMA allocations are vmalloced memory */
		if (is_vmalloc_addr(va)) {
#ifdef BCMDONGLEHOST
			/* align to next cache line boundary */
			if (size % cache_line_size()) {
				size += cache_line_size();
				size = size & (cache_line_size() - 1);
			}
#endif /* BCMDONGLEHOST */
			do {
				struct page *pg = vmalloc_to_page(va);
				unsigned int offset = (unsigned int) (offset_in_page(va));
				unsigned int map_size = (size + offset > PAGE_SIZE)
								? PAGE_SIZE : (size + offset);
				/* map_size + offset <= PAGE_SIZE */
				map_size -= offset;

				dma_handle = dma_map_page(&(pdev->dev),
						pg, offset, map_size, DMA_TX);
				if (dma_mapping_error(&(pdev->dev), dma_handle)) {
					ASSERT(!"dma mapping error\n");
					return;
				}
				dma_sync_single_for_device(&(pdev->dev),
					dma_handle, map_size, DMA_TX);
				dma_unmap_page(&(pdev->dev), dma_handle, map_size, DMA_TX);

				/* point to next page to flush */
				va = (void *)((uintptr)va + map_size);
				size -= map_size;
			} while (size);
			return;
		}
#endif /* CMWIFI_33940 */
#if !defined(DHD_USE_COHERENT_MEM_FOR_RING)
		dma_handle = dma_map_single(&(pdev->dev), va, size,  DMA_TX);
		dma_sync_single_for_device(&(pdev->dev), dma_handle, size, DMA_TX);
		dma_unmap_single(&(pdev->dev), dma_handle, size, DMA_TX);
#endif /* !defined(DHD_USE_COHERENT_MEM_FOR_RING) */
	}
}

inline void BCMFASTPATH
osl_cache_inv(osl_t* osh, void *va, uint size)
{
	dma_addr_t dma_handle;
	struct pci_dev * pdev;
	pdev = osh->pdev;

	BCM_REFERENCE(dma_handle);
	BCM_REFERENCE(pdev);

#ifdef CMWIFI_33940
	if (is_vmalloc_addr(va)) {
#ifdef BCMDONGLEHOST
		/* align to next cache line boundary */
		if (size % cache_line_size()) {
			size += cache_line_size();
			size = size & (cache_line_size() - 1);
		}
#endif /* BCMDONGLEHOST */
		do {
			struct page *pg = vmalloc_to_page(va);
			unsigned int offset = (unsigned int) (offset_in_page(va));
			unsigned int map_size = (size + offset > PAGE_SIZE)
							? PAGE_SIZE : (size + offset);

			/* map_size + offset <= PAGE_SIZE */
			map_size -= offset;

			dma_handle = dma_map_page(&(pdev->dev),
					pg, offset, map_size, DMA_RX);
			if (dma_mapping_error(&(pdev->dev), dma_handle)) {
				ASSERT(!"dma mapping error\n");
				return;
			}
			dma_sync_single_for_cpu(&(pdev->dev), dma_handle, map_size, DMA_RX);
			dma_unmap_page(&(pdev->dev), dma_handle, map_size, DMA_RX);

			/* point to next page to flush */
			va = (void *)((uintptr)va + map_size);
			size -= map_size;
		} while (size);
		return;
	}
#endif /* CMWIFI_33940 */

#if !defined(DHD_USE_COHERENT_MEM_FOR_RING)
	dma_handle = dma_map_single(&(pdev->dev), va, size,  DMA_RX);
	dma_sync_single_for_cpu(&(pdev->dev), dma_handle, size, DMA_RX);
	dma_unmap_single(&(pdev->dev), dma_handle, size, DMA_RX);
#endif /* !defined(DHD_USE_COHERENT_MEM_FOR_RING) */
}

inline void BCMFASTPATH
osl_prefetch(const void *ptr)
{
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__("pld\t%0" :: "o"(*(const char *)ptr) : "cc");
#else
	asm volatile("prfm pldl1keep, %a0\n" : : "p" (ptr));
#endif /* __ARM_ARCH_7A__ */
}

#elif (defined(__ARM_ARCH_7A__) && !defined(DHD_USE_COHERENT_MEM_FOR_RING))

inline void BCMFASTPATH
osl_cache_flush(osl_t *osh, void *va, uint size)
{
#if !defined(OSL_CACHE_COHERENT)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	struct device *dmadev = NULL;
	dma_addr_t dma_handle;

	if (osh) {
		dmadev = &((struct pci_dev *)osh->pdev)->dev;
	}

	if (size > 0)
	{
		dma_handle = dma_map_single(dmadev, va, size, DMA_TO_DEVICE);
		dma_sync_single_for_device(dmadev, dma_handle, size, DMA_TO_DEVICE);
		dma_unmap_single(dmadev, dma_handle, size, DMA_TO_DEVICE);
	}
#else
	if (size > 0) {
		dma_sync_single_for_device(NULL, virt_to_dma(NULL, va), size,
			DMA_TO_DEVICE);
	}
#endif /* KERNEL >= 5.10 */
#endif /* !OSL_CACHE_COHERENT */
}

inline void BCMFASTPATH
osl_cache_inv(osl_t *osh, void *va, uint size)
{
#if !defined(OSL_CACHE_COHERENT)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	struct device *dmadev = NULL;
	dma_addr_t dma_handle;

	if (osh) {
		dmadev = &((struct pci_dev *)osh->pdev)->dev;
	}

	dma_handle = dma_map_single(dmadev, va, size, DMA_FROM_DEVICE);
	dma_sync_single_for_cpu(dmadev, dma_handle, size, DMA_FROM_DEVICE);
	dma_unmap_single(dmadev, dma_handle, size, DMA_FROM_DEVICE);
#else
	dma_sync_single_for_cpu(NULL, virt_to_dma(NULL, va), size, DMA_FROM_DEVICE);
#endif
#endif /* !OSL_CACHE_COHERENT */
}

inline void BCMFASTPATH
osl_prefetch(const void *ptr)
{
	__asm__ __volatile__("pld\t%0" :: "o"(*(const char *)ptr) : "cc");
}

#elif (defined(BCM_ROUTER) && defined(__aarch64__))

inline void BCMFASTPATH
osl_cache_flush(osl_t *osh, void *va, uint size)
{
#if !defined(OSL_CACHE_COHERENT)
	if (size > 0)
		dma_sync_single_for_device(DMA_OPS_DEV(osh), virt_to_phys(va), size, DMA_TX);
#endif
}

inline void BCMFASTPATH
osl_cache_inv(osl_t *osh, void *va, uint size)
{
#if !defined(OSL_CACHE_COHERENT)
	dma_sync_single_for_cpu(DMA_OPS_DEV(osh), virt_to_phys(va), size, DMA_RX);
#endif
}

inline void osl_prefetch(const void *ptr)
{
	asm volatile("prfm pldl1keep, %a0\n" : : "p" (ptr));
}

#endif /* !mips && !__ARM_ARCH_7A__ && !BCM_ROUTER */

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	uint val = 0;
	uint retry = PCI_CFG_RETRY;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	/* only 4byte access supported */
	ASSERT(size == 4);

	do {
		pci_read_config_dword(osh->pdev, offset, &val);
		if (val != 0xffffffff)
			break;
	} while (retry--);

#ifdef BCMDBG
	if (retry < PCI_CFG_RETRY)
		printk("PCI CONFIG READ access to %d required %d retries\n", offset,
		       (PCI_CFG_RETRY - retry));
#endif /* BCMDBG */

	return (val);
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	uint retry = PCI_CFG_RETRY;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	/* only 4byte access supported */
	ASSERT(size == 4);

	do {
		pci_write_config_dword(osh->pdev, offset, val);
		if (offset != PCI_BAR0_WIN)
			break;
		if (osl_pci_read_config(osh, offset, size) == val)
			break;
	} while (retry--);

#ifdef BCMDBG
	if (retry < PCI_CFG_RETRY)
		printk("PCI CONFIG WRITE access to %d required %d retries\n", offset,
		       (PCI_CFG_RETRY - retry));
#endif /* BCMDBG */
}

/* return bus # for the pci device pointed by osh->pdev */
uint
osl_pci_bus(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

#if defined(__ARM_ARCH_7A__) || defined(BCM_ROUTER)
	return pci_domain_nr(((struct pci_dev *)osh->pdev)->bus);
#else
	return ((struct pci_dev *)osh->pdev)->bus->number;
#endif
}

/* return unit # for the device pointed by osh */
uint
osl_unit(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	return osh->unit;
}

void
osl_unit_set(osl_t *osh, uint unit)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	osh->unit = unit;
}

void*
osl_get_timers_context(osl_t *osh)
{
	return osh->timers_context;
}

void
osl_set_timers_context(osl_t *osh, void *context)
{
	osh->timers_context = context;
}

/* return slot # for the pci device pointed by osh->pdev */
uint
osl_pci_slot(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

#if defined(__ARM_ARCH_7A__) || defined(BCM_ROUTER)
	return PCI_SLOT(((struct pci_dev *)osh->pdev)->devfn) + 1;
#else
	return PCI_SLOT(((struct pci_dev *)osh->pdev)->devfn);
#endif
}

/* return domain # for the pci device pointed by osh->pdev */
uint
osl_pcie_domain(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return pci_domain_nr(((struct pci_dev *)osh->pdev)->bus);
}

/* return bus # for the pci device pointed by osh->pdev */
uint
osl_pcie_bus(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return ((struct pci_dev *)osh->pdev)->bus->number;
}

void
osl_pcie_aspm_enable(osl_t *osh, uint linkcap_offset, bool aspm)
{
	struct pci_dev *dev;
	uint val;
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	dev = (struct pci_dev *)(osh->pdev);
	pci_read_config_dword(dev, linkcap_offset, &val);

	if (aspm == FALSE) {
		val &= ~(PCIE_ASPM_ENAB | PCIE_CLKREQ_ENAB);
	}
	else {
		val |= (PCIE_ASPM_ENAB | PCIE_CLKREQ_ENAB);
	}
	pci_write_config_dword(dev, linkcap_offset, val);
}

void
osl_pcie_mps_limit(osl_t *osh, uint devctl_offset, uint mps)
{
	struct pci_dev *dev, *parent;
	uint val;
	uint dev_mps, dev_mrr, mpss;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev && (mps >= 128));

	dev = (struct pci_dev *)(osh->pdev);
	pci_read_config_dword(dev, devctl_offset, &val);
	dev_mps = 128 << ((val & PCIE_CAP_DEVCTRL_MPS_MASK) >> PCIE_CAP_DEVCTRL_MPS_SHIFT);
	dev_mrr = 128 << ((val & PCIE_CAP_DEVCTRL_MRRS_MASK) >> PCIE_CAP_DEVCTRL_MRRS_SHIFT);
	printk("%s: MPS %u MRR %u devctl 0x%x\n", __FUNCTION__, dev_mps, dev_mrr, val);

	if (dev_mps <= mps)
		return;

	parent = dev->bus->self;
	ASSERT(parent);
	if (parent->bus->self) {
		printk("%s: Stop configuring MPS because PCIE parent is bridge not RC!\n",
			__FUNCTION__);
		return;
	}

	mpss = mps >> 8;
	ASSERT(mpss <= PCIE_CAP_DEVCTRL_MPS_4096B);

	printk("%s: override PCIE device MPS from %u to %u\n", __FUNCTION__, dev_mps, mps);
	val &= ~PCIE_CAP_DEVCTRL_MPS_MASK;
	val |= (mpss << PCIE_CAP_DEVCTRL_MPS_SHIFT);
	pci_write_config_dword(dev, devctl_offset, val);

	printk("%s: override PCIE RC MPS to %u\n", __FUNCTION__, mps);
	pci_read_config_dword(parent, devctl_offset, &val);
	val &= ~PCIE_CAP_DEVCTRL_MPS_MASK;
	val |= (mpss << PCIE_CAP_DEVCTRL_MPS_SHIFT);
	pci_write_config_dword(parent, devctl_offset, val);
} /* osl_pcie_mps_limit */

void
osl_pcie_mrrs_config(osl_t *osh, uint devctl_offset, uint mrrs)
{
	struct pci_dev *dev;
	uint val;
	uint dev_mps, dev_mrrs;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
	ASSERT(mrrs <= PCIE_CAP_DEVCTRL_MRRS_4096B);

	dev = (struct pci_dev *)(osh->pdev);
	pci_read_config_dword(dev, devctl_offset, &val);
	dev_mps = (val & PCIE_CAP_DEVCTRL_MPS_MASK) >> PCIE_CAP_DEVCTRL_MPS_SHIFT;
	dev_mrrs = (val & PCIE_CAP_DEVCTRL_MRRS_MASK) >> PCIE_CAP_DEVCTRL_MRRS_SHIFT;
	printk("%s: MPS %u MRR %u devctl 0x%x\n", __FUNCTION__,
		(128 << dev_mps), (128 << dev_mrrs), val);

	if (dev_mrrs == mrrs)
		return;

	printk("%s: override PCIE device MRRS from %u to %u\n", __FUNCTION__,
		(128 << dev_mrrs), (128 << mrrs));
	pcie_set_readrq(dev, (128 << mrrs));
} /* osl_pcie_mrrs_config */

/* return the pci device pointed by osh->pdev */
struct pci_dev *
osl_pci_device(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return osh->pdev;
}

static void *
_osl_malloc(osl_t *osh, uint size)
{
	void *addr;
	gfp_t flags;

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

#ifdef CONFIG_DHD_USE_STATIC_BUF
	if (bcm_static_buf)
	{
		unsigned long irq_flags;
		int i = 0;
		if ((size >= PAGE_SIZE)&&(size <= STATIC_BUF_SIZE))
		{
			spin_lock_irqsave(&bcm_static_buf->static_lock, irq_flags);

			for (i = 0; i < STATIC_BUF_MAX_NUM; i++)
			{
				if (bcm_static_buf->buf_use[i] == 0)
					break;
			}

			if (i == STATIC_BUF_MAX_NUM)
			{
				spin_unlock_irqrestore(&bcm_static_buf->static_lock, irq_flags);
				printk("all static buff in use!\n");
				goto original;
			}

			bcm_static_buf->buf_use[i] = 1;
			spin_unlock_irqrestore(&bcm_static_buf->static_lock, irq_flags);

			bzero(bcm_static_buf->buf_ptr+STATIC_BUF_SIZE*i, size);
			if (osh)
				atomic_add(size, &osh->cmn->malloced);

			return ((void *)(bcm_static_buf->buf_ptr+STATIC_BUF_SIZE*i));
		}
	}
original:
#endif /* CONFIG_DHD_USE_STATIC_BUF */

#ifdef CONFIG_WIFI_RETAIN_ALLOC
	if ((addr = osl_retain_alloc_get(osh, NULL, size, OSL_ALLOC_TYPE_KMALLOC)) == NULL) {
#endif /* CONFIG_WIFI_RETAIN_ALLOC */
		flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
		if ((addr = kmalloc(size, flags)) == NULL) {
			if (osh)
				osh->failed++;
			return (NULL);
		}
#ifdef CONFIG_WIFI_RETAIN_ALLOC
	}
#endif /* CONFIG_WIFI_RETAIN_ALLOC */
	if (osh && osh->cmn)
		atomic_add(size, &osh->cmn->malloced);

#if defined(BCM_BTRACE)
	if (osh) {
		btrace(osh, BTRACE_EVT_CAT_MEMORY, BTRACE_EVT_TYPE_ALLOC, btrace_get_id(addr), size,
			osh->cmn ? atomic_read(&osh->cmn->malloced) : 0, btrace_get_caller());
	}
#endif /* BCM_BTRACE */

	return (addr);
} /* _osl_malloc */

#define MEMLIST_LOCK(osh, flags)	spin_lock_irqsave(&(osh)->cmn->dbgmem_lock, flags)
#define MEMLIST_UNLOCK(osh, flags)	spin_unlock_irqrestore(&(osh)->cmn->dbgmem_lock, flags)

#if defined(BCMDBG_MEM) || defined(CONFIG_BCM_TINY_KCORE)

static void
#ifdef BCMDBG_MEM
osl_dbg_memlist_add(bcm_mem_link_t **head, bcm_mem_link_t *ml, osl_t *osh, uint size,
                    int line, const char *file, const char *func)
#else
osl_dbg_memlist_add(bcm_mem_link_t **head, bcm_mem_link_t *ml, osl_t *osh, uint size)
#endif
{
	unsigned long flags = 0;
#ifdef BCMDBG_MEM
	const char *basename;
#endif /* BCMDBG_MEM */
	ASSERT(osh != NULL && osh->cmn != NULL);
	MEMLIST_LOCK(osh, flags);

	ml->size = size;
#ifdef BCMDBG_MEM
	ml->line = line;
	ml->osh = (void *)osh;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(ml->file, basename, BCM_MEM_FILENAME_LEN);
	ml->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	strncpy(ml->func, func, BCM_MEM_FUNCTION_LEN);
	ml->func[BCM_MEM_FUNCTION_LEN - 1] = '\0';
#endif /* BCMDBG_MEM */

	/* link this block */
	ml->prev = NULL;
	ml->next = *head;
	if (ml->next)
		ml->next->prev = ml;
	*head = ml;
	MEMLIST_UNLOCK(osh, flags);
} /* osl_dbg_memlist_add */

/**
 * @return  TRUE if it is ok to free
 */
static bool
#ifdef BCMDBG_MEM
osl_dbg_memlist_free(bcm_mem_link_t **head, bcm_mem_link_t *ml, osl_t *osh, void *addr, uint size,
                     int line, const char *file, const char *func)
#else /* !BCMDBG_MEM */
osl_dbg_memlist_free(bcm_mem_link_t **head, bcm_mem_link_t *ml, osl_t *osh, void *addr, uint size)
#endif /* BCMDBG_MEM */
{
	unsigned long flags = 0;
	bool regular_mem;

	BCM_REFERENCE(regular_mem);
	ASSERT(osh != NULL && osh->cmn != NULL);
	/* unlink this block */
	MEMLIST_LOCK(osh, flags);
	regular_mem = (*head == osh->cmn->dbgmem_list);
	if (ml->prev)
		ml->prev->next = ml->next;
	if (ml->next)
		ml->next->prev = ml->prev;
	if (*head == ml)
		*head = ml->next;
	ml->next = ml->prev = NULL;
	MEMLIST_UNLOCK(osh, flags);

#ifdef BCMDBG_MEM
	if (ml->size == 0) {
		printk("osh%d: %s: double free on addr %p size %d at line %d file %s function %s\n",
			osh->unit, __FUNCTION__, addr, size, line, file, func);
		ASSERT(ml->size);
		return FALSE;
	}

	if (ml->size != size) {
		printk("osh%d: %s: dealloc size does not match alloc size\n",
		        osh->unit, __FUNCTION__);
		printk("Dealloc addr %p size %d at line %d file %s function %s\n",
			addr, size, line, file, func);
		printk("Alloc size %d line %d file %s function %s\n",
			ml->size, ml->line, ml->file, ml->func);
		ASSERT(ml->size == size);
		return FALSE;
	}

	if (((osl_t*)ml->osh)->cmn != osh->cmn) {
		printk("osh%d:%s: alloc osh %p does not match dealloc osh %p\n",
			osh->unit, __FUNCTION__, ((osl_t*)ml->osh)->cmn, osh->cmn);
		printk("Dealloc addr %p size %d at line %d file %s function %s\n",
			addr, size, line, file, func);
		printk("Alloc size %d line %d file %s function %s\n",
			ml->size, ml->line, ml->file, ml->func);
		ASSERT(((osl_t*)ml->osh)->cmn == osh->cmn);
		return FALSE;
	}
#endif /* BCMDBG_MEM */
#ifdef CONFIG_BCM_TINY_KCORE
	MEMLIST_LOCK(osh, flags);
	if (regular_mem && osh->cmn->use_pending_freelist == TRUE) {
		ml->next = osh->cmn->pending_freelist;
		osh->cmn->pending_freelist = ml;
		osh->cmn->pending_free_count++;
		MEMLIST_UNLOCK(osh, flags);
		return FALSE;
	}
	MEMLIST_UNLOCK(osh, flags);
#endif /* CONFIG_BCM_TINY_KCORE */

	ml->size = 0;

	return TRUE;
} /* osl_dbg_memlist_free */

#endif /* BCMDBG_MEM || CONFIG_BCM_TINY_KCORE */

#ifdef BCMDBG_MEM
/* In BCMDBG_MEM configurations osl_malloc is only used internally in
 * the implementation of osl_debug_malloc.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_malloc static in
 * the BCMDBG_MEM case.
 */
static void *
osl_malloc(osl_t *osh, uint size, int line, const char* file, const char *func)
#else
void *
osl_malloc(osl_t *osh, uint size)
#endif
{
	void *ptr;
	bcm_mem_link_t *ml = NULL;
	uint alloc_size = size;

	if (!size) {
		printk("%s: allocating zero sized mem at %ps\n", __FUNCTION__, CALL_SITE);
		ASSERT(0);
	}

	/*
	 * linked list of allocations is maintained for BCMDBG_MEM and
	 * CONFIG_BCM_TINY_KCORE. For the latter only allocation size
	 * stored to include the allocated regions in coredump file.
	 */
#if defined(BCMDBG_MEM) || defined(CONFIG_BCM_TINY_KCORE)
	alloc_size += sizeof(*ml);
	ml = ptr = _osl_malloc(osh, alloc_size);
#else
	ptr = _osl_malloc(osh, alloc_size);
#endif
	if (!ptr) {
		return ptr;
	}

	if (osh && osh->cmn) {
#if defined(BCMDBG_MEM)
		osl_dbg_memlist_add(&osh->cmn->dbgmem_list, ml, osh, size, line, file, func);
#elif defined(CONFIG_BCM_TINY_KCORE)
		osl_dbg_memlist_add(&osh->cmn->dbgmem_list, ml, osh, size);
#endif
	}

	if (ml)
		ptr = ml + 1;

	return ptr;
} /* osl_malloc */

#ifndef BCMDBG_MEM
void *
osl_mallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_malloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#endif

/**
 * @param[in] addr  May be NULL, in which case no action is performed.
 */
static void
_osl_mfree(osl_t *osh, void *addr, uint size)
{
#ifdef CONFIG_DHD_USE_STATIC_BUF
	unsigned long flags;
#endif /* CONFIG_DHD_USE_STATIC_BUF */

	if (addr == NULL) {
		/* the 'de facto' implementation of free() ignores NULL pointers */
		printk("%s NULL\n", __FUNCTION__);
		return;
	}

#ifdef CONFIG_DHD_USE_STATIC_BUF
	if (bcm_static_buf)
	{
		if ((addr > (void *)bcm_static_buf) && ((unsigned char *)addr
			<= ((unsigned char *)bcm_static_buf + STATIC_BUF_TOTAL_LEN)))
		{
			int buf_idx = 0;

			buf_idx = ((unsigned char *)addr - bcm_static_buf->buf_ptr)/STATIC_BUF_SIZE;

			spin_lock_irqsave(&bcm_static_buf->static_lock, flags);
			bcm_static_buf->buf_use[buf_idx] = 0;
			spin_unlock_irqrestore(&bcm_static_buf->static_lock, flags);

			if (osh && osh->cmn) {
				ASSERT(osh->magic == OS_HANDLE_MAGIC);
				atomic_sub(size, &osh->cmn->malloced);
			}
			return;
		}
	}
#endif /* CONFIG_DHD_USE_STATIC_BUF */
	if (osh && osh->cmn) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

		ASSERT(size <= osl_malloced(osh));

		atomic_sub(size, &osh->cmn->malloced);
	}

#ifdef CONFIG_WIFI_RETAIN_ALLOC
	if (osl_retain_alloc_save(osh, addr, NULL, size, OSL_ALLOC_TYPE_KMALLOC)) {
		return;
	}
#endif /* CONFIG_WIFI_RETAIN_ALLOC */

#if defined(BCM_BTRACE)
	if (osh) {
		btrace(osh, BTRACE_EVT_CAT_MEMORY, BTRACE_EVT_TYPE_FREE, btrace_get_id(addr), size,
			osh->cmn ? atomic_read(&osh->cmn->malloced) : 0, btrace_get_caller());
	}
#endif /* BCM_BTRACE */

	kfree(addr);
} /* _osl_mfree */

#ifdef BCMDBG_MEM
/*
 * In BCMDBG_MEM configurations osl_mfree is only used internally in
 * the implementation of osl_debug_mfree.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_mfree static in
 * the BCMDBG_MEM case.
 */
static void
osl_mfree(osl_t *osh, void *addr, uint size, int line, const char* file, const char *func)
#else /* !BCMDBG_MEM */
void
osl_mfree(osl_t *osh, void *addr, uint size)
#endif /* BCMDBG_MEM */
{
#if defined(BCMDBG_MEM) || defined(CONFIG_BCM_TINY_KCORE)
	bcm_mem_link_t *ml = (bcm_mem_link_t *)((char *)addr - sizeof(*ml));
#endif /* BCMDBG_MEM || CONFIG_BCM_TINY_KCORE */
	bool do_free = TRUE;

	if (addr == NULL) {
		return;
	}

	if (osh && osh->cmn) {
#ifdef BCMDBG_MEM
		do_free = osl_dbg_memlist_free(&osh->cmn->dbgmem_list, ml, osh, addr, size,
		                               line, file, func);
#elif defined(CONFIG_BCM_TINY_KCORE)
		do_free = osl_dbg_memlist_free(&osh->cmn->dbgmem_list, ml, osh, addr, size);
#endif
	}

#if defined(BCMDBG_MEM) || defined(CONFIG_BCM_TINY_KCORE)
	addr = ml;
	size += sizeof(*ml);
#endif /* BCMDBG_MEM || CONFIG_BCM_TINY_KCORE */

	if (do_free) {
		_osl_mfree(osh, addr, size);
	}
} /* osl_mfree */

#ifdef BCMDBG_MEM
/* In BCMDBG_MEM configurations osl_vmalloc is only used internally in
 * the implementation of osl_debug_vmalloc.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_malloc static in
 * the BCMDBG_MEM case.
 */
static void *
osl_vmalloc(osl_t *osh, uint size, int line, const char* file, const char *func)
#else
void *
osl_vmalloc(osl_t *osh, uint size)
#endif
{
	void *ptr;
#if defined(BCMDBG_MEM)
	bcm_mem_link_t *ml = NULL;
#endif /* BCMDBG_MEM */
	uint alloc_size = size;

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

#if defined(BCMDBG_MEM)
	alloc_size += sizeof(*ml);
#endif /* (BCMDBG_MEM) */

	if ((ptr = vmalloc(alloc_size)) == NULL) {
		if (osh)
			osh->failed++;
		return (NULL);
	}

#if defined(BCMDBG_MEM)
	ml = ptr;
#endif /* BCMDBG_MEM */

	if (osh && osh->cmn) {
		atomic_add(size, &osh->cmn->malloced);
#if defined(BCMDBG_MEM)
		osl_dbg_memlist_add(&osh->cmn->dbgvmem_list, ml, osh, size, line, file, func);
#endif /* BCMDBG_MEM */
	}

#if defined(BCMDBG_MEM)
	if (ml)
		ptr = ml + 1;
#endif /* BCMDBG_MEM */

	return ptr;
} /* osl_vmalloc */

#ifndef BCMDBG_MEM
void *
osl_vmallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_vmalloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#endif

#ifdef BCMDBG_MEM
/*
 * In BCMDBG_MEM configurations osl_mfree is only used internally in
 * the implementation of osl_debug_mfree.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_mfree static in
 * the BCMDBG_MEM case.
 */
static void
osl_vmfree(osl_t *osh, void *addr, uint size, int line, const char* file, const char *func)
#else /* !BCMDBG_MEM */
void
osl_vmfree(osl_t *osh, void *addr, uint size)
#endif /* BCMDBG_MEM */
{
#if defined(BCMDBG_MEM)
	bcm_mem_link_t *ml = (bcm_mem_link_t *)((char *)addr - sizeof(*ml));
#endif /* BCMDBG_MEM */
	bool do_free = TRUE;

	if (osh && osh->cmn) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);
		ASSERT(size <= osl_malloced(osh));
		atomic_sub(size, &osh->cmn->malloced);
#ifdef BCMDBG_MEM
		do_free = osl_dbg_memlist_free(&osh->cmn->dbgvmem_list, ml, osh, addr, size,
		                               line, file, func);
#endif /* BCMDBG_MEM */
	}

#if defined(BCMDBG_MEM)
	addr = ml;
#endif /* BCMDBG_MEM */

	if (do_free) {
		vfree(addr);
	}
} /* osl_vmfree */

uint
osl_check_memleak(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (osh->cmn == NULL) {
		return 0;
	}

	return (atomic_read(&osh->cmn->malloced));
}

uint
osl_malloced(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (atomic_read(&osh->cmn->malloced));
}

uint
osl_malloc_failed(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->failed);
}

#if defined(CONFIG_BCM_TINY_KCORE)
/**
 * iterate over allocations related to given context, ie. osh instance
 *
 * @param[in] ctx osh instance
 * @param[in,out] entry allocation entry information
 */
static int
osl_get_alloc_entry(void *ctx, struct tkcore_alloc_entry *entry)
{
	bcm_mem_link_t *ml;
	osl_t *osh = ctx;

	/*
	 * before iterating the osh->cmn->dbgmem_list we first pass
	 * osh and osh->cmn to the tiny kernel coredump framework so
	 * we can also inspect the content of these instances which
	 * are not on the dbgmem_list as these can not be allocated
	 * with osl_malloc(). The MEMLIST_LOCK is taken upon returning
	 * the first entry on the dbgmem_list and it is held until all
	 * entries have been returned.
	 */
	if (!entry->addr) {
		entry->addr = osh;
		entry->size = sizeof(*osh);
		goto done;
	} else if (entry->addr == osh) {
		entry->addr = osh->cmn;
		entry->size = sizeof(*osh->cmn);
		goto done;
	} else if (entry->addr == osh->cmn) {
		MEMLIST_LOCK(osh, osh->cmn->dbgmem_flags);
		ml = osh->cmn->dbgmem_list;
	} else {
		ml = entry->addr;
		ml = ml->next;
	}
	if (!ml) {
		if (osh->cmn->pending_freelist)
			pr_emerg("%s: wl%u: pending freelist not NULL\n", __FUNCTION__, osh->unit);
		if (osh->cmn->pending_free_count)
			pr_emerg("%s: wl%u: pending freelist count non-zero\n", __FUNCTION__,
				osh->unit);
		osh->cmn->use_pending_freelist = TRUE;
		MEMLIST_UNLOCK(osh, osh->cmn->dbgmem_flags);
		return -ENOENT;
	}
	entry->addr = (char *)ml;
	entry->size = ml->size + sizeof(*ml);
done:
	return 0;
} /* osl_get_alloc_entry */

#ifdef TKCORE_API_VERSION
static void
osl_alloc_entries_ready(void *ctx)
{
	bcm_mem_link_t *ml, *tmp;
	osl_t *osh = ctx;
	unsigned long flags;

	MEMLIST_LOCK(osh, flags);
	osh->cmn->use_pending_freelist = FALSE;
	ml = osh->cmn->pending_freelist;
	osh->cmn->pending_freelist = NULL;
	MEMLIST_UNLOCK(osh, flags);

	while (ml) {
		tmp = ml;
		ml = ml->next;
		_osl_mfree(osh, tmp, tmp->size + sizeof(*tmp));
		osh->cmn->pending_free_count--;
	}
}
#endif /* TKCORE_API_VERSION */

static void osl_register_alloc_iter(osl_t *osh)
{
#ifdef TKCORE_API_VERSION
	struct tkcore_alloc_iter_info aii;

	aii.ctx = osh;
	aii.iter_fn = osl_get_alloc_entry;
	aii.done_fn = osl_alloc_entries_ready;
	tkcore_add_alloc_iterator(&aii);
#else
	tkcore_add_alloc_iterator(osh, osl_get_alloc_entry);
#endif
}
#else
static inline void osl_register_alloc_iter(osl_t *osh)
{
}
#endif /* CONFIG_BCM_TINY_KCORE */

#ifdef BCMDBG_MEM
void *
osl_debug_malloc(osl_t *osh, uint size, int line, const char* file, const char *func)
{
	void *ptr;

	if (!size) {
		printk("%s: allocating zero sized mem at %s line %d\n", __FUNCTION__, file, line);
		ASSERT(0);
	}

	ptr = osl_malloc(osh, size, line, file, func);

	return ptr;
} /* osl_debug_malloc */

void *
osl_debug_mallocz(osl_t *osh, uint size, int line, const char* file, const char *func)
{
	void *ptr;

	ptr = osl_debug_malloc(osh, size, line, file, func);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

/** @param[in] addr  May be NULL, in which case no action is performed. */
void
osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, const char* file, const char *func)
{
	ASSERT(osh == NULL || osh->magic == OS_HANDLE_MAGIC);

	if (addr == 0) {
		return;
	}

	osl_mfree(osh, addr, size, line, file, func);
} /* osl_debug_mfree */

void *
osl_debug_vmalloc(osl_t *osh, uint size, int line, const char* file, const char *func)
{
	void *ptr;

	if (!size) {
		printk("%s: allocating zero sized mem at %s line %d\n", __FUNCTION__, file, line);
		ASSERT(0);
	}

	ptr = osl_vmalloc(osh, size, line, file, func);

	return ptr;
} /* osl_debug_vmalloc */

void *
osl_debug_vmallocz(osl_t *osh, uint size, int line, const char* file, const char *func)
{
	void *ptr;

	ptr = osl_debug_vmalloc(osh, size, line, file, func);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

/** @param[in] addr  May be NULL, in which case no action is performed. */
void
osl_debug_vmfree(osl_t *osh, void *addr, uint size, int line, const char* file, const char *func)
{
	ASSERT(osh == NULL || osh->magic == OS_HANDLE_MAGIC);

	if (addr == NULL) {
		return;
	}

	osl_vmfree(osh, addr, size, line, file, func);
} /* osl_debug_vmfree */

static void
osl_dgbmem_list_dump(bcm_mem_link_t *head, osl_t *osh, struct bcmstrbuf *b)
{
	bcm_mem_link_t *p;

	for (p = head; p; p = p->next) {
		if (b != NULL) {
			bcm_bprintf(b, "%p %6d %s:%d %s\n",
				(char*)p + sizeof(bcm_mem_link_t),
				p->size, p->file, p->line, p->func);
		} else {
			printk("%p %6d %s:%d %s\n",
				(char*)p + sizeof(bcm_mem_link_t),
				p->size, p->file, p->line, p->func);
		}

		/* Detects loop-to-self so we don't enter infinite loop */
		if (p == p->next) {
			if (b != NULL)
				bcm_bprintf(b, "WARNING: loop-to-self "
					"p %p p->next %p\n", p, p->next);
			else
				printk("WARNING: loop-to-self "
					"p %p p->next %p\n", p, p->next);

			break;
		}
	}
}

int
osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (osh->cmn == NULL) {
		return 0;
	}

	if (osl_check_memleak(osh) && osh->cmn->dbgmem_list) {
		if (b != NULL)
			bcm_bprintf(b, "   Address   Size File:line Function\n");
		else
			printk("   Address   Size File:line Function\n");

		osl_dgbmem_list_dump(osh->cmn->dbgmem_list, osh, b);
	}

	if (osh->cmn->dbgvmem_list) {
		if (b != NULL)
			bcm_bprintf(b, "Vmem\n   Address   Size File:line function\n");
		else
			printk("Vmem\n   Address   Size File:line function\n");
		osl_dgbmem_list_dump(osh->cmn->dbgvmem_list, osh, b);
	}

#ifdef BCMDBG_DMAMEM
	if (osh->cmn->dbgdma_consistent_mem_list) {
		if (b != NULL)
			bcm_bprintf(b, "DMA consistent mem\n"
			               "Address   Size File:line function\n");
		else
			printk("DMA consistent mem\n"
			       "   Address   Size File:line function\n");
		osl_dgbmem_list_dump(osh->cmn->dbgdma_consistent_mem_list, osh, b);
	}

	if (osh->cmn->dbgdma_contiguous_mem_list) {
		if (b != NULL)
			bcm_bprintf(b, "DMA contiguous mem\n"
			               "Address   Size File:line function\n");
		else
			printk("DMA contiguous mem\n"
			       "   Address   Size File:line function\n");
		osl_dgbmem_list_dump(osh->cmn->dbgdma_contiguous_mem_list, osh, b);
	}
#endif /* BCMDBG_DMAMEM */

	return 0;
} /* osl_debug_memdump */

#endif	/* BCMDBG_MEM */

/* Note that this function (despite what its parameters may suggest) does NOT guarantee alignment
 * of the returned buffer. It does allocate sufficient bytes for caller to honor the requirement
 *
 * @param[in] align_bits  The caller wishes this number of least significant address bits to be zero
 *                        at the start of the allocation. This function may over-allocate in order
 *                        to satisfy that request, by having the allocated buffer start with a
 *                        padding region. The caller then has to skip this padding region to get
 *                        to its first usable DMA buffer byte.
 *                        Examples:
 *                           - align_bits=0: no over-allocation.
 *                           - align_bits=1: over-allocation of 2 padding bytes so a 2 bytes aligned
 *                                           start address can always be achieved by the caller.
 */
void *
#if defined(BCMDBG_DMAMEM)
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap,
                         uint line, const char* file, const char *func)
#else /* !BCMDBG_DMAMEM */
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap)
#endif /* BCMDBG_DMAMEM */
{
	void *va;
	uint16 align = (1 << align_bits);
	dma_addr_t pap_lin = 0;

	BCM_REFERENCE(pap_lin);
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (align_bits != 0) {
#if defined(BCM_DMA_MALLOC)
		printf("<<<<< Please review the align_bits with BCM_DMA_MALLOC >>>>>>\n");
		printf("%s: align_bits should be 0 with the DMA memory allocator\n", __FUNCTION__);
#endif /* BCM_DMA_MALLOC */
		/* We cannot guarantee alignment so overallocate to allow caller to compensate */
		size += align;
	}

	*alloced = size;

#ifdef BCMDBG_DMAMEM
	size += sizeof(bcm_mem_link_t);
#endif /* BCMDBG_DMAMEM */

/* By Default enabled DHD_USE_COHERENT_MEM_FOR_RING makes Descriptors memory comes
 * from Global CMA pool using dma_alloc_coherent() call for non-secdma drivers.
 * This will also ensures getting corret address when PCIe address are mapped beyond 4GB
 * range like 7278 STB platforms.For 7278 __virt_to_phys won't work since PCIe address
 * range is mapped beyond 4GB range.
 */
#if defined(BCM_ROUTER) || ((defined(STB) || defined(STBAP)) && \
	!defined(DHD_USE_COHERENT_MEM_FOR_RING))
#ifdef CONFIG_WIFI_RETAIN_ALLOC
	if ((va = osl_retain_alloc_get(osh, NULL, size, OSL_ALLOC_TYPE_KMALLOC)) == NULL) {
		va = kmalloc(size, GFP_ATOMIC | __GFP_ZERO);
	}
#else  /* !CONFIG_WIFI_RETAIN_ALLOC */
	va = kmalloc(size, GFP_ATOMIC | __GFP_ZERO);
#endif /* CONFIG_WIFI_RETAIN_ALLOC */
#else /* !(BCM_ROUTER || ((STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING)) */
	{
		struct pci_dev *hwdev = osh->pdev;
#if defined(MLO_IPC)
		struct device *dev;
#endif /* MLO_IPC */
		gfp_t flags;
#ifdef DHD_ALLOC_COHERENT_MEM_FROM_ATOMIC_POOL
		flags = GFP_ATOMIC;
#else
		flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
#endif /* DHD_ALLOC_COHERENT_MEM_FROM_ATOMIC_POOL */

#if defined(MLO_IPC)
		if (hwdev == NULL && (dev = mlo_ipc_get_dev()) != NULL) {
			va = dma_alloc_coherent(dev, size, &pap_lin, flags);
		} else
#endif /* MLO_IPC */
		va = dma_alloc_coherent(&hwdev->dev, size, &pap_lin, flags);
	}
#endif /* BCM_ROUTER || ((STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING) */

	if (va == NULL) {
		return NULL;
	}

	if (osh->cmn != NULL) {
		osh->cmn->dma_alloc_consistent_bytes += *alloced;
	}

#ifdef BCMDBG_DMAMEM
	if (osh->cmn != NULL) {
		osl_dbg_memlist_add(&osh->cmn->dbgdma_consistent_mem_list,
		                    (bcm_mem_link_t *)va, osh, *alloced, line, file, func);
	}

	size -= sizeof(bcm_mem_link_t);
	va = (uint8*)va + sizeof(bcm_mem_link_t);

	{
		uintptr addr_ul;

		PHYSADDRTOULONG(*pap, addr_ul);
		addr_ul += sizeof(bcm_mem_link_t);
		ULONGTOPHYSADDR(addr_ul, *pap);
	}
#if !(defined(BCM_ROUTER) || (defined(STB) || defined(STBAP)) && \
	!defined(DHD_USE_COHERENT_MEM_FOR_RING))
	pap_lin += sizeof(bcm_mem_link_t);
#endif
#endif /* BCMDBG_DMAMEM */

	{
#if defined(BCM_ROUTER) || ((defined(STB) || defined(STBAP)) && \
	!defined(DHD_USE_COHERENT_MEM_FOR_RING))
#if !(defined(STB) || defined(STBAP)) && !(defined(CMWIFI) && (LINUX_VERSION_CODE >= \
	KERNEL_VERSION(5, 4, 0)))
		ASSERT(virt_addr_valid(va));	/* otherwise not safe to use __virt_to_phys() */
		ULONGTOPHYSADDR(__virt_to_phys((ulong)va), *pap);
#else
		struct pci_dev *hwdev = osh->pdev;
#if defined(MLO_IPC)
		struct device *dev;
		if (hwdev == NULL && (dev = mlo_ipc_get_dev()) != NULL) {
			pap_lin = dma_map_single(dev, va, size, DMA_BIDIRECTIONAL);
		}
		else
#endif /* MLO_IPC */
		pap_lin = dma_map_single(&hwdev->dev, va, size, DMA_BIDIRECTIONAL);
#ifdef BCMDMA64OSL
		PHYSADDRLOSET(*pap, pap_lin & 0xffffffff);
		PHYSADDRHISET(*pap, (pap_lin >> 32) & 0xffffffff);
#else
		*pap = (dmaaddr_t)pap_lin;
#endif /* BCMDMA64OSL */
#endif /* STB || STBAP || (CMWIFI && KERNEL >= (5,4,0)) */
#else /* !((BCM_ROUTER || STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING) */
#ifdef BCMDMA64OSL
		PHYSADDRLOSET(*pap, pap_lin & 0xffffffff);
		PHYSADDRHISET(*pap, (pap_lin >> 32) & 0xffffffff);
#else
		*pap = (dmaaddr_t)pap_lin;
#endif /* BCMDMA64OSL */
#endif /* BCM_ROUTER || ((STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING) */
	}

	return va;
} /* osl_dma_alloc_consistent */

void
#if defined(BCMDBG_DMAMEM)
osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa,
                        uint line, const char* file, const char *func)
#else /* !BCMDBG_DMAMEM */
osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa)
#endif /* BCMDBG_DMAMEM */
{
struct pci_dev *hwdev;
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#ifdef BCMDBG_DMAMEM
	bool do_free = TRUE;
#endif /* BCMDBG_DMAMEM */
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (va == NULL) {
		return;
	}

#ifdef BCMDBG_MEM
	if (osh->cmn != NULL) {
		osh->cmn->dma_alloc_consistent_bytes -= size;
	}
#endif /* BCMDBG_MEM */

	hwdev = osh->pdev;
#ifdef CMWIFI_33940
	/*
	 * apparently CMWIFI_33940 also defines BCMDMA64OSL otherwise
	 * compilation would fail because paddr would not be declared.
	 * The only reason to do this thus adding more condtional code
	 * would be if this platform runs a kernel prior to v5.4. If not
	 * this is totally unnecessary.
	 */
	PHYSADDRTOULONG(pa, paddr);
	if (hwdev) {
		dma_unmap_single(&hwdev->dev, paddr, size, DMA_BIDIRECTIONAL);
	}
#if defined(MLO_IPC)
	else {
		struct device *dev;
		if ((dev = mlo_ipc_get_dev()) != NULL) {
			dma_unmap_single(dev, paddr, size, DMA_BIDIRECTIONAL);
		}
	}
#endif /* MLO_IPC */
#else /* !CMWIFI_33940 */

#if defined(BCM_ROUTER) || ((defined(STB) || defined(STBAP)) && \
	!defined(DHD_USE_COHERENT_MEM_FOR_RING))
#if defined(STB) || defined(STBAP) || (defined(CMWIFI) && (LINUX_VERSION_CODE >= \
	KERNEL_VERSION(5, 4, 0)))
#ifdef BCMDMA64OSL
		PHYSADDRTOULONG(pa, paddr);
		dma_unmap_single(&hwdev->dev, paddr, size, DMA_BIDIRECTIONAL);
#else
		dma_unmap_single(&hwdev->dev, pa, size, DMA_BIDIRECTIONAL);
#endif /* BCMDMA64OSL */
#else /* !STB && ! STBAP && !(CMWIFI && KERNEL >= (5,4,0)) */
	BCM_REFERENCE(hwdev);
#ifdef BCMDMA64OSL
	BCM_REFERENCE(paddr);
#endif /* BCMDMA64OSL */
#endif /* !STB && !STBAP && !(CMWIFI && KERNEL >= (5,4,0) */
#endif /* BCM_ROUTER || (STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING) */

#ifdef BCMDBG_DMAMEM
	va = (uint8*)va - sizeof(bcm_mem_link_t);
	if (osh->cmn != NULL) {
		do_free = osl_dbg_memlist_free(&osh->cmn->dbgdma_consistent_mem_list,
		                               (bcm_mem_link_t *)va, osh,
		                               (uint8*)va + sizeof(bcm_mem_link_t), size,
		                               line, file, func);
	}

	if (!do_free) {
		return;
	}

	{
		uintptr addr_ul;

		PHYSADDRTOULONG(pa, addr_ul);
		addr_ul -= sizeof(bcm_mem_link_t);
		ULONGTOPHYSADDR(addr_ul, pa);
	}
	size += sizeof(bcm_mem_link_t);
#endif /* BCMDBG_DMAMEM */
#endif /* CMWIFI_33940 */

#if defined(BCM_ROUTER) || ((defined(STB) || defined(STBAP)) && \
	!defined(DHD_USE_COHERENT_MEM_FOR_RING))
#ifdef CONFIG_WIFI_RETAIN_ALLOC
	if (osl_retain_alloc_save(osh, va, NULL, size, OSL_ALLOC_TYPE_KMALLOC)) {
		return;
	}
#endif /* CONFIG_WIFI_RETAIN_ALLOC */
	kfree(va);
#else
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pa, paddr);
	dma_free_coherent(&hwdev->dev, size, va, paddr);
#else
	dma_free_coherent(&hwdev->dev, size, va, (dma_addr_t)pa);
#endif /* BCMDMA64OSL */
#endif /* BCM_ROUTER || ((STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING) */
} /* osl_dma_free_consistent */

#if defined(BCMDMA64OSL) && defined(BCMVADDR32OSL)
/*
 * return physical address for the corresponding virtual address in linear
 * low address range
 *
 * osh: Not used (for future use)
 *
 * phys_addr_t: u64 for CONFIG_PHYS_ADDR_T_64BIT/BCMDMA64OSL (arm/arm64)
 *              u32 otherwise
 */
phys_addr_t
osl_get_virt_to_phys(osl_t *osh, void *va)
{
	BCM_REFERENCE(osh);

	ASSERT(virt_addr_valid(va));	/* otherwise not safe to use virt_to_phys() */

	return virt_to_phys(va);
}
#endif /* BCMDMA64OSL && BCMVADDR32OSL */

void *
osl_virt_to_phys(void *va)
{
	return (void *)(uintptr)virt_to_phys(va);
}

void *
osl_phys_to_virt(void *va)
{
	return (void *)(uintptr)phys_to_virt((uintptr)va);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
#include <linux/dma-contiguous.h>
#else
#include <linux/dma-map-ops.h>
#endif /* KERNEL < 5.10 */

#if defined(CMWIFI) && defined(CMWIFI_33940)
/*
* There is no hardware memory coherency on CMWIFI SoCs
* It implies there is no capability difference between memory allocate from sys pool
* or from CMA pool except that allocations from the CMA pool can be > 4MB
* and sys mem is limited to <= 4MB blocks of continguous memory.
* For smaller blocks <= 1MB, there is no need to have them occupy the CMA pool,
*/
#define OSL_STEER_CMA_SMALL_ALLOC_TO_SYSMEM
#define OSL_STEER_CMA_SMALL_ALLOC_SIZE			(1024 * 1024)
#endif /* CMWIFI && CMWIFI_33940 */

void * /* Allocate Memory from CMA reserved memory region */
#if defined(BCMDBG_DMAMEM)
osl_dma_alloc_contiguous(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap,
                         uint line, const char* file, const char *func)
#else /* !BCMDBG_DMAMEM */
osl_dma_alloc_contiguous(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap)
#endif /* BCMDBG_DMAMEM */
{
	uint8		*va = NULL;
	dma_addr_t	pap_lin;
	gfp_t		flags;
	uint16		align = (1 << align_bits);
	struct pci_dev *hwdev;
	struct device *dev = NULL;

	ASSERT(osh != NULL);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);
	hwdev = osh->pdev;

#if defined(MLO_IPC)
	if (hwdev == NULL) {
		dev = mlo_ipc_get_dev();
	}
#endif /* MLO_IPC */
	if (dev == NULL) {
		dev = &hwdev->dev;
	}

	if (align_bits != 0) {
		/* We cannot guarantee alignment so overallocate to allow caller to compensate */
		size += align;
	}
	*alloced = size;

#if defined(OSL_STEER_CMA_SMALL_ALLOC_TO_SYSMEM)
	if  (size <= OSL_STEER_CMA_SMALL_ALLOC_SIZE) {
		va = osl_dma_alloc_consistent(osh, size, align_bits, alloced, pap);
#ifdef BCMDBG_MEM
		if (osh->cmn != NULL && va != NULL) {
			osh->cmn->dma_alloc_contiguous_bytes += *alloced;
		}
#endif /* BCMDBG_MEM */

		return va;
	}
#endif /* OSL_STEER_CMA_SMALL_ALLOC_TO_SYSMEM */

	flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;

#ifdef BCMDBG_DMAMEM
	size += sizeof(bcm_mem_link_t);
#endif /* BCMDBG_DMAMEM */

#if defined(BCM_CMA_RSV_MEM)
	va = wlan_cma_rsvmem_alloc(osh, size, &pap_lin);
#endif /* BCM_CMA_RSV_MEM */

#ifdef CONFIG_WIFI_RETAIN_ALLOC
	if (va == NULL) {
		dev = osl_retain_alloc_dev;
		va = osl_retain_alloc_get(osh, &pap_lin, size, OSL_ALLOC_TYPE_DMA_MEM);
	}
#endif /* CONFIG_WIFI_RETAIN_ALLOC */

	if (va == NULL) {
		va = dma_alloc_coherent(dev, size, &pap_lin, flags);
	}

	if (va == NULL) {
		return NULL;
	}

#ifdef BCMDBG_DMAMEM
	pap_lin += sizeof(bcm_mem_link_t);
	size -= sizeof(bcm_mem_link_t);
	if (osh->cmn != NULL) {
		osl_dbg_memlist_add(&osh->cmn->dbgdma_contiguous_mem_list,
		                    (bcm_mem_link_t *)va, osh, *alloced, line, file, func);
	}
	va += sizeof(bcm_mem_link_t);
#endif /* BCMDBG_DMAMEM */

#if defined(BCMDMA64OS_L) && defined(CMWIFI)
	/* clear and flush the memory */
	memset((void*)va, 0, size);
#ifdef CONFIG_WIFI_RETAIN_ALLOC
	dma_sync_single_for_device(osl_retain_alloc_dev, pap_lin, size, DMA_TX);
	/* map addr from allocator device to user device dma range */
	pap_lin = phys_to_dma(&hwdev->dev, pap_lin);
#else /* !CONFIG_WIFI_RETAIN_ALLOC */
	dma_sync_single_for_device(&hwdev->dev, pap_lin, size, DMA_TX);
#endif /* CONFIG_WIFI_RETAIN_ALLOC */
#endif /* BCMDMA64OSL && CMWIFI */

#ifdef BCMDMA64OSL
	PHYSADDRLOSET(*pap, pap_lin & 0xffffffff);
	PHYSADDRHISET(*pap, (pap_lin >> 32) & 0xffffffff);
#else
	*pap = (dmaaddr_t)pap_lin;
#endif /* BCMDMA64OSL */

#ifdef BCMDBG_MEM
	if (osh->cmn != NULL) {
		osh->cmn->dma_alloc_contiguous_bytes += *alloced;
	}
#endif /* BCMDBG_MEM */

	return va;
} /* osl_dma_alloc_contiguous() */

void /* Release memory to CMA reserved region */
#if defined(BCMDBG_DMAMEM)
osl_dma_free_contiguous(osl_t *osh, void * va, uint size, dmaaddr_t pa,
                        uint line, const char* file, const char *func)
#else /* !BCMDBG_DMAMEM */
osl_dma_free_contiguous(osl_t *osh, void * va, uint size, dmaaddr_t pa)
#endif /* BCMDBG_DMAMEM */
{
	struct pci_dev *hwdev;
	dma_addr_t paddr;
	struct device *dev = NULL;
	uint8 *va8 = va;
	bool do_free = TRUE;
#ifdef BCMDBG_MEM
	const uint orig_size = size;	/**< the function caller supplied size */
#endif /* BCMDBG_MEM */

	ASSERT(osh != NULL);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	if (va == NULL) {
		return;
	}

	hwdev = osh->pdev;
#if defined(MLO_IPC)
	if (hwdev == NULL) {
		dev = mlo_ipc_get_dev();
	}
#endif /* MLO_IPC */

	if (dev == NULL) {
		dev = &hwdev->dev;
	}

#if defined(OSL_STEER_CMA_SMALL_ALLOC_TO_SYSMEM)
	if  (size <= OSL_STEER_CMA_SMALL_ALLOC_SIZE) {
#ifdef BCMDBG_MEM
		if (osh->cmn != NULL && va8 != NULL) {
			osh->cmn->dma_alloc_contiguous_bytes -= orig_size;
		}
#endif /* BCMDBG_MEM */
		return	osl_dma_free_consistent(osh, va8, size, pa);
	}
#endif /* OSL_STEER_CMA_SMALL_ALLOC_TO_SYSMEM */

#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pa, paddr);
#else /* !BCMDMA640SL */
	paddr = (dma_addr_t)pa;
#endif /* BCMDMA64OSL */

#ifdef BCMDBG_DMAMEM
	va8 -= sizeof(bcm_mem_link_t);
	if (osh->cmn != NULL) {
		do_free = osl_dbg_memlist_free(&osh->cmn->dbgdma_contiguous_mem_list,
		                               (bcm_mem_link_t *)va8, osh,
		                               va8 + sizeof(bcm_mem_link_t), orig_size,
		                               line, file, func);
	}

	paddr -= sizeof(bcm_mem_link_t);
	size += sizeof(bcm_mem_link_t);
#endif /* BCMDBG_DMAMEM */

	if (do_free == FALSE) {
		return;
	}

#ifdef BCMDBG_MEM
	if (osh->cmn != NULL && va8 != NULL) {
		osh->cmn->dma_alloc_contiguous_bytes -= orig_size;
	}
#endif /* BCMDBG_MEM */

#if defined(BCM_CMA_RSV_MEM)
	if (wlan_cma_rsvmem_free(osh, size, va8, paddr) == 0) {
		/* Part of CMA Reserved Memory, handled */
		return;
	}
#endif /* BCM_CMA_RSV_MEM */

#ifdef CONFIG_WIFI_RETAIN_ALLOC
#if defined(CMWIFI) && defined(BCMDMA64OSL)
	/* map from user device dma to allocator address */
	paddr = dma_to_phys(&hwdev->dev, paddr);
#endif /* CMWIFI && BCMDMA64OSL */
	if (osl_retain_alloc_save(osh, va8, &paddr, size, OSL_ALLOC_TYPE_DMA_MEM)) {
		/* Part of retain allocated memory */
		return;
	}
	dev = osl_retain_alloc_dev;
#endif /* !CONFIG_WIFI_RETAIN_ALLOC */

	dma_free_coherent(dev, size, va8, paddr);
} /* osl_dma_free_contiguous() */

#include <asm/cacheflush.h>
void BCMFASTPATH
osl_dma_flush(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	return;
}

void BCMFASTPATH
osl_dma_sync(osl_t *osh, dmaaddr_t pa, uint size, int direction)
{
	struct pci_dev * pdev;
	dma_addr_t paddr;

	pdev = osh->pdev;
	BCM_REFERENCE(paddr);
#ifdef BCMDMA64OSL

	PHYSADDRTOULONG(pa, paddr);

	if (direction == DMA_TX) { /* to device */
		dma_sync_single_for_device(&(pdev->dev), paddr, size, DMA_TX);
	} else if (direction == DMA_RX) { /* from device */
		dma_sync_single_for_cpu(&(pdev->dev), paddr, size, DMA_RX);
	}
#else
	if (direction == DMA_TX) { /* to device */
		dma_sync_single_for_device(&(pdev->dev), pa, size, DMA_TX);
	} else if (direction == DMA_RX) { /* from device */
		dma_sync_single_for_cpu(&(pdev->dev), pa, size, DMA_RX);
	}
#endif /* BCMDMA64OSL */
	return;
}

dmaaddr_t BCMFASTPATH
osl_dma_map(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	struct pci_dev *pdev;
	int dir;
	dmaaddr_t ret_addr;
	dma_addr_t map_addr;
	int ret;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	pdev = osh->pdev;

#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
	if (wler_is_native_buffer((uintptr)va)) {
		if (direction == DMA_TX)
			wler_dma_flush((uintptr)va, size);
		else
			wler_dma_invalidate((uintptr)va, size);
#if defined(CMWIFI_33940)
		map_addr = osl_phys_to_dma(osh, WLER_PHYS((uintptr)va));
		ULONGTOPHYSADDR(map_addr, ret_addr);
		return ret_addr;
#else /* CMWIFI_33940 */
		return (WLER_PHYS((uintptr)va));
#endif /* CMWIFI_33940 */
	}
#endif /* CMWIFI && CMWIFI_EROUTER */

	dir = (direction == DMA_TX) ? DMA_TO_DEVICE : DMA_FROM_DEVICE;

#if defined(OSL_CACHE_COHERENT)
	if (osl_is_flag_set(osh, OSL_ACP_COHERENCE)) {
		phys_addr_t pa = virt_to_phys(va);
		/* coverity[result_independent_of_operands] */
		if (pa < ACP_WIN_LIMIT) {
			ULONGTOPHYSADDR(pa, ret_addr);
			return (ret_addr);
		}
	}
#endif /* OSL_CACHE_COHERENT */

	map_addr = dma_map_single(&pdev->dev, va, size, dir);

	ret = dma_mapping_error(&pdev->dev, map_addr);
	if (ret) {
		printk("%s: Failed to map memory\n", __FUNCTION__);
		PHYSADDRLOSET(ret_addr, 0);
		PHYSADDRHISET(ret_addr, 0);
	} else {
		PHYSADDRLOSET(ret_addr, map_addr & 0xffffffff);
		PHYSADDRHISET(ret_addr, (map_addr >> 32) & 0xffffffff);
	}

	return ret_addr;
} /* osl_dma_map */

void BCMFASTPATH
osl_dma_unmap(osl_t *osh, dmaaddr_t pa, uint size, int direction)
{
	struct pci_dev * pdev;
	int dir;
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
	uintptr buf;
#endif /* CMWIFI && CMWIFI_EROUTER && !CMWIFI_33940 */

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	pdev = osh->pdev;

#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
#if defined(CMWIFI_33940)
	PHYSADDRTOULONG(pa, paddr);
	buf = WLER_VIRT(paddr);
	if (!wler_is_native_buffer(buf)) {
		dma_unmap_single(&pdev->dev, paddr, size, direction);
		return;
	}
#else /* !CMWIFI_33940 */
	buf = WLER_VIRT(pa);
#endif /* CMWIFI_33940 */
	if (wler_is_native_buffer(buf)) {
		if (direction == DMA_TX)
			wler_dma_flush(buf, size);
		else
			wler_dma_invalidate(buf, size);
		return;
	}
#endif /* CMWIFI && CMWIFI_EROUTER */

#if defined(OSL_CACHE_COHERENT)
	if (osl_is_flag_set(osh, OSL_ACP_COHERENCE)) {
		ulong ulpa;
		PHYSADDRTOULONG(pa, ulpa);
		/* coverity[result_independent_of_operands] */
		if (ulpa < ACP_WIN_LIMIT)
			return;
	}
#endif /* OSL_CACHE_COHERENT */

	dir = (direction == DMA_TX) ? DMA_TO_DEVICE : DMA_FROM_DEVICE;
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pa, paddr);
	dma_unmap_single(&pdev->dev, paddr, size, dir);
#else
	dma_unmap_single(&pdev->dev, (uint32)pa, size, dir);
#endif /* BCMDMA64OSL */

} /* osl_dma_unmap */

/* OSL function for CPU relax */
inline void BCMFASTPATH
osl_cpu_relax(void)
{
	cpu_relax();
}

extern void osl_preempt_disable(osl_t *osh)
{
	preempt_disable();
}

extern void osl_preempt_enable(osl_t *osh)
{
	preempt_enable();
}

#if defined(BCMDBG_ASSERT)
void
osl_assert(const char *exp, const char *file, const char *func, int line, osl_t *osh)
{
	char tempbuf[256];
	const char *basename;
	uint len = 0;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	if (osh != NULL) {
		snprintf(tempbuf, 64, "wl%d: ", osh->unit);
		len = strlen(tempbuf);
	}

	tkcore_save_all_cpu_state();

#ifdef BCMDBG_ASSERT
	snprintf(tempbuf + len, sizeof(tempbuf) - len, "assertion failed: file \"%s\", line %d, "
		"%s(): \"%s\" \n", basename, line, func, exp);
	/* Print assert message and give it time to be written to /var/log/messages */
	if (!in_interrupt() && g_assert_type == ASSERT_T0_PANIC) {
		const int delay = 3;
		printk("%s", tempbuf);
		printk("panic in %d seconds\n", delay);
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(delay * HZ);
	}
#endif /* BCMDBG_ASSERT */

	if (g_assert_type != ASSERT_T0_PANIC && g_assert_type != ASSERT_T2_PROC_DIE) {
		/* ASSERT_T0_PANIC and ASSERT_T2_PROC_DIE already dump stack */
		dump_stack();
	}

	switch (g_assert_type) {
	case ASSERT_T0_PANIC:
		panic("%s", tempbuf);
		break;
	case ASSERT_T1_REINIT:
		/* fall through */
	case ASSERT_T3_WLC_OUT:
		/* fall through */
	case ASSERT_T5_DMP_AND_HANG:
		printk("%s", tempbuf);
		break;
	case ASSERT_T2_PROC_DIE:
		printk("%s", tempbuf);
		BUG();
		break;
	case ASSERT_T4_REINIT:
		/* fall through */
	default:
		break;
	}
} /* osl_assert */
#endif

void
osl_delay(uint usec)
{
	uint d;

#if defined(BCMQT) && BCMQT == VEL_COREONLY_SIM
	usec *= 10; /* Linux runs on QEMU whereas RTL is running on slower emulator */
#endif /* BCMQT && BCMQT == VEL_COREONLY_SIM */

	while (usec > 0) {
		d = MIN(usec, 1000);
		udelay(d);
		usec -= d;
	}
}

void
osl_sleep(uint ms)
{
	if (ms < 20)
		usleep_range(ms*1000, ms*1000 + 1000);
	else
		msleep(ms);
}

time64_us_t BTRACE_NOINSTR_FUNC
osl_sysuptime_us(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
	struct timespec ts;

	getrawmonotonic(&ts);
	return ktime_to_us(timespec_to_ktime(ts));
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
	struct timespec64 ts64;

	getrawmonotonic64(&ts64);
	return ktime_to_us(timespec64_to_ktime(ts64));
#else
#if defined(BCM_ROUTER) && !defined(CMWIFI)
	return ktime_to_us(ktime_get_raw_ns());
#else /* BCM_ROUTER */
	struct timespec64 ts64;

	ktime_get_raw_ts64(&ts64);
	return ktime_to_us(timespec64_to_ktime(ts64));
#endif /* BCM_ROUTER  && !CMWIFI */
#endif /* LINUX_VERSION_CODE */
}

time64_ns_t BTRACE_NOINSTR_FUNC
osl_sysuptime_ns(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
	struct timespec ts;

	getrawmonotonic(&ts);
	return ktime_to_ns(timespec_to_ktime(ts));
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
	struct timespec64 ts64;

	getrawmonotonic64(&ts64);
	return ktime_to_ns(timespec64_to_ktime(ts64));
#else
#if defined(BCM_ROUTER) && !defined(CMWIFI)
	return ktime_to_ns(ktime_get_raw_ns());
#else /* BCM_ROUTER */
	struct timespec64 ts64;

	ktime_get_raw_ts64(&ts64);
	return ktime_to_ns(timespec64_to_ktime(ts64));
#endif /* BCM_ROUTER  && !CMWIFI */
#endif /* LINUX_VERSION_CODE */
}

#if defined(DSLCPE_DELAY)

void
osl_oshsh_init(osl_t *osh, shared_osl_t* oshsh)
{
	extern unsigned long loops_per_jiffy;
	osh->oshsh = oshsh;
	osh->oshsh->MIPS = loops_per_jiffy / (500000/HZ);
}

int
in_long_delay(osl_t *osh)
{
	return osh->oshsh->long_delay;
}

void
osl_long_delay(osl_t *osh, uint usec, bool yield)
{
	uint d;
	bool yielded = TRUE;
	int usec_to_delay = usec;
	unsigned long tick1, tick2, tick_diff = 0;

	/* delay at least requested usec */
	while (usec_to_delay > 0) {
		if (!yield || !yielded) {
			d = MIN(usec_to_delay, 10);
			udelay(d);
			usec_to_delay -= d;
		}
		if (usec_to_delay > 0) {
			osh->oshsh->long_delay++;
			OSL_GETCYCLES(tick1);
			spin_unlock_bh(osh->oshsh->lock);
			if (usec_to_delay > 0 && !in_irq() && !in_softirq() && !in_interrupt()) {
				schedule();
				yielded = TRUE;
			} else {
				yielded = FALSE;
			}
			spin_lock_bh(osh->oshsh->lock);
			OSL_GETCYCLES(tick2);

			if (yielded) {
				tick_diff = TICKDIFF(tick2, tick1);
				tick_diff = (tick_diff * 2)/(osh->oshsh->MIPS);
				if (tick_diff) {
					usec_to_delay -= tick_diff;
				} else
					yielded = 0;
			}
			osh->oshsh->long_delay--;
			ASSERT(osh->oshsh->long_delay >= 0);
		}
	}
} /* osl_long_delay */

#endif /* DSLCPE_DELAY */

/*
 * OSLREGOPS specifies the use of osl_XXX routines to be used for register access
 */
#ifdef OSLREGOPS
uint8
osl_readb(osl_t *osh, volatile uint8 *r)
{
	osl_rreg_fn_t rreg	= ((osl_pubinfo_t*)osh)->rreg_fn;
	void *ctx		= ((osl_pubinfo_t*)osh)->reg_ctx;

	return (uint8)((rreg)(ctx, (volatile void*)r, sizeof(uint8)));
}

uint16
osl_readw(osl_t *osh, volatile uint16 *r)
{
	osl_rreg_fn_t rreg	= ((osl_pubinfo_t*)osh)->rreg_fn;
	void *ctx		= ((osl_pubinfo_t*)osh)->reg_ctx;

	return (uint16)((rreg)(ctx, (volatile void*)r, sizeof(uint16)));
}

uint32
osl_readl(osl_t *osh, volatile uint32 *r)
{
	osl_rreg_fn_t rreg	= ((osl_pubinfo_t*)osh)->rreg_fn;
	void *ctx		= ((osl_pubinfo_t*)osh)->reg_ctx;

	return (uint32)((rreg)(ctx, (volatile void*)r, sizeof(uint32)));
}

void
osl_writeb(osl_t *osh, volatile uint8 *r, uint8 v)
{
	osl_wreg_fn_t wreg	= ((osl_pubinfo_t*)osh)->wreg_fn;
	void *ctx		= ((osl_pubinfo_t*)osh)->reg_ctx;

	((wreg)(ctx, (volatile void*)r, v, sizeof(uint8)));
}

void
osl_writew(osl_t *osh, volatile uint16 *r, uint16 v)
{
	osl_wreg_fn_t wreg	= ((osl_pubinfo_t*)osh)->wreg_fn;
	void *ctx		= ((osl_pubinfo_t*)osh)->reg_ctx;

	((wreg)(ctx, (volatile void*)r, v, sizeof(uint16)));
}

void
osl_writel(osl_t *osh, volatile uint32 *r, uint32 v)
{
	osl_wreg_fn_t wreg	= ((osl_pubinfo_t*)osh)->wreg_fn;
	void *ctx		= ((osl_pubinfo_t*)osh)->reg_ctx;

	((wreg)(ctx, (volatile void*)r, v, sizeof(uint32)));
}
#endif /* OSLREGOPS */

/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 */
#ifdef BINOSL

int
osl_printf(const char *format, ...)
{
	va_list args;
	static char printbuf[1024];
	int len;

	/* sprintf into a local buffer because there *is* no "vprintk()".. */
	va_start(args, format);
	len = vsnprintf(printbuf, 1024, format, args);
	va_end(args);

	if (len > sizeof(printbuf)) {
		printk("osl_printf: buffer overrun\n");
		return (0);
	}

	return (printk("%s", printbuf));
}

int
osl_sprintf(char *buf, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = vsprintf(buf, format, args);
	va_end(args);
	return (rc);
}

int
osl_snprintf(char *buf, size_t n, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = vsnprintf(buf, n, format, args);
	va_end(args);
	return (rc);
}

int
osl_vsprintf(char *buf, const char *format, va_list ap)
{
	return (vsprintf(buf, format, ap));
}

int
osl_vsnprintf(char *buf, size_t n, const char *format, va_list ap)
{
	return (vsnprintf(buf, n, format, ap));
}

int
osl_strcmp(const char *s1, const char *s2)
{
	return (strcmp(s1, s2));
}

int
osl_strncmp(const char *s1, const char *s2, uint n)
{
	return (strncmp(s1, s2, n));
}

int
osl_strlen(const char *s)
{
	return (strlen(s));
}

char*
osl_strcpy(char *d, const char *s)
{
	return (strcpy(d, s));
}

char*
osl_strncpy(char *d, const char *s, uint n)
{
	return (strncpy(d, s, n));
}

char*
osl_strchr(const char *s, int c)
{
	return (strchr(s, c));
}

char*
osl_strrchr(const char *s, int c)
{
	return (strrchr(s, c));
}

void*
osl_memset(void *d, int c, size_t n)
{
	return memset(d, c, n);
}

void*
osl_memcpy(void *d, const void *s, size_t n)
{
	return memcpy(d, s, n);
}

void*
osl_memmove(void *d, const void *s, size_t n)
{
	return memmove(d, s, n);
}

int
osl_memcmp(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}

uint32
osl_readl(volatile uint32 *r)
{
	return (readl(r));
}

uint16
osl_readw(volatile uint16 *r)
{
	return (readw(r));
}

uint8
osl_readb(volatile uint8 *r)
{
	return (readb(r));
}

void
osl_writel(uint32 v, volatile uint32 *r)
{
	writel(v, r);
}

void
osl_writew(uint16 v, volatile uint16 *r)
{
	writew(v, r);
}

void
osl_writeb(uint8 v, volatile uint8 *r)
{
	writeb(v, r);
}

void *
osl_uncached(void *va)
{
#ifdef mips
	return ((void*)KSEG1ADDR(va));
#else
	return ((void*)va);
#endif /* mips */
}

void *
osl_cached(void *va)
{
#ifdef mips
	return ((void*)KSEG0ADDR(va));
#else
	return ((void*)va);
#endif /* mips */
}

uint
osl_getcycles(void)
{
	uint cycles;

#if defined(mips)
	cycles = read_c0_count() * 2;
#elif defined(__i386__)
	rdtscl(cycles);
#else
	cycles = 0;
#endif /* defined(mips) */
	return cycles;
}

void *
osl_reg_map(uint32 pa, uint size)
{
	return (ioremap_nocache((unsigned long)pa, (unsigned long)size));
}

void
osl_reg_unmap(void *va)
{
	iounmap(va);
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
#ifdef mips
	return get_dbe(*val, (uint32 *)addr);
#else
	*val = readl((uint32 *)(uintptr)addr);
	return 0;
#endif /* mips */
}
#endif	/* BINOSL */

uint32
osl_rand(void)
{
	uint32 rand;

	get_random_bytes(&rand, sizeof(rand));

	return rand;
}

/* Linux Kernel: File Operations: start */
void *
osl_os_open_image(char *filename)
{
	struct file *fp;

	fp = filp_open(filename, O_RDONLY, 0);
	/*
	 * 2.6.11 (FC4) supports filp_open() but later revs don't?
	 * Alternative:
	 * fp = open_namei(AT_FDCWD, filename, O_RD, 0);
	 * ???
	 */
	 if (IS_ERR(fp))
		 fp = NULL;

	 return fp;
}

int
osl_os_get_image_block(char *buf, int len, void *image)
{
	struct file *fp = (struct file *)image;

	if (!image)
		return 0;

	return kernel_read(fp, buf, len, &fp->f_pos);
}

void
osl_os_close_image(void *image)
{
	if (image)
		filp_close((struct file *)image, NULL);
}

int
osl_os_image_size(void *image)
{
	int len = 0, curroffset;

	if (image) {
		/* store the current offset */
		curroffset = generic_file_llseek(image, 0, 1);
		/* goto end of file to get length */
		len = generic_file_llseek(image, 0, 2);
		/* restore back the offset */
		generic_file_llseek(image, curroffset, 0);
	}
	return len;
}

int
osl_os_get_image_stat(const char *name, struct kstat *stat)
{
	int err;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0) || defined(CONFIG_SET_FS)
	mm_segment_t old_fs;

	old_fs = get_fs();
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0)
	set_fs(get_ds());
#else
	set_fs(KERNEL_DS);
#endif /* KERNEL < 5.1 */
#endif /* KERNEL < 5.10 || CONFIG_SET_FS */

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	err = vfs_stat(name, stat);
#else
	{
		struct path p;

		err = kern_path(name, 0, &p);
		if (!err) {
			err = vfs_getattr(&p, stat, STATX_BASIC_STATS, 0);
			path_put(&p);
		}
	}
#endif /* KERNEL < 5.10 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0) || defined(CONFIG_SET_FS)
	set_fs(old_fs);
#endif /* KERNEL < 5.10 || CONFIG_SET_FS */

	if (err) {
		printk(KERN_ERR "%s: no file info for %s (%d)\n",
			__FUNCTION__, name, err);
	}
	return err;
}

/* Linux Kernel: File Operations: end */

#if (defined(STB) && defined(__arm__))
inline void osl_pcie_rreg(osl_t *osh, ulong addr, volatile void *v, uint size)
{
	unsigned long flags = 0;
	int pci_access = 0;
	int acp_war_enab = ACP_WAR_ENAB();

	if (osh && BUSTYPE(osh->bustype) == PCI_BUS)
		pci_access = 1;

	if (pci_access && acp_war_enab)
		spin_lock_irqsave(&l2x0_reg_lock, flags);

	switch (size) {
	case sizeof(uint8):
		*(volatile uint8*)v = readb((volatile uint8*)(addr));
		break;
	case sizeof(uint16):
		*(volatile uint16*)v = readw((volatile uint16*)(addr));
		break;
	case sizeof(uint32):
		*(volatile uint32*)v = readl((volatile uint32*)(addr));
		break;
	case sizeof(uint64):
		*(volatile uint64*)v = *((volatile uint64*)(addr));
		break;
	}

	if (pci_access && acp_war_enab) {
		/* coverity[dead_error_line] */
		spin_unlock_irqrestore(&l2x0_reg_lock, flags);
	}
} /* osl_pcie_rreg */
#endif /* STB && __arm__ */

#if defined(BCM_BACKPLANE_TIMEOUT)
inline void osl_bpt_rreg(osl_t *osh, ulong addr, volatile void *v, uint size)
{
	bool poll_timeout = FALSE;
	static int in_si_clear = FALSE;

	switch (size) {
	case sizeof(uint8):
		*(volatile uint8*)v = readb((volatile uint8*)(addr));
		if (*(volatile uint8*)v == 0xff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint16):
		*(volatile uint16*)v = readw((volatile uint16*)(addr));
		if (*(volatile uint16*)v == 0xffff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint32):
		*(volatile uint32*)v = readl((volatile uint32*)(addr));
		if (*(volatile uint32*)v == 0xffffffff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint64):
		*(volatile uint64*)v = *((volatile uint64*)(addr));
		if (*(volatile uint64*)v == 0xffffffffffffffff)
			poll_timeout = TRUE;
		break;
	}

	if (osh && osh->sih && (in_si_clear == FALSE) && poll_timeout && osh->bpt_cb) {
		in_si_clear = TRUE;
		osh->bpt_cb((void *)osh->sih, (void *)addr);
		in_si_clear = FALSE;
	}
} /* osl_bpt_rreg */
#endif /* BCM_BACKPLANE_TIMEOUT */

#if defined(BCM_ROUTER) && !defined(CMWIFI)

static void
_bsp_get_cfe_mac(unsigned int instance_id, char *mac)
{
	unsigned long ulId = MAC_ADDRESS_WLAN + (unsigned long)instance_id;
	kerSysGetMacAddress(mac, ulId);
}

/* used for NIC */
void
osl_adjust_mac(unsigned int instance_id, char *mac)
{
	int i = 0;
	char macaddrbuf[ETH_ALEN] = {0};
	const char macaddr0[] = "00:00:00:00:00:00";
	const char macaddr1[] = "FF:FF:FF:FF:FF:FF";
	if (strncasecmp(mac, macaddr0, 18) == 0 ||
			strncasecmp(mac, macaddr1, 18) == 0) {
		_bsp_get_cfe_mac(instance_id, macaddrbuf);
		for (i = 0; i < (ETH_ALEN-1); i++)
			sprintf(mac+i*3, "%2.2X:", macaddrbuf[i]);
		sprintf(mac + i*3, "%2.2X", macaddrbuf[i]);
	}
}

/* when matching certain parten macaddr= in memblock, replace it with mac from BSP
 * MAC address pool, used for DHD
 */
int
osl_nvram_vars_adjust_mac(unsigned int instance_id, char *memblock, uint* len)
{
	char *locbufp = memblock;
	char macaddrbuf[8] = "macaddr=";
	int i;
	for (i = 0; i < (*len); i++, locbufp++) {
		if (*locbufp == '\0')
			break;
		if (strncasecmp(locbufp, macaddrbuf, 8) == 0) {
			osl_adjust_mac(instance_id, locbufp+8);
			break;
		}
		while (*locbufp != '\0') {
			i++;
			locbufp++;
		}
	}
	return  BCME_OK;
}
#endif /* BCM_ROUTER */

struct file*
open_file(const char *file_name, uint32 flags)
{
	struct file *fp = NULL;

	/* open file to write */
	fp = filp_open(file_name, flags, 0664);
	if (IS_ERR(fp)) {
		printf("%s: open file(%s) error, err = %ld\n", __FUNCTION__,
				file_name, PTR_ERR(fp));
		fp = NULL;
	}
	return fp;
} /* open_file */

void
close_file(struct file *fp)
{
	if (fp != NULL) {
		/* close file before return */
		filp_close(fp, current->files);
	}
} /* close_file */

/**
 * @param[inout] pos  Position in file, updated on return.
 * @return BCME_* status code
 */
int
append_to_file(struct file *fp, uint8 *buf, int size, loff_t *pos)
{
	int ret = BCME_OK;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	ret = kernel_write(fp, buf, size, pos);
	if (ret < 0) {
		printf("write file error, err = %d\n", ret);
	} else {
		ret = BCME_OK;
	}
#else
	fp->f_op->write(fp, buf, size, pos);
#endif

	return ret;
} /* append_to_file */

/* Returns the pid of a the userspace process running with the given name */
struct task_struct *
_get_task_info(const char *pname)
{
	struct task_struct *task;
	if (!pname)
		return NULL;

	for_each_process(task) {
		if (strcmp(pname, task->comm) == 0)
			return task;
	}

	return NULL;
}

int
osl_create_directory(char *pathname, int mode)
{
	int err = 0;
#if defined(BCMDONGLEHOST) || defined(BCM_ROUTER) || LINUX_VERSION_CODE < \
	KERNEL_VERSION(6, 3, 0)
	struct path path;
	struct dentry *dentry;

	dentry = kern_path_create(AT_FDCWD, pathname, &path, LOOKUP_DIRECTORY);
	if (IS_ERR(dentry)) {
		err = PTR_ERR(dentry);
		printf("%s: kern_path_create failed %d\n",
			__FUNCTION__, err);
		return BCME_ERROR;
	}

	err = security_path_mkdir(&path, dentry, mode);
	if (err) {
		printf("%s: create path %s not allowed %d\n",
			__FUNCTION__, pathname, err);
		goto exit;
	}

	/*
	 * this code is conditional because in v6.3 kernel
	 * vfs_mkdir requires use of GPL-only symbol.
	 */
	err = osl_vfs_mkdir(path, dentry, mode);
	if (err) {
		printf("%s: failed to create %s %d\n",
			__FUNCTION__, pathname, err);
	}
exit:
	done_path_create(&path, dentry);
#else
	pr_err("%s: no kernel support; create %s manually\n",
		__FUNCTION__, pathname);
#endif /* BCMDONGLEHOST || BCM_ROUTER || KERNEL < 6.3 */
	return err ? BCME_ERROR : BCME_OK;
}

/* Linux specific multipurpose spinlock API */
void *
osl_spin_lock_init(osl_t *osh)
{
	/* Adding 4 bytes since the sizeof(spinlock_t) could be 0 */
	/* if CONFIG_SMP and CONFIG_DEBUG_SPINLOCK are not defined */
	/* and this results in kernel asserts in internal builds */
	spinlock_t * lock = MALLOC(osh, sizeof(spinlock_t) + 4);
	if (lock) {
		spin_lock_init(lock);
	}
	return ((void *)lock);
}

void
osl_spin_lock_deinit(osl_t *osh, void *lock)
{
	if (lock) {
		MFREE(osh, lock, sizeof(spinlock_t) + 4);
	}
}

unsigned long
osl_spin_lock(void *lock)
{
	unsigned long flags = 0;

	if (lock) {
#ifdef DHD_USE_SPIN_LOCK_BH
		/* Calling spin_lock_bh with both irq and non-irq context will lead to deadlock */
		ASSERT(!in_irq());
		spin_lock_bh((spinlock_t *)lock);
#else
		spin_lock_irqsave((spinlock_t *)lock, flags);
#endif /* DHD_USE_SPIN_LOCK_BH */
	}

	return flags;
}

void
osl_spin_unlock(void *lock, unsigned long flags)
{
	if (lock) {
#ifdef DHD_USE_SPIN_LOCK_BH
		/* Calling spin_lock_bh with both irq and non-irq context will lead to deadlock */
		ASSERT(!in_irq());
		spin_unlock_bh((spinlock_t *)lock);
#else
		spin_unlock_irqrestore((spinlock_t *)lock, flags);
#endif /* DHD_USE_SPIN_LOCK_BH */
	}
}

unsigned long
osl_spin_lock_irq(void *lock)
{
	unsigned long flags = 0;

	if (lock) {
		spin_lock_irqsave((spinlock_t *)lock, flags);
	}

	return flags;
}

void
osl_spin_unlock_irq(void *lock, unsigned long flags)
{
	if (lock) {
		spin_unlock_irqrestore((spinlock_t *)lock, flags);
	}
}

unsigned long
osl_spin_lock_bh(void *lock)
{
	unsigned long flags = 0;

	if (lock) {
		/* Calling spin_lock_bh with both irq and non-irq context will lead to deadlock */
		ASSERT(!in_irq());
		spin_lock_bh((spinlock_t *)lock);
	}

	return flags;
}

void
osl_spin_unlock_bh(void *lock, unsigned long flags)
{
	if (lock) {
		/* Calling spin_lock_bh with both irq and non-irq context will lead to deadlock */
		ASSERT(!in_irq());
		spin_unlock_bh((spinlock_t *)lock);
	}
}

#ifdef CONFIG_WIFI_RETAIN_ALLOC

/*
 * WiFi Driver requires several MB blocks of contiguous memory.
 *
 * With radio power save feature using dynamic driver bind/unbind,
 * these large blocks will be allocated/freed at every power down/up,
 * and over time kernel memory fragmentation results in driver
 * bind failures due to allocation failure.
 * CMA - mitigates these issues to some extent, but in presence
 * of nonWiFi drivers making use of CMA allocator, CMA allocations
 * too can fail.
 * To prevent these alloc failures - WiFi driver caches large allocs.
 * Large allocations >1MB are cached until the rmmod of the driver.
 * As long as kernel is successfully able to allocate the memory at boot,
 * rebind operations on devices will be successful.
 */
static bool
osl_retain_alloc_save(osl_t *osh, void *addr, dma_addr_t *pa, uint size, osl_alloc_type_t type)
{
	unsigned long flags;
	int i;
	bool taken = false;

	if (size < osl_retain_alloc_min_size[type])
		return false;

	if (!osl_retain_alloc_dev || !osl_retain_alloc_info_tbl)
		return false;

	spin_lock_irqsave(&osl_retain_alloc_lock, flags);
	for (i = 0; i < OSL_REUSE_ALLOC_TABLE_SIZE; i++) {
		if (osl_retain_alloc_info_tbl[i].valid)
			continue;
		osl_retain_alloc_info_tbl[i].valid = true;
		osl_retain_alloc_info_tbl[i].unit = osl_unit(osh);
		osl_retain_alloc_info_tbl[i].va = addr;
		if (pa) {
			osl_retain_alloc_info_tbl[i].pa = *pa;
		}
		osl_retain_alloc_info_tbl[i].size = size;
		osl_retain_alloc_info_tbl[i].mem_type = type;
		taken = true;
		break;
	}
	spin_unlock_irqrestore(&osl_retain_alloc_lock, flags);

	if (!taken) {
	    printf("%s : warning, reuse failed, reuse alloc table full\n", __FUNCTION__);
	}

	return taken;
}

static void *
osl_retain_alloc_get(osl_t *osh, dma_addr_t *pa, uint size, osl_alloc_type_t type)
{
	unsigned long flags;
	void *addr = NULL;
	int i;

	if (size < osl_retain_alloc_min_size[type])
		return NULL;

	if (!osl_retain_alloc_dev || !osl_retain_alloc_info_tbl)
		return NULL;

	spin_lock_irqsave(&osl_retain_alloc_lock, flags);
	for (i = 0; i < OSL_REUSE_ALLOC_TABLE_SIZE; i++) {
		if (!osl_retain_alloc_info_tbl[i].valid)
			continue;
		if (osl_unit(osh) != osl_retain_alloc_info_tbl[i].unit)
			continue;
		if (type != osl_retain_alloc_info_tbl[i].mem_type)
			continue;
		if (size != osl_retain_alloc_info_tbl[i].size)
			continue;
		osl_retain_alloc_info_tbl[i].valid = false;
		addr = osl_retain_alloc_info_tbl[i].va;
		if (pa)
			*pa = osl_retain_alloc_info_tbl[i].pa;
		break;
	}
	spin_unlock_irqrestore(&osl_retain_alloc_lock, flags);
	return addr;
}

static void
osl_retain_alloc_stats(osl_retain_alloc_info_t *tbl, int tbl_sz)
{
	int i;
	int count[OSL_ALLOC_TYPE_LAST] = {0};
	int size[OSL_ALLOC_TYPE_LAST] = {0};

	for (i = 0; i < tbl_sz; i++) {
		if (!tbl[i].valid)
			continue;
		count[tbl[i].mem_type]++;
		size[tbl[i].mem_type] += tbl[i].size;
	}

	for (i = 0; i < OSL_ALLOC_TYPE_LAST; i++) {
		switch (i) {
			case OSL_ALLOC_TYPE_KMALLOC:
				printf("Freeing: alloc type: kmalloc %d allocs, total mem = %d\n",
					count[i], size[i]);
				break;
			case OSL_ALLOC_TYPE_DMA_MEM:
				printf("Freeing: alloc type: dma mem %d allocs, total mem = %d\n",
					count[i], size[i]);
				break;
			default:
				break;
		}
	}
}

static void
osl_retain_alloc_release(osl_retain_alloc_info_t *tbl, int tbl_sz)
{
	int i;
	if (!osl_retain_alloc_dev)
		return;

	for (i = 0; i < tbl_sz; i++) {

		if (!tbl[i].valid)
			continue;
		tbl[i].valid = false;

		switch (tbl[i].mem_type) {
			case OSL_ALLOC_TYPE_KMALLOC:
				kfree(tbl[i].va);
				break;
			case OSL_ALLOC_TYPE_DMA_MEM:
				dma_free_coherent(osl_retain_alloc_dev,
					tbl[i].size,
					tbl[i].va,
					tbl[i].pa);
				break;
			default:
				break;
		}
	}
}

void
osl_retain_alloc_attach(void)
{
	int tblsz = sizeof(osl_retain_alloc_info_t) * OSL_REUSE_ALLOC_TABLE_SIZE;

	if ((osl_retain_alloc_dev = kmalloc(sizeof(struct device), GFP_KERNEL)) != NULL) {
		memset((void*)osl_retain_alloc_dev, 0, sizeof(struct device));
		if ((osl_retain_alloc_info_tbl = kmalloc(tblsz, GFP_KERNEL)) != NULL) {
			memset((void*)osl_retain_alloc_info_tbl, 0, tblsz);
#if !defined(CMWIFI)
			arch_setup_dma_ops(osl_retain_alloc_dev, 0, 0, NULL, true);
#endif /* CMWIFI */
			dma_coerce_mask_and_coherent(osl_retain_alloc_dev, DMA_BIT_MASK(64));
		}
		else {
			kfree(osl_retain_alloc_dev);
			osl_retain_alloc_dev = NULL;
		}
	}
	spin_lock_init(&osl_retain_alloc_lock);

	return;
}

void
osl_retain_alloc_detach(void)
{
	osl_retain_alloc_info_t *tbl;
	int tbl_sz;

	if (!osl_retain_alloc_dev || !osl_retain_alloc_info_tbl) {
		return;
	}

	tbl = osl_retain_alloc_info_tbl;
	tbl_sz = OSL_REUSE_ALLOC_TABLE_SIZE;

	osl_retain_alloc_stats(tbl, tbl_sz);
	/* no need for lock during free */
	osl_retain_alloc_release(tbl, tbl_sz);
	kfree(osl_retain_alloc_dev);
	kfree(osl_retain_alloc_info_tbl);
	osl_retain_alloc_dev = NULL;
	osl_retain_alloc_info_tbl = NULL;
}
#endif /* CONFIG_WIFI_RETAIN_ALLOC */

void osl_dev_hold(void *ndev)
{
	dev_hold(ndev);
}

void osl_dev_put(void *ndev)
{
	dev_put(ndev);
}

#ifdef CMWIFI
/*
 * map addr from allocator device to device dma range
 * cases where pcie dma addr != cpu phys addr
 */
dma_addr_t osl_phys_to_dma(osl_t *osh, phys_addr_t pa)
{
	dma_addr_t ret_addr = pa;
#ifdef BCMDMA64OSL
	struct pci_dev *hwdev = osh->pdev;
	ASSERT(hwdev);
	ret_addr = phys_to_dma(&hwdev->dev, pa);
#endif /* BCMDMA640SL */
	return ret_addr;
}

/*
 * map addr from device dma to allocator device
 * cases where pcie dma addr != cpu phys addr
 */
phys_addr_t osl_dma_to_phys(osl_t *osh, dma_addr_t pa)
{
	dma_addr_t ret_addr = pa;
#ifdef BCMDMA64OSL
	struct pci_dev *hwdev = osh->pdev;
	ASSERT(hwdev);
	ret_addr = dma_to_phys(&hwdev->dev, pa);
#endif /* BCMDMA640SL */
	return ret_addr;
}
#endif /* CMWIFI */

bool
osl_pcie_has_root_port_ptm_enabled(void *dev)
{
#if defined(CONFIG_PCIE_PTM)
	struct pci_dev *root = (struct pci_dev *)dev;

	while (root && !(pci_pcie_type(root) == PCI_EXP_TYPE_ROOT_PORT))  {
		root = root->bus->self;
	}
	if (root) {
		return root->ptm_enabled;
	}
#endif /* CONFIG_PCIE_PTM */
	return FALSE;
}

#if defined(BCM_DMA_MALLOC)
/* Init memt pool from memt_bank */
void
dma_memt_bank_init(osl_t *osh)
{
	int i;

	for (i = 0; i < DMA_MEMT_BANK_NUM; i++) {
		osh->dma_memt_bank[i] = 0;
	}

	osh->dma_memt_bank_count = 0;

	dll_init(&osh->dma_memt_pool);
}

void
dma_memt_bank_deinit(osl_t *osh)
{
	int i;

	for (i = 0; i < DMA_MEMT_BANK_NUM; i++) {
		if (osh->dma_memt_bank[i]) {
			MFREE(osh, osh->dma_memt_bank[i],
				sizeof(dma_mem_t) * DMA_MEMT_PER_BANK);
			osh->dma_memt_bank[i] = 0;

			osh->dma_memt_bank_count--;
		}
	}

	if (osh->dma_memt_bank_count != 0) {
		printf("%s: dma_memt_bank_count (%d) is nonzero after release\n",
			__FUNCTION__, osh->dma_memt_bank_count);
		ASSERT(osh->dma_memt_bank_count == 0);
	}
}

void
dma_memt_bank_add(osl_t *osh)
{
	dma_mem_t *bank;
	dma_mem_t *meta;
	dll_t temp;
	int i;

	if (osh->dma_memt_bank_count == DMA_MEMT_BANK_NUM) {
		printf("<<<<< Please review the max memt bank count >>>>>>\n");
		printf("%s: no available entry\n", __FUNCTION__);
		return;
	}

	bank = (dma_mem_t *)MALLOCZ(osh, sizeof(dma_mem_t) * DMA_MEMT_PER_BANK);
	if (bank == NULL) {
		printf("%s:: out of memory, malloced %u bytes\n", __FUNCTION__,
			(uint)(sizeof(dma_mem_t) * DMA_MEMT_PER_BANK));
		return;
	}

	osh->dma_memt_bank[osh->dma_memt_bank_count] = bank;
	osh->dma_memt_bank_count++;

	dll_init(&temp);
	for (i = 0; i < DMA_MEMT_PER_BANK; i++) {
		meta = bank + i;
		dll_init(&meta->node);
		dll_append(&temp, &meta->node);
	}

	dll_join(&temp, &osh->dma_memt_pool);
}

/* memt alloc/free */
dma_mem_t *
dma_memt_alloc(osl_t *osh)
{
	if (dll_empty(&osh->dma_memt_pool)) {
		/* Try to allocate a bank */
		dma_memt_bank_add(osh);
	}

	if (!dll_empty(&osh->dma_memt_pool)) {
		dll_t *head = dll_head_p(&osh->dma_memt_pool);

		dll_delete(head);
		dll_init(head);
		return (dma_mem_t *)head;
	}

	printf("%s: Out of memory: cannot allocate a DMA memory metadata\n", __FUNCTION__);
	return NULL;
}

static void
dma_memt_free(osl_t *osh, dma_mem_t *meta)
{
	memset(meta, 0, sizeof(*meta));
	dll_init(&meta->node);

	dll_append(&osh->dma_memt_pool, &meta->node);
}

void
dma_arena_init(osl_t *osh)
{
	/* Init arena */
	int i;

	for (i = 0; i < DMA_MEM_ARENA_NUM; i++) {
		osh->dma_arena[i].size = 0;
		osh->dma_arena[i].base = 0;
		osh->dma_arena[i].base_pa = 0;
	}

	osh->dma_arena_count = 0;
}

void
dma_arena_deinit(osl_t *osh)
{
	uint size;
	void *va;
	dma_addr_t base_pa;
	int i;
	dmaaddr_t pa;

	for (i = 0; i < DMA_MEM_ARENA_NUM; i++) {
		if (osh->dma_arena[i].base) {
			va = osh->dma_arena[i].base;
			base_pa = osh->dma_arena[i].base_pa;
			size = osh->dma_arena[i].size;

			ULONGTOPHYSADDR(base_pa, pa);
#if !defined(BCMDBG_DMAMEM)
			osl_dma_free_consistent(osh, va, size, pa);
#else /* BCMDBG_DMAMEM */
			osl_dma_free_consistent(osh, va, size, pa,
				__LINE__, __FILE__, __FUNCTION__);
#endif /* BCMDBG_DMAMEM */

			osh->dma_arena[i].base = 0;
			osh->dma_arena[i].base_pa = 0;
			osh->dma_arena[i].size = 0;

			osh->dma_arena_count--;
		}
	}

	if (osh->dma_arena_count != 0) {
		printf("%s: dma_arena_count (%d) is nonzero after release\n",
			__FUNCTION__, osh->dma_arena_count);
		ASSERT(osh->dma_arena_count == 0);
	}
}

int
dma_arena_add(osl_t *osh)
{
	void *base;
	dma_addr_t base_pa = 0;
	uint size = DMA_ARENA_BLOCK_SIZE;
	dma_mem_t *_new;
	int i;
	uint alloced;
	dmaaddr_t pap;

	if (osh->dma_arena_count >= DMA_MEM_ARENA_NUM) {
		printf("<<<<< Please review the max arena count >>>>>>\n");
		printf("%s: dma_arena_count (%u) is all used\n", __FUNCTION__,
			osh->dma_arena_count);
		return BCME_ERROR;
	}
#if defined(BCMDBG_DMAMEM)
	base = osl_dma_alloc_consistent(osh, size, 0, &alloced, &pap,
	                         __LINE__, __FILE__, __FUNCTION__);
#else /* !BCMDBG_DMAMEM */
	base = osl_dma_alloc_consistent(osh, size, 0, &alloced, &pap);
#endif /* !BCMDBG_DMAMEM */
	if (!base) {
		printf("%s: Cannot allocate DMA memory\n", __FUNCTION__);
		return BCME_ERROR;
	}

	PHYSADDRTOULONG(pap, base_pa);

	i = osh->dma_arena_count;
	osh->dma_arena[i].size = size;
	osh->dma_arena[i].base = base;
	osh->dma_arena[i].base_pa = base_pa;
	osh->dma_arena_count++;

	_new = dma_memt_alloc(osh);
	_new->size = size;
	_new->arena = i;
	_new->buf = base;
	_new->buf_pa = base_pa;

	/* add to free memory */
	_dma_msort_insert(_new, &osh->dma_memfree);

	return 0;
}

static void
_dma_msort_insert(dma_mem_t *meta, dll_t *list)
{
	dll_t *pos;
	dma_mem_t *next;

	dll_for_each(pos, list) {
		next = (dma_mem_t *)pos;
		if (meta->buf_pa < next->buf_pa) {
			/* next is the first larger */
			break;
		}
	}
	if (pos == list) {
		/* Append to the list */
		dll_append(list, &meta->node);
	} else {
		/* Insert before the first larger entry */
		dll_insert(&meta->node, dll_prev_p(pos));
	}
}

static void
_dma_mfree(osl_t *osh, dma_mem_t *meta)
{
	dma_mem_t *next, *prev;

	/* Insert with LSB sorted order */
	_dma_msort_insert(meta, &osh->dma_memfree);

	/* Coalesce with next if appropriate */
	next = (dma_mem_t *)dll_next_p(&meta->node);
	if (&next->node != &osh->dma_memfree &&
		next->buf_pa == (meta->buf_pa + meta->size) &&
		next->arena == meta->arena) {
		/* Merge the next */
		meta->size += next->size;

		dll_delete(&next->node);
		dma_memt_free(osh, next);
	}

	/* Coalesce with prev if appropriate */
	prev = (dma_mem_t *)dll_prev_p(&meta->node);
	if (&prev->node != &osh->dma_memfree &&
		meta->buf_pa == (prev->buf_pa + prev->size) &&
		meta->arena == prev->arena) {
		/* Merge to the prev */
		prev->size += meta->size;

		dll_delete(&meta->node);
		dma_memt_free(osh, meta);
	}
}

void *
dma_malloc_align_boundary(osl_t *osh, uint boundarybits, uint size,
	uint16 *alignbits, uint *alloced, dmaaddr_t *pa)
{
	unsigned int align, boundary;
	unsigned int waste = DMA_ARENA_BLOCK_SIZE;
	dma_mem_t *curr, *this = 0;
	dma_mem_t *_new;
	int rem;
	uintptr_t top, addr;
	dll_t *pos;
	struct pci_dev * pdev = osh->pdev;
#if defined(MLO_IPC)
	struct device *dev;
#endif /* MLO_IPC */

	/* Make sure size is aligned to minimum */
	size = ALIGN(size, DMA_MIN_ALIGN);

	if (*alignbits < DMA_MIN_ALIGN_BITS) {
		*alignbits = DMA_MIN_ALIGN_BITS;
	}
	if (*alignbits > DMA_MAX_ALIGN_BITS) {
		printf("<<<<< Please review the max alignment bits >>>>>>\n"
			"DMA memory alignbits %u exceeds"
			" and is changed to the max supported %u\n",
			*alignbits, DMA_MAX_ALIGN_BITS);
		*alignbits = DMA_MAX_ALIGN_BITS;
	}
	align = 1 << *alignbits;

	if (ISPOWEROF2(size)) {
		align = MAX(size, align);
	}

	if (boundarybits != 0) {
		boundary = 1 << boundarybits;
		if (*alignbits > boundarybits) {
			printf("<<<<< Please review the alignment and boundary bits >>>>>>\n"
				"DMA memory alignbits %u is larger than boundarybits %u\n",
				*alignbits, boundarybits);
		}
	}

reiterate:
	dll_for_each(pos, &osh->dma_memfree) {
		curr = (dma_mem_t *)pos;
		if (curr->size >= size) {
			uintptr_t lowest = (uintptr_t)curr->buf_pa;
			uintptr_t end = lowest + curr->size;
			uintptr_t highest = end - size;
			uintptr_t a = ALIGN_DOWN(highest, align);
			uintptr_t b;

			if (a < lowest) {
				continue;
			}

			if (boundarybits != 0) {
				/* a + size must not reach or cross the boundary */
				b = ALIGN(a, boundary);

				/* note: a == b means a is on the boundary */
				if ((a < b) && ((a + size) >= b)) {
					a = ALIGN_DOWN(b - size - 1, align);
					if (a < lowest) {
						continue;
					}
				}
			}

			if ((a - lowest) < waste) {
				waste = a - lowest;
				this = curr;
				top = end;	/* For splitting */
				addr = a;

				if (waste == 0) {
					break;
				}
			}
		}
	}

	if (this == 0) {
		if (dma_arena_add(osh) != 0) {
			printf("%s: dma_arena_add failed\n", __FUNCTION__);
			return 0;
		}

		goto reiterate;
	}

	/* Anything above? */
	rem = (top - addr) - size;
	if (rem) {
		/* Split off the top */
		this->size -= rem;

		_new = dma_memt_alloc(osh);
		if (!_new) {
			return NULL;
		}

		dll_init(&_new->node);
		_new->buf = this->buf + this->size;
		_new->buf_pa = this->buf_pa + this->size;
		_new->arena = this->arena;
		_new->size = rem;

		dll_insert(&_new->node, &this->node);
	}

	/* Anything below? */
	rem = this->size - size;
	if (rem) {
		/* Split this */
		dll_t *prev = dll_prev_p(&this->node);
		void *orig_buf = this->buf;
		dma_addr_t orig_buf_pa = this->buf_pa;

		this->buf = orig_buf + rem;
		this->buf_pa = orig_buf_pa + rem;
		this->size = size;

		_new = dma_memt_alloc(osh);
		if (!_new) {
			return NULL;
		}

		dll_init(&_new->node);
		_new->buf = orig_buf;
		_new->buf_pa = orig_buf_pa;
		_new->arena = this->arena;
		_new->size = rem;

		dll_insert(&_new->node, prev);
	}

	/* Remove from the free list and insert to the dma mem inuse list */
	dll_delete(&this->node);

	_dma_msort_insert(this, &osh->dma_meminuse);

	bzero(this->buf, this->size);
#if defined(MLO_IPC)
	if (pdev == NULL && (dev = mlo_ipc_get_dev()) != NULL) {
		dma_map_single(dev, this->buf, this->size, DMA_BIDIRECTIONAL);
	} else
#endif /* MLO_IPC */
	dma_map_single(&(pdev->dev), this->buf, this->size, DMA_BIDIRECTIONAL);

	*alloced = this->size;
	ULONGTOPHYSADDR(this->buf_pa, *pa);

#if defined(BCM_DMA_MALLOC_DEBUG)
	printf("%s: OSH <%p> size %u align %u pa %pad\n", __FUNCTION__, osh, size,
		align, &this->buf_pa);
#endif /* BCM_DMA_MALLOC_DEBUG */

	return this->buf;
}

void *
dma_malloc_align(osl_t *osh, uint size, uint16 alignbits, uint *alloced, dmaaddr_t *pa)
{
	uint16 _alignbits = alignbits;

	/* Should change the linux and rte function to reflect the
	 * alignment returned
	 */
	return dma_malloc_align_boundary(osh, 0, size, &_alignbits, alloced, pa);
}

void
dma_mfree(osl_t *osh, void *buf, dmaaddr_t pa, unsigned int size)
{
	dll_t *pos;
	dma_mem_t *meta = 0;
	dma_addr_t paddr;

	PHYSADDRTOULONG(pa, paddr);

	/* Search in dma_meminuse */
	dll_for_each(pos, &osh->dma_meminuse) {
		meta = (dma_mem_t *)pos;
		if (meta->buf_pa == paddr) {
			if (meta->size != size) {
				printf("%s: The memory size (%u) to free"
					" does not match (%u)\n",
					__FUNCTION__, size, meta->size);
			}
			dll_delete(pos);
			break;
		}
	}

	if (meta == 0) {
		printf("%s: Cannot find address to free\n", __FUNCTION__);
		return;
	}

	/* Free with mem_t */
	_dma_mfree(osh, meta);
}

void
dma_malloc_dump(osl_t *osh, struct bcmstrbuf *b)
{
	int i;
	dll_t *pos;
	dma_mem_t *p;

	/* Print arena segments */
	if (b) {
		bcm_bprintf(b, "OSH <%p> DMA arena segment count = %d\n",
			osh, osh->dma_arena_count);
	}
#if defined(BCM_DMA_MALLOC_DEBUG)
	else {
		printf("OSH <%p> DMA arena segment count = %d\n",
			osh, osh->dma_arena_count);
	}
#endif /* BCM_DMA_MALLOC_DEBUG */
	for (i = 0; i < osh->dma_arena_count; i++) {
		if (b) {
			bcm_bprintf(b, "Entry %d: base %p pa %pad size %u(0x%x)\n",
				i,
				osh->dma_arena[i].base,
				&osh->dma_arena[i].base_pa,
				osh->dma_arena[i].size,
				osh->dma_arena[i].size);
		}
#if defined(BCM_DMA_MALLOC_DEBUG)
		else {
			printf("Entry %d: base %p pa %pad size %u(0x%x)\n",
				i,
				osh->dma_arena[i].base,
				&osh->dma_arena[i].base_pa,
				osh->dma_arena[i].size,
				osh->dma_arena[i].size);
		}
#endif /* BCM_DMA_MALLOC_DEBUG */
	}

#if defined(BCM_DMA_MALLOC_DEBUG)
	/* Print DMA meminuse list */
	if (b) {
		bcm_bprintf(b, "DMA meminuse list\n");
	} else {
		printf("DMA meminuse list\n");
	}
	dll_for_each(pos, &osh->dma_meminuse) {
		p = (dma_mem_t *)pos;
		if (b) {
			bcm_bprintf(b, "buf %p pa %pad size %u(0x%x)\n",
				p->buf, &p->buf_pa, p->size, p->size);
		} else  {
			printf("buf %p pa %pad size %u(0x%x)\n",
				p->buf, &p->buf_pa, p->size, p->size);
		}
	}
#endif /* BCM_DMA_MALLOC_DEBUG */

	/* Print DMA memfree list */
	if (b) {
		bcm_bprintf(b, "DMA memfree list\n");
	}
#if defined(BCM_DMA_MALLOC_DEBUG)
	else {
		printf("DMA memfree list\n");
	}
#endif /* BCM_DMA_MALLOC_DEBUG */
	dll_for_each(pos, &osh->dma_memfree) {
		p = (dma_mem_t *)pos;
		if (b) {
			bcm_bprintf(b, "buf %p pa %pad size %u(0x%x)\n",
				p->buf, &p->buf_pa, p->size, p->size);
		}
#if defined(BCM_DMA_MALLOC_DEBUG)
		else {
			printf("buf %p pa %pad size %u(0x%x)\n",
				p->buf, &p->buf_pa, p->size, p->size);
		}
#endif /* BCM_DMA_MALLOC_DEBUG */
	}
}

int
dma_malloc_init(osl_t *osh)
{
#if defined(BCM_DMA_MALLOC_DEBUG)
	printf("%s: OSH <%p>\n", __FUNCTION__, osh);
#endif /* BCM_DMA_MALLOC_DEBUG */

	dma_memt_bank_init(osh);
	dma_memt_bank_add(osh);

	dll_init(&osh->dma_memfree);
	dll_init(&osh->dma_meminuse);

	/* Call after bank and memfree inited */
	dma_arena_init(osh);
	dma_arena_add(osh);

#if defined(BCM_DMA_MALLOC_DEBUG)
	/* Debug print */
	dma_malloc_dump(osh, NULL);
#endif /* BCM_DMA_MALLOC_DEBUG */

	return 0;
}

int
dma_malloc_deinit(osl_t *osh)
{
#if defined(BCM_DMA_MALLOC_DEBUG)
	printf("%s: OSH <%p>\n", __FUNCTION__, osh);

	/* Debug print */
	dma_malloc_dump(osh, NULL);
#endif /* BCM_DMA_MALLOC_DEBUG */

	dma_arena_deinit(osh);
	dma_memt_bank_deinit(osh);
	return 0;
}
#endif /* BCM_DMA_MALLOC */
