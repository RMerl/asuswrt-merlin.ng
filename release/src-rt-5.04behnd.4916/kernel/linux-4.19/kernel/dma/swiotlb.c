/*
 * Dynamic DMA mapping support.
 *
 * This implementation is a fallback for platforms that do not support
 * I/O TLBs (aka DMA address translation hardware).
 * Copyright (C) 2000 Asit Mallick <Asit.K.Mallick@intel.com>
 * Copyright (C) 2000 Goutham Rao <goutham.rao@intel.com>
 * Copyright (C) 2000, 2003 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 *
 * 03/05/07 davidm	Switch from PCI-DMA to generic device DMA API.
 * 00/12/13 davidm	Rename to swiotlb.c and add mark_clean() to avoid
 *			unnecessary i-cache flushing.
 * 04/07/.. ak		Better overflow handling. Assorted fixes.
 * 05/09/10 linville	Add support for syncing ranges, support syncing for
 *			DMA_BIDIRECTIONAL mappings, miscellaneous cleanup.
 * 08/12/11 beckyb	Add highmem support
 */

#define pr_fmt(fmt) "software IO TLB: " fmt

#include <linux/cache.h>
#include <linux/dma-direct.h>
#include <linux/mm.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/swiotlb.h>
#include <linux/pfn.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/highmem.h>
#include <linux/gfp.h>
#include <linux/scatterlist.h>
#include <linux/mem_encrypt.h>
#include <linux/set_memory.h>
#if defined CONFIG_BCM_KF_ARM64_BCM963XX && defined(CONFIG_BCM94908) && defined(CONFIG_BCM_HND_EAP)
#include <linux/memblock.h>
#endif

#include <asm/io.h>
#include <asm/dma.h>

#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/iommu-helper.h>

#define CREATE_TRACE_POINTS
#include <trace/events/swiotlb.h>

#define OFFSET(val,align) ((unsigned long)	\
	                   ( (val) & ( (align) - 1)))

#define SLABS_PER_PAGE (1 << (PAGE_SHIFT - IO_TLB_SHIFT))

/*
 * Minimum IO TLB size to bother booting with.  Systems with mainly
 * 64bit capable cards will only lightly use the swiotlb.  If we can't
 * allocate a contiguous 1MB, we're probably in trouble anyway.
 */
#define IO_TLB_MIN_SLABS ((1<<20) >> IO_TLB_SHIFT)

enum swiotlb_force swiotlb_force;

/*
 * Used to do a quick range check in swiotlb_tbl_unmap_single and
 * swiotlb_tbl_sync_single_*, to see if the memory was in fact allocated by this
 * API.
 */
static phys_addr_t io_tlb_start, io_tlb_end;

/*
 * The number of IO TLB blocks (in groups of 64) between io_tlb_start and
 * io_tlb_end.  This is command line adjustable via setup_io_tlb_npages.
 */
static unsigned long io_tlb_nslabs;

/*
 * When the IOMMU overflows we return a fallback buffer. This sets the size.
 */
static unsigned long io_tlb_overflow = 32*1024;

static phys_addr_t io_tlb_overflow_buffer;

/*
 * This is a free list describing the number of free entries available from
 * each index
 */
static unsigned int *io_tlb_list;
static unsigned int io_tlb_index;

/*
 * Max segment that we can provide which (if pages are contingous) will
 * not be bounced (unless SWIOTLB_FORCE is set).
 */
unsigned int max_segment;

/*
 * We need to save away the original address corresponding to a mapped entry
 * for the sync operations.
 */
#define INVALID_PHYS_ADDR (~(phys_addr_t)0)
static phys_addr_t *io_tlb_orig_addr;

/*
 * Protect the above data structures in the map and unmap calls
 */
static DEFINE_SPINLOCK(io_tlb_lock);

static int late_alloc;

static int __init
setup_io_tlb_npages(char *str)
{
	if (isdigit(*str)) {
		io_tlb_nslabs = simple_strtoul(str, &str, 0);
		/* avoid tail segment of size < IO_TLB_SEGSIZE */
		io_tlb_nslabs = ALIGN(io_tlb_nslabs, IO_TLB_SEGSIZE);
	}
	if (*str == ',')
		++str;
	if (!strcmp(str, "force")) {
		swiotlb_force = SWIOTLB_FORCE;
	} else if (!strcmp(str, "noforce")) {
		swiotlb_force = SWIOTLB_NO_FORCE;
		io_tlb_nslabs = 1;
	}

	return 0;
}
early_param("swiotlb", setup_io_tlb_npages);
/* make io_tlb_overflow tunable too? */

unsigned long swiotlb_nr_tbl(void)
{
	return io_tlb_nslabs;
}
EXPORT_SYMBOL_GPL(swiotlb_nr_tbl);

unsigned int swiotlb_max_segment(void)
{
	return max_segment;
}
EXPORT_SYMBOL_GPL(swiotlb_max_segment);

void swiotlb_set_max_segment(unsigned int val)
{
	if (swiotlb_force == SWIOTLB_FORCE)
		max_segment = 1;
	else
		max_segment = rounddown(val, PAGE_SIZE);
}

/* default to 64MB */
#define IO_TLB_DEFAULT_SIZE (64UL<<20)
unsigned long swiotlb_size_or_default(void)
{
	unsigned long size;

	size = io_tlb_nslabs << IO_TLB_SHIFT;

	return size ? size : (IO_TLB_DEFAULT_SIZE);
}

static bool no_iotlb_memory;

void swiotlb_print_info(void)
{
	unsigned long bytes = io_tlb_nslabs << IO_TLB_SHIFT;

	if (no_iotlb_memory) {
		pr_warn("No low mem\n");
		return;
	}

	pr_info("mapped [mem %#010llx-%#010llx] (%luMB)\n",
	       (unsigned long long)io_tlb_start,
	       (unsigned long long)io_tlb_end,
	       bytes >> 20);
}

/*
 * Early SWIOTLB allocation may be too early to allow an architecture to
 * perform the desired operations.  This function allows the architecture to
 * call SWIOTLB when the operations are possible.  It needs to be called
 * before the SWIOTLB memory is used.
 */
void __init swiotlb_update_mem_attributes(void)
{
	void *vaddr;
	unsigned long bytes;

	if (no_iotlb_memory || late_alloc)
		return;

	vaddr = phys_to_virt(io_tlb_start);
	bytes = PAGE_ALIGN(io_tlb_nslabs << IO_TLB_SHIFT);
	set_memory_decrypted((unsigned long)vaddr, bytes >> PAGE_SHIFT);
	memset(vaddr, 0, bytes);

	vaddr = phys_to_virt(io_tlb_overflow_buffer);
	bytes = PAGE_ALIGN(io_tlb_overflow);
	set_memory_decrypted((unsigned long)vaddr, bytes >> PAGE_SHIFT);
	memset(vaddr, 0, bytes);
}

int __init swiotlb_init_with_tbl(char *tlb, unsigned long nslabs, int verbose)
{
	void *v_overflow_buffer;
	unsigned long i, bytes;

	bytes = nslabs << IO_TLB_SHIFT;

	io_tlb_nslabs = nslabs;
	io_tlb_start = __pa(tlb);
	io_tlb_end = io_tlb_start + bytes;

	/*
	 * Get the overflow emergency buffer
	 */
	v_overflow_buffer = memblock_virt_alloc_low_nopanic(
						PAGE_ALIGN(io_tlb_overflow),
						PAGE_SIZE);
	if (!v_overflow_buffer)
		return -ENOMEM;

	io_tlb_overflow_buffer = __pa(v_overflow_buffer);

	/*
	 * Allocate and initialize the free list array.  This array is used
	 * to find contiguous free memory regions of size up to IO_TLB_SEGSIZE
	 * between io_tlb_start and io_tlb_end.
	 */
	io_tlb_list = memblock_virt_alloc(
				PAGE_ALIGN(io_tlb_nslabs * sizeof(int)),
				PAGE_SIZE);
	io_tlb_orig_addr = memblock_virt_alloc(
				PAGE_ALIGN(io_tlb_nslabs * sizeof(phys_addr_t)),
				PAGE_SIZE);
	for (i = 0; i < io_tlb_nslabs; i++) {
		io_tlb_list[i] = IO_TLB_SEGSIZE - OFFSET(i, IO_TLB_SEGSIZE);
		io_tlb_orig_addr[i] = INVALID_PHYS_ADDR;
	}
	io_tlb_index = 0;
	no_iotlb_memory = false;

	if (verbose)
		swiotlb_print_info();

	swiotlb_set_max_segment(io_tlb_nslabs << IO_TLB_SHIFT);
	return 0;
}

/*
 * Statically reserve bounce buffer space and initialize bounce buffer data
 * structures for the software IO TLB used to implement the DMA API.
 */
void  __init
swiotlb_init(int verbose)
{
	size_t default_size = IO_TLB_DEFAULT_SIZE;
	unsigned char *vstart;
	unsigned long bytes;

#if defined CONFIG_BCM_KF_ARM64_BCM963XX
#if defined(CONFIG_BCM94908) && defined(CONFIG_BCM_HND_EAP)
	/*
	 * ZONE_DMA is restricted to 1GB due to a runner HW limitation. We
	 * still don't need swiotlb in this case though so check against the
	 * 4GB boundary here rather than zone_dma.
	 */
	if (!swiotlb_force &&
		(max_pfn <= ((min((memblock_start_of_DRAM() & GENMASK_ULL(63, 32))
				  + (1ULL << 32),
				memblock_end_of_DRAM())) >> PAGE_SHIFT)))
		return;
#elif defined CONFIG_ARM64
	if(!swiotlb_force && (max_pfn <= (arm64_dma_phys_limit >> PAGE_SHIFT)))
		return;
#endif
#endif
	if (!io_tlb_nslabs) {
		io_tlb_nslabs = (default_size >> IO_TLB_SHIFT);
		io_tlb_nslabs = ALIGN(io_tlb_nslabs, IO_TLB_SEGSIZE);
	}

	bytes = io_tlb_nslabs << IO_TLB_SHIFT;

	/* Get IO TLB memory from the low pages */
	vstart = memblock_virt_alloc_low_nopanic(PAGE_ALIGN(bytes), PAGE_SIZE);
	if (vstart && !swiotlb_init_with_tbl(vstart, io_tlb_nslabs, verbose))
		return;

	if (io_tlb_start) {
		memblock_free_early(io_tlb_start,
				    PAGE_ALIGN(io_tlb_nslabs << IO_TLB_SHIFT));
		io_tlb_start = 0;
	}
	pr_warn("Cannot allocate buffer");
	no_iotlb_memory = true;
}

/*
 * Systems with larger DMA zones (those that don't support ISA) can
 * initialize the swiotlb later using the slab allocator if needed.
 * This should be just like above, but with some error catching.
 */
int
swiotlb_late_init_with_default_size(size_t default_size)
{
	unsigned long bytes, req_nslabs = io_tlb_nslabs;
	unsigned char *vstart = NULL;
	unsigned int order;
	int rc = 0;

	if (!io_tlb_nslabs) {
		io_tlb_nslabs = (default_size >> IO_TLB_SHIFT);
		io_tlb_nslabs = ALIGN(io_tlb_nslabs, IO_TLB_SEGSIZE);
	}

	/*
	 * Get IO TLB memory from the low pages
	 */
	order = get_order(io_tlb_nslabs << IO_TLB_SHIFT);
	io_tlb_nslabs = SLABS_PER_PAGE << order;
	bytes = io_tlb_nslabs << IO_TLB_SHIFT;

	while ((SLABS_PER_PAGE << order) > IO_TLB_MIN_SLABS) {
		vstart = (void *)__get_free_pages(GFP_DMA | __GFP_NOWARN,
						  order);
		if (vstart)
			break;
		order--;
	}

	if (!vstart) {
		io_tlb_nslabs = req_nslabs;
		return -ENOMEM;
	}
	if (order != get_order(bytes)) {
		pr_warn("only able to allocate %ld MB\n",
			(PAGE_SIZE << order) >> 20);
		io_tlb_nslabs = SLABS_PER_PAGE << order;
	}
	rc = swiotlb_late_init_with_tbl(vstart, io_tlb_nslabs);
	if (rc)
		free_pages((unsigned long)vstart, order);

	return rc;
}

int
swiotlb_late_init_with_tbl(char *tlb, unsigned long nslabs)
{
	unsigned long i, bytes;
	unsigned char *v_overflow_buffer;

	bytes = nslabs << IO_TLB_SHIFT;

	io_tlb_nslabs = nslabs;
	io_tlb_start = virt_to_phys(tlb);
	io_tlb_end = io_tlb_start + bytes;

	set_memory_decrypted((unsigned long)tlb, bytes >> PAGE_SHIFT);
	memset(tlb, 0, bytes);

	/*
	 * Get the overflow emergency buffer
	 */
	v_overflow_buffer = (void *)__get_free_pages(GFP_DMA,
						     get_order(io_tlb_overflow));
	if (!v_overflow_buffer)
		goto cleanup2;

	set_memory_decrypted((unsigned long)v_overflow_buffer,
			io_tlb_overflow >> PAGE_SHIFT);
	memset(v_overflow_buffer, 0, io_tlb_overflow);
	io_tlb_overflow_buffer = virt_to_phys(v_overflow_buffer);

	/*
	 * Allocate and initialize the free list array.  This array is used
	 * to find contiguous free memory regions of size up to IO_TLB_SEGSIZE
	 * between io_tlb_start and io_tlb_end.
	 */
	io_tlb_list = (unsigned int *)__get_free_pages(GFP_KERNEL,
	                              get_order(io_tlb_nslabs * sizeof(int)));
	if (!io_tlb_list)
		goto cleanup3;

	io_tlb_orig_addr = (phys_addr_t *)
		__get_free_pages(GFP_KERNEL,
				 get_order(io_tlb_nslabs *
					   sizeof(phys_addr_t)));
	if (!io_tlb_orig_addr)
		goto cleanup4;

	for (i = 0; i < io_tlb_nslabs; i++) {
		io_tlb_list[i] = IO_TLB_SEGSIZE - OFFSET(i, IO_TLB_SEGSIZE);
		io_tlb_orig_addr[i] = INVALID_PHYS_ADDR;
	}
	io_tlb_index = 0;
	no_iotlb_memory = false;

	swiotlb_print_info();

	late_alloc = 1;

	swiotlb_set_max_segment(io_tlb_nslabs << IO_TLB_SHIFT);

	return 0;

cleanup4:
	free_pages((unsigned long)io_tlb_list, get_order(io_tlb_nslabs *
	                                                 sizeof(int)));
	io_tlb_list = NULL;
cleanup3:
	free_pages((unsigned long)v_overflow_buffer,
		   get_order(io_tlb_overflow));
	io_tlb_overflow_buffer = 0;
cleanup2:
	io_tlb_end = 0;
	io_tlb_start = 0;
	io_tlb_nslabs = 0;
	max_segment = 0;
	return -ENOMEM;
}

void __init swiotlb_exit(void)
{
	if (!io_tlb_orig_addr)
		return;

	if (late_alloc) {
		free_pages((unsigned long)phys_to_virt(io_tlb_overflow_buffer),
			   get_order(io_tlb_overflow));
		free_pages((unsigned long)io_tlb_orig_addr,
			   get_order(io_tlb_nslabs * sizeof(phys_addr_t)));
		free_pages((unsigned long)io_tlb_list, get_order(io_tlb_nslabs *
								 sizeof(int)));
		free_pages((unsigned long)phys_to_virt(io_tlb_start),
			   get_order(io_tlb_nslabs << IO_TLB_SHIFT));
	} else {
		memblock_free_late(io_tlb_overflow_buffer,
				   PAGE_ALIGN(io_tlb_overflow));
		memblock_free_late(__pa(io_tlb_orig_addr),
				   PAGE_ALIGN(io_tlb_nslabs * sizeof(phys_addr_t)));
		memblock_free_late(__pa(io_tlb_list),
				   PAGE_ALIGN(io_tlb_nslabs * sizeof(int)));
		memblock_free_late(io_tlb_start,
				   PAGE_ALIGN(io_tlb_nslabs << IO_TLB_SHIFT));
	}
	io_tlb_nslabs = 0;
	max_segment = 0;
}

int is_swiotlb_buffer(phys_addr_t paddr)
{
	return paddr >= io_tlb_start && paddr < io_tlb_end;
}

/*
 * Bounce: copy the swiotlb buffer back to the original dma location
 */
static void swiotlb_bounce(phys_addr_t orig_addr, phys_addr_t tlb_addr,
			   size_t size, enum dma_data_direction dir)
{
	unsigned long pfn = PFN_DOWN(orig_addr);
	unsigned char *vaddr = phys_to_virt(tlb_addr);

	if (PageHighMem(pfn_to_page(pfn))) {
		/* The buffer does not have a mapping.  Map it in and copy */
		unsigned int offset = orig_addr & ~PAGE_MASK;
		char *buffer;
		unsigned int sz = 0;
		unsigned long flags;

		while (size) {
			sz = min_t(size_t, PAGE_SIZE - offset, size);

			local_irq_save(flags);
			buffer = kmap_atomic(pfn_to_page(pfn));
			if (dir == DMA_TO_DEVICE)
				memcpy(vaddr, buffer + offset, sz);
			else
				memcpy(buffer + offset, vaddr, sz);
			kunmap_atomic(buffer);
			local_irq_restore(flags);

			size -= sz;
			pfn++;
			vaddr += sz;
			offset = 0;
		}
	} else if (dir == DMA_TO_DEVICE) {
		memcpy(vaddr, phys_to_virt(orig_addr), size);
	} else {
		memcpy(phys_to_virt(orig_addr), vaddr, size);
	}
}

phys_addr_t swiotlb_tbl_map_single(struct device *hwdev,
				   dma_addr_t tbl_dma_addr,
				   phys_addr_t orig_addr, size_t size,
				   enum dma_data_direction dir,
				   unsigned long attrs)
{
	unsigned long flags;
	phys_addr_t tlb_addr;
	unsigned int nslots, stride, index, wrap;
	int i;
	unsigned long mask;
	unsigned long offset_slots;
	unsigned long max_slots;

	if (no_iotlb_memory)
		panic("Can not allocate SWIOTLB buffer earlier and can't now provide you with the DMA bounce buffer");

	if (mem_encrypt_active())
		pr_warn_once("%s is active and system is using DMA bounce buffers\n",
			     sme_active() ? "SME" : "SEV");

	mask = dma_get_seg_boundary(hwdev);

	tbl_dma_addr &= mask;

	offset_slots = ALIGN(tbl_dma_addr, 1 << IO_TLB_SHIFT) >> IO_TLB_SHIFT;

	/*
 	 * Carefully handle integer overflow which can occur when mask == ~0UL.
 	 */
	max_slots = mask + 1
		    ? ALIGN(mask + 1, 1 << IO_TLB_SHIFT) >> IO_TLB_SHIFT
		    : 1UL << (BITS_PER_LONG - IO_TLB_SHIFT);

	/*
	 * For mappings greater than or equal to a page, we limit the stride
	 * (and hence alignment) to a page size.
	 */
	nslots = ALIGN(size, 1 << IO_TLB_SHIFT) >> IO_TLB_SHIFT;
	if (size >= PAGE_SIZE)
		stride = (1 << (PAGE_SHIFT - IO_TLB_SHIFT));
	else
		stride = 1;

	BUG_ON(!nslots);

	/*
	 * Find suitable number of IO TLB entries size that will fit this
	 * request and allocate a buffer from that IO TLB pool.
	 */
	spin_lock_irqsave(&io_tlb_lock, flags);
	index = ALIGN(io_tlb_index, stride);
	if (index >= io_tlb_nslabs)
		index = 0;
	wrap = index;

	do {
		while (iommu_is_span_boundary(index, nslots, offset_slots,
					      max_slots)) {
			index += stride;
			if (index >= io_tlb_nslabs)
				index = 0;
			if (index == wrap)
				goto not_found;
		}

		/*
		 * If we find a slot that indicates we have 'nslots' number of
		 * contiguous buffers, we allocate the buffers from that slot
		 * and mark the entries as '0' indicating unavailable.
		 */
		if (io_tlb_list[index] >= nslots) {
			int count = 0;

			for (i = index; i < (int) (index + nslots); i++)
				io_tlb_list[i] = 0;
			for (i = index - 1; (OFFSET(i, IO_TLB_SEGSIZE) != IO_TLB_SEGSIZE - 1) && io_tlb_list[i]; i--)
				io_tlb_list[i] = ++count;
			tlb_addr = io_tlb_start + (index << IO_TLB_SHIFT);

			/*
			 * Update the indices to avoid searching in the next
			 * round.
			 */
			io_tlb_index = ((index + nslots) < io_tlb_nslabs
					? (index + nslots) : 0);

			goto found;
		}
		index += stride;
		if (index >= io_tlb_nslabs)
			index = 0;
	} while (index != wrap);

not_found:
	spin_unlock_irqrestore(&io_tlb_lock, flags);
	if (!(attrs & DMA_ATTR_NO_WARN) && printk_ratelimit())
		dev_warn(hwdev, "swiotlb buffer is full (sz: %zd bytes)\n", size);
	return SWIOTLB_MAP_ERROR;
found:
	spin_unlock_irqrestore(&io_tlb_lock, flags);

	/*
	 * Save away the mapping from the original address to the DMA address.
	 * This is needed when we sync the memory.  Then we sync the buffer if
	 * needed.
	 */
	for (i = 0; i < nslots; i++)
		io_tlb_orig_addr[index+i] = orig_addr + (i << IO_TLB_SHIFT);
	/*
	 * When dir == DMA_FROM_DEVICE we could omit the copy from the orig
	 * to the tlb buffer, if we knew for sure the device will
	 * overwirte the entire current content. But we don't. Thus
	 * unconditional bounce may prevent leaking swiotlb content (i.e.
	 * kernel memory) to user-space.
	 */
	if (orig_addr)
		swiotlb_bounce(orig_addr, tlb_addr, size, DMA_TO_DEVICE);
	return tlb_addr;
}

/*
 * Allocates bounce buffer and returns its physical address.
 */
static phys_addr_t
map_single(struct device *hwdev, phys_addr_t phys, size_t size,
	   enum dma_data_direction dir, unsigned long attrs)
{
	dma_addr_t start_dma_addr;

	if (swiotlb_force == SWIOTLB_NO_FORCE) {
		dev_warn_ratelimited(hwdev, "Cannot do DMA to address %pa\n",
				     &phys);
		return SWIOTLB_MAP_ERROR;
	}

	start_dma_addr = __phys_to_dma(hwdev, io_tlb_start);
	return swiotlb_tbl_map_single(hwdev, start_dma_addr, phys, size,
				      dir, attrs);
}

/*
 * tlb_addr is the physical address of the bounce buffer to unmap.
 */
void swiotlb_tbl_unmap_single(struct device *hwdev, phys_addr_t tlb_addr,
			      size_t size, enum dma_data_direction dir,
			      unsigned long attrs)
{
	unsigned long flags;
	int i, count, nslots = ALIGN(size, 1 << IO_TLB_SHIFT) >> IO_TLB_SHIFT;
	int index = (tlb_addr - io_tlb_start) >> IO_TLB_SHIFT;
	phys_addr_t orig_addr = io_tlb_orig_addr[index];

	/*
	 * First, sync the memory before unmapping the entry
	 */
	if (orig_addr != INVALID_PHYS_ADDR &&
	    !(attrs & DMA_ATTR_SKIP_CPU_SYNC) &&
	    ((dir == DMA_FROM_DEVICE) || (dir == DMA_BIDIRECTIONAL)))
		swiotlb_bounce(orig_addr, tlb_addr, size, DMA_FROM_DEVICE);

	/*
	 * Return the buffer to the free list by setting the corresponding
	 * entries to indicate the number of contiguous entries available.
	 * While returning the entries to the free list, we merge the entries
	 * with slots below and above the pool being returned.
	 */
	spin_lock_irqsave(&io_tlb_lock, flags);
	{
		count = ((index + nslots) < ALIGN(index + 1, IO_TLB_SEGSIZE) ?
			 io_tlb_list[index + nslots] : 0);
		/*
		 * Step 1: return the slots to the free list, merging the
		 * slots with superceeding slots
		 */
		for (i = index + nslots - 1; i >= index; i--) {
			io_tlb_list[i] = ++count;
			io_tlb_orig_addr[i] = INVALID_PHYS_ADDR;
		}
		/*
		 * Step 2: merge the returned slots with the preceding slots,
		 * if available (non zero)
		 */
		for (i = index - 1; (OFFSET(i, IO_TLB_SEGSIZE) != IO_TLB_SEGSIZE -1) && io_tlb_list[i]; i--)
			io_tlb_list[i] = ++count;
	}
	spin_unlock_irqrestore(&io_tlb_lock, flags);
}

void swiotlb_tbl_sync_single(struct device *hwdev, phys_addr_t tlb_addr,
			     size_t size, enum dma_data_direction dir,
			     enum dma_sync_target target)
{
	int index = (tlb_addr - io_tlb_start) >> IO_TLB_SHIFT;
	phys_addr_t orig_addr = io_tlb_orig_addr[index];

	if (orig_addr == INVALID_PHYS_ADDR)
		return;
	orig_addr += (unsigned long)tlb_addr & ((1 << IO_TLB_SHIFT) - 1);

	switch (target) {
	case SYNC_FOR_CPU:
		if (likely(dir == DMA_FROM_DEVICE || dir == DMA_BIDIRECTIONAL))
			swiotlb_bounce(orig_addr, tlb_addr,
				       size, DMA_FROM_DEVICE);
		else
			BUG_ON(dir != DMA_TO_DEVICE);
		break;
	case SYNC_FOR_DEVICE:
		if (likely(dir == DMA_TO_DEVICE || dir == DMA_BIDIRECTIONAL))
			swiotlb_bounce(orig_addr, tlb_addr,
				       size, DMA_TO_DEVICE);
		else
			BUG_ON(dir != DMA_FROM_DEVICE);
		break;
	default:
		BUG();
	}
}

static inline bool dma_coherent_ok(struct device *dev, dma_addr_t addr,
		size_t size)
{
	u64 mask = DMA_BIT_MASK(32);

	if (dev && dev->coherent_dma_mask)
		mask = dev->coherent_dma_mask;
	return addr + size - 1 <= mask;
}

static void *
swiotlb_alloc_buffer(struct device *dev, size_t size, dma_addr_t *dma_handle,
		unsigned long attrs)
{
	phys_addr_t phys_addr;

	if (swiotlb_force == SWIOTLB_NO_FORCE)
		goto out_warn;

	phys_addr = swiotlb_tbl_map_single(dev,
			__phys_to_dma(dev, io_tlb_start),
			0, size, DMA_FROM_DEVICE, attrs);
	if (phys_addr == SWIOTLB_MAP_ERROR)
		goto out_warn;

	*dma_handle = __phys_to_dma(dev, phys_addr);
	if (!dma_coherent_ok(dev, *dma_handle, size))
		goto out_unmap;

	memset(phys_to_virt(phys_addr), 0, size);
	return phys_to_virt(phys_addr);

out_unmap:
	dev_warn(dev, "hwdev DMA mask = 0x%016Lx, dev_addr = 0x%016Lx\n",
		(unsigned long long)dev->coherent_dma_mask,
		(unsigned long long)*dma_handle);

	/*
	 * DMA_TO_DEVICE to avoid memcpy in unmap_single.
	 * DMA_ATTR_SKIP_CPU_SYNC is optional.
	 */
	swiotlb_tbl_unmap_single(dev, phys_addr, size, DMA_TO_DEVICE,
			DMA_ATTR_SKIP_CPU_SYNC);
out_warn:
	if (!(attrs & DMA_ATTR_NO_WARN) && printk_ratelimit()) {
		dev_warn(dev,
			"swiotlb: coherent allocation failed, size=%zu\n",
			size);
		dump_stack();
	}
	return NULL;
}

static bool swiotlb_free_buffer(struct device *dev, size_t size,
		dma_addr_t dma_addr)
{
	phys_addr_t phys_addr = dma_to_phys(dev, dma_addr);

	WARN_ON_ONCE(irqs_disabled());

	if (!is_swiotlb_buffer(phys_addr))
		return false;

	/*
	 * DMA_TO_DEVICE to avoid memcpy in swiotlb_tbl_unmap_single.
	 * DMA_ATTR_SKIP_CPU_SYNC is optional.
	 */
	swiotlb_tbl_unmap_single(dev, phys_addr, size, DMA_TO_DEVICE,
				 DMA_ATTR_SKIP_CPU_SYNC);
	return true;
}

/*
 * Map a single buffer of the indicated size for DMA in streaming mode.  The
 * physical address to use is returned.
 *
 * Once the device is given the dma address, the device owns this memory until
 * either swiotlb_unmap_page or swiotlb_dma_sync_single is performed.
 */
dma_addr_t swiotlb_map_page(struct device *dev, struct page *page,
			    unsigned long offset, size_t size,
			    enum dma_data_direction dir,
			    unsigned long attrs)
{
	phys_addr_t map, phys = page_to_phys(page) + offset;
	dma_addr_t dev_addr = phys_to_dma(dev, phys);

	BUG_ON(dir == DMA_NONE);
	/*
	 * If the address happens to be in the device's DMA window,
	 * we can safely return the device addr and not worry about bounce
	 * buffering it.
	 */
	if (dma_capable(dev, dev_addr, size) && swiotlb_force != SWIOTLB_FORCE)
		return dev_addr;

	trace_swiotlb_bounced(dev, dev_addr, size, swiotlb_force);

	/* Oh well, have to allocate and map a bounce buffer. */
	map = map_single(dev, phys, size, dir, attrs);
	if (map == SWIOTLB_MAP_ERROR)
		return __phys_to_dma(dev, io_tlb_overflow_buffer);

	dev_addr = __phys_to_dma(dev, map);

	/* Ensure that the address returned is DMA'ble */
	if (dma_capable(dev, dev_addr, size))
		return dev_addr;

	attrs |= DMA_ATTR_SKIP_CPU_SYNC;
	swiotlb_tbl_unmap_single(dev, map, size, dir, attrs);

	return __phys_to_dma(dev, io_tlb_overflow_buffer);
}

/*
 * Unmap a single streaming mode DMA translation.  The dma_addr and size must
 * match what was provided for in a previous swiotlb_map_page call.  All
 * other usages are undefined.
 *
 * After this call, reads by the cpu to the buffer are guaranteed to see
 * whatever the device wrote there.
 */
static void unmap_single(struct device *hwdev, dma_addr_t dev_addr,
			 size_t size, enum dma_data_direction dir,
			 unsigned long attrs)
{
	phys_addr_t paddr = dma_to_phys(hwdev, dev_addr);

	BUG_ON(dir == DMA_NONE);

	if (is_swiotlb_buffer(paddr)) {
		swiotlb_tbl_unmap_single(hwdev, paddr, size, dir, attrs);
		return;
	}

	if (dir != DMA_FROM_DEVICE)
		return;

	/*
	 * phys_to_virt doesn't work with hihgmem page but we could
	 * call dma_mark_clean() with hihgmem page here. However, we
	 * are fine since dma_mark_clean() is null on POWERPC. We can
	 * make dma_mark_clean() take a physical address if necessary.
	 */
	dma_mark_clean(phys_to_virt(paddr), size);
}

void swiotlb_unmap_page(struct device *hwdev, dma_addr_t dev_addr,
			size_t size, enum dma_data_direction dir,
			unsigned long attrs)
{
	unmap_single(hwdev, dev_addr, size, dir, attrs);
}

/*
 * Make physical memory consistent for a single streaming mode DMA translation
 * after a transfer.
 *
 * If you perform a swiotlb_map_page() but wish to interrogate the buffer
 * using the cpu, yet do not wish to teardown the dma mapping, you must
 * call this function before doing so.  At the next point you give the dma
 * address back to the card, you must first perform a
 * swiotlb_dma_sync_for_device, and then the device again owns the buffer
 */
static void
swiotlb_sync_single(struct device *hwdev, dma_addr_t dev_addr,
		    size_t size, enum dma_data_direction dir,
		    enum dma_sync_target target)
{
	phys_addr_t paddr = dma_to_phys(hwdev, dev_addr);

	BUG_ON(dir == DMA_NONE);

	if (is_swiotlb_buffer(paddr)) {
		swiotlb_tbl_sync_single(hwdev, paddr, size, dir, target);
		return;
	}

	if (dir != DMA_FROM_DEVICE)
		return;

	dma_mark_clean(phys_to_virt(paddr), size);
}

void
swiotlb_sync_single_for_cpu(struct device *hwdev, dma_addr_t dev_addr,
			    size_t size, enum dma_data_direction dir)
{
	swiotlb_sync_single(hwdev, dev_addr, size, dir, SYNC_FOR_CPU);
}

void
swiotlb_sync_single_for_device(struct device *hwdev, dma_addr_t dev_addr,
			       size_t size, enum dma_data_direction dir)
{
	swiotlb_sync_single(hwdev, dev_addr, size, dir, SYNC_FOR_DEVICE);
}

/*
 * Map a set of buffers described by scatterlist in streaming mode for DMA.
 * This is the scatter-gather version of the above swiotlb_map_page
 * interface.  Here the scatter gather list elements are each tagged with the
 * appropriate dma address and length.  They are obtained via
 * sg_dma_{address,length}(SG).
 *
 * NOTE: An implementation may be able to use a smaller number of
 *       DMA address/length pairs than there are SG table elements.
 *       (for example via virtual mapping capabilities)
 *       The routine returns the number of addr/length pairs actually
 *       used, at most nents.
 *
 * Device ownership issues as mentioned above for swiotlb_map_page are the
 * same here.
 */
int
swiotlb_map_sg_attrs(struct device *hwdev, struct scatterlist *sgl, int nelems,
		     enum dma_data_direction dir, unsigned long attrs)
{
	struct scatterlist *sg;
	int i;

	BUG_ON(dir == DMA_NONE);

	for_each_sg(sgl, sg, nelems, i) {
		phys_addr_t paddr = sg_phys(sg);
		dma_addr_t dev_addr = phys_to_dma(hwdev, paddr);

		if (swiotlb_force == SWIOTLB_FORCE ||
		    !dma_capable(hwdev, dev_addr, sg->length)) {
			phys_addr_t map = map_single(hwdev, sg_phys(sg),
						     sg->length, dir, attrs);
			if (map == SWIOTLB_MAP_ERROR) {
				/* Don't panic here, we expect map_sg users
				   to do proper error handling. */
				attrs |= DMA_ATTR_SKIP_CPU_SYNC;
				swiotlb_unmap_sg_attrs(hwdev, sgl, i, dir,
						       attrs);
				sg_dma_len(sgl) = 0;
				return 0;
			}
			sg->dma_address = __phys_to_dma(hwdev, map);
		} else
			sg->dma_address = dev_addr;
		sg_dma_len(sg) = sg->length;
	}
	return nelems;
}

/*
 * Unmap a set of streaming mode DMA translations.  Again, cpu read rules
 * concerning calls here are the same as for swiotlb_unmap_page() above.
 */
void
swiotlb_unmap_sg_attrs(struct device *hwdev, struct scatterlist *sgl,
		       int nelems, enum dma_data_direction dir,
		       unsigned long attrs)
{
	struct scatterlist *sg;
	int i;

	BUG_ON(dir == DMA_NONE);

	for_each_sg(sgl, sg, nelems, i)
		unmap_single(hwdev, sg->dma_address, sg_dma_len(sg), dir,
			     attrs);
}

/*
 * Make physical memory consistent for a set of streaming mode DMA translations
 * after a transfer.
 *
 * The same as swiotlb_sync_single_* but for a scatter-gather list, same rules
 * and usage.
 */
static void
swiotlb_sync_sg(struct device *hwdev, struct scatterlist *sgl,
		int nelems, enum dma_data_direction dir,
		enum dma_sync_target target)
{
	struct scatterlist *sg;
	int i;

	for_each_sg(sgl, sg, nelems, i)
		swiotlb_sync_single(hwdev, sg->dma_address,
				    sg_dma_len(sg), dir, target);
}

void
swiotlb_sync_sg_for_cpu(struct device *hwdev, struct scatterlist *sg,
			int nelems, enum dma_data_direction dir)
{
	swiotlb_sync_sg(hwdev, sg, nelems, dir, SYNC_FOR_CPU);
}

void
swiotlb_sync_sg_for_device(struct device *hwdev, struct scatterlist *sg,
			   int nelems, enum dma_data_direction dir)
{
	swiotlb_sync_sg(hwdev, sg, nelems, dir, SYNC_FOR_DEVICE);
}

int
swiotlb_dma_mapping_error(struct device *hwdev, dma_addr_t dma_addr)
{
	return (dma_addr == __phys_to_dma(hwdev, io_tlb_overflow_buffer));
}

/*
 * Return whether the given device DMA address mask can be supported
 * properly.  For example, if your device can only drive the low 24-bits
 * during bus mastering, then you would pass 0x00ffffff as the mask to
 * this function.
 */
int
swiotlb_dma_supported(struct device *hwdev, u64 mask)
{
#if defined CONFIG_BCM_KF_ARM64_BCM963XX
#if defined(CONFIG_BCM94908) && defined(CONFIG_BCM_HND_EAP)
	/*
	 * ZONE_DMA is restricted to 1GB due to a runner HW limitation. We
	 * still don't need swiotlb in this case though so check against the
	 * 4GB boundary here rather than zone_dma.
	 */
	if (!swiotlb_force &&
		(max_pfn <= ((min((memblock_start_of_DRAM() & GENMASK_ULL(63, 32))
				  + (1ULL << 32),
				memblock_end_of_DRAM())) >> PAGE_SHIFT)))
		return 1;
#elif defined CONFIG_ARM64
	if(!swiotlb_force && (max_pfn <= (arm64_dma_phys_limit >> PAGE_SHIFT)))
		return 1;
#endif
#endif
	return __phys_to_dma(hwdev, io_tlb_end - 1) <= mask;
}

void *swiotlb_alloc(struct device *dev, size_t size, dma_addr_t *dma_handle,
		gfp_t gfp, unsigned long attrs)
{
	void *vaddr;

	/* temporary workaround: */
	if (gfp & __GFP_NOWARN)
		attrs |= DMA_ATTR_NO_WARN;

	/*
	 * Don't print a warning when the first allocation attempt fails.
	 * swiotlb_alloc_coherent() will print a warning when the DMA memory
	 * allocation ultimately failed.
	 */
	gfp |= __GFP_NOWARN;

	vaddr = dma_direct_alloc(dev, size, dma_handle, gfp, attrs);
	if (!vaddr)
		vaddr = swiotlb_alloc_buffer(dev, size, dma_handle, attrs);
	return vaddr;
}

void swiotlb_free(struct device *dev, size_t size, void *vaddr,
		dma_addr_t dma_addr, unsigned long attrs)
{
	if (!swiotlb_free_buffer(dev, size, dma_addr))
		dma_direct_free(dev, size, vaddr, dma_addr, attrs);
}

const struct dma_map_ops swiotlb_dma_ops = {
	.mapping_error		= swiotlb_dma_mapping_error,
	.alloc			= swiotlb_alloc,
	.free			= swiotlb_free,
	.sync_single_for_cpu	= swiotlb_sync_single_for_cpu,
	.sync_single_for_device	= swiotlb_sync_single_for_device,
	.sync_sg_for_cpu	= swiotlb_sync_sg_for_cpu,
	.sync_sg_for_device	= swiotlb_sync_sg_for_device,
	.map_sg			= swiotlb_map_sg_attrs,
	.unmap_sg		= swiotlb_unmap_sg_attrs,
	.map_page		= swiotlb_map_page,
	.unmap_page		= swiotlb_unmap_page,
	.dma_supported		= dma_direct_supported,
};
EXPORT_SYMBOL(swiotlb_dma_ops);
