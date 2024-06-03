// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application memory management
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <efi_loader.h>
#include <malloc.h>
#include <mapmem.h>
#include <watchdog.h>
#include <linux/list_sort.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/* Magic number identifying memory allocated from pool */
#define EFI_ALLOC_POOL_MAGIC 0x1fe67ddf6491caa2

efi_uintn_t efi_memory_map_key;

struct efi_mem_list {
	struct list_head link;
	struct efi_mem_desc desc;
};

#define EFI_CARVE_NO_OVERLAP		-1
#define EFI_CARVE_LOOP_AGAIN		-2
#define EFI_CARVE_OVERLAPS_NONRAM	-3

/* This list contains all memory map items */
LIST_HEAD(efi_mem);

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
void *efi_bounce_buffer;
#endif

/**
 * efi_pool_allocation - memory block allocated from pool
 *
 * @num_pages:	number of pages allocated
 * @checksum:	checksum
 *
 * U-Boot services each EFI AllocatePool request as a separate
 * (multiple) page allocation.  We have to track the number of pages
 * to be able to free the correct amount later.
 * EFI requires 8 byte alignment for pool allocations, so we can
 * prepend each allocation with an 64 bit header tracking the
 * allocation size, and hand out the remainder to the caller.
 */
struct efi_pool_allocation {
	u64 num_pages;
	u64 checksum;
	char data[] __aligned(ARCH_DMA_MINALIGN);
};

/**
 * checksum() - calculate checksum for memory allocated from pool
 *
 * @alloc:	allocation header
 * Return:	checksum, always non-zero
 */
static u64 checksum(struct efi_pool_allocation *alloc)
{
	u64 addr = (uintptr_t)alloc;
	u64 ret = (addr >> 32) ^ (addr << 32) ^ alloc->num_pages ^
		  EFI_ALLOC_POOL_MAGIC;
	if (!ret)
		++ret;
	return ret;
}

/*
 * Sorts the memory list from highest address to lowest address
 *
 * When allocating memory we should always start from the highest
 * address chunk, so sort the memory list such that the first list
 * iterator gets the highest address and goes lower from there.
 */
static int efi_mem_cmp(void *priv, struct list_head *a, struct list_head *b)
{
	struct efi_mem_list *mema = list_entry(a, struct efi_mem_list, link);
	struct efi_mem_list *memb = list_entry(b, struct efi_mem_list, link);

	if (mema->desc.physical_start == memb->desc.physical_start)
		return 0;
	else if (mema->desc.physical_start < memb->desc.physical_start)
		return 1;
	else
		return -1;
}

static uint64_t desc_get_end(struct efi_mem_desc *desc)
{
	return desc->physical_start + (desc->num_pages << EFI_PAGE_SHIFT);
}

static void efi_mem_sort(void)
{
	struct list_head *lhandle;
	struct efi_mem_list *prevmem = NULL;
	bool merge_again = true;

	list_sort(NULL, &efi_mem, efi_mem_cmp);

	/* Now merge entries that can be merged */
	while (merge_again) {
		merge_again = false;
		list_for_each(lhandle, &efi_mem) {
			struct efi_mem_list *lmem;
			struct efi_mem_desc *prev = &prevmem->desc;
			struct efi_mem_desc *cur;
			uint64_t pages;

			lmem = list_entry(lhandle, struct efi_mem_list, link);
			if (!prevmem) {
				prevmem = lmem;
				continue;
			}

			cur = &lmem->desc;

			if ((desc_get_end(cur) == prev->physical_start) &&
			    (prev->type == cur->type) &&
			    (prev->attribute == cur->attribute)) {
				/* There is an existing map before, reuse it */
				pages = cur->num_pages;
				prev->num_pages += pages;
				prev->physical_start -= pages << EFI_PAGE_SHIFT;
				prev->virtual_start -= pages << EFI_PAGE_SHIFT;
				list_del(&lmem->link);
				free(lmem);

				merge_again = true;
				break;
			}

			prevmem = lmem;
		}
	}
}

/** efi_mem_carve_out - unmap memory region
 *
 * @map:		memory map
 * @carve_desc:		memory region to unmap
 * @overlap_only_ram:	the carved out region may only overlap RAM
 * Return Value:	the number of overlapping pages which have been
 *			removed from the map,
 *			EFI_CARVE_NO_OVERLAP, if the regions don't overlap,
 *			EFI_CARVE_OVERLAPS_NONRAM, if the carve and map overlap,
 *			and the map contains anything but free ram
 *			(only when overlap_only_ram is true),
 *			EFI_CARVE_LOOP_AGAIN, if the mapping list should be
 *			traversed again, as it has been altered.
 *
 * Unmaps all memory occupied by the carve_desc region from the list entry
 * pointed to by map.
 *
 * In case of EFI_CARVE_OVERLAPS_NONRAM it is the callers responsibility
 * to re-add the already carved out pages to the mapping.
 */
static s64 efi_mem_carve_out(struct efi_mem_list *map,
			     struct efi_mem_desc *carve_desc,
			     bool overlap_only_ram)
{
	struct efi_mem_list *newmap;
	struct efi_mem_desc *map_desc = &map->desc;
	uint64_t map_start = map_desc->physical_start;
	uint64_t map_end = map_start + (map_desc->num_pages << EFI_PAGE_SHIFT);
	uint64_t carve_start = carve_desc->physical_start;
	uint64_t carve_end = carve_start +
			     (carve_desc->num_pages << EFI_PAGE_SHIFT);

	/* check whether we're overlapping */
	if ((carve_end <= map_start) || (carve_start >= map_end))
		return EFI_CARVE_NO_OVERLAP;

	/* We're overlapping with non-RAM, warn the caller if desired */
	if (overlap_only_ram && (map_desc->type != EFI_CONVENTIONAL_MEMORY))
		return EFI_CARVE_OVERLAPS_NONRAM;

	/* Sanitize carve_start and carve_end to lie within our bounds */
	carve_start = max(carve_start, map_start);
	carve_end = min(carve_end, map_end);

	/* Carving at the beginning of our map? Just move it! */
	if (carve_start == map_start) {
		if (map_end == carve_end) {
			/* Full overlap, just remove map */
			list_del(&map->link);
			free(map);
		} else {
			map->desc.physical_start = carve_end;
			map->desc.virtual_start = carve_end;
			map->desc.num_pages = (map_end - carve_end)
					      >> EFI_PAGE_SHIFT;
		}

		return (carve_end - carve_start) >> EFI_PAGE_SHIFT;
	}

	/*
	 * Overlapping maps, just split the list map at carve_start,
	 * it will get moved or removed in the next iteration.
	 *
	 * [ map_desc |__carve_start__| newmap ]
	 */

	/* Create a new map from [ carve_start ... map_end ] */
	newmap = calloc(1, sizeof(*newmap));
	newmap->desc = map->desc;
	newmap->desc.physical_start = carve_start;
	newmap->desc.virtual_start = carve_start;
	newmap->desc.num_pages = (map_end - carve_start) >> EFI_PAGE_SHIFT;
	/* Insert before current entry (descending address order) */
	list_add_tail(&newmap->link, &map->link);

	/* Shrink the map to [ map_start ... carve_start ] */
	map_desc->num_pages = (carve_start - map_start) >> EFI_PAGE_SHIFT;

	return EFI_CARVE_LOOP_AGAIN;
}

uint64_t efi_add_memory_map(uint64_t start, uint64_t pages, int memory_type,
			    bool overlap_only_ram)
{
	struct list_head *lhandle;
	struct efi_mem_list *newlist;
	bool carve_again;
	uint64_t carved_pages = 0;
	struct efi_event *evt;

	EFI_PRINT("%s: 0x%llx 0x%llx %d %s\n", __func__,
		  start, pages, memory_type, overlap_only_ram ? "yes" : "no");

	if (memory_type >= EFI_MAX_MEMORY_TYPE)
		return EFI_INVALID_PARAMETER;

	if (!pages)
		return start;

	++efi_memory_map_key;
	newlist = calloc(1, sizeof(*newlist));
	newlist->desc.type = memory_type;
	newlist->desc.physical_start = start;
	newlist->desc.virtual_start = start;
	newlist->desc.num_pages = pages;

	switch (memory_type) {
	case EFI_RUNTIME_SERVICES_CODE:
	case EFI_RUNTIME_SERVICES_DATA:
		newlist->desc.attribute = EFI_MEMORY_WB | EFI_MEMORY_RUNTIME;
		break;
	case EFI_MMAP_IO:
		newlist->desc.attribute = EFI_MEMORY_RUNTIME;
		break;
	default:
		newlist->desc.attribute = EFI_MEMORY_WB;
		break;
	}

	/* Add our new map */
	do {
		carve_again = false;
		list_for_each(lhandle, &efi_mem) {
			struct efi_mem_list *lmem;
			s64 r;

			lmem = list_entry(lhandle, struct efi_mem_list, link);
			r = efi_mem_carve_out(lmem, &newlist->desc,
					      overlap_only_ram);
			switch (r) {
			case EFI_CARVE_OVERLAPS_NONRAM:
				/*
				 * The user requested to only have RAM overlaps,
				 * but we hit a non-RAM region. Error out.
				 */
				return 0;
			case EFI_CARVE_NO_OVERLAP:
				/* Just ignore this list entry */
				break;
			case EFI_CARVE_LOOP_AGAIN:
				/*
				 * We split an entry, but need to loop through
				 * the list again to actually carve it.
				 */
				carve_again = true;
				break;
			default:
				/* We carved a number of pages */
				carved_pages += r;
				carve_again = true;
				break;
			}

			if (carve_again) {
				/* The list changed, we need to start over */
				break;
			}
		}
	} while (carve_again);

	if (overlap_only_ram && (carved_pages != pages)) {
		/*
		 * The payload wanted to have RAM overlaps, but we overlapped
		 * with an unallocated region. Error out.
		 */
		return 0;
	}

	/* Add our new map */
        list_add_tail(&newlist->link, &efi_mem);

	/* And make sure memory is listed in descending order */
	efi_mem_sort();

	/* Notify that the memory map was changed */
	list_for_each_entry(evt, &efi_events, link) {
		if (evt->group &&
		    !guidcmp(evt->group,
			     &efi_guid_event_group_memory_map_change)) {
			efi_signal_event(evt);
			break;
		}
	}

	return start;
}

/**
 * efi_check_allocated() - validate address to be freed
 *
 * Check that the address is within allocated memory:
 *
 * * The address must be in a range of the memory map.
 * * The address may not point to EFI_CONVENTIONAL_MEMORY.
 *
 * Page alignment is not checked as this is not a requirement of
 * efi_free_pool().
 *
 * @addr:		address of page to be freed
 * @must_be_allocated:	return success if the page is allocated
 * Return:		status code
 */
static efi_status_t efi_check_allocated(u64 addr, bool must_be_allocated)
{
	struct efi_mem_list *item;

	list_for_each_entry(item, &efi_mem, link) {
		u64 start = item->desc.physical_start;
		u64 end = start + (item->desc.num_pages << EFI_PAGE_SHIFT);

		if (addr >= start && addr < end) {
			if (must_be_allocated ^
			    (item->desc.type == EFI_CONVENTIONAL_MEMORY))
				return EFI_SUCCESS;
			else
				return EFI_NOT_FOUND;
		}
	}

	return EFI_NOT_FOUND;
}

static uint64_t efi_find_free_memory(uint64_t len, uint64_t max_addr)
{
	struct list_head *lhandle;

	/*
	 * Prealign input max address, so we simplify our matching
	 * logic below and can just reuse it as return pointer.
	 */
	max_addr &= ~EFI_PAGE_MASK;

	list_for_each(lhandle, &efi_mem) {
		struct efi_mem_list *lmem = list_entry(lhandle,
			struct efi_mem_list, link);
		struct efi_mem_desc *desc = &lmem->desc;
		uint64_t desc_len = desc->num_pages << EFI_PAGE_SHIFT;
		uint64_t desc_end = desc->physical_start + desc_len;
		uint64_t curmax = min(max_addr, desc_end);
		uint64_t ret = curmax - len;

		/* We only take memory from free RAM */
		if (desc->type != EFI_CONVENTIONAL_MEMORY)
			continue;

		/* Out of bounds for max_addr */
		if ((ret + len) > max_addr)
			continue;

		/* Out of bounds for upper map limit */
		if ((ret + len) > desc_end)
			continue;

		/* Out of bounds for lower map limit */
		if (ret < desc->physical_start)
			continue;

		/* Return the highest address in this map within bounds */
		return ret;
	}

	return 0;
}

/*
 * Allocate memory pages.
 *
 * @type		type of allocation to be performed
 * @memory_type		usage type of the allocated memory
 * @pages		number of pages to be allocated
 * @memory		allocated memory
 * @return		status code
 */
efi_status_t efi_allocate_pages(int type, int memory_type,
				efi_uintn_t pages, uint64_t *memory)
{
	u64 len = pages << EFI_PAGE_SHIFT;
	efi_status_t ret;
	uint64_t addr;

	/* Check import parameters */
	if (memory_type >= EFI_PERSISTENT_MEMORY_TYPE &&
	    memory_type <= 0x6FFFFFFF)
		return EFI_INVALID_PARAMETER;
	if (!memory)
		return EFI_INVALID_PARAMETER;

	switch (type) {
	case EFI_ALLOCATE_ANY_PAGES:
		/* Any page */
		addr = efi_find_free_memory(len, -1ULL);
		if (!addr)
			return EFI_OUT_OF_RESOURCES;
		break;
	case EFI_ALLOCATE_MAX_ADDRESS:
		/* Max address */
		addr = efi_find_free_memory(len, *memory);
		if (!addr)
			return EFI_OUT_OF_RESOURCES;
		break;
	case EFI_ALLOCATE_ADDRESS:
		/* Exact address, reserve it. The addr is already in *memory. */
		ret = efi_check_allocated(*memory, false);
		if (ret != EFI_SUCCESS)
			return EFI_NOT_FOUND;
		addr = *memory;
		break;
	default:
		/* UEFI doesn't specify other allocation types */
		return EFI_INVALID_PARAMETER;
	}

	/* Reserve that map in our memory maps */
	if (efi_add_memory_map(addr, pages, memory_type, true) != addr)
		/* Map would overlap, bail out */
		return  EFI_OUT_OF_RESOURCES;

	*memory = addr;

	return EFI_SUCCESS;
}

void *efi_alloc(uint64_t len, int memory_type)
{
	uint64_t ret = 0;
	uint64_t pages = efi_size_in_pages(len);
	efi_status_t r;

	r = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES, memory_type, pages,
			       &ret);
	if (r == EFI_SUCCESS)
		return (void*)(uintptr_t)ret;

	return NULL;
}

/**
 * efi_free_pages() - free memory pages
 *
 * @memory:	start of the memory area to be freed
 * @pages:	number of pages to be freed
 * Return:	status code
 */
efi_status_t efi_free_pages(uint64_t memory, efi_uintn_t pages)
{
	uint64_t r = 0;
	efi_status_t ret;

	ret = efi_check_allocated(memory, true);
	if (ret != EFI_SUCCESS)
		return ret;

	/* Sanity check */
	if (!memory || (memory & EFI_PAGE_MASK) || !pages) {
		printf("%s: illegal free 0x%llx, 0x%zx\n", __func__,
		       memory, pages);
		return EFI_INVALID_PARAMETER;
	}

	r = efi_add_memory_map(memory, pages, EFI_CONVENTIONAL_MEMORY, false);
	/* Merging of adjacent free regions is missing */

	if (r == memory)
		return EFI_SUCCESS;

	return EFI_NOT_FOUND;
}

/**
 * efi_allocate_pool - allocate memory from pool
 *
 * @pool_type:	type of the pool from which memory is to be allocated
 * @size:	number of bytes to be allocated
 * @buffer:	allocated memory
 * Return:	status code
 */
efi_status_t efi_allocate_pool(int pool_type, efi_uintn_t size, void **buffer)
{
	efi_status_t r;
	u64 addr;
	struct efi_pool_allocation *alloc;
	u64 num_pages = efi_size_in_pages(size +
					  sizeof(struct efi_pool_allocation));

	if (!buffer)
		return EFI_INVALID_PARAMETER;

	if (size == 0) {
		*buffer = NULL;
		return EFI_SUCCESS;
	}

	r = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES, pool_type, num_pages,
			       &addr);
	if (r == EFI_SUCCESS) {
		alloc = (struct efi_pool_allocation *)(uintptr_t)addr;
		alloc->num_pages = num_pages;
		alloc->checksum = checksum(alloc);
		*buffer = alloc->data;
	}

	return r;
}

/**
 * efi_free_pool() - free memory from pool
 *
 * @buffer:	start of memory to be freed
 * Return:	status code
 */
efi_status_t efi_free_pool(void *buffer)
{
	efi_status_t ret;
	struct efi_pool_allocation *alloc;

	if (!buffer)
		return EFI_INVALID_PARAMETER;

	ret = efi_check_allocated((uintptr_t)buffer, true);
	if (ret != EFI_SUCCESS)
		return ret;

	alloc = container_of(buffer, struct efi_pool_allocation, data);

	/* Check that this memory was allocated by efi_allocate_pool() */
	if (((uintptr_t)alloc & EFI_PAGE_MASK) ||
	    alloc->checksum != checksum(alloc)) {
		printf("%s: illegal free 0x%p\n", __func__, buffer);
		return EFI_INVALID_PARAMETER;
	}
	/* Avoid double free */
	alloc->checksum = 0;

	ret = efi_free_pages((uintptr_t)alloc, alloc->num_pages);

	return ret;
}

/*
 * Get map describing memory usage.
 *
 * @memory_map_size	on entry the size, in bytes, of the memory map buffer,
 *			on exit the size of the copied memory map
 * @memory_map		buffer to which the memory map is written
 * @map_key		key for the memory map
 * @descriptor_size	size of an individual memory descriptor
 * @descriptor_version	version number of the memory descriptor structure
 * @return		status code
 */
efi_status_t efi_get_memory_map(efi_uintn_t *memory_map_size,
				struct efi_mem_desc *memory_map,
				efi_uintn_t *map_key,
				efi_uintn_t *descriptor_size,
				uint32_t *descriptor_version)
{
	efi_uintn_t map_size = 0;
	int map_entries = 0;
	struct list_head *lhandle;
	efi_uintn_t provided_map_size;

	if (!memory_map_size)
		return EFI_INVALID_PARAMETER;

	provided_map_size = *memory_map_size;

	list_for_each(lhandle, &efi_mem)
		map_entries++;

	map_size = map_entries * sizeof(struct efi_mem_desc);

	*memory_map_size = map_size;

	if (provided_map_size < map_size)
		return EFI_BUFFER_TOO_SMALL;

	if (!memory_map)
		return EFI_INVALID_PARAMETER;

	if (descriptor_size)
		*descriptor_size = sizeof(struct efi_mem_desc);

	if (descriptor_version)
		*descriptor_version = EFI_MEMORY_DESCRIPTOR_VERSION;

	/* Copy list into array */
	/* Return the list in ascending order */
	memory_map = &memory_map[map_entries - 1];
	list_for_each(lhandle, &efi_mem) {
		struct efi_mem_list *lmem;

		lmem = list_entry(lhandle, struct efi_mem_list, link);
		*memory_map = lmem->desc;
		memory_map--;
	}

	if (map_key)
		*map_key = efi_memory_map_key;

	return EFI_SUCCESS;
}

__weak void efi_add_known_memory(void)
{
	u64 ram_top = board_get_usable_ram_top(0) & ~EFI_PAGE_MASK;
	int i;

	/*
	 * ram_top is just outside mapped memory. So use an offset of one for
	 * mapping the sandbox address.
	 */
	ram_top = (uintptr_t)map_sysmem(ram_top - 1, 0) + 1;

	/* Fix for 32bit targets with ram_top at 4G */
	if (!ram_top)
		ram_top = 0x100000000ULL;

	/* Add RAM */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		u64 ram_end, ram_start, pages;

		ram_start = (uintptr_t)map_sysmem(gd->bd->bi_dram[i].start, 0);
		ram_end = ram_start + gd->bd->bi_dram[i].size;

		/* Remove partial pages */
		ram_end &= ~EFI_PAGE_MASK;
		ram_start = (ram_start + EFI_PAGE_MASK) & ~EFI_PAGE_MASK;

		if (ram_end <= ram_start) {
			/* Invalid mapping, keep going. */
			continue;
		}

		pages = (ram_end - ram_start) >> EFI_PAGE_SHIFT;

		efi_add_memory_map(ram_start, pages,
				   EFI_CONVENTIONAL_MEMORY, false);

		/*
		 * Boards may indicate to the U-Boot memory core that they
		 * can not support memory above ram_top. Let's honor this
		 * in the efi_loader subsystem too by declaring any memory
		 * above ram_top as "already occupied by firmware".
		 */
		if (ram_top < ram_start) {
			/* ram_top is before this region, reserve all */
			efi_add_memory_map(ram_start, pages,
					   EFI_BOOT_SERVICES_DATA, true);
		} else if ((ram_top >= ram_start) && (ram_top < ram_end)) {
			/* ram_top is inside this region, reserve parts */
			pages = (ram_end - ram_top) >> EFI_PAGE_SHIFT;

			efi_add_memory_map(ram_top, pages,
					   EFI_BOOT_SERVICES_DATA, true);
		}
	}
}

/* Add memory regions for U-Boot's memory and for the runtime services code */
static void add_u_boot_and_runtime(void)
{
	unsigned long runtime_start, runtime_end, runtime_pages;
	unsigned long runtime_mask = EFI_PAGE_MASK;
	unsigned long uboot_start, uboot_pages;
	unsigned long uboot_stack_size = 16 * 1024 * 1024;

	/* Add U-Boot */
	uboot_start = (gd->start_addr_sp - uboot_stack_size) & ~EFI_PAGE_MASK;
	uboot_pages = (gd->ram_top - uboot_start) >> EFI_PAGE_SHIFT;
	efi_add_memory_map(uboot_start, uboot_pages, EFI_LOADER_DATA, false);

#if defined(__aarch64__)
	/*
	 * Runtime Services must be 64KiB aligned according to the
	 * "AArch64 Platforms" section in the UEFI spec (2.7+).
	 */

	runtime_mask = SZ_64K - 1;
#endif

	/*
	 * Add Runtime Services. We mark surrounding boottime code as runtime as
	 * well to fulfill the runtime alignment constraints but avoid padding.
	 */
	runtime_start = (ulong)&__efi_runtime_start & ~runtime_mask;
	runtime_end = (ulong)&__efi_runtime_stop;
	runtime_end = (runtime_end + runtime_mask) & ~runtime_mask;
	runtime_pages = (runtime_end - runtime_start) >> EFI_PAGE_SHIFT;
	efi_add_memory_map(runtime_start, runtime_pages,
			   EFI_RUNTIME_SERVICES_CODE, false);
}

int efi_memory_init(void)
{
	efi_add_known_memory();

	if (!IS_ENABLED(CONFIG_SANDBOX))
		add_u_boot_and_runtime();

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
	/* Request a 32bit 64MB bounce buffer region */
	uint64_t efi_bounce_buffer_addr = 0xffffffff;

	if (efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS, EFI_LOADER_DATA,
			       (64 * 1024 * 1024) >> EFI_PAGE_SHIFT,
			       &efi_bounce_buffer_addr) != EFI_SUCCESS)
		return -1;

	efi_bounce_buffer = (void*)(uintptr_t)efi_bounce_buffer_addr;
#endif

	return 0;
}
