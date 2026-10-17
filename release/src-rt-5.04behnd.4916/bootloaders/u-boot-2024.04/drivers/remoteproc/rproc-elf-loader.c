// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <elf.h>
#include <log.h>
#include <remoteproc.h>
#include <asm/cache.h>
#include <dm/device_compat.h>
#include <linux/compat.h>
#include <linux/printk.h>

/**
 * struct resource_table - firmware resource table header
 * @ver: version number
 * @num: number of resource entries
 * @reserved: reserved (must be zero)
 * @offset: array of offsets pointing at the various resource entries
 *
 * A resource table is essentially a list of system resources required
 * by the remote processor. It may also include configuration entries.
 * If needed, the remote processor firmware should contain this table
 * as a dedicated ".resource_table" ELF section.
 *
 * Some resources entries are mere announcements, where the host is informed
 * of specific remoteproc configuration. Other entries require the host to
 * do something (e.g. allocate a system resource). Sometimes a negotiation
 * is expected, where the firmware requests a resource, and once allocated,
 * the host should provide back its details (e.g. address of an allocated
 * memory region).
 *
 * The header of the resource table, as expressed by this structure,
 * contains a version number (should we need to change this format in the
 * future), the number of available resource entries, and their offsets
 * in the table.
 *
 * Immediately following this header are the resource entries themselves.
 */
struct resource_table {
	u32 ver;
	u32 num;
	u32 reserved[2];
	u32 offset[0];
} __packed;

/* Basic function to verify ELF32 image format */
int rproc_elf32_sanity_check(ulong addr, ulong size)
{
	Elf32_Ehdr *ehdr;
	char class;

	if (!addr) {
		pr_debug("Invalid fw address?\n");
		return -EFAULT;
	}

	if (size < sizeof(Elf32_Ehdr)) {
		pr_debug("Image is too small\n");
		return -ENOSPC;
	}

	ehdr = (Elf32_Ehdr *)addr;
	class = ehdr->e_ident[EI_CLASS];

	if (!IS_ELF(*ehdr) || ehdr->e_type != ET_EXEC || class != ELFCLASS32) {
		pr_debug("Not an executable ELF32 image\n");
		return -EPROTONOSUPPORT;
	}

	/* We assume the firmware has the same endianness as the host */
# ifdef __LITTLE_ENDIAN
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
# else /* BIG ENDIAN */
	if (ehdr->e_ident[EI_DATA] != ELFDATA2MSB) {
# endif
		pr_debug("Unsupported firmware endianness\n");
		return -EILSEQ;
	}

	if (size < ehdr->e_shoff + sizeof(Elf32_Shdr)) {
		pr_debug("Image is too small\n");
		return -ENOSPC;
	}

	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
		pr_debug("Image is corrupted (bad magic)\n");
		return -EBADF;
	}

	if (ehdr->e_phnum == 0) {
		pr_debug("No loadable segments\n");
		return -ENOEXEC;
	}

	if (ehdr->e_phoff > size) {
		pr_debug("Firmware size is too small\n");
		return -ENOSPC;
	}

	return 0;
}

/* Basic function to verify ELF64 image format */
int rproc_elf64_sanity_check(ulong addr, ulong size)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)addr;
	char class;

	if (!addr) {
		pr_debug("Invalid fw address?\n");
		return -EFAULT;
	}

	if (size < sizeof(Elf64_Ehdr)) {
		pr_debug("Image is too small\n");
		return -ENOSPC;
	}

	class = ehdr->e_ident[EI_CLASS];

	if (!IS_ELF(*ehdr) || ehdr->e_type != ET_EXEC || class != ELFCLASS64) {
		pr_debug("Not an executable ELF64 image\n");
		return -EPROTONOSUPPORT;
	}

	/* We assume the firmware has the same endianness as the host */
# ifdef __LITTLE_ENDIAN
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
# else /* BIG ENDIAN */
	if (ehdr->e_ident[EI_DATA] != ELFDATA2MSB) {
# endif
		pr_debug("Unsupported firmware endianness\n");
		return -EILSEQ;
	}

	if (size < ehdr->e_shoff + sizeof(Elf64_Shdr)) {
		pr_debug("Image is too small\n");
		return -ENOSPC;
	}

	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
		pr_debug("Image is corrupted (bad magic)\n");
		return -EBADF;
	}

	if (ehdr->e_phnum == 0) {
		pr_debug("No loadable segments\n");
		return -ENOEXEC;
	}

	if (ehdr->e_phoff > size) {
		pr_debug("Firmware size is too small\n");
		return -ENOSPC;
	}

	return 0;
}

int rproc_elf32_load_image(struct udevice *dev, unsigned long addr, ulong size)
{
	Elf32_Ehdr *ehdr; /* Elf header structure pointer */
	Elf32_Phdr *phdr; /* Program header structure pointer */
	const struct dm_rproc_ops *ops;
	unsigned int i, ret;

	ret =  rproc_elf32_sanity_check(addr, size);
	if (ret) {
		dev_err(dev, "Invalid ELF32 Image %d\n", ret);
		return ret;
	}

	ehdr = (Elf32_Ehdr *)addr;
	phdr = (Elf32_Phdr *)(addr + ehdr->e_phoff);

	ops = rproc_get_ops(dev);

	/* Load each program header */
	for (i = 0; i < ehdr->e_phnum; i++, phdr++) {
		void *dst = (void *)(uintptr_t)phdr->p_paddr;
		void *src = (void *)addr + phdr->p_offset;

		if (phdr->p_type != PT_LOAD)
			continue;

		if (ops->device_to_virt)
			dst = ops->device_to_virt(dev, (ulong)dst,
						  phdr->p_memsz);

		dev_dbg(dev, "Loading phdr %i to 0x%p (%i bytes)\n",
			i, dst, phdr->p_filesz);
		if (phdr->p_filesz)
			memcpy(dst, src, phdr->p_filesz);
		if (phdr->p_filesz != phdr->p_memsz)
			memset(dst + phdr->p_filesz, 0x00,
			       phdr->p_memsz - phdr->p_filesz);
		flush_cache(rounddown((unsigned long)dst, ARCH_DMA_MINALIGN),
			    roundup((unsigned long)dst + phdr->p_filesz,
				    ARCH_DMA_MINALIGN) -
			    rounddown((unsigned long)dst, ARCH_DMA_MINALIGN));
	}

	return 0;
}

int rproc_elf64_load_image(struct udevice *dev, ulong addr, ulong size)
{
	const struct dm_rproc_ops *ops = rproc_get_ops(dev);
	u64 da, memsz, filesz, offset;
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	int i, ret = 0;
	void *ptr;

	dev_dbg(dev, "%s: addr = 0x%lx size = 0x%lx\n", __func__, addr, size);

	if (rproc_elf64_sanity_check(addr, size))
		return -EINVAL;

	ehdr = (Elf64_Ehdr *)addr;
	phdr = (Elf64_Phdr *)(addr + (ulong)ehdr->e_phoff);

	/* go through the available ELF segments */
	for (i = 0; i < ehdr->e_phnum; i++, phdr++) {
		da = phdr->p_paddr;
		memsz = phdr->p_memsz;
		filesz = phdr->p_filesz;
		offset = phdr->p_offset;

		if (phdr->p_type != PT_LOAD)
			continue;

		dev_dbg(dev, "%s:phdr: type %d da 0x%llx memsz 0x%llx filesz 0x%llx\n",
			__func__, phdr->p_type, da, memsz, filesz);

		ptr = (void *)(uintptr_t)da;
		if (ops->device_to_virt) {
			ptr = ops->device_to_virt(dev, da, phdr->p_memsz);
			if (!ptr) {
				dev_err(dev, "bad da 0x%llx mem 0x%llx\n", da,
					memsz);
				ret = -EINVAL;
				break;
			}
		}

		if (filesz)
			memcpy(ptr, (void *)addr + offset, filesz);
		if (filesz != memsz)
			memset(ptr + filesz, 0x00, memsz - filesz);

		flush_cache(rounddown((ulong)ptr, ARCH_DMA_MINALIGN),
			    roundup((ulong)ptr + filesz, ARCH_DMA_MINALIGN) -
			    rounddown((ulong)ptr, ARCH_DMA_MINALIGN));
	}

	return ret;
}

int rproc_elf_load_image(struct udevice *dev, ulong addr, ulong size)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;

	if (!addr) {
		dev_err(dev, "Invalid firmware address\n");
		return -EFAULT;
	}

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return rproc_elf64_load_image(dev, addr, size);
	else
		return rproc_elf32_load_image(dev, addr, size);
}

static ulong rproc_elf32_get_boot_addr(ulong addr)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;

	return ehdr->e_entry;
}

static ulong rproc_elf64_get_boot_addr(ulong addr)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)addr;

	return ehdr->e_entry;
}

ulong rproc_elf_get_boot_addr(struct udevice *dev, ulong addr)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return rproc_elf64_get_boot_addr(addr);
	else
		return rproc_elf32_get_boot_addr(addr);
}

/*
 * Search for the resource table in an ELF32 image.
 * Returns the address of the resource table section if found, NULL if there is
 * no resource table section, or error pointer.
 */
static Elf32_Shdr *rproc_elf32_find_rsc_table(struct udevice *dev,
					      ulong fw_addr, ulong fw_size)
{
	int ret;
	unsigned int i;
	const char *name_table;
	struct resource_table *table;
	const u8 *elf_data = (void *)fw_addr;
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)fw_addr;
	Elf32_Shdr *shdr;

	ret = rproc_elf32_sanity_check(fw_addr, fw_size);
	if (ret) {
		pr_debug("Invalid ELF32 Image %d\n", ret);
		return ERR_PTR(ret);
	}

	/* look for the resource table and handle it */
	shdr = (Elf32_Shdr *)(elf_data + ehdr->e_shoff);
	name_table = (const char *)(elf_data +
				    shdr[ehdr->e_shstrndx].sh_offset);

	for (i = 0; i < ehdr->e_shnum; i++, shdr++) {
		u32 size = shdr->sh_size;
		u32 offset = shdr->sh_offset;

		if (strcmp(name_table + shdr->sh_name, ".resource_table"))
			continue;

		table = (struct resource_table *)(elf_data + offset);

		/* make sure we have the entire table */
		if (offset + size > fw_size) {
			pr_debug("resource table truncated\n");
			return ERR_PTR(-ENOSPC);
		}

		/* make sure table has at least the header */
		if (sizeof(*table) > size) {
			pr_debug("header-less resource table\n");
			return ERR_PTR(-ENOSPC);
		}

		/* we don't support any version beyond the first */
		if (table->ver != 1) {
			pr_debug("unsupported fw ver: %d\n", table->ver);
			return ERR_PTR(-EPROTONOSUPPORT);
		}

		/* make sure reserved bytes are zeroes */
		if (table->reserved[0] || table->reserved[1]) {
			pr_debug("non zero reserved bytes\n");
			return ERR_PTR(-EBADF);
		}

		/* make sure the offsets array isn't truncated */
		if (table->num * sizeof(table->offset[0]) +
				 sizeof(*table) > size) {
			pr_debug("resource table incomplete\n");
			return ERR_PTR(-ENOSPC);
		}

		return shdr;
	}

	return NULL;
}

/* Load the resource table from an ELF32 image */
int rproc_elf32_load_rsc_table(struct udevice *dev, ulong fw_addr,
			       ulong fw_size, ulong *rsc_addr, ulong *rsc_size)
{
	const struct dm_rproc_ops *ops;
	Elf32_Shdr *shdr;
	void *src, *dst;

	shdr = rproc_elf32_find_rsc_table(dev, fw_addr, fw_size);
	if (!shdr)
		return -ENODATA;
	if (IS_ERR(shdr))
		return PTR_ERR(shdr);

	ops = rproc_get_ops(dev);
	*rsc_addr = (ulong)shdr->sh_addr;
	*rsc_size = (ulong)shdr->sh_size;

	src = (void *)fw_addr + shdr->sh_offset;
	if (ops->device_to_virt)
		dst = (void *)ops->device_to_virt(dev, *rsc_addr, *rsc_size);
	else
		dst = (void *)rsc_addr;

	dev_dbg(dev, "Loading resource table to 0x%8lx (%ld bytes)\n",
		(ulong)dst, *rsc_size);

	memcpy(dst, src, *rsc_size);
	flush_cache(rounddown((unsigned long)dst, ARCH_DMA_MINALIGN),
		    roundup((unsigned long)dst + *rsc_size,
			    ARCH_DMA_MINALIGN) -
		    rounddown((unsigned long)dst, ARCH_DMA_MINALIGN));

	return 0;
}

/*
 * Search for the resource table in an ELF64 image.
 * Returns the address of the resource table section if found, NULL if there is
 * no resource table section, or error pointer.
 */
static Elf64_Shdr *rproc_elf64_find_rsc_table(struct udevice *dev,
					      ulong fw_addr, ulong fw_size)
{
	int ret;
	unsigned int i;
	const char *name_table;
	struct resource_table *table;
	const u8 *elf_data = (void *)fw_addr;
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)fw_addr;
	Elf64_Shdr *shdr;

	ret = rproc_elf64_sanity_check(fw_addr, fw_size);
	if (ret) {
		pr_debug("Invalid ELF64 Image %d\n", ret);
		return ERR_PTR(ret);
	}

	/* look for the resource table and handle it */
	shdr = (Elf64_Shdr *)(elf_data + ehdr->e_shoff);
	name_table = (const char *)(elf_data +
				    shdr[ehdr->e_shstrndx].sh_offset);

	for (i = 0; i < ehdr->e_shnum; i++, shdr++) {
		u64 size = shdr->sh_size;
		u64 offset = shdr->sh_offset;

		if (strcmp(name_table + shdr->sh_name, ".resource_table"))
			continue;

		table = (struct resource_table *)(elf_data + offset);

		/* make sure we have the entire table */
		if (offset + size > fw_size) {
			pr_debug("resource table truncated\n");
			return ERR_PTR(-ENOSPC);
		}

		/* make sure table has at least the header */
		if (sizeof(*table) > size) {
			pr_debug("header-less resource table\n");
			return ERR_PTR(-ENOSPC);
		}

		/* we don't support any version beyond the first */
		if (table->ver != 1) {
			pr_debug("unsupported fw ver: %d\n", table->ver);
			return ERR_PTR(-EPROTONOSUPPORT);
		}

		/* make sure reserved bytes are zeroes */
		if (table->reserved[0] || table->reserved[1]) {
			pr_debug("non zero reserved bytes\n");
			return ERR_PTR(-EBADF);
		}

		/* make sure the offsets array isn't truncated */
		if (table->num * sizeof(table->offset[0]) +
				 sizeof(*table) > size) {
			pr_debug("resource table incomplete\n");
			return ERR_PTR(-ENOSPC);
		}

		return shdr;
	}

	return NULL;
}

/* Load the resource table from an ELF64 image */
int rproc_elf64_load_rsc_table(struct udevice *dev, ulong fw_addr,
			       ulong fw_size, ulong *rsc_addr, ulong *rsc_size)
{
	const struct dm_rproc_ops *ops;
	Elf64_Shdr *shdr;
	void *src, *dst;

	shdr = rproc_elf64_find_rsc_table(dev, fw_addr, fw_size);
	if (!shdr)
		return -ENODATA;
	if (IS_ERR(shdr))
		return PTR_ERR(shdr);

	ops = rproc_get_ops(dev);
	*rsc_addr = (ulong)shdr->sh_addr;
	*rsc_size = (ulong)shdr->sh_size;

	src = (void *)fw_addr + shdr->sh_offset;
	if (ops->device_to_virt)
		dst = (void *)ops->device_to_virt(dev, *rsc_addr, *rsc_size);
	else
		dst = (void *)rsc_addr;

	dev_dbg(dev, "Loading resource table to 0x%8lx (%ld bytes)\n",
		(ulong)dst, *rsc_size);

	memcpy(dst, src, *rsc_size);
	flush_cache(rounddown((unsigned long)dst, ARCH_DMA_MINALIGN),
		    roundup((unsigned long)dst + *rsc_size,
			    ARCH_DMA_MINALIGN) -
		    rounddown((unsigned long)dst, ARCH_DMA_MINALIGN));

	return 0;
}

/* Load the resource table from an ELF32 or ELF64 image */
int rproc_elf_load_rsc_table(struct udevice *dev, ulong fw_addr,
			     ulong fw_size, ulong *rsc_addr, ulong *rsc_size)

{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)fw_addr;

	if (!fw_addr)
		return -EFAULT;

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return rproc_elf64_load_rsc_table(dev, fw_addr, fw_size,
						  rsc_addr, rsc_size);
	else
		return rproc_elf32_load_rsc_table(dev, fw_addr, fw_size,
						  rsc_addr, rsc_size);
}
