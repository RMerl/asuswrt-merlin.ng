// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <os.h>
#include <asm/io.h>
#include <asm/setjmp.h>
#include <asm/state.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

/* Enable access to PCI memory with map_sysmem() */
static bool enable_pci_map;

#ifdef CONFIG_PCI
/* Last device that was mapped into memory, and length of mapping */
static struct udevice *map_dev;
unsigned long map_len;
#endif

void sandbox_exit(void)
{
	/* Do this here while it still has an effect */
	os_fd_restore();
	if (state_uninit())
		os_exit(2);

	if (dm_uninit())
		os_exit(2);

	/* This is considered normal termination for now */
	os_exit(0);
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	struct sandbox_state *state = state_get_current();

	if (!state->skip_delays)
		os_usleep(usec);
}

int cleanup_before_linux(void)
{
	return 0;
}

int cleanup_before_linux_select(int flags)
{
	return 0;
}

/**
 * is_in_sandbox_mem() - Checks if a pointer is within sandbox's emulated DRAM
 *
 * This provides a way to check if a pointer is owned by sandbox (and is within
 * its RAM) or not. Sometimes pointers come from a test which conceptually runs
 * output sandbox, potentially with direct access to the C-library malloc()
 * function, or the sandbox stack (which is not actually within the emulated
 * DRAM.
 *
 * Such pointers obviously cannot be mapped into sandbox's DRAM, so we must
 * detect them an process them separately, by recording a mapping to a tag,
 * which we can use to map back to the pointer later.
 *
 * @ptr: Pointer to check
 * @return true if this is within sandbox emulated DRAM, false if not
 */
static bool is_in_sandbox_mem(const void *ptr)
{
	return (const uint8_t *)ptr >= gd->arch.ram_buf &&
		(const uint8_t *)ptr < gd->arch.ram_buf + gd->ram_size;
}

/**
 * phys_to_virt() - Converts a sandbox RAM address to a pointer
 *
 * Sandbox uses U-Boot addresses from 0 to the size of DRAM. These index into
 * the emulated DRAM buffer used by sandbox. This function converts such an
 * address to a pointer into this buffer, which can be used to access the
 * memory.
 *
 * If the address is outside this range, it is assumed to be a tag
 */
void *phys_to_virt(phys_addr_t paddr)
{
	struct sandbox_mapmem_entry *mentry;
	struct sandbox_state *state;

	/* If the address is within emulated DRAM, calculate the value */
	if (paddr < gd->ram_size)
		return (void *)(gd->arch.ram_buf + paddr);

	/*
	 * Otherwise search out list of tags for the correct pointer previously
	 * created by map_to_sysmem()
	 */
	state = state_get_current();
	list_for_each_entry(mentry, &state->mapmem_head, sibling_node) {
		if (mentry->tag == paddr) {
			debug("%s: Used map from %lx to %p\n", __func__,
			      (ulong)paddr, mentry->ptr);
			return mentry->ptr;
		}
	}

	printf("%s: Cannot map sandbox address %lx (SDRAM from 0 to %lx)\n",
	       __func__, (ulong)paddr, (ulong)gd->ram_size);
	os_abort();

	/* Not reached */
	return NULL;
}

struct sandbox_mapmem_entry *find_tag(const void *ptr)
{
	struct sandbox_mapmem_entry *mentry;
	struct sandbox_state *state = state_get_current();

	list_for_each_entry(mentry, &state->mapmem_head, sibling_node) {
		if (mentry->ptr == ptr) {
			debug("%s: Used map from %p to %lx\n", __func__, ptr,
			      mentry->tag);
			return mentry;
		}
	}
	return NULL;
}

phys_addr_t virt_to_phys(void *ptr)
{
	struct sandbox_mapmem_entry *mentry;

	/*
	 * If it is in emulated RAM, don't bother looking for a tag. Just
	 * calculate the pointer using the provides offset into the RAM buffer.
	 */
	if (is_in_sandbox_mem(ptr))
		return (phys_addr_t)((uint8_t *)ptr - gd->arch.ram_buf);

	mentry = find_tag(ptr);
	if (!mentry) {
		/* Abort so that gdb can be used here */
		printf("%s: Cannot map sandbox address %p (SDRAM from 0 to %lx)\n",
		       __func__, ptr, (ulong)gd->ram_size);
		os_abort();
	}
	debug("%s: Used map from %p to %lx\n", __func__, ptr, mentry->tag);

	return mentry->tag;
}

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
#if defined(CONFIG_PCI) && !defined(CONFIG_SPL_BUILD)
	unsigned long plen = len;
	void *ptr;

	map_dev = NULL;
	if (enable_pci_map && !pci_map_physmem(paddr, &len, &map_dev, &ptr)) {
		if (plen != len) {
			printf("%s: Warning: partial map at %x, wanted %lx, got %lx\n",
			       __func__, (uint)paddr, len, plen);
		}
		map_len = len;
		return ptr;
	}
#endif

	return phys_to_virt(paddr);
}

void unmap_physmem(const void *ptr, unsigned long flags)
{
#ifdef CONFIG_PCI
	if (map_dev) {
		pci_unmap_physmem(ptr, map_len, map_dev);
		map_dev = NULL;
	}
#endif
}

phys_addr_t map_to_sysmem(const void *ptr)
{
	struct sandbox_mapmem_entry *mentry;

	/*
	 * If it is in emulated RAM, don't bother creating a tag. Just return
	 * the offset into the RAM buffer.
	 */
	if (is_in_sandbox_mem(ptr))
		return (u8 *)ptr - gd->arch.ram_buf;

	/*
	 * See if there is an existing tag with this pointer. If not, set up a
	 * new one.
	 */
	mentry = find_tag(ptr);
	if (!mentry) {
		struct sandbox_state *state = state_get_current();

		mentry = malloc(sizeof(*mentry));
		if (!mentry) {
			printf("%s: Error: Out of memory\n", __func__);
			os_exit(ENOMEM);
		}
		mentry->tag = state->next_tag++;
		mentry->ptr = (void *)ptr;
		list_add_tail(&mentry->sibling_node, &state->mapmem_head);
		debug("%s: Added map from %p to %lx\n", __func__, ptr,
		      (ulong)mentry->tag);
	}

	/*
	 * Return the tag as the address to use. A later call to map_sysmem()
	 * will return ptr
	 */
	return mentry->tag;
}

void sandbox_set_enable_pci_map(int enable)
{
	enable_pci_map = enable;
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
}

int sandbox_read_fdt_from_file(void)
{
	struct sandbox_state *state = state_get_current();
	const char *fname = state->fdt_fname;
	void *blob;
	loff_t size;
	int err;
	int fd;

	blob = map_sysmem(CONFIG_SYS_FDT_LOAD_ADDR, 0);
	if (!state->fdt_fname) {
		err = fdt_create_empty_tree(blob, 256);
		if (!err)
			goto done;
		printf("Unable to create empty FDT: %s\n", fdt_strerror(err));
		return -EINVAL;
	}

	err = os_get_filesize(fname, &size);
	if (err < 0) {
		printf("Failed to file FDT file '%s'\n", fname);
		return err;
	}
	fd = os_open(fname, OS_O_RDONLY);
	if (fd < 0) {
		printf("Failed to open FDT file '%s'\n", fname);
		return -EACCES;
	}
	if (os_read(fd, blob, size) != size) {
		os_close(fd);
		return -EIO;
	}
	os_close(fd);

done:
	gd->fdt_blob = blob;

	return 0;
}

ulong timer_get_boot_us(void)
{
	static uint64_t base_count;
	uint64_t count = os_get_nsec();

	if (!base_count)
		base_count = count;

	return (count - base_count) / 1000;
}
