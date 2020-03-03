/*
 * Copyright (C) 2000 - 2007 Jeff Dike (jdike@{addtoit,linux.intel}.com)
 * Licensed under the GPL
 */

#include <linux/module.h>
#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/pfn.h>
#include <asm/page.h>
#include <as-layout.h>
#include <init.h>
#include <kern.h>
#include <mem_user.h>
#include <os.h>

static int physmem_fd = -1;

/* Changed during early boot */
unsigned long high_physmem;
EXPORT_SYMBOL(high_physmem);

extern unsigned long long physmem_size;

void __init mem_total_pages(unsigned long physmem, unsigned long iomem,
		     unsigned long highmem)
{
	unsigned long phys_pages, highmem_pages;
	unsigned long iomem_pages, total_pages;

	phys_pages    = physmem >> PAGE_SHIFT;
	iomem_pages   = iomem   >> PAGE_SHIFT;
	highmem_pages = highmem >> PAGE_SHIFT;

	total_pages   = phys_pages + iomem_pages + highmem_pages;

	max_mapnr = total_pages;
}

void map_memory(unsigned long virt, unsigned long phys, unsigned long len,
		int r, int w, int x)
{
	__u64 offset;
	int fd, err;

	fd = phys_mapping(phys, &offset);
	err = os_map_memory((void *) virt, fd, offset, len, r, w, x);
	if (err) {
		if (err == -ENOMEM)
			printk(KERN_ERR "try increasing the host's "
			       "/proc/sys/vm/max_map_count to <physical "
			       "memory size>/4096\n");
		panic("map_memory(0x%lx, %d, 0x%llx, %ld, %d, %d, %d) failed, "
		      "err = %d\n", virt, fd, offset, len, r, w, x, err);
	}
}

extern int __syscall_stub_start;

/**
 * setup_physmem() - Setup physical memory for UML
 * @start:	Start address of the physical kernel memory,
 *		i.e start address of the executable image.
 * @reserve_end:	end address of the physical kernel memory.
 * @len:	Length of total physical memory that should be mapped/made
 *		available, in bytes.
 * @highmem:	Number of highmem bytes that should be mapped/made available.
 *
 * Creates an unlinked temporary file of size (len + highmem) and memory maps
 * it on the last executable image address (uml_reserved).
 *
 * The offset is needed as the length of the total physical memory
 * (len + highmem) includes the size of the memory used be the executable image,
 * but the mapped-to address is the last address of the executable image
 * (uml_reserved == end address of executable image).
 *
 * The memory mapped memory of the temporary file is used as backing memory
 * of all user space processes/kernel tasks.
 */
void __init setup_physmem(unsigned long start, unsigned long reserve_end,
			  unsigned long len, unsigned long long highmem)
{
	unsigned long reserve = reserve_end - start;
	unsigned long pfn = PFN_UP(__pa(reserve_end));
	unsigned long delta = (len - reserve) >> PAGE_SHIFT;
	unsigned long offset, bootmap_size;
	long map_size;
	int err;

	offset = uml_reserved - uml_physmem;
	map_size = len - offset;
	if(map_size <= 0) {
		printf("Too few physical memory! Needed=%d, given=%d\n",
		       offset, len);
		exit(1);
	}

	physmem_fd = create_mem_file(len + highmem);

	err = os_map_memory((void *) uml_reserved, physmem_fd, offset,
			    map_size, 1, 1, 1);
	if (err < 0) {
		printf("setup_physmem - mapping %ld bytes of memory at 0x%p "
		       "failed - errno = %d\n", map_size,
		       (void *) uml_reserved, err);
		exit(1);
	}

	/*
	 * Special kludge - This page will be mapped in to userspace processes
	 * from physmem_fd, so it needs to be written out there.
	 */
	os_seek_file(physmem_fd, __pa(&__syscall_stub_start));
	os_write_file(physmem_fd, &__syscall_stub_start, PAGE_SIZE);
	os_fsync_file(physmem_fd);

	bootmap_size = init_bootmem(pfn, pfn + delta);
	free_bootmem(__pa(reserve_end) + bootmap_size,
		     len - bootmap_size - reserve);
}

int phys_mapping(unsigned long phys, unsigned long long *offset_out)
{
	int fd = -1;

	if (phys < physmem_size) {
		fd = physmem_fd;
		*offset_out = phys;
	}
	else if (phys < __pa(end_iomem)) {
		struct iomem_region *region = iomem_regions;

		while (region != NULL) {
			if ((phys >= region->phys) &&
			    (phys < region->phys + region->size)) {
				fd = region->fd;
				*offset_out = phys - region->phys;
				break;
			}
			region = region->next;
		}
	}
	else if (phys < __pa(end_iomem) + highmem) {
		fd = physmem_fd;
		*offset_out = phys - iomem_size;
	}

	return fd;
}

static int __init uml_mem_setup(char *line, int *add)
{
	char *retptr;
	physmem_size = memparse(line,&retptr);
	return 0;
}
__uml_setup("mem=", uml_mem_setup,
"mem=<Amount of desired ram>\n"
"    This controls how much \"physical\" memory the kernel allocates\n"
"    for the system. The size is specified as a number followed by\n"
"    one of 'k', 'K', 'm', 'M', which have the obvious meanings.\n"
"    This is not related to the amount of memory in the host.  It can\n"
"    be more, and the excess, if it's ever used, will just be swapped out.\n"
"	Example: mem=64M\n\n"
);

extern int __init parse_iomem(char *str, int *add);

__uml_setup("iomem=", parse_iomem,
"iomem=<name>,<file>\n"
"    Configure <file> as an IO memory region named <name>.\n\n"
);

/*
 * This list is constructed in parse_iomem and addresses filled in in
 * setup_iomem, both of which run during early boot.  Afterwards, it's
 * unchanged.
 */
struct iomem_region *iomem_regions;

/* Initialized in parse_iomem and unchanged thereafter */
int iomem_size;

unsigned long find_iomem(char *driver, unsigned long *len_out)
{
	struct iomem_region *region = iomem_regions;

	while (region != NULL) {
		if (!strcmp(region->driver, driver)) {
			*len_out = region->size;
			return region->virt;
		}

		region = region->next;
	}

	return 0;
}
EXPORT_SYMBOL(find_iomem);

static int setup_iomem(void)
{
	struct iomem_region *region = iomem_regions;
	unsigned long iomem_start = high_physmem + PAGE_SIZE;
	int err;

	while (region != NULL) {
		err = os_map_memory((void *) iomem_start, region->fd, 0,
				    region->size, 1, 1, 0);
		if (err)
			printk(KERN_ERR "Mapping iomem region for driver '%s' "
			       "failed, errno = %d\n", region->driver, -err);
		else {
			region->virt = iomem_start;
			region->phys = __pa(region->virt);
		}

		iomem_start += region->size + PAGE_SIZE;
		region = region->next;
	}

	return 0;
}

__initcall(setup_iomem);
