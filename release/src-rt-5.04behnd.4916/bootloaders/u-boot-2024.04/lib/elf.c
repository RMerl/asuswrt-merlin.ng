// SPDX-License-Identifier: BSD-2-Clause
/*
   Copyright (c) 2001 William L. Pitts
*/

#include <command.h>
#include <cpu_func.h>
#include <elf.h>
#include <env.h>
#include <net.h>
#include <vxworks.h>
#ifdef CONFIG_X86
#include <vesa.h>
#include <asm/e820.h>
#include <linux/linkage.h>
#endif

/*
 * A very simple ELF64 loader, assumes the image is valid, returns the
 * entry point address.
 *
 * Note if U-Boot is 32-bit, the loader assumes the to segment's
 * physical address and size is within the lower 32-bit address space.
 */
unsigned long load_elf64_image_phdr(unsigned long addr)
{
	Elf64_Ehdr *ehdr; /* Elf header structure pointer */
	Elf64_Phdr *phdr; /* Program header structure pointer */
	int i;

	ehdr = (Elf64_Ehdr *)addr;
	phdr = (Elf64_Phdr *)(addr + (ulong)ehdr->e_phoff);

	/* Load each program header */
	for (i = 0; i < ehdr->e_phnum; ++i) {
		void *dst = (void *)(ulong)phdr->p_paddr;
		void *src = (void *)addr + phdr->p_offset;

		debug("Loading phdr %i to 0x%p (%lu bytes)\n",
		      i, dst, (ulong)phdr->p_filesz);
		if (phdr->p_filesz)
			memcpy(dst, src, phdr->p_filesz);
		if (phdr->p_filesz != phdr->p_memsz)
			memset(dst + phdr->p_filesz, 0x00,
			       phdr->p_memsz - phdr->p_filesz);
		flush_cache(rounddown((unsigned long)dst, ARCH_DMA_MINALIGN),
			    roundup(phdr->p_memsz, ARCH_DMA_MINALIGN));
		++phdr;
	}

	if (ehdr->e_machine == EM_PPC64 && (ehdr->e_flags &
					    EF_PPC64_ELFV1_ABI)) {
		/*
		 * For the 64-bit PowerPC ELF V1 ABI, e_entry is a function
		 * descriptor pointer with the first double word being the
		 * address of the entry point of the function.
		 */
		uintptr_t addr = ehdr->e_entry;

		return *(Elf64_Addr *)addr;
	}

	return ehdr->e_entry;
}

unsigned long load_elf64_image_shdr(unsigned long addr)
{
	Elf64_Ehdr *ehdr; /* Elf header structure pointer */
	Elf64_Shdr *shdr; /* Section header structure pointer */
	unsigned char *strtab = 0; /* String table pointer */
	unsigned char *image; /* Binary image pointer */
	int i; /* Loop counter */

	ehdr = (Elf64_Ehdr *)addr;

	/* Find the section header string table for output info */
	shdr = (Elf64_Shdr *)(addr + (ulong)ehdr->e_shoff +
			     (ehdr->e_shstrndx * sizeof(Elf64_Shdr)));

	if (shdr->sh_type == SHT_STRTAB)
		strtab = (unsigned char *)(addr + (ulong)shdr->sh_offset);

	/* Load each appropriate section */
	for (i = 0; i < ehdr->e_shnum; ++i) {
		shdr = (Elf64_Shdr *)(addr + (ulong)ehdr->e_shoff +
				     (i * sizeof(Elf64_Shdr)));

		if (!(shdr->sh_flags & SHF_ALLOC) ||
		    shdr->sh_addr == 0 || shdr->sh_size == 0) {
			continue;
		}

		if (strtab) {
			debug("%sing %s @ 0x%08lx (%ld bytes)\n",
			      (shdr->sh_type == SHT_NOBITS) ? "Clear" : "Load",
			       &strtab[shdr->sh_name],
			       (unsigned long)shdr->sh_addr,
			       (long)shdr->sh_size);
		}

		if (shdr->sh_type == SHT_NOBITS) {
			memset((void *)(uintptr_t)shdr->sh_addr, 0,
			       shdr->sh_size);
		} else {
			image = (unsigned char *)addr + (ulong)shdr->sh_offset;
			memcpy((void *)(uintptr_t)shdr->sh_addr,
			       (const void *)image, shdr->sh_size);
		}
		flush_cache(rounddown(shdr->sh_addr, ARCH_DMA_MINALIGN),
			    roundup((shdr->sh_addr + shdr->sh_size),
				     ARCH_DMA_MINALIGN) -
				rounddown(shdr->sh_addr, ARCH_DMA_MINALIGN));
	}

	if (ehdr->e_machine == EM_PPC64 && (ehdr->e_flags &
					    EF_PPC64_ELFV1_ABI)) {
		/*
		 * For the 64-bit PowerPC ELF V1 ABI, e_entry is a function
		 * descriptor pointer with the first double word being the
		 * address of the entry point of the function.
		 */
		uintptr_t addr = ehdr->e_entry;

		return *(Elf64_Addr *)addr;
	}

	return ehdr->e_entry;
}

/*
 * A very simple ELF loader, assumes the image is valid, returns the
 * entry point address.
 *
 * The loader firstly reads the EFI class to see if it's a 64-bit image.
 * If yes, call the ELF64 loader. Otherwise continue with the ELF32 loader.
 */
unsigned long load_elf_image_phdr(unsigned long addr)
{
	Elf32_Ehdr *ehdr; /* Elf header structure pointer */
	Elf32_Phdr *phdr; /* Program header structure pointer */
	int i;

	ehdr = (Elf32_Ehdr *)addr;
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return load_elf64_image_phdr(addr);

	phdr = (Elf32_Phdr *)(addr + ehdr->e_phoff);

	/* Load each program header */
	for (i = 0; i < ehdr->e_phnum; ++i) {
		void *dst = (void *)(uintptr_t)phdr->p_paddr;
		void *src = (void *)addr + phdr->p_offset;

		debug("Loading phdr %i to 0x%p (%i bytes)\n",
		      i, dst, phdr->p_filesz);
		if (phdr->p_filesz)
			memcpy(dst, src, phdr->p_filesz);
		if (phdr->p_filesz != phdr->p_memsz)
			memset(dst + phdr->p_filesz, 0x00,
			       phdr->p_memsz - phdr->p_filesz);
		flush_cache(rounddown((unsigned long)dst, ARCH_DMA_MINALIGN),
			    roundup(phdr->p_memsz, ARCH_DMA_MINALIGN));
		++phdr;
	}

	return ehdr->e_entry;
}

unsigned long load_elf_image_shdr(unsigned long addr)
{
	Elf32_Ehdr *ehdr; /* Elf header structure pointer */
	Elf32_Shdr *shdr; /* Section header structure pointer */
	unsigned char *strtab = 0; /* String table pointer */
	unsigned char *image; /* Binary image pointer */
	int i; /* Loop counter */

	ehdr = (Elf32_Ehdr *)addr;
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return load_elf64_image_shdr(addr);

	/* Find the section header string table for output info */
	shdr = (Elf32_Shdr *)(addr + ehdr->e_shoff +
			     (ehdr->e_shstrndx * sizeof(Elf32_Shdr)));

	if (shdr->sh_type == SHT_STRTAB)
		strtab = (unsigned char *)(addr + shdr->sh_offset);

	/* Load each appropriate section */
	for (i = 0; i < ehdr->e_shnum; ++i) {
		shdr = (Elf32_Shdr *)(addr + ehdr->e_shoff +
				     (i * sizeof(Elf32_Shdr)));

		if (!(shdr->sh_flags & SHF_ALLOC) ||
		    shdr->sh_addr == 0 || shdr->sh_size == 0) {
			continue;
		}

		if (strtab) {
			debug("%sing %s @ 0x%08lx (%ld bytes)\n",
			      (shdr->sh_type == SHT_NOBITS) ? "Clear" : "Load",
			       &strtab[shdr->sh_name],
			       (unsigned long)shdr->sh_addr,
			       (long)shdr->sh_size);
		}

		if (shdr->sh_type == SHT_NOBITS) {
			memset((void *)(uintptr_t)shdr->sh_addr, 0,
			       shdr->sh_size);
		} else {
			image = (unsigned char *)addr + shdr->sh_offset;
			memcpy((void *)(uintptr_t)shdr->sh_addr,
			       (const void *)image, shdr->sh_size);
		}
		flush_cache(rounddown(shdr->sh_addr, ARCH_DMA_MINALIGN),
			    roundup((shdr->sh_addr + shdr->sh_size),
				    ARCH_DMA_MINALIGN) -
			    rounddown(shdr->sh_addr, ARCH_DMA_MINALIGN));
	}

	return ehdr->e_entry;
}

/*
 * Determine if a valid ELF image exists at the given memory location.
 * First look at the ELF header magic field, then make sure that it is
 * executable.
 */
int valid_elf_image(unsigned long addr)
{
	Elf32_Ehdr *ehdr; /* Elf header structure pointer */

	ehdr = (Elf32_Ehdr *)addr;

	if (!IS_ELF(*ehdr)) {
		printf("## No elf image at address 0x%08lx\n", addr);
		return 0;
	}

	if (ehdr->e_type != ET_EXEC) {
		printf("## Not a 32-bit elf image at address 0x%08lx\n", addr);
		return 0;
	}

	return 1;
}
