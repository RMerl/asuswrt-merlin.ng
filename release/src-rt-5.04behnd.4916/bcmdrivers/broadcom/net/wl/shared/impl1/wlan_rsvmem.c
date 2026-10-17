/*
   Copyright (c) 2024 Broadcom
   All Rights Reserved
   <:label-BRCM:2024:DUAL/GPL:standard

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
 * WiFi CMA Reserved Memory for large contiguous DMA buffers
 *
 * WiFi CMA reserve memory is allocated for WiFi drivers and is not shared with
 * other drivers.  Use user allocated WiFi CMA reserve memory for DMA buffers
 * instead of system shared CMA memory, if supported and enabled by SDK.
 *
 * This feature uses existing 'retain alloc' infrastructure for simple memory
 * management without worrying of fragmentation.
 */
#if defined(BCM_CMA_RSV_MEM)
#include <typedefs.h>
/* WLAN */
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>

/* SDK */
#include <bcm_assert.h>
#include <bcm_rsvmem.h>


/*
 * Definitions
 */
/* Local debugging */
#define CC_WRM_DBG              (0)

/** Errors always enabled ... */
#define WRM_LOG(fmt, arg...)    printk(CLRg "[WRM]: " fmt CLRnl, ##arg)
#define WRM_ERROR(fmt, arg...)  printk(CLRr "[WRM]: " fmt CLRnl, ##arg)

/** Function entry and exit enabled at level 1 and above */
#if (CC_WRM_DBG >= 1)
#define WRM_TRACE(fmt, arg...)  printk(CLRb "[WRM]: " fmt CLRnl, ##arg)
#else
#define WRM_TRACE(fmt, arg...)  while (0) { }
#endif /* CC_WRM_DBG < 1 */

/** Per allocation and free information at level 2 and above */
#if (CC_WRM_DBG >= 2)
#define WRM_INFO(fmt, arg...)   printk(CLRc "[WRM]: " fmt CLRnl, ##arg)
#else
#define WRM_INFO(fmt, arg...)   while (0) { }
#endif /* CC_WRM_DBG < 2 */

/* Allocation Blocks */
#define WRM_BLOCK_MAX           32
#define WRM_BLOCK_INVALID       0
#define WRM_BLOCK_ALLOCED       1
#define WRM_BLOCK_FREED         2

/* Default Reserve memory name */
#define WLAN_RSVMEM_NAME        "wlan_cma"

/*
 * Macros
 */
#define foreach_valid_block(wrm, block, idx)                            \
	for ((idx = 0, block = wrm->alloc.block);                           \
	     (idx < wrm->alloc.max && block->status != WRM_BLOCK_INVALID);  \
	    (idx++, block++))

/*
 * Structures
 */
/* Allocated Memory block */
typedef struct wlan_rsvmem_block {
	uint8        *va;           /* Virtual address of allocated memory */
	dma_addr_t   pa;            /* Physical address of allocated memory */
	uint32       size;          /* Size of allocated memory */
	int          status;        /* Allocation status */
	uint8        unit;          /* Radio unit allocating the memory */
} wlan_rsvmem_block_t;

typedef struct wlan_rsvmem {
	char        name[16];       /* Name of CMA Reserved Memory region */
	uint8       *va_base;       /* virtual address base */
	phys_addr_t pa_base;        /* physical address base */
	uint32      size;           /* memory size for allocation */

	/* Allocation */
	struct wlan_rsvmem_alloc {
	    uint32      offset;            /* memory offset allocation */
	    uint32      max;               /* maximum allocations */
	    spinlock_t  lock;              /* lock for alloc/free */
	    wlan_rsvmem_block_t block[0];  /* Allocated Blocks */
	} alloc;
} wlan_rsvmem_t;

/*
 * Global variables
 */
wlan_rsvmem_t *wlan_cma_rsvmem_cb = NULL;

char block_status_str[][8] = {
	"INVALID", "ALLOCED", "FREED"
};

/*
 * Local functions
 */
/*
 * Check whether the address is part of reserve memory or not
 */
static bool
wlan_rsvmem_addr(wlan_rsvmem_t *wrm, void *addr)
{
	bool is_wrm_addr = FALSE;

	if (wrm) {
	    void *va_end;

	    /* The allocation is always page size */
	    va_end = (void*)(wrm->va_base + wrm->size - PAGE_SIZE);
	    is_wrm_addr = ((addr >= (void*)wrm->va_base) && (addr <= va_end));
	}

	WRM_INFO("%s(0x%px) = %d\n", __FUNCTION__, addr, is_wrm_addr);

	return is_wrm_addr;
}

/*
 * Dump reserved memory information and its allocated blocks
 */
static void
wlan_rsvmem_dump(wlan_rsvmem_t *wrm)
{
	wlan_rsvmem_block_t *block;
	int idx;

	if (!wrm)
	    return;

	WRM_LOG(">>%s:\n", __FUNCTION__);
	WRM_LOG("[%s] Memory Info: va [0x%px] pa [%pa] size [0x%x]\n",
	    wrm->name, wrm->va_base, &wrm->pa_base, wrm->size);

	WRM_LOG("[%s] Alloc Info: offset [0x%x] max blocks [0x%x]\n",
	    wrm->name, wrm->alloc.offset, wrm->alloc.max);

	block = wrm->alloc.block;
	WRM_LOG("[%s] %7s %4s %18s %18s %10s\n", wrm->name, "status", "unit",
	    "virt addr", "phys addr", "size");

	foreach_valid_block(wrm, block, idx) {
	    WRM_LOG("[%s] %7s %4d 0x%px 0x%px 0x%08x\n",
	        wrm->name, block_status_str[block->status], block->unit, block->va,
	        &block->pa, block->size);
	}
	WRM_LOG("<<%s:\n\n", __FUNCTION__);

	return;
}

/*
 * External functions
 */
/*
 * Initialize named reserved memory for WiFi contiguous memory
 */
void* wlan_rsvmem_init(char *name)
{
	wlan_rsvmem_t *wrm = NULL;
	uint32 size = sizeof(wlan_rsvmem_t);
	int idx;

	WRM_TRACE("[%s]: >> %s()\n", module_name(THIS_MODULE), __FUNCTION__);

	size += sizeof(wlan_rsvmem_block_t)*WRM_BLOCK_MAX;
	wrm = kzalloc(size, GFP_KERNEL);
	if (wrm != NULL) {

	    WRM_INFO("Allocated CMA reserved memory context [0x%px]\n", wrm);

	    /* Get Reserved Memory information */
	    snprintf(wrm->name, sizeof(wrm->name),
	        (name) ? name : WLAN_RSVMEM_NAME);

	    WRM_INFO("looking for [%s] reserved memory\n", wrm->name);
	    if (BcmMemReserveGetByName(wrm->name, (void**)&wrm->va_base,
	            &wrm->pa_base, &wrm->size) == 0) {
	        WRM_LOG("found [%s] reserved memory with va [0x%px] pa [%pa] "
	            "size [0x%x]\n",
	            wrm->name, wrm->va_base, &wrm->pa_base, wrm->size);

	        wrm->alloc.offset = 0;
	        wrm->alloc.max = WRM_BLOCK_MAX;
	        spin_lock_init(&wrm->alloc.lock);
	        for (idx = 0; idx < wrm->alloc.max; idx++) {
	            wrm->alloc.block[idx].status = WRM_BLOCK_INVALID;
	            wrm->alloc.block[idx].size = 0;
	        }
#if (CC_WRM_DBG >= 2)
	        wlan_rsvmem_dump(wrm);
#endif /* CC_WRM_DBG >= 2 */

	        if (!name)
	            wlan_cma_rsvmem_cb = wrm;
	    } else {
	        WRM_INFO("[%s] reserved memory not found\n", wrm->name);
	        kfree(wrm);
	        wrm = NULL;
	    }
	} else {
	    WRM_ERROR("Failed to allocate CMA reserved memory context\n");
	}

	WRM_TRACE("<<%s(0x%px)\n", __FUNCTION__, wrm);

	return wrm;
}

void wlan_rsvmem_fini(void *ctxt)
{
	wlan_rsvmem_t *wrm;

	WRM_TRACE(">> %s(0x%px)\n", __FUNCTION__, ctxt);

	/* If context is not specified use 'wlan_cma' reserve memory context */
	wrm = (ctxt) ? (wlan_rsvmem_t*)ctxt : wlan_cma_rsvmem_cb;

	if (!wrm)
	    return;

	/* Nothing to do with reserved memory */
	wlan_rsvmem_dump(wrm);

	/* Free the context */
	WRM_INFO("Freeing [%s] CMA Reserve Memory context [0x%px]\n",
	    wrm->name, wrm);
	kfree(wrm);

	if (!ctxt)
	    wlan_cma_rsvmem_cb = NULL;
	WRM_TRACE("<< %s\n", __FUNCTION__);

	return;
}

/*
 * External exported functions
 */
/*
 * Allocate memory from the reserve memory for the given wifi unit
 * If exists, re-Use previously freed memory for the same radio with same size
 */
void *
wlan_rsvmem_alloc(void *ctxt, int unit, uint size, dma_addr_t *pap)
{
	wlan_rsvmem_t *wrm;
	wlan_rsvmem_block_t *block;
	int idx;
	void *addr = NULL;
	unsigned long flags;

	WRM_TRACE(">>%s(0x%x)\n", __FUNCTION__, size);

	/* If context is not specified use 'wlan_cma' reserve memory context */
	wrm = (ctxt) ? (wlan_rsvmem_t*)ctxt : wlan_cma_rsvmem_cb;

	if (!wrm || !wrm->size)
	    return addr;

	/* Align size to the nearest page size */
	size = ALIGN_SIZE(size, PAGE_SIZE);

	spin_lock_irqsave(&wrm->alloc.lock, flags);

	foreach_valid_block(wrm, block, idx) {
	    if ((unit == block->unit) && (size == block->size) &&
	        (block->status == WRM_BLOCK_FREED)) {
	        /* Reuse previosly allocated cma reserved memory address */
	        addr = block->va;
	        if (pap)
	            *pap = block->pa;
	        block->status = WRM_BLOCK_ALLOCED;

	        WRM_LOG("[%s] unit [%d] Reuse va [0x%px] pa [%pa] size [0x%x]\n",
	            wrm->name, unit, addr, pap, size);

	        break;
	    }
	}

	if (!addr) {
	    /* allocate 'size' from available reseved memory space */
	    if (idx >= wrm->alloc.max) {
	        WRM_ERROR("[%s]: alloc block > %d\n", wrm->name, wrm->alloc.max);
	    } else if ((wrm->alloc.offset + size) <= wrm->size) {
	        addr = (void*)(wrm->va_base + wrm->alloc.offset);
	        if (pap)
	            *pap = (dma_addr_t)(wrm->pa_base + wrm->alloc.offset);
	        wrm->alloc.offset += size;
	        WRM_LOG("[%s] unit [%d] Alloc va [0x%px] pa [%pa] size [0x%x]\n",
	            wrm->name, unit, addr, pap, size);

	        /* Fill the block information */
	        block->unit = unit;
	        block->va = addr;
	        if (pap) {
	            block->pa = *pap;
	        }
	        block->size = size;
	        block->status = WRM_BLOCK_ALLOCED;
	    } else {
	        WRM_INFO("[%s] Failed to allocate size [0x%x], available [0x%x]\n",
	            wrm->name, size, (wrm->size - wrm->alloc.offset));
	    }
	}

	spin_unlock_irqrestore(&wrm->alloc.lock, flags);

	WRM_TRACE("<<%s(0x%px)\n", __FUNCTION__, addr);

	return addr;
}
EXPORT_SYMBOL(wlan_rsvmem_alloc);

/*
 * Free memory from the reserve memory if valid reserved memory
 *
 * Returns 0: on freeing valid reserve memory
 *         -EINVAL otherwise
 */
int
wlan_rsvmem_free(void *ctxt, int unit, uint size, void * va, dma_addr_t pa)
{
	wlan_rsvmem_t *wrm;
	wlan_rsvmem_block_t *block;
	int idx;
	unsigned long flags;
	int rc = -EINVAL;

	WRM_TRACE(">>%s(0x%px, 0x%x)\n", __FUNCTION__, va, size);

	/* If context is not specified use 'wlan_cma' reserve memory context */
	wrm = (ctxt) ? (wlan_rsvmem_t*)ctxt : wlan_cma_rsvmem_cb;

	if (!wrm || !wrm->size)
	    return rc;

	if (!wlan_rsvmem_addr(wrm, va))
	    return rc;

	spin_lock_irqsave(&wrm->alloc.lock, flags);

	foreach_valid_block(wrm, block, idx) {
	    if ((unit == block->unit) && (size == block->size) &&
	        (va == block->va) && (block->status == WRM_BLOCK_ALLOCED)) {
	        /* Found the memory block */
	        rc = 0;
	        block->status = WRM_BLOCK_FREED;

	        WRM_LOG("[%s] unit [%d] Free  va [0x%px] pa [%pa] size [0x%x]\n",
	            wrm->name, unit, block->va, &block->pa, block->size);

	        break;
	    }
	}

	spin_unlock_irqrestore(&wrm->alloc.lock, flags);

	if (rc != 0) {
	    /* Unable to find the allocated block */
	    WRM_ERROR("[%s] Failed to free unit [%d] va [0x%px] pa [%pa]"
	        " size [0x%x]\n",
	        wrm->name, unit, block->va, &block->pa, block->size);
	    wlan_rsvmem_dump(wrm);
	}

	WRM_TRACE("<<%s\n", __FUNCTION__);

	return rc;
}
EXPORT_SYMBOL(wlan_rsvmem_free);
#endif /* BCM_CMA_RSV_MEM */
