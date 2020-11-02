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
 * $Id: linux_osl.c 785666 2020-04-02 13:56:26Z $
 */

#define LINUX_PORT

#include <typedefs.h>
#include <bcmendian.h>
#include <linuxver.h>
#include <bcmdefs.h>

#ifdef mips
#include <asm/paccess.h>
#include <asm/cache.h>
#include <asm/r4kcache.h>
#undef ABS
#endif /* mips */

#if !defined(STBLINUX)
#if defined(__ARM_ARCH_7A__) && !defined(DHD_USE_COHERENT_MEM_FOR_RING)
#include <asm/cacheflush.h>
#endif /* __ARM_ARCH_7A__ && !DHD_USE_COHERENT_MEM_FOR_RING */
#endif // endif

#include <linux/random.h>

#include <osl.h>
#include <bcmutils.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <pcicfg.h>
#include <pcie_core.h>

#ifdef BCM_SECURE_DMA
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <linux/skbuff.h>
#include <stbutils.h>
#include <linux/highmem.h>
#include <linux/dma-mapping.h>
#include <asm/memory.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
#include <linux/dma-contiguous.h>
#endif /* KERNEL_VERSION(4, 9, 89) */
#endif /* BCM_SECURE_DMA */

#include <linux/fs.h>

#if (defined(BCM47XX_CA9) || defined(STB))
#include <linux/spinlock.h>
extern spinlock_t l2x0_reg_lock;
#endif /* BCM47XX_CA9 || STB */

#if defined(BCM_BLOG) && defined(BCM_ROUTER_DHD)
#include <dhd_blog.h>
#endif // endif

#ifdef BCM_OBJECT_TRACE
#include <bcmutils.h>
#endif /* BCM_OBJECT_TRACE */
#include "linux_osl_priv.h"

#define PCI_CFG_RETRY		10

#define DUMPBUFSZ 1024

#if defined(BCA_HNDROUTER)
#include <board.h>
#endif // endif

#ifdef BCM_SECURE_DMA
static void * osl_secdma_ioremap(osl_t *osh, struct page *page, size_t size,
	bool iscache, bool isdecr);
static void osl_secdma_iounmap(osl_t *osh, void *contig_base_va, size_t size);
static int osl_secdma_init_elem_mem_block(osl_t *osh, size_t mbsize, int max,
	sec_mem_elem_t **list);
static void osl_secdma_deinit_elem_mem_block(osl_t *osh, size_t mbsize, int max,
	void *sec_list_base);
static sec_mem_elem_t * osl_secdma_alloc_mem_elem(osl_t *osh, void *va, uint size,
	int direction, struct sec_cma_info *ptr_cma_info, uint offset, uint buftype);
static void osl_secdma_free_mem_elem(osl_t *osh, sec_mem_elem_t *sec_mem_elem);
static void *osl_secdma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits,
	dmaaddr_t *pap);
static void osl_secdma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa);
static void osl_secdma_allocator_cleanup(osl_t *osh);
#endif /* BCM_SECURE_DMA */

/* PCMCIA attribute space access macros */
#if defined(CONFIG_PCMCIA) || defined(CONFIG_PCMCIA_MODULE)
struct pcmcia_dev {
	dev_link_t link;	/* PCMCIA device pointer */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
	dev_node_t node;	/* PCMCIA node structure */
#endif // endif
	void *base;		/* Mapped attribute memory window */
	size_t size;		/* Size of window */
	void *drv;		/* Driver data */
};
#endif /* defined(CONFIG_PCMCIA) || defined(CONFIG_PCMCIA_MODULE) */

uint32 g_assert_type = 0; /* By Default Kernel Panic */

module_param(g_assert_type, int, 0);
#ifdef	BCM_SECURE_DMA
#define	SECDMA_MODULE_PARAMS	0
#define	SECDMA_EXT_FILE	1
unsigned long secdma_addr = 0;
unsigned long secdma_addr2 = 0;
u32 secdma_size = 0;
u32 secdma_size2 = 0;
module_param(secdma_addr, ulong, 0);
module_param(secdma_size, int, 0);
module_param(secdma_addr2, ulong, 0);
module_param(secdma_size2, int, 0);
static int secdma_found = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
static int global_cma = 0;
#endif /* KERNEL_VERSION(4, 9, 89) */
#endif /* BCM_SECURE_DMA */

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

/* When an new error code is added to bcmutils.h, add os
 * specific error translation here as well
 */
/* check if BCME_LAST changed since the last time this function was updated */
#if BCME_LAST != -68
#error "You need to add a OS error translation in the linuxbcmerrormap \
	for new error code defined in bcmutils.h"
#endif // endif
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

#ifdef SHARED_OSL_CMN
osl_t *
osl_attach(void *pdev, uint bustype, bool pkttag, void **osl_cmn)
{
#else
osl_t *
osl_attach(void *pdev, uint bustype, bool pkttag)
{
	void **osl_cmn = NULL;
#endif /* SHARED_OSL_CMN */
	osl_t *osh;
	gfp_t flags;
#ifdef BCM_SECURE_DMA
	u32 secdma_memsize;
#endif /* BCM_SECURE_DMA */
#if defined(STB) || defined(STBAP)
	struct pci_dev * pcidev;
	pcidev = pdev;
#endif // endif

	flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
	if (!(osh = kmalloc(sizeof(osl_t), flags)))
		return osh;

	ASSERT(osh);

	bzero(osh, sizeof(osl_t));

	/*
	 * RSDB chips will have one osh per wlc. So, two wlc's of RSDB will lead to two osh. Memory
	 * allocator for each wlc make use of separate osh(osl_t) to keep track of the number of
	 * bytes allocated and freed. In some scenarios there is the possibility of memory allocated
	 * for one wlc to get moved to another wlc. For example: moving bsscfg from one wlc to other
	 * wlc or moving the scan results from one wlc to other in case of parallel scanning.
	 * Because of movement of memory from one wlc to other, memory allocator(osh) in each wlc
	 * will not be able to keep track of the exact number of bytes allocated and freed for that
	 * wlc. So, we need to have a common osl structure which will be shared between both wlc's.
	 * XXX RB:19131
	 */
	if (osl_cmn == NULL || *osl_cmn == NULL) {
		if (!(osh->cmn = kmalloc(sizeof(osl_cmn_t), flags))) {
			kfree(osh);
			return NULL;
		}
		bzero(osh->cmn, sizeof(osl_cmn_t));
		if (osl_cmn)
			*osl_cmn = osh->cmn;
		atomic_set(&osh->cmn->malloced, 0);
		osh->cmn->dbgmem_list = NULL;
		spin_lock_init(&(osh->cmn->dbgmem_lock));

		spin_lock_init(&(osh->cmn->pktalloc_lock));

	} else {
		osh->cmn = *osl_cmn;
	}
	atomic_add(1, &osh->cmn->refcount);

	bcm_object_trace_init();

	/* Check that error map has the right number of entries in it */
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(linuxbcmerrormap) - 1));

	osh->failed = 0;
	osh->pdev = pdev;
	OSH_PUB(osh).pkttag = pkttag;
	osh->bustype = bustype;
	osh->magic = OS_HANDLE_MAGIC;

#if !defined(BCMDBUS)
#if (defined(STB) || defined(STBAP))
	if (pci_set_dma_mask(pcidev, DMA_BIT_MASK(64))) {
		if (pci_set_dma_mask(pcidev, DMA_BIT_MASK(32))) {
			printk("%s: DMA set 32bit mask failed.\n", __FUNCTION__);
		}
		printk("%s: DMA set 64bit mask failed.\n", __FUNCTION__);
		return NULL;
	}
#endif /* (STB || STBAP) */
#endif /* BCMDBUS */

#ifdef BCM_SECURE_DMA
	if ((secdma_addr != 0) && (secdma_size != 0)) {
		printk("linux_osl.c: Buffer info passed via module params, using it.\n");
		if (secdma_found == 0) {
			osh->contig_base_alloc = (phys_addr_t)secdma_addr;
			secdma_memsize = secdma_size;
		} else if (secdma_found == 1) {
			osh->contig_base_alloc = (phys_addr_t)secdma_addr2;
			secdma_memsize = secdma_size2;
		} else {
			printk("linux_osl.c secdma: secDMA instances %d \n", secdma_found);
			kfree(osh);
			return NULL;
		}
		osh->contig_base = (phys_addr_t)osh->contig_base_alloc;
		printf("linux_osl.c: secdma_cma_size = 0x%x\n", secdma_memsize);
		printf("linux_osl.c: secdma_cma_addr = 0x%x \n",
			(unsigned int)osh->contig_base_alloc);
		osh->stb_ext_params = SECDMA_MODULE_PARAMS;
	}
	else if (stbpriv_init(osh) == 0) {
		printk("linux_osl.c: stbpriv.txt found. Get buffer info.\n");
		if (secdma_found == 0) {
			osh->contig_base_alloc =
				(phys_addr_t)bcm_strtoul(stbparam_get("secdma_cma_addr"), NULL, 0);
			secdma_memsize = bcm_strtoul(stbparam_get("secdma_cma_size"), NULL, 0);
		} else if (secdma_found == 1) {
			osh->contig_base_alloc =
				(phys_addr_t)bcm_strtoul(stbparam_get("secdma_cma_addr2"), NULL, 0);
			secdma_memsize = bcm_strtoul(stbparam_get("secdma_cma_size2"), NULL, 0);
		} else {
			printk("linux_osl.c secdma: secDMA instances %d \n", secdma_found);
			kfree(osh);
			return NULL;
		}
		osh->contig_base = (phys_addr_t)osh->contig_base_alloc;
		printf("linux_osl.c: secdma_cma_size = 0x%x\n", secdma_memsize);
		printf("linux_osl.c: secdma_cma_addr = 0x%x \n",
			(unsigned int)osh->contig_base_alloc);
		osh->stb_ext_params = SECDMA_EXT_FILE;
	}
	else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
		printk("linux_osl.c: SECDMA supports Global CMA,"
			"Please give 'cma=64M coherent_mem=32M' to kernel command line args.\n");
		global_cma++;
#else
		printk("linux_osl.c: secDMA no longer supports internal buffer allocation.\n");
		kfree(osh);
		return NULL;
#endif /* KERNEL_VERSION(4, 9, 89) */
	}
	secdma_found++;

	/* Setup the DMA descriptors memory */
#ifdef BCM47XX_CA9
	osh->contig_base_coherent_va = osl_secdma_ioremap(osh,
		phys_to_page((u32)osh->contig_base_alloc),
		SECDMA_DESC_MEMBLOCK_SIZE, TRUE, TRUE);
#else

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
	if (global_cma > 0) {
		osh->contig_base_cma_page = dma_alloc_from_contiguous(&pcidev->dev,
			(SECDMA_MEMBLOCK_SIZE >> PAGE_SHIFT), 0);
		if (!osh->contig_base_cma_page) {
			printk("linux_osl.c: osl_attach - dma_alloc_from_contiguous() allocation "
				"failed with no memory \n");
			return NULL;
		}
		osh->contig_base_alloc = page_to_phys(osh->contig_base_cma_page);
	}
#endif /* KERNEL_VERSION(4, 9, 89) */

	osh->contig_base_coherent_va = osl_secdma_ioremap(osh,
		phys_to_page((u32)osh->contig_base_alloc),
		SECDMA_DESC_MEMBLOCK_SIZE, FALSE, TRUE);
#endif /* BCM47XX_CA9 */

	if (osh->contig_base_coherent_va == NULL) {
		goto error;
	}
	osh->contig_base_coherent_pa = osh->contig_base_alloc;
	osh->secdma_coherant_pfree = (uint8 *)osh->contig_base_coherent_va;

	osh->contig_base_alloc += SECDMA_DESC_MEMBLOCK_SIZE;

#ifdef BCMDONGLEHOST
	/* Setup the RX control buffers */
	osh->contig_base_alloc_va = osl_secdma_ioremap(osh,
		phys_to_page((u32)osh->contig_base_alloc),
		(uint)SECDMA_RXCTRL_MEMBLOCK_SIZE, TRUE, FALSE);
	if (osh->contig_base_alloc_va == NULL) {
		goto error;
	}
	osh->contig_base_rxbufctl_va = osh->contig_base_alloc_va;
	osl_secdma_init_elem_mem_block(osh,
		SECDMA_RXCTRL_BUF_SIZE, SECDMA_RXCTRL_BUF_CNT, &osh->sec_list_rxbufctl);
	osh->sec_list_base_rxbufctl = osh->sec_list_rxbufctl;
#endif /* BCMDONGLEHOST */

	/* Setup the DMA TX buffers */
	osh->contig_base_alloc_va = osl_secdma_ioremap(osh,
		phys_to_page((u32)osh->contig_base_alloc), (uint)SECDMA_TXBUF_MEMBLOCK_SIZE,
		TRUE, FALSE);
	if (osh->contig_base_alloc_va == NULL) {
		goto error;
	}
	osh->contig_base_txbuf_va = osh->contig_base_alloc_va;
	/* sec_list_txbuf Pool is common buffers pool
	 * for all buffers requirement, apart from rxbuf and rxctl_buf pools.
	*/
	osl_secdma_init_elem_mem_block(osh,
		SECDMA_DATA_BUF_SIZE, (uint)SECDMA_TXBUF_CNT, &osh->sec_list_txbuf);
	osh->sec_list_base_txbuf = osh->sec_list_txbuf;

	/* Setup the DMA RX buffers */
	osh->contig_base_alloc_va = osl_secdma_ioremap(osh,
		phys_to_page((u32)osh->contig_base_alloc),
		(uint)SECDMA_RXBUF_MEMBLOCK_SIZE, TRUE, FALSE);
	if (osh->contig_base_alloc_va == NULL) {
		goto error;
	}
	osh->contig_base_rxbuf_va = osh->contig_base_alloc_va;
	osl_secdma_init_elem_mem_block(osh,
		SECDMA_DATA_BUF_SIZE, (uint)SECDMA_RXBUF_CNT, &osh->sec_list_rxbuf);
	osh->sec_list_base_rxbuf = osh->sec_list_rxbuf;
	printk("%s No of buffers for RXCTL:%d, TX:%d, RX:%d\n", __FUNCTION__,
		(uint)SECDMA_RXCTRL_BUF_CNT, (uint)SECDMA_TXBUF_CNT, (uint)SECDMA_RXBUF_CNT);

	/* Setup the PHY STS RX buffers */
	osh->contig_base_sts_phyrx_pa = osh->contig_base_alloc;
	osh->contig_base_sts_phyrx_va = osl_secdma_ioremap(osh,
		phys_to_page((u32)osh->contig_base_alloc),
		(uint)SECDMA_STS_PHYRX_MEMBLOCK_SIZE, TRUE, FALSE);

	printk("%s: PHY RX STS va:0x%p, pa:0x%llx \n", __FUNCTION__, osh->contig_base_sts_phyrx_va,
	  osh->contig_base_sts_phyrx_pa);

	if (osh->contig_base_sts_phyrx_va == NULL) {
		printk(" PHY RX STS buffer IOREMAP failed\n");
		goto error;
	}

#endif /* BCM_SECURE_DMA */

	switch (bustype) {
		case PCI_BUS:
		case SI_BUS:
		case PCMCIA_BUS:
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

#ifdef BCM_SKB_FREE_OFFLOAD
	/* Enable SKB_FREE_OFFLOAD to another core */
	OSH_PUB(osh).skb_free_offload = 1;
#endif /* BCM_SKB_FREE_OFFLOAD */

	return osh;
#ifdef BCM_SECURE_DMA
error:
	osl_secdma_allocator_cleanup(osh);
	secdma_found--;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
	if (global_cma) {
		global_cma--;
	}
#endif /* KERNEL_VERSION(4, 9, 89) */
	if (osh->cmn) {
		kfree(osh->cmn);
	}
	kfree(osh);
	return NULL;
#endif /* BCM_SECURE_DMA */
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
#ifdef BCM_SECURE_DMA
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
	struct pci_dev * pcidev;
	pcidev = osh->pdev;
#endif /* KERNEL_VERSION(4, 9, 89) */
#endif /* BCM_SECURE_DMA */

	if (osh == NULL)
		return;

#ifdef BCM_SECURE_DMA
	if (osh->stb_ext_params == SECDMA_EXT_FILE)
		stbpriv_exit(osh);
	osl_secdma_allocator_cleanup(osh);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
	if (global_cma > 0) {
		if (!dma_release_from_contiguous(&pcidev->dev, osh->contig_base_cma_page,
			(SECDMA_MEMBLOCK_SIZE >> PAGE_SHIFT))) {
				dev_err(&pcidev->dev, "contig_base_alloc_cma_page"
					"dma release failed!\n");
		}
	global_cma--;
	}
#endif /* KERNEL_VERSION(4, 9, 89) */
	secdma_found--;
#endif /* BCM_SECURE_DMA */

	bcm_object_trace_deinit();

	ASSERT(osh->magic == OS_HANDLE_MAGIC);
	atomic_sub(1, &osh->cmn->refcount);
	if (atomic_read(&osh->cmn->refcount) == 0) {
		kfree(osh->cmn);
	}
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

#if (defined(BCM47XX_CA9) || defined(STB))
inline bool BCMFASTPATH
#else
bool
#endif /* BCM47XX_CA9 */
osl_is_flag_set(osl_t *osh, uint32 mask)
{
	return (osh->flags & mask);
}

#if defined(mips)
inline void BCMFASTPATH
osl_cache_flush(void *va, uint size)
{
	unsigned long l = ROUNDDN((unsigned long)va, L1_CACHE_BYTES);
	unsigned long e = ROUNDUP((unsigned long)(va)+size, L1_CACHE_BYTES);
	while (l < e)
	{
		flush_dcache_line(l);                         /* Hit_Writeback_Inv_D  */
		l += L1_CACHE_BYTES;                          /* next cache line base */
	}
}

inline void BCMFASTPATH
osl_cache_inv(void *va, uint size)
{
	unsigned long l = ROUNDDN((unsigned long)va, L1_CACHE_BYTES);
	unsigned long e = ROUNDUP((unsigned long)(va)+size, L1_CACHE_BYTES);
	while (l < e)
	{
		invalidate_dcache_line(l);                    /* Hit_Invalidate_D     */
		l += L1_CACHE_BYTES;                          /* next cache line base */
	}
}

inline void BCMFASTPATH
osl_prefetch(const void *ptr)
{
	__asm__ __volatile__(
		".set mips4\npref %0,(%1)\n.set mips0\n" :: "i" (Pref_Load), "r" (ptr));
}

#elif (defined(__ARM_ARCH_7A__) && !defined(DHD_USE_COHERENT_MEM_FOR_RING)) || \
	defined(STB_SOC_WIFI)

inline int BCMFASTPATH
osl_arch_is_coherent(void)
{
#if defined(BCM47XX_CA9) || defined(OSL_CACHE_COHERENT)
	return arch_is_coherent();
#else
	return 0;
#endif // endif
}

inline int BCMFASTPATH
osl_acp_war_enab(void)
{
#ifdef BCM47XX_CA9
	return ACP_WAR_ENAB();
#else
	return 0;
#endif /* BCM47XX_CA9 */
}

inline void BCMFASTPATH
osl_cache_flush(void *va, uint size)
{
#ifdef BCM47XX_CA9
	if (arch_is_coherent() || (ACP_WAR_ENAB() && (virt_to_phys(va) < ACP_WIN_LIMIT)))
		return;
#endif /* BCM47XX_CA9 */

	if (size > 0)
#ifdef STB_SOC_WIFI
		dma_sync_single_for_device(OSH_NULL, virt_to_phys(va), size, DMA_TX);
#else /* STB_SOC_WIFI */
		dma_sync_single_for_device(OSH_NULL, virt_to_dma(OSH_NULL, va), size,
			DMA_TO_DEVICE);
#endif /* STB_SOC_WIFI */
}

inline void BCMFASTPATH
osl_cache_inv(void *va, uint size)
{
#ifdef BCM47XX_CA9
	if (arch_is_coherent() || (ACP_WAR_ENAB() && (virt_to_phys(va) < ACP_WIN_LIMIT)))
		return;
#endif /* BCM47XX_CA9 */

#ifdef STB_SOC_WIFI
	dma_sync_single_for_cpu(OSH_NULL, virt_to_phys(va), size, DMA_RX);
#else /* STB_SOC_WIFI */
	dma_sync_single_for_cpu(OSH_NULL, virt_to_dma(OSH_NULL, va), size, DMA_FROM_DEVICE);
#endif /* STB_SOC_WIFI */
}

inline void BCMFASTPATH
osl_prefetch(const void *ptr)
{
#if !defined(STB_SOC_WIFI)
	__asm__ __volatile__("pld\t%0" :: "o"(*(const char *)ptr) : "cc");
#endif // endif
}

#elif (defined(BCA_HNDROUTER) && defined(__aarch64__))

inline void BCMFASTPATH
osl_cache_flush(void *va, uint size)
{
	if (size > 0)
		dma_sync_single_for_device(OSH_NULL, virt_to_phys(va), size, DMA_TX);
}

inline void BCMFASTPATH
osl_cache_inv(void *va, uint size)
{
	dma_sync_single_for_cpu(OSH_NULL, virt_to_phys(va), size, DMA_RX);
}

inline void osl_prefetch(const void *ptr)
{
	asm volatile("prfm pldl1keep, %a0\n" : : "p" (ptr));
}

#endif /* !mips && !__ARM_ARCH_7A__ && !BCA_HNDROUTER */

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
		/* PR15065: PCI_BAR0_WIN is believed to be the only pci cfg write that can occur
		 * when dma activity is possible
		 */
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

#if (defined(__ARM_ARCH_7A__) && LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)) || \
	defined(BCA_HNDROUTER)
	return pci_domain_nr(((struct pci_dev *)osh->pdev)->bus);
#else
	return ((struct pci_dev *)osh->pdev)->bus->number;
#endif // endif
}

/* return slot # for the pci device pointed by osh->pdev */
uint
osl_pci_slot(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

#if (defined(__ARM_ARCH_7A__) && LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)) || \
	defined(BCA_HNDROUTER)
	return PCI_SLOT(((struct pci_dev *)osh->pdev)->devfn) + 1;
#else
	return PCI_SLOT(((struct pci_dev *)osh->pdev)->devfn);
#endif // endif
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

/* return the pci device pointed by osh->pdev */
struct pci_dev *
osl_pci_device(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return osh->pdev;
}

static void
osl_pcmcia_attr(osl_t *osh, uint offset, char *buf, int size, bool write)
{
} /* osl_pcmcia_attr */

void
osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size)
{
	osl_pcmcia_attr(osh, offset, (char *) buf, size, FALSE);
}

void
osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size)
{
	osl_pcmcia_attr(osh, offset, (char *) buf, size, TRUE);
}

void *
osl_malloc(osl_t *osh, uint size)
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

	flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
	if ((addr = kmalloc(size, flags)) == NULL) {
		if (osh)
			osh->failed++;
		return (NULL);
	}
	if (osh && osh->cmn)
		atomic_add(size, &osh->cmn->malloced);

	return (addr);
} /* osl_malloc */

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

/**
 * @param[in] addr  May be NULL, in which case no action is performed.
 */
void
osl_mfree(osl_t *osh, void *addr, uint size)
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
	kfree(addr);
} /* osl_mfree */

void *
osl_vmalloc(osl_t *osh, uint size)
{
	void *addr;

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);
	if ((addr = vmalloc(size)) == NULL) {
		if (osh)
			osh->failed++;
		return (NULL);
	}
	if (osh && osh->cmn)
		atomic_add(size, &osh->cmn->malloced);

	return (addr);
}

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

void
osl_vmfree(osl_t *osh, void *addr, uint size)
{
	if (osh && osh->cmn) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

		ASSERT(size <= osl_malloced(osh));

		atomic_sub(size, &osh->cmn->malloced);
	}
	vfree(addr);
}

uint
osl_check_memleak(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (atomic_read(&osh->cmn->refcount) == 1)
		return (atomic_read(&osh->cmn->malloced));
	else
		return 0;
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

uint
osl_dma_consistent_align(void)
{
	return (PAGE_SIZE);
}

void*
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap)
{
	void *va;
	uint16 align = (1 << align_bits);
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
		size += align;
	*alloced = size;

#ifndef	BCM_SECURE_DMA
/* By Default enabled DHD_USE_COHERENT_MEM_FOR_RING makes Descriptors memory comes
 * from Global CMA pool using dma_alloc_coherent() call for non-secdma drivers.
 * This will also ensures getting corret address when PCIe address are mapped beyond 4GB
 * range like 7278 STB platforms.For 7278 __virt_to_phys won't work since PCIe address
 * range is mapped beyond 4GB range.
 */
#if defined(STB_SOC_WIFI) || defined(BCA_HNDROUTER) || ((defined(STB) || \
	defined(STBAP)) && !defined(DHD_USE_COHERENT_MEM_FOR_RING))
	va = kmalloc(size, GFP_ATOMIC | __GFP_ZERO);
	if (va)
#if !(defined(STB) || defined(STBAP))
		*pap = (ulong)__virt_to_phys((ulong)va);
#else
	{
		dma_addr_t pap_lin = 0;
		struct pci_dev *hwdev = osh->pdev;
		pap_lin = dma_map_single(&hwdev->dev, va, size, DMA_BIDIRECTIONAL);
#ifdef BCMDMA64OSL
		PHYSADDRLOSET(*pap, pap_lin & 0xffffffff);
		PHYSADDRHISET(*pap, (pap_lin >> 32) & 0xffffffff);
#else
		*pap = (dmaaddr_t)pap_lin;
#endif /* BCMDMA64OSL */
	}
#endif /* STB || STBAP */

#else
	{
		dma_addr_t pap_lin;
		struct pci_dev *hwdev = osh->pdev;
		gfp_t flags;
#ifdef DHD_ALLOC_COHERENT_MEM_FROM_ATOMIC_POOL
		flags = GFP_ATOMIC;
#else
		flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
#endif /* DHD_ALLOC_COHERENT_MEM_FROM_ATOMIC_POOL */
		va = dma_alloc_coherent(&hwdev->dev, size, &pap_lin, flags); // a linux function
#ifdef BCMDMA64OSL
		PHYSADDRLOSET(*pap, pap_lin & 0xffffffff);
		PHYSADDRHISET(*pap, (pap_lin >> 32) & 0xffffffff);
#else
		*pap = (dmaaddr_t)pap_lin;
#endif /* BCMDMA64OSL */
	}
#endif /* STB_SOC_WIFI || BCA_HNDROUTER || ((STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING) */
#else
	va = osl_secdma_alloc_consistent(osh, size, align_bits, pap);
#endif /* BCM_SECURE_DMA */

	return va;
} /* osl_dma_alloc_consistent */

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa)
{
#ifndef BCM_SECURE_DMA
#if ((defined(STB) || defined(STBAP)) && !defined(DHD_USE_COHERENT_MEM_FOR_RING))
struct pci_dev *hwdev = osh->pdev;
#endif /* STB */
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#endif /* BCM_SECURE_DMA */

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

#ifndef BCM_SECURE_DMA
#if defined(STB_SOC_WIFI) || defined(BCA_HNDROUTER) || ((defined(STB) || \
	defined(STBAP)) && !defined(DHD_USE_COHERENT_MEM_FOR_RING))
#if defined(STB) || defined(STBAP)
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pa, paddr);
	dma_unmap_single(&hwdev->dev, paddr, size, DMA_BIDIRECTIONAL);
#else
	dma_unmap_single(&hwdev->dev, pa, size, DMA_BIDIRECTIONAL);
#endif /* BCMDMA64OSL */
#endif /* STB || STBAP */
	kfree(va);
#else
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pa, paddr);
	pci_free_consistent(osh->pdev, size, va, paddr);
#else
	pci_free_consistent(osh->pdev, size, va, (dma_addr_t)pa);
#endif /* BCMDMA64OSL */
#endif /* STB_SOC_WIFI || BCA_HNDROUTER || ((STB || STBAP) && !DHD_USE_COHERENT_MEM_FOR_RING) */
#else
	osl_secdma_free_consistent(osh, va, size, pa);
#endif /* BCM_SECURE_DMA */
} /* osl_dma_free_consistent */

void *
osl_virt_to_phys(void *va)
{
	return (void *)(uintptr)virt_to_phys(va);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
#include <asm/cacheflush.h>
void BCMFASTPATH
osl_dma_flush(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	return;
}
#endif /* LINUX_VERSION_CODE >= 2.6.36 */

void BCMFASTPATH
osl_dma_sync(osl_t *osh, dmaaddr_t pa, uint size, int direction)
{
	struct pci_dev * pdev;
	dma_addr_t paddr;

	pdev = osh->pdev;
	BCM_REFERENCE(pdev);
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
		dma_sync_single_for_device(OSH_NULL, pa, size, DMA_TX);
	} else if (direction == DMA_RX) { /* from device */
		dma_sync_single_for_cpu(OSH_NULL, pa, size, DMA_RX);
	}
#endif /* BCMDMA64OSL */
	return;
}

dmaaddr_t BCMFASTPATH
osl_dma_map(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	int dir;
	dmaaddr_t ret_addr;
	dma_addr_t map_addr;
	int ret;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	dir = (direction == DMA_TX)? PCI_DMA_TODEVICE: PCI_DMA_FROMDEVICE;

#if defined(BCM47XX_CA9) && defined(BCMDMASGLISTOSL)
	if (dmah != NULL) {
		int32 nsegs, i, totsegs = 0, totlen = 0;
		struct scatterlist *sg, _sg[MAX_DMA_SEGS * 2];
		struct scatterlist *s;
		struct sk_buff *skb;

		for (skb = (struct sk_buff *)p; skb != NULL; skb = PKTNEXT(osh, skb)) {
			sg = &_sg[totsegs];
			if (skb_is_nonlinear(skb)) {
				nsegs = skb_to_sgvec(skb, sg, 0, PKTLEN(osh, skb));
				ASSERT((nsegs > 0) && (totsegs + nsegs <= MAX_DMA_SEGS));
				if (osl_is_flag_set(osh, OSL_ACP_COHERENCE)) {
					for_each_sg(sg, s, nsegs, i) {
						if (sg_phys(s) >= ACP_WIN_LIMIT) {
							dma_map_page(
							&((struct pci_dev *)osh->pdev)->dev,
							sg_page(s), s->offset, s->length, dir);
						}
					}
				} else
					pci_map_sg(osh->pdev, sg, nsegs, dir);
			} else {
				nsegs = 1;
				ASSERT(totsegs + nsegs <= MAX_DMA_SEGS);
				sg->page_link = 0;
				sg_set_buf(sg, PKTDATA(osh, skb), PKTLEN(osh, skb));

				/* not do cache ops */
				if (osl_is_flag_set(osh, OSL_ACP_COHERENCE) &&
					(virt_to_phys(PKTDATA(osh, skb)) < ACP_WIN_LIMIT))
					goto no_cache_ops;

#ifdef CTFMAP
				/* Map size bytes (not skb->len) for ctf bufs */
				pci_map_single(osh->pdev, PKTDATA(osh, skb),
				    PKTISCTF(osh, skb) ? CTFMAPSZ : PKTLEN(osh, skb), dir);
#else
				pci_map_single(osh->pdev, PKTDATA(osh, skb), PKTLEN(osh, skb), dir);

#endif // endif
			}
no_cache_ops:
			totsegs += nsegs;
			totlen += PKTLEN(osh, skb);
		}

		dmah->nsegs = totsegs;
		dmah->origsize = totlen;
		for (i = 0, sg = _sg; i < totsegs; i++, sg++) {
			dmah->segs[i].addr = sg_phys(sg);
			dmah->segs[i].length = sg->length;
		}
		return dmah->segs[0].addr;
	}
#endif /* BCM47XX_CA9 && BCMDMASGLISTOSL */

#if defined(BCM47XX_CA9) || defined(OSL_CACHE_COHERENT)
	if (osl_is_flag_set(osh, OSL_ACP_COHERENCE)) {
		uint pa = virt_to_phys(va);
		if (pa < ACP_WIN_LIMIT)
			return (pa);
	}
#endif /* BCM47XX_CA9 */

#ifdef STB_SOC_WIFI
#if (__LINUX_ARM_ARCH__ == 8)
	/* need to flush or invalidate the cache here */
	if (dir == DMA_TX) { /* to device */
		osl_cache_flush(va, size);
	} else if (dir == DMA_RX) { /* from device */
		osl_cache_inv(va, size);
	} else { /* both */
		osl_cache_flush(va, size);
		osl_cache_inv(va, size);
	}
	return virt_to_phys(va);
#else /* (__LINUX_ARM_ARCH__ == 8) */
	return dma_map_single(osh->pdev, va, size, dir);
#endif /* (__LINUX_ARM_ARCH__ == 8) */
#else /* ! STB_SOC_WIFI */
	map_addr = pci_map_single(osh->pdev, va, size, dir);
#endif	/* ! STB_SOC_WIFI */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	ret = pci_dma_mapping_error(osh->pdev, map_addr);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 5))
	ret = pci_dma_mapping_error(map_addr);
#else
	ret = 0;
#endif // endif
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
	int dir;
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

#if defined(BCM47XX_CA9) || defined(OSL_CACHE_COHERENT)
	if (osl_is_flag_set(osh, OSL_ACP_COHERENCE) && (pa < ACP_WIN_LIMIT))
		return;
#endif /* BCM47XX_CA9 */

	dir = (direction == DMA_TX)? PCI_DMA_TODEVICE: PCI_DMA_FROMDEVICE;
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pa, paddr);
	pci_unmap_single(osh->pdev, paddr, size, dir);
#else /* BCMDMA64OSL */

#ifdef STB_SOC_WIFI
#if (__LINUX_ARM_ARCH__ == 8)
	if (dir == DMA_TX) { /* to device */
		dma_sync_single_for_device(OSH_NULL, pa, size, DMA_TX);
	} else if (dir == DMA_RX) { /* from device */
		dma_sync_single_for_cpu(OSH_NULL, pa, size, DMA_RX);
	} else { /* both */
		dma_sync_single_for_device(OSH_NULL, pa, size, DMA_TX);
		dma_sync_single_for_cpu(OSH_NULL, pa, size, DMA_RX);
	}
#else /* (__LINUX_ARM_ARCH__ == 8) */
	dma_unmap_single(osh->pdev, (uintptr)pa, size, dir);
#endif /* (__LINUX_ARM_ARCH__ == 8) */
#else /* STB_SOC_WIFI */
	pci_unmap_single(osh->pdev, (uint32)pa, size, dir);
#endif /* STB_SOC_WIFI */

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

void
osl_delay(uint usec)
{
	uint d;

	while (usec > 0) {
		d = MIN(usec, 1000);
		udelay(d);
		usec -= d;
	}
}

void
osl_sleep(uint ms)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	if (ms < 20)
		usleep_range(ms*1000, ms*1000 + 1000);
	else
#endif // endif
	msleep(ms);
}

uint64
osl_sysuptime_us(void)
{
	uint64 usec;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct timeval tv;

	do_gettimeofday(&tv);

	/* tv_usec content is fraction of a second */
	usec = (uint64)tv.tv_sec * 1000000ul + tv.tv_usec;
#else
	struct timespec64 ts64;

	ktime_get_real_ts64(&ts64);
	usec = ktime_to_ns(timespec64_to_ktime(ts64)) / 1000;
#endif // endif
	return usec;
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

uint32
osl_sysuptime(void)
{
	return ((uint32)jiffies * (1000 / HZ));
}

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

/* Linux Kernel: File Operations: end */

#if (defined(BCM47XX_CA9) || (defined(STB) && defined(__arm__)))
inline void osl_pcie_rreg(osl_t *osh, ulong addr, volatile void *v, uint size)
{
	unsigned long flags = 0;
	int pci_access = 0;
#if defined(BCM_GMAC3)
	const int acp_war_enab = 1;
#else  /* !BCM_GMAC3 */
	int acp_war_enab = ACP_WAR_ENAB();
#endif /* !BCM_GMAC3 */

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

	if (pci_access && acp_war_enab)
		spin_unlock_irqrestore(&l2x0_reg_lock, flags);
} /* osl_pcie_rreg */
#endif /* BCM47XX_CA9 || (STB && __arm__) */

#if defined(BCM_BACKPLANE_TIMEOUT)
inline void osl_bpt_rreg(osl_t *osh, ulong addr, volatile void *v, uint size)
{
	bool poll_timeout = FALSE;
	static int in_si_clear = FALSE;
#if defined(BCM47XX_CA9)
	unsigned long flags = 0;
	int pci_access = 0;

	if (osh && BUSTYPE(osh->bustype) == PCI_BUS)
		pci_access = 1;

	if (pci_access && ACP_WAR_ENAB())
		spin_lock_irqsave(&l2x0_reg_lock, flags);
#endif /* BCM47XX_CA9 */

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

#if defined(BCM47XX_CA9)
	if (pci_access && ACP_WAR_ENAB())
		spin_unlock_irqrestore(&l2x0_reg_lock, flags);
#endif /* BCM47XX_CA9 */

	if (osh && osh->sih && (in_si_clear == FALSE) && poll_timeout && osh->bpt_cb) {
		in_si_clear = TRUE;
		osh->bpt_cb((void *)osh->sih, (void *)addr);
		in_si_clear = FALSE;
	}
} /* osl_bpt_rreg */
#endif /* BCM_BACKPLANE_TIMEOUT */

#ifdef BCM_SECURE_DMA
static void *
osl_secdma_ioremap(osl_t *osh, struct page *page, size_t size, bool iscache, bool isdecr)
{

	struct page **map;
	int order, i;
	void *addr = NULL;

	size = PAGE_ALIGN(size);
	order = get_order(size);
	map = kmalloc(sizeof(struct page *) << order, GFP_ATOMIC);

	if (map == NULL)
		return NULL;

	for (i = 0; i < (size >> PAGE_SHIFT); i++)
		map[i] = page + i;

	if (iscache) {
		addr = vmap(map, size >> PAGE_SHIFT, VM_MAP, PAGE_KERNEL);
		if (isdecr) {
			osh->contig_delta_va_pa = ((uint8 *)addr - page_to_phys(page));
		}
	}
	else {
		addr = vmap(map, size >> PAGE_SHIFT, VM_MAP,
			(__pgprot(pgprot_val(pgprot_noncached(PAGE_KERNEL)) |
				pgprot_val(pgprot_writecombine(PAGE_KERNEL)))));
		if (isdecr) {
			osh->contig_delta_va_pa = ((uint8 *)addr - page_to_phys(page));
		}
	}
	kfree(map);
	return (void *)addr;
} /* osl_secdma_ioremap */

static void
osl_secdma_iounmap(osl_t *osh, void *contig_base_va, size_t size)
{
	if (contig_base_va) {
		vunmap(contig_base_va);
	}
}

static int
osl_secdma_init_elem_mem_block(osl_t *osh, size_t mbsize, int max, sec_mem_elem_t **list)
{
	int i;
	int ret = BCME_OK;
	sec_mem_elem_t *sec_mem_elem;

	if ((sec_mem_elem = kmalloc(sizeof(sec_mem_elem_t)*(max), GFP_ATOMIC)) != NULL) {

		*list = sec_mem_elem;
		bzero(sec_mem_elem, sizeof(sec_mem_elem_t)*(max));
		for (i = 0; i < max-1; i++) {
			sec_mem_elem->next = (sec_mem_elem + 1);
			sec_mem_elem->size = mbsize;
			sec_mem_elem->pa = osh->contig_base_alloc;
			sec_mem_elem->va = osh->contig_base_alloc_va;

			sec_mem_elem->pa_page = phys_to_page(sec_mem_elem->pa);
			osh->contig_base_alloc += mbsize;
			osh->contig_base_alloc_va = ((uint8 *)osh->contig_base_alloc_va +  mbsize);

			sec_mem_elem = sec_mem_elem + 1;
		}
		sec_mem_elem->next = NULL;
		sec_mem_elem->size = mbsize;
		sec_mem_elem->pa = osh->contig_base_alloc;
		sec_mem_elem->va = osh->contig_base_alloc_va;

		sec_mem_elem->pa_page = phys_to_page(sec_mem_elem->pa);
		osh->contig_base_alloc += mbsize;
		osh->contig_base_alloc_va = ((uint8 *)osh->contig_base_alloc_va +  mbsize);

	} else {
		printf("%s sec mem elem kmalloc failed\n", __FUNCTION__);
		ret = BCME_ERROR;
	}
	return ret;
} /* osl_secdma_init_elem_mem_block */

static void
osl_secdma_deinit_elem_mem_block(osl_t *osh, size_t mbsize, int max, void *sec_list_base)
{
	if (sec_list_base)
		kfree(sec_list_base);
}
static void
osl_secdma_allocator_cleanup(osl_t *osh)
{
	osl_secdma_deinit_elem_mem_block(osh,
		SECDMA_DATA_BUF_SIZE, (uint)SECDMA_TXBUF_CNT, osh->sec_list_txbuf);
	osl_secdma_deinit_elem_mem_block(osh,
		SECDMA_DATA_BUF_SIZE, (uint)SECDMA_RXBUF_CNT, osh->sec_list_base_rxbuf);

	osl_secdma_iounmap(osh, osh->contig_base_coherent_va, SECDMA_DESC_MEMBLOCK_SIZE);
	osl_secdma_iounmap(osh, osh->contig_base_txbuf_va, (uint)SECDMA_TXBUF_MEMBLOCK_SIZE);
	osl_secdma_iounmap(osh, osh->contig_base_rxbuf_va, (uint)SECDMA_RXBUF_MEMBLOCK_SIZE);
	osl_secdma_iounmap(osh,
		osh->contig_base_sts_phyrx_va, (uint)SECDMA_STS_PHYRX_MEMBLOCK_SIZE);
#ifdef BCMDONGLEHOST
	osl_secdma_deinit_elem_mem_block(osh,
		SECDMA_RXCTRL_BUF_SIZE, SECDMA_RXCTRL_BUF_CNT, osh->sec_list_base_rxbufctl);
	osl_secdma_iounmap(osh, osh->contig_base_rxbufctl_va, SECDMA_RXCTRL_MEMBLOCK_SIZE);
#endif /* BCMDONGLEHOST */
}

static sec_mem_elem_t * BCMFASTPATH
osl_secdma_alloc_mem_elem(osl_t *osh, void *va, uint size, int direction,
	struct sec_cma_info *ptr_cma_info, uint offset, uint buftype)
{
	sec_mem_elem_t *sec_mem_elem = NULL;

	if (buftype == SECDMA_RXBUF_POST) {
		if (osh->sec_list_rxbuf) {
			sec_mem_elem = osh->sec_list_rxbuf;
			sec_mem_elem->buftype = SECDMA_RXBUF_POST;
			osh->sec_list_rxbuf = sec_mem_elem->next;

			sec_mem_elem->next = NULL;

			if (ptr_cma_info->sec_alloc_list_tail) {
				ptr_cma_info->sec_alloc_list_tail->next = sec_mem_elem;
				ptr_cma_info->sec_alloc_list_tail = sec_mem_elem;
			} else {
				/* First allocation: If tail is NULL,
				 * sec_alloc_list MUST also be NULL
				 */
				ASSERT(ptr_cma_info->sec_alloc_list == NULL);
				ptr_cma_info->sec_alloc_list = sec_mem_elem;
				ptr_cma_info->sec_alloc_list_tail = sec_mem_elem;
			}
		}
	}
#ifdef BCMDONGLEHOST
	else if (buftype == SECDMA_RXCTR_BUF_POST) {
		if (osh->sec_list_rxbufctl) {
			sec_mem_elem = osh->sec_list_rxbufctl;
			sec_mem_elem->buftype = SECDMA_RXCTR_BUF_POST;
			osh->sec_list_rxbufctl = sec_mem_elem->next;

			sec_mem_elem->next = NULL;

			if (ptr_cma_info->sec_alloc_list_tail) {
				ptr_cma_info->sec_alloc_list_tail->next = sec_mem_elem;
				ptr_cma_info->sec_alloc_list_tail = sec_mem_elem;
			}
			else {
				/* First allocation: If tail is NULL,
				 * sec_alloc_list MUST also be NULL
				 */
				ASSERT(ptr_cma_info->sec_alloc_list == NULL);
				ptr_cma_info->sec_alloc_list = sec_mem_elem;
				ptr_cma_info->sec_alloc_list_tail = sec_mem_elem;
			}
		}
	}
#endif /* BCMDONGLEHOST */
	else {
		/* sec_list_txbuf Pool is common 4K buffers pool
		 * for all buffers requirement, apart from rxbuf and rxctl_buf pools.
		 */
		if (osh->sec_list_txbuf) {
			sec_mem_elem = osh->sec_list_txbuf;
			sec_mem_elem->buftype = SECDMA_TXBUF_POST;
			osh->sec_list_txbuf = sec_mem_elem->next;

			sec_mem_elem->next = NULL;
			if (ptr_cma_info->sec_alloc_list_tail) {
				ptr_cma_info->sec_alloc_list_tail->next = sec_mem_elem;
				ptr_cma_info->sec_alloc_list_tail = sec_mem_elem;
			}
			else {
				/* First allocation: If tail is NULL,
				 * sec_alloc_list MUST also be NULL
				 */
				ASSERT(ptr_cma_info->sec_alloc_list == NULL);
				ptr_cma_info->sec_alloc_list = sec_mem_elem;
				ptr_cma_info->sec_alloc_list_tail = sec_mem_elem;
			}
		}
	}

	return sec_mem_elem;
} /* osl_secdma_alloc_mem_elem */

static void BCMFASTPATH
osl_secdma_free_mem_elem(osl_t *osh, sec_mem_elem_t *sec_mem_elem)
{
	sec_mem_elem->dma_handle = 0x0;
	sec_mem_elem->pkt = NULL;

	if (sec_mem_elem->buftype == SECDMA_RXBUF_POST) {
		sec_mem_elem->next = osh->sec_list_rxbuf;
		osh->sec_list_rxbuf = sec_mem_elem;
	}
#ifdef BCMDONGLEHOST
	else if (sec_mem_elem->buftype == SECDMA_RXCTR_BUF_POST) {
		sec_mem_elem->next = osh->sec_list_rxbufctl;
		osh->sec_list_rxbufctl = sec_mem_elem;
	}
#endif /* BCMDONGLEHOST */
	else if (sec_mem_elem->buftype == SECDMA_TXBUF_POST) {
		sec_mem_elem->next = osh->sec_list_txbuf;
		osh->sec_list_txbuf = sec_mem_elem;
		}
	else {
		printk("%s: Cannot identify this buffer\n", __FUNCTION__);
	}

}

static sec_mem_elem_t * BCMFASTPATH
osl_secdma_find_rem_elem(osl_t *osh, struct sec_cma_info *ptr_cma_info, dma_addr_t dma_handle)
{
	sec_mem_elem_t *sec_mem_elem = ptr_cma_info->sec_alloc_list;
	sec_mem_elem_t *sec_prv_elem = ptr_cma_info->sec_alloc_list;

	if (!sec_mem_elem) {
		printk("osl_secdma_find_rem_elem ptr_cma_info->sec_alloc_list is NULL \n");
		return NULL;
	}

	if (sec_mem_elem->dma_handle == dma_handle) {

		ptr_cma_info->sec_alloc_list = sec_mem_elem->next;

		if (sec_mem_elem == ptr_cma_info->sec_alloc_list_tail) {
			ptr_cma_info->sec_alloc_list_tail = NULL;
			ASSERT(ptr_cma_info->sec_alloc_list == NULL);
		}

		return sec_mem_elem;
	}
	sec_mem_elem = sec_mem_elem->next;

	while (sec_mem_elem != NULL) {

		if (sec_mem_elem->dma_handle == dma_handle) {

			sec_prv_elem->next = sec_mem_elem->next;
			if (sec_mem_elem == ptr_cma_info->sec_alloc_list_tail)
				ptr_cma_info->sec_alloc_list_tail = sec_prv_elem;

			return sec_mem_elem;
		}
		sec_prv_elem = sec_mem_elem;
		sec_mem_elem = sec_mem_elem->next;
	}
	return NULL;
}

static sec_mem_elem_t *
osl_secdma_rem_first_elem(osl_t *osh, struct sec_cma_info *ptr_cma_info)
{
	sec_mem_elem_t *sec_mem_elem = ptr_cma_info->sec_alloc_list;

	if (sec_mem_elem) {

		ptr_cma_info->sec_alloc_list = sec_mem_elem->next;

		if (ptr_cma_info->sec_alloc_list == NULL)
			ptr_cma_info->sec_alloc_list_tail = NULL;

		return sec_mem_elem;

	} else
		return NULL;
}

static void * BCMFASTPATH
osl_secdma_last_elem(osl_t *osh, struct sec_cma_info *ptr_cma_info)
{
	return ptr_cma_info->sec_alloc_list_tail;
}

dma_addr_t BCMFASTPATH
osl_secdma_map_txmeta(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *dmah, void *ptr_cma_info)
{
	sec_mem_elem_t *sec_mem_elem;
	struct page *pa_page;
	uint loffset;
	void *vaorig = ((uint8 *)va + size);
	dma_addr_t dma_handle = 0x0;
	struct pci_dev * pdev;
	pdev = osh->pdev;

	/* packet will be the one added with osl_secdma_map() just before this call */

	sec_mem_elem = osl_secdma_last_elem(osh, ptr_cma_info);

	if (sec_mem_elem && sec_mem_elem->pkt == vaorig) {

		pa_page = phys_to_page(sec_mem_elem->pa);
		loffset = sec_mem_elem->pa -(sec_mem_elem->pa & ~(PAGE_SIZE-1));

		dma_handle = dma_map_page(&pdev->dev, pa_page, loffset,
			size, (direction == DMA_TX ? DMA_TO_DEVICE:DMA_FROM_DEVICE));

	} else {
		printf("%s: error orig va not found va = 0x%p \n",
			__FUNCTION__, vaorig);
	}
	return dma_handle;
}

bool
osl_secdma_buffs_is_avail(osl_t *osh)
{
	return (osh->sec_list_txbuf) ? TRUE : FALSE;
}

bool
osl_secdma_rx_buffs_is_avail(osl_t *osh)
{
	return (osh->sec_list_rxbuf) ? TRUE : FALSE;
}
#ifdef BCMDONGLEHOST
bool
osl_secdma_rxctl_buffs_is_avail(osl_t *osh)
{
	return (osh->sec_list_rxbufctl) ? TRUE : FALSE;
}
#endif /* BCMDONGLEHOST */

dma_addr_t BCMFASTPATH
osl_secdma_map(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *dmah, void *ptr_cma_info, uint offset, uint buftype)
{
	sec_mem_elem_t *sec_mem_elem;
	struct page *pa_page;
	void *pa_kmap_va = NULL;
	uint buflen = 0;
	dma_addr_t dma_handle = 0x0;
	uint loffset;
	struct pci_dev * pdev;
#ifdef NOT_YET
	int *fragva;
	struct sk_buff *skb;
	int i = 0;
#endif /* NOT_YET */

	ASSERT((direction == DMA_RX) || (direction == DMA_TX));
	pdev = osh->pdev;
	sec_mem_elem = osl_secdma_alloc_mem_elem(osh,
		va, size, direction, ptr_cma_info, offset, buftype);

	if (sec_mem_elem == NULL) {
		return 0;
	}

	sec_mem_elem->pkt = va;
	sec_mem_elem->direction = direction;
	pa_page = sec_mem_elem->pa_page;

	loffset = sec_mem_elem->pa -(sec_mem_elem->pa & ~(PAGE_SIZE-1));

	pa_kmap_va = sec_mem_elem->va;
	pa_kmap_va = ((uint8 *)pa_kmap_va + offset);
	buflen = size;

	if (direction == DMA_TX) {
		memcpy((uint8*)pa_kmap_va+offset, va, size);

#ifdef NOT_YET
		if (p == NULL) {

			memcpy(pa_kmap_va+offset, va, size);
			buflen = size;
		}
		else {
			for (skb = (struct sk_buff *)p; skb != NULL; skb = PKTNEXT(osh, skb)) {
				if (skb_is_nonlinear(skb)) {

					for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
						skb_frag_t *f = &skb_shinfo(skb)->frags[i];
						fragva = kmap_atomic(skb_frag_page(f));
						memcpy((pa_kmap_va+offset+buflen),
						(fragva + f->page_offset), skb_frag_size(f));
						kunmap_atomic(fragva);
						buflen += skb_frag_size(f);
					}
				}
				else {
					memcpy((pa_kmap_va+offset+buflen), skb->data, skb->len);
					buflen += skb->len;
				}
			}

		}
#endif /* NOT_YET */
		if (dmah) {
			dmah->nsegs = 1;
			dmah->origsize = buflen;
		}
	}
	else
	{
		if ((p != NULL) && (dmah != NULL)) {
			dmah->nsegs = 1;
			dmah->origsize = buflen;
		}
	}
#ifdef WLC_HIGH

	if (direction == DMA_RX) {
		*(uint32 *)(pa_kmap_va) = 0x0;
		flush_kernel_vmap_range(pa_kmap_va, sizeof(int));
	}
#endif // endif
	dma_handle = dma_map_page(&pdev->dev, pa_page, loffset+offset, buflen,
		(direction == DMA_TX ? DMA_TO_DEVICE:DMA_FROM_DEVICE));
	if (dmah) {
#ifdef BCMDMA64OSL
		PHYSADDRLOSET(dmah->segs[0].addr, dma_handle & 0xffffffff);
		PHYSADDRHISET(dmah->segs[0].addr, (dma_handle >> 32) & 0xffffffff);
#else
		dmah->segs[0].addr = (dmaaddr_t)dma_handle;
#endif /* BCMDMA64OSL */
		dmah->segs[0].length = buflen;
	}
	sec_mem_elem->dma_handle = dma_handle;
	/* kunmap_atomic(pa_kmap_va-loffset); */
	return dma_handle;
} /* osl_secdma_map */

dma_addr_t BCMFASTPATH
osl_secdma_dd_map(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *map)
{

	struct page *pa_page;
	phys_addr_t pa;
	dma_addr_t dma_handle = 0x0;
	uint loffset;
	struct pci_dev * pdev;
	pdev = osh->pdev;

	pa = ((uint8 *)va - (uint8 *)osh->contig_delta_va_pa);
	pa_page = phys_to_page(pa);
	loffset = pa -(pa & ~(PAGE_SIZE-1));

	dma_handle = dma_map_page(&pdev->dev, pa_page, loffset, size,
		(direction == DMA_TX ? DMA_TO_DEVICE:DMA_FROM_DEVICE));

	return dma_handle;
}

void BCMFASTPATH
osl_secdma_unmap(osl_t *osh, dma_addr_t dma_handle, uint size, int direction,
void *p, hnddma_seg_map_t *map,	void *ptr_cma_info, uint offset)
{
	sec_mem_elem_t *sec_mem_elem;
#ifdef NOT_YET
	struct page *pa_page;
#endif // endif
	void *pa_kmap_va = NULL;
	uint buflen = 0;
	void *va;
	int read_count = 0;
	struct pci_dev * pdev;
	pdev = osh->pdev;
	BCM_REFERENCE(buflen);
	BCM_REFERENCE(read_count);

	sec_mem_elem = osl_secdma_find_rem_elem(osh, ptr_cma_info, dma_handle);
	ASSERT(sec_mem_elem);

	if (sec_mem_elem == NULL)
		return;

	va = sec_mem_elem->pkt;
	va = (uint8 *)va - offset;

#ifdef NOT_YET
	pa_page = sec_mem_elem->pa_page;
#endif // endif

	if (direction == DMA_RX) {

		if (p == NULL) {

			/* pa_kmap_va = kmap_atomic(pa_page);
			* pa_kmap_va += loffset;
			*/
			pa_kmap_va = sec_mem_elem->va;
			dma_unmap_page(&pdev->dev, sec_mem_elem->dma_handle, size, DMA_FROM_DEVICE);
			memcpy(va, pa_kmap_va, size);
			/* kunmap_atomic(pa_kmap_va); */
		}
#ifdef NOT_YET
		else {
			buflen = 0;
			for (skb = (struct sk_buff *)p; (buflen < size) &&
				(skb != NULL); skb = skb->next) {
				if (skb_is_nonlinear(skb)) {
					pa_kmap_va = kmap_atomic(pa_page);
					for (i = 0; (buflen < size) &&
						(i < skb_shinfo(skb)->nr_frags); i++) {
						skb_frag_t *f = &skb_shinfo(skb)->frags[i];
						cpuaddr = kmap_atomic(skb_frag_page(f));
						memcpy((cpuaddr + f->page_offset),
							(pa_kmap_va+buflen), skb_frag_size(f));
						kunmap_atomic(cpuaddr);
						buflen += skb_frag_size(f);
					}
						kunmap_atomic(pa_kmap_va);
				}
				else {
					pa_kmap_va = kmap_atomic(pa_page);
					memcpy(skb->data, (pa_kmap_va + buflen), skb->len);
					kunmap_atomic(pa_kmap_va);
					buflen += skb->len;
				}

			}

		}
#endif /* NOT YET */
	} else {
		dma_unmap_page(&pdev->dev, sec_mem_elem->dma_handle, size+offset, DMA_TO_DEVICE);
	}

	osl_secdma_free_mem_elem(osh, sec_mem_elem);
} /* osl_secdma_unmap */

void
osl_secdma_unmap_all(osl_t *osh, void *ptr_cma_info)
{

	sec_mem_elem_t *sec_mem_elem;
	struct pci_dev * pdev;
	pdev = osh->pdev;

	sec_mem_elem = osl_secdma_rem_first_elem(osh, ptr_cma_info);

	while (sec_mem_elem != NULL) {

		dma_unmap_page(&pdev->dev, sec_mem_elem->dma_handle,
			sec_mem_elem->size,
			sec_mem_elem->direction == DMA_TX ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		osl_secdma_free_mem_elem(osh, sec_mem_elem);

		sec_mem_elem = osl_secdma_rem_first_elem(osh, ptr_cma_info);
	}
}

static void *
osl_secdma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, dmaaddr_t *pap)
{
	void *temp_va = NULL;
	dma_addr_t temp_pa = 0;
	dma_addr_t dma_handle = 0;
	struct pci_dev * pdev;
	uint total_alloc;
	uint offset, size_align;

	pdev = osh->pdev;
	ASSERT(size);
	/* Align size, allocated addr will also be aligned */
	size_align = ROUNDUP(size, SECDMA_DESC_ADDR_ALIGN);
	total_alloc = (uint)(osh->secdma_coherant_pfree - (uint8 *)osh->contig_base_coherent_va);
	if ((total_alloc + size_align) < SECDMA_DESC_MEMBLOCK_SIZE) {
		temp_va = (void *)((uint8 *)osh->contig_base_coherent_va + total_alloc);
		temp_pa = (dma_addr_t)(osh->contig_base_coherent_pa + total_alloc);
		osh->secdma_coherant_pfree += size_align;
		}
	else {
		printk("%s:Coherent mem allocation FAILED for the requested size = %d\n",
			__FUNCTION__, size_align);
		return NULL;
		}
	/* Confirm that PAGE_SIZE is 2^N */
	ASSERT((PAGE_SIZE & (PAGE_SIZE - 1)) == 0);
	offset = temp_pa -(temp_pa & ~(PAGE_SIZE-1));
	dma_handle = dma_map_page(&pdev->dev, phys_to_page(temp_pa), offset, size_align,
		DMA_BIDIRECTIONAL);
	/* printk("%s: va:0x%p, pa:0x%llx, dma_handle:0x%llx, offset:%d\n", __FUNCTION__, temp_va,
	 * temp_pa, dma_handle, (uint)(temp_pa % PAGE_SIZE));
	 */

#ifdef BCMDMA64OSL
	PHYSADDRLOSET(*pap, dma_handle & 0xffffffff);
	PHYSADDRHISET(*pap, (dma_handle >> 32) & 0xffffffff);
#else
	*pap = dma_handle;
#endif // endif
	return temp_va;
} /* osl_secdma_alloc_consistent */

static void
osl_secdma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pdma)
{
	struct pci_dev * pdev;
	uint size_align;
	dma_addr_t paddr;
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pdma, paddr);
#else
	paddr = pdma;
#endif // endif
	size_align = ROUNDUP(size, SECDMA_DESC_ADDR_ALIGN);
	/* printk("Free DD: dma addr 0x%llx, size:%d\n", paddr, size_align); */
	pdev = osh->pdev;
	if (va == (osh->secdma_coherant_pfree - size_align)) {
		osh->secdma_coherant_pfree -= size_align;
	}
	dma_unmap_page(&pdev->dev, paddr, size_align, DMA_BIDIRECTIONAL);
} /* osl_secdma_free_consistent */

void *
osl_secdma_map_sts_phyrx(osl_t *osh, uint size, uint16 align_bits, dmaaddr_t *pap)
{
	dma_addr_t dma_handle = 0;
	struct pci_dev * pdev;
	uint size_align;

	pdev = osh->pdev;
	ASSERT(size);
	size_align = ROUNDUP(size, 8);

	dma_handle = dma_map_page(&pdev->dev, phys_to_page(osh->contig_base_sts_phyrx_pa),
		0, size_align, DMA_FROM_DEVICE);

#ifdef BCMDMA64OSL
	PHYSADDRLOSET(*pap, dma_handle & 0xffffffff);
	PHYSADDRHISET(*pap, (dma_handle >> 32) & 0xffffffff);
#else
	*pap = dma_handle;
#endif // endif
	return (osh->contig_base_sts_phyrx_va);
}

void
osl_secdma_unmap_sts_phyrx(osl_t *osh, void *va, uint size, dmaaddr_t pdma)
{
	struct pci_dev * pdev;
	uint size_align;
	dma_addr_t paddr;
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pdma, paddr);
#else
	paddr = pdma;
#endif // endif
	size_align = ROUNDUP(size, SECDMA_PHYRXSTS_ALIGN);
	/* printk("Free DD: dma addr 0x%llx, size:%d\n", paddr, size_align); */
	pdev = osh->pdev;
	dma_unmap_page(&pdev->dev, paddr, size_align, DMA_FROM_DEVICE);
} /* osl_secdma_free_consistent */
#endif /* BCM_SECURE_DMA */

#if defined(BCA_HNDROUTER)

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

#endif /* BCA_HNDROUTER */
