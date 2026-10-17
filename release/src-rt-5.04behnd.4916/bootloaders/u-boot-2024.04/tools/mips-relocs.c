// SPDX-License-Identifier: GPL-2.0+
/*
 * MIPS Relocation Data Generator
 *
 * Copyright (c) 2017 Imagination Technologies Ltd.
 */

#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <asm/relocs.h>

#define hdr_field(pfx, idx, field) ({				\
	uint64_t _val;						\
	unsigned int _size;					\
								\
	if (is_64) {						\
		_val = pfx##hdr64[idx].field;			\
		_size = sizeof(pfx##hdr64[0].field);		\
	} else {						\
		_val = pfx##hdr32[idx].field;			\
		_size = sizeof(pfx##hdr32[0].field);		\
	}							\
								\
	switch (_size) {					\
	case 1:							\
		break;						\
	case 2:							\
		_val = is_be ? be16toh(_val) : le16toh(_val);	\
		break;						\
	case 4:							\
		_val = is_be ? be32toh(_val) : le32toh(_val);	\
		break;						\
	case 8:							\
		_val = is_be ? be64toh(_val) : le64toh(_val);	\
		break;						\
	}							\
								\
	_val;							\
})

#define set_hdr_field(pfx, idx, field, val) ({			\
	uint64_t _val;						\
	unsigned int _size;					\
								\
	if (is_64)						\
		_size = sizeof(pfx##hdr64[0].field);		\
	else							\
		_size = sizeof(pfx##hdr32[0].field);		\
								\
	switch (_size) {					\
	case 1:							\
		_val = val;					\
		break;						\
	case 2:							\
		_val = is_be ? htobe16(val) : htole16(val);	\
		break;						\
	case 4:							\
		_val = is_be ? htobe32(val) : htole32(val);	\
		break;						\
	case 8:							\
		_val = is_be ? htobe64(val) : htole64(val);	\
		break;						\
	default:						\
		/* We should never reach here */		\
		_val = 0;					\
		assert(0);					\
		break;						\
	}							\
								\
	if (is_64)						\
		pfx##hdr64[idx].field = _val;			\
	else							\
		pfx##hdr32[idx].field = _val;			\
})

#define ehdr_field(field) \
	hdr_field(e, 0, field)
#define phdr_field(idx, field) \
	hdr_field(p, idx, field)
#define shdr_field(idx, field) \
	hdr_field(s, idx, field)

#define set_phdr_field(idx, field, val) \
	set_hdr_field(p, idx, field, val)
#define set_shdr_field(idx, field, val) \
	set_hdr_field(s, idx, field, val)

#define shstr(idx) (&shstrtab[idx])

bool is_64, is_be;
uint64_t text_base;

struct mips_reloc {
	uint8_t type;
	uint64_t offset;
} *relocs;
size_t relocs_sz, relocs_idx;

static int add_reloc(unsigned int type, uint64_t off)
{
	struct mips_reloc *new;
	size_t new_sz;

	switch (type) {
	case R_MIPS_NONE:
	case R_MIPS_LO16:
	case R_MIPS_PC16:
	case R_MIPS_HIGHER:
	case R_MIPS_HIGHEST:
	case R_MIPS_PC21_S2:
	case R_MIPS_PC26_S2:
		/* Skip these relocs */
		return 0;

	default:
		break;
	}

	if (relocs_idx == relocs_sz) {
		new_sz = relocs_sz ? relocs_sz * 2 : 128;
		new = realloc(relocs, new_sz * sizeof(*relocs));
		if (!new) {
			fprintf(stderr, "Out of memory\n");
			return -ENOMEM;
		}

		relocs = new;
		relocs_sz = new_sz;
	}

	relocs[relocs_idx++] = (struct mips_reloc){
		.type = type,
		.offset = off,
	};

	return 0;
}

static int parse_mips32_rel(const void *_rel)
{
	const Elf32_Rel *rel = _rel;
	uint32_t off, type;

	off = is_be ? be32toh(rel->r_offset) : le32toh(rel->r_offset);
	off -= text_base;

	type = is_be ? be32toh(rel->r_info) : le32toh(rel->r_info);
	type = ELF32_R_TYPE(type);

	return add_reloc(type, off);
}

static int parse_mips64_rela(const void *_rel)
{
	const Elf64_Rela *rel = _rel;
	uint64_t off, type;

	off = is_be ? be64toh(rel->r_offset) : le64toh(rel->r_offset);
	off -= text_base;

	type = rel->r_info >> (64 - 8);

	return add_reloc(type, off);
}

static void output_uint(uint8_t **buf, uint64_t val)
{
	uint64_t tmp;

	do {
		tmp = val & 0x7f;
		val >>= 7;
		tmp |= !!val << 7;
		*(*buf)++ = tmp;
	} while (val);
}

static int compare_relocs(const void *a, const void *b)
{
	const struct mips_reloc *ra = a, *rb = b;

	return ra->offset - rb->offset;
}

int main(int argc, char *argv[])
{
	unsigned int i, j, i_rel_shdr, sh_type, sh_entsize, sh_entries;
	size_t rel_size, rel_actual_size;
	const char *shstrtab, *sh_name, *rel_pfx;
	int (*parse_fn)(const void *rel);
	uint8_t *buf_start, *buf;
	const Elf32_Ehdr *ehdr32;
	const Elf64_Ehdr *ehdr64;
	uintptr_t sh_offset;
	Elf32_Shdr *shdr32;
	Elf64_Shdr *shdr64;
	struct stat st;
	int err, fd;
	void *elf;
	bool skip;

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Unable to open input file %s\n", argv[1]);
		err = errno;
		goto out_ret;
	}

	err = fstat(fd, &st);
	if (err) {
		fprintf(stderr, "Unable to fstat() input file\n");
		goto out_close_fd;
	}

	elf = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (elf == MAP_FAILED) {
		fprintf(stderr, "Unable to mmap() input file\n");
		err = errno;
		goto out_close_fd;
	}

	ehdr32 = elf;
	ehdr64 = elf;

	if (memcmp(&ehdr32->e_ident[EI_MAG0], ELFMAG, SELFMAG)) {
		fprintf(stderr, "Input file is not an ELF\n");
		err = -EINVAL;
		goto out_free_relocs;
	}

	if (ehdr32->e_ident[EI_VERSION] != EV_CURRENT) {
		fprintf(stderr, "Unrecognised ELF version\n");
		err = -EINVAL;
		goto out_free_relocs;
	}

	switch (ehdr32->e_ident[EI_CLASS]) {
	case ELFCLASS32:
		is_64 = false;
		break;
	case ELFCLASS64:
		is_64 = true;
		break;
	default:
		fprintf(stderr, "Unrecognised ELF class\n");
		err = -EINVAL;
		goto out_free_relocs;
	}

	switch (ehdr32->e_ident[EI_DATA]) {
	case ELFDATA2LSB:
		is_be = false;
		break;
	case ELFDATA2MSB:
		is_be = true;
		break;
	default:
		fprintf(stderr, "Unrecognised ELF data encoding\n");
		err = -EINVAL;
		goto out_free_relocs;
	}

	if (ehdr_field(e_type) != ET_EXEC) {
		fprintf(stderr, "Input ELF is not an executable\n");
		printf("type 0x%lx\n", ehdr_field(e_type));
		err = -EINVAL;
		goto out_free_relocs;
	}

	if (ehdr_field(e_machine) != EM_MIPS) {
		fprintf(stderr, "Input ELF does not target MIPS\n");
		err = -EINVAL;
		goto out_free_relocs;
	}

	shdr32 = elf + ehdr_field(e_shoff);
	shdr64 = elf + ehdr_field(e_shoff);
	shstrtab = elf + shdr_field(ehdr_field(e_shstrndx), sh_offset);

	i_rel_shdr = UINT_MAX;
	for (i = 0; i < ehdr_field(e_shnum); i++) {
		sh_name = shstr(shdr_field(i, sh_name));

		if (!strcmp(sh_name, ".data.reloc")) {
			i_rel_shdr = i;
			continue;
		}

		if (!strcmp(sh_name, ".text")) {
			text_base = shdr_field(i, sh_addr);
			continue;
		}
	}
	if (i_rel_shdr == UINT_MAX) {
		fprintf(stderr, "Unable to find .rel section\n");
		err = -EINVAL;
		goto out_free_relocs;
	}
	if (!text_base) {
		fprintf(stderr, "Unable to find .text base address\n");
		err = -EINVAL;
		goto out_free_relocs;
	}

	rel_pfx = is_64 ? ".rela" : ".rel";

	for (i = 0; i < ehdr_field(e_shnum); i++) {
		sh_type = shdr_field(i, sh_type);
		if ((sh_type != SHT_REL) && (sh_type != SHT_RELA))
			continue;

		sh_name = shstr(shdr_field(i, sh_name));
		if (strncmp(sh_name, rel_pfx, strlen(rel_pfx))) {
			fprintf(stderr, "WARNING: Unexpected reloc section name '%s'\n", sh_name);
			continue;
		}
		if (!strcmp(sh_name, ".rel") || !strcmp(sh_name, ".rel.dyn"))
			continue;

		/*
		 * Skip reloc sections which either don't correspond to another
		 * section in the ELF, or whose corresponding section isn't
		 * loaded as part of the U-Boot binary (ie. doesn't have the
		 * alloc flags set).
		 */
		skip = true;
		for (j = 0; j < ehdr_field(e_shnum); j++) {
			if (strcmp(&sh_name[strlen(rel_pfx)], shstr(shdr_field(j, sh_name))))
				continue;

			skip = !(shdr_field(j, sh_flags) & SHF_ALLOC);
			break;
		}
		if (skip)
			continue;

		sh_offset = shdr_field(i, sh_offset);
		sh_entsize = shdr_field(i, sh_entsize);
		sh_entries = shdr_field(i, sh_size) / sh_entsize;

		if (sh_type == SHT_REL) {
			if (is_64) {
				fprintf(stderr, "REL-style reloc in MIPS64 ELF?\n");
				err = -EINVAL;
				goto out_free_relocs;
			} else {
				parse_fn = parse_mips32_rel;
			}
		} else {
			if (is_64) {
				parse_fn = parse_mips64_rela;
			} else {
				fprintf(stderr, "RELA-style reloc in MIPS32 ELF?\n");
				err = -EINVAL;
				goto out_free_relocs;
			}
		}

		for (j = 0; j < sh_entries; j++) {
			err = parse_fn(elf + sh_offset + (j * sh_entsize));
			if (err)
				goto out_free_relocs;
		}
	}

	/* Sort relocs in ascending order of offset */
	qsort(relocs, relocs_idx, sizeof(*relocs), compare_relocs);

	/* Make reloc offsets relative to their predecessor */
	for (i = relocs_idx - 1; i > 0; i--)
		relocs[i].offset -= relocs[i - 1].offset;

	/* Write the relocations to the .rel section */
	buf = buf_start = elf + shdr_field(i_rel_shdr, sh_offset);
	for (i = 0; i < relocs_idx; i++) {
		output_uint(&buf, relocs[i].type);
		output_uint(&buf, relocs[i].offset >> 2);
	}

	/* Write a terminating R_MIPS_NONE (0) */
	output_uint(&buf, R_MIPS_NONE);

	/* Ensure the relocs didn't overflow the .rel section */
	rel_size = shdr_field(i_rel_shdr, sh_size);
	rel_actual_size = buf - buf_start;
	if (rel_actual_size > rel_size) {
		fprintf(stderr, "Relocations overflow available space of 0x%lx (required 0x%lx)!\n",
			rel_size, rel_actual_size);
		fprintf(stderr, "Please adjust CONFIG_MIPS_RELOCATION_TABLE_SIZE to at least 0x%lx\n",
			(rel_actual_size + 0x100) & ~0xFF);
		err = -ENOMEM;
		goto out_free_relocs;
	}

	/* Make sure data is written back to the file */
	err = msync(elf, st.st_size, MS_SYNC);
	if (err) {
		fprintf(stderr, "Failed to msync: %d\n", errno);
		goto out_free_relocs;
	}

out_free_relocs:
	free(relocs);
	munmap(elf, st.st_size);
out_close_fd:
	close(fd);
out_ret:
	return err;
}
