// SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * 64-bit and little-endian target only until we need to support a different
 * arch that needs this.
 */

#include <elf.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#ifndef EM_AARCH64
#define EM_AARCH64		183
#endif

#ifndef R_AARCH64_RELATIVE
#define R_AARCH64_RELATIVE	1027
#endif

#ifndef EM_M68K
#define EM_M68K			4
#endif

#ifndef R_68K_NONE
#define R_68K_NONE		0
#endif

#ifndef R_68K_32
#define R_68K_32		1
#endif

#ifndef R_68K_GLOB_DAT
#define R_68K_GLOB_DAT		20
#endif

#ifndef R_68K_JMP_SLOT
#define R_68K_JMP_SLOT		21
#endif

#ifndef R_68K_RELATIVE
#define R_68K_RELATIVE		22
#endif

#ifndef EM_MICROBLAZE
#define EM_MICROBLAZE		189
#endif

#ifndef R_MICROBLAZE_NONE
#define R_MICROBLAZE_NONE	0
#endif

#ifndef R_MICROBLAZE_32
#define R_MICROBLAZE_32		1
#endif

#ifndef R_MICROBLAZE_REL
#define R_MICROBLAZE_REL	16
#endif

#ifndef R_MICROBLAZE_GLOB_DAT
#define R_MICROBLAZE_GLOB_DAT	18
#endif

static int ei_class;
static int ei_data;
static int machine;

static uint64_t rela_start, rela_end, text_base, dyn_start;

static const bool debug_en;

static void debug(const char *fmt, ...)
{
	va_list args;

	if (debug_en) {
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

static uint16_t elf16_to_cpu(uint16_t data)
{
	if (ei_data == ELFDATA2LSB)
		return le16_to_cpu(data);

	return be16_to_cpu(data);
}

static uint32_t elf32_to_cpu(uint32_t data)
{
	if (ei_data == ELFDATA2LSB)
		return le32_to_cpu(data);

	return be32_to_cpu(data);
}

static uint32_t cpu_to_elf32(uint32_t data)
{
	if (ei_data == ELFDATA2LSB)
		return cpu_to_le32(data);

	return cpu_to_be32(data);
}

static bool supported_rela(Elf64_Rela *rela)
{
	uint64_t mask = 0xffffffffULL; /* would be different on 32-bit */
	uint32_t type = rela->r_info & mask;

	switch (type) {
	case R_AARCH64_RELATIVE:
		return true;
	default:
		fprintf(stderr, "warning: unsupported relocation type %"
				PRIu32 " at %" PRIx64 "\n",
			type, rela->r_offset);

		return false;
	}
}

static int decode_elf64(FILE *felf, char **argv)
{
	size_t size;
	Elf64_Ehdr header;
	uint64_t section_header_base, section_header_size;
	uint64_t sh_addr, sh_offset, sh_size;
	Elf64_Half sh_index, sh_num;
	Elf64_Shdr *sh_table; /* Elf symbol table */
	int ret, i;
	char *sh_str;

	debug("64bit version\n");

	/* Make sure we are at start */
	rewind(felf);

	size = fread(&header, 1, sizeof(header), felf);
	if (size != sizeof(header)) {
		fclose(felf);
		return 25;
	}

	machine = le16_to_cpu(header.e_machine);
	debug("Machine\t%d\n", machine);

	if (machine != EM_AARCH64) {
		fprintf(stderr, "%s: Not supported machine type\n", argv[0]);
		return 30;
	}

	text_base = le64_to_cpu(header.e_entry);
	section_header_base = le64_to_cpu(header.e_shoff);
	section_header_size = le16_to_cpu(header.e_shentsize) *
			      le16_to_cpu(header.e_shnum);

	sh_table = malloc(section_header_size);
	if (!sh_table) {
		fprintf(stderr, "%s: Cannot allocate space for section header\n",
			argv[0]);
		fclose(felf);
		return 26;
	}

	ret = fseek(felf, section_header_base, SEEK_SET);
	if (ret) {
		fprintf(stderr, "%s: Can't set pointer to section header: %x/%lx\n",
			argv[0], ret, section_header_base);
		free(sh_table);
		fclose(felf);
		return 26;
	}

	size = fread(sh_table, 1, section_header_size, felf);
	if (size != section_header_size) {
		fprintf(stderr, "%s: Can't read section header: %lx/%lx\n",
			argv[0], size, section_header_size);
		free(sh_table);
		fclose(felf);
		return 27;
	}

	sh_index = le16_to_cpu(header.e_shstrndx);
	sh_size = le64_to_cpu(sh_table[sh_index].sh_size);
	debug("e_shstrndx %x, sh_size %lx\n", sh_index, sh_size);

	sh_str = malloc(sh_size);
	if (!sh_str) {
		fprintf(stderr, "malloc failed\n");
		free(sh_table);
		fclose(felf);
		return 28;
	}

	/*
	 * Specifies the byte offset from the beginning of the file
	 * to the first byte in the section.
	 */
	sh_offset = le64_to_cpu(sh_table[sh_index].sh_offset);
	sh_num = le16_to_cpu(header.e_shnum);

	ret = fseek(felf, sh_offset, SEEK_SET);
	if (ret) {
		fprintf(stderr, "Setting up sh_offset failed\n");
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 29;
	}

	size = fread(sh_str, 1, sh_size, felf);
	if (size != sh_size) {
		fprintf(stderr, "%s: Can't read section: %lx/%lx\n",
			argv[0], size, sh_size);
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 30;
	}

	for (i = 0; i < sh_num; i++) {
		char *sh_name = sh_str + le32_to_cpu(sh_table[i].sh_name);

		debug("%s\n", sh_name);

		sh_addr = le64_to_cpu(sh_table[i].sh_addr);
		sh_offset = le64_to_cpu(sh_table[i].sh_offset);
		sh_size = le64_to_cpu(sh_table[i].sh_size);

		if (!strcmp(".rela.dyn", sh_name)) {
			debug("Found section\t\".rela_dyn\"\n");
			debug(" at addr\t0x%08x\n", sh_addr);
			debug(" at offset\t0x%08x\n", sh_offset);
			debug(" of size\t0x%08x\n", sh_size);
			rela_start = sh_addr;
			rela_end = rela_start + sh_size;
			break;
		}
	}

	/* Clean up */
	free(sh_str);
	free(sh_table);
	fclose(felf);

	debug("text_base\t0x%08lx\n", text_base);
	debug("rela_start\t0x%08lx\n", rela_start);
	debug("rela_end\t0x%08lx\n", rela_end);

	if (!rela_start)
		return 1;

	return 0;
}

static int decode_elf32(FILE *felf, char **argv)
{
	size_t size;
	Elf32_Ehdr header;
	uint64_t section_header_base, section_header_size;
	uint32_t sh_addr, sh_offset, sh_size;
	Elf32_Half sh_index, sh_num;
	Elf32_Shdr *sh_table; /* Elf symbol table */
	int ret, i;
	char *sh_str;

	debug("32bit version\n");

	/* Make sure we are at start */
	rewind(felf);

	size = fread(&header, 1, sizeof(header), felf);
	if (size != sizeof(header)) {
		fclose(felf);
		return 25;
	}

	machine = elf16_to_cpu(header.e_machine);
	debug("Machine %d\n", machine);

	if (machine != EM_MICROBLAZE && machine != EM_M68K) {
		fprintf(stderr, "%s: Not supported machine type\n", argv[0]);
		return 30;
	}

	text_base = elf32_to_cpu(header.e_entry);
	/*
	 * M68K ELF entry point is MONITOR_BASE, not TEXT_BASE.
	 * TEXT_BASE is always MONITOR_BASE &~ 0x7ff, so clear
	 * those bits here.
	 */
	if (machine == EM_M68K)
		text_base &= ~0x7ff;

	section_header_base = elf32_to_cpu(header.e_shoff);
	section_header_size = elf16_to_cpu(header.e_shentsize) *
			      elf16_to_cpu(header.e_shnum);

	sh_table = malloc(section_header_size);
	if (!sh_table) {
		fprintf(stderr, "%s: Cannot allocate space for section header\n",
			argv[0]);
		fclose(felf);
		return 26;
	}

	ret = fseek(felf, section_header_base, SEEK_SET);
	if (ret) {
		fprintf(stderr, "%s: Can't set pointer to section header: %x/%lx\n",
			argv[0], ret, section_header_base);
		free(sh_table);
		fclose(felf);
		return 26;
	}

	size = fread(sh_table, 1, section_header_size, felf);
	if (size != section_header_size) {
		fprintf(stderr, "%s: Can't read section header: %lx/%lx\n",
			argv[0], size, section_header_size);
		free(sh_table);
		fclose(felf);
		return 27;
	}

	sh_index = elf16_to_cpu(header.e_shstrndx);
	sh_size = elf32_to_cpu(sh_table[sh_index].sh_size);
	debug("e_shstrndx %x, sh_size %lx\n", sh_index, sh_size);

	sh_str = malloc(sh_size);
	if (!sh_str) {
		fprintf(stderr, "malloc failed\n");
		free(sh_table);
		fclose(felf);
		return 28;
	}

	/*
	 * Specifies the byte offset from the beginning of the file
	 * to the first byte in the section.
	 */
	sh_offset = elf32_to_cpu(sh_table[sh_index].sh_offset);
	sh_num = elf16_to_cpu(header.e_shnum);

	ret = fseek(felf, sh_offset, SEEK_SET);
	if (ret) {
		fprintf(stderr, "Setting up sh_offset failed\n");
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 29;
	}

	size = fread(sh_str, 1, sh_size, felf);
	if (size != sh_size) {
		fprintf(stderr, "%s: Can't read section: %lx/%x\n",
			argv[0], size, sh_size);
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 30;
	}

	for (i = 0; i < sh_num; i++) {
		char *sh_name = sh_str + elf32_to_cpu(sh_table[i].sh_name);

		debug("%s\n", sh_name);

		sh_addr = elf32_to_cpu(sh_table[i].sh_addr);
		sh_offset = elf32_to_cpu(sh_table[i].sh_offset);
		sh_size = elf32_to_cpu(sh_table[i].sh_size);

		if (!strcmp(".rela.dyn", sh_name)) {
			debug("Found section\t\".rela_dyn\"\n");
			debug(" at addr\t0x%08x\n", sh_addr);
			debug(" at offset\t0x%08x\n", sh_offset);
			debug(" of size\t0x%08x\n", sh_size);
			rela_start = sh_addr;
			rela_end = rela_start + sh_size;
		}
		if (!strcmp(".dynsym", sh_name)) {
			debug("Found section\t\".dynsym\"\n");
			debug(" at addr\t0x%08x\n", sh_addr);
			debug(" at offset\t0x%08x\n", sh_offset);
			debug(" of size\t0x%08x\n", sh_size);
			dyn_start = sh_addr;
		}
	}

	/* Clean up */
	free(sh_str);
	free(sh_table);
	fclose(felf);

	debug("text_base\t0x%08lx\n", text_base);
	debug("rela_start\t0x%08lx\n", rela_start);
	debug("rela_end\t0x%08lx\n", rela_end);
	debug("dyn_start\t0x%08lx\n", dyn_start);

	if (!rela_start)
		return 1;

	return 0;
}

static int decode_elf(char **argv)
{
	FILE *felf;
	size_t size;
	unsigned char e_ident[EI_NIDENT];

	felf = fopen(argv[2], "r+b");
	if (!felf) {
		fprintf(stderr, "%s: Cannot open %s: %s\n",
			argv[0], argv[5], strerror(errno));
		return 2;
	}

	size = fread(e_ident, 1, EI_NIDENT, felf);
	if (size != EI_NIDENT) {
		fclose(felf);
		return 25;
	}

	/* Check if this is really ELF file */
	if (e_ident[0] != 0x7f &&
	    e_ident[1] != 'E' &&
	    e_ident[2] != 'L' &&
	    e_ident[3] != 'F') {
		fclose(felf);
		return 1;
	}

	ei_class = e_ident[4];
	debug("EI_CLASS(1=32bit, 2=64bit) %d\n", ei_class);

	ei_data = e_ident[5];
	debug("EI_DATA(1=little endian, 2=big endian) %d\n", ei_data);

	if (ei_class == 2)
		return decode_elf64(felf, argv);

	return decode_elf32(felf, argv);
}

static int rela_elf64(char **argv, FILE *f)
{
	int i, num;

	if ((rela_end - rela_start) % sizeof(Elf64_Rela)) {
		fprintf(stderr, "%s: rela size isn't a multiple of Elf64_Rela\n", argv[0]);
		return 3;
	}

	num = (rela_end - rela_start) / sizeof(Elf64_Rela);

	for (i = 0; i < num; i++) {
		Elf64_Rela rela, swrela;
		uint64_t pos = rela_start + sizeof(Elf64_Rela) * i;
		uint64_t addr;

		if (fseek(f, pos, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %" PRIx64
					" failed: %s\n",
				argv[0], argv[1], pos, strerror(errno));
		}

		if (fread(&rela, sizeof(rela), 1, f) != 1) {
			fprintf(stderr, "%s: %s: read rela failed at %"
					PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		swrela.r_offset = le64_to_cpu(rela.r_offset);
		swrela.r_info = le64_to_cpu(rela.r_info);
		swrela.r_addend = le64_to_cpu(rela.r_addend);

		if (!supported_rela(&swrela))
			continue;

		debug("Rela %" PRIx64 " %" PRIu64 " %" PRIx64 "\n",
		      swrela.r_offset, swrela.r_info, swrela.r_addend);

		if (swrela.r_offset < text_base) {
			fprintf(stderr, "%s: %s: bad rela at %" PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		addr = swrela.r_offset - text_base;

		if (fseek(f, addr, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %"
					PRIx64 " failed: %s\n",
				argv[0], argv[1], addr, strerror(errno));
		}

		if (fwrite(&rela.r_addend, sizeof(rela.r_addend), 1, f) != 1) {
			fprintf(stderr, "%s: %s: write failed at %" PRIx64 "\n",
				argv[0], argv[1], addr);
			return 4;
		}
	}

	return 0;
}

static bool supported_rela32(Elf32_Rela *rela, uint32_t *type)
{
	uint32_t mask = 0xffULL; /* would be different on 32-bit */
	*type = rela->r_info & mask;

	debug("Type:\t");

	if (machine == EM_M68K) {
		switch (*type) {
		case R_68K_32:
			debug("R_68K_32\n");
			return true;
		case R_68K_GLOB_DAT:
			debug("R_68K_GLOB_DAT\n");
			return true;
		case R_68K_JMP_SLOT:
			debug("R_68K_JMP_SLOT\n");
			return true;
		case R_68K_NONE:
			debug("R_68K_NONE - ignoring - do nothing\n");
			return false;
		case R_68K_RELATIVE:
			debug("R_68K_RELATIVE\n");
			return true;
		}
	} else {
		switch (*type) {
		case R_MICROBLAZE_32:
			debug("R_MICROBLAZE_32\n");
			return true;
		case R_MICROBLAZE_GLOB_DAT:
			debug("R_MICROBLAZE_GLOB_DAT\n");
			return true;
		case R_MICROBLAZE_NONE:
			debug("R_MICROBLAZE_NONE - ignoring - do nothing\n");
			return false;
		case R_MICROBLAZE_REL:
			debug("R_MICROBLAZE_REL\n");
			return true;
		}
	}
	fprintf(stderr, "warning: unsupported relocation type %"
		PRIu32 " at %" PRIx32 "\n", *type, rela->r_offset);

	return false;
}

static int rela_elf32(char **argv, FILE *f)
{
	int i, num, index;
	uint32_t value, type;

	if ((rela_end - rela_start) % sizeof(Elf32_Rela)) {
		fprintf(stderr, "%s: rela size isn't a multiple of Elf32_Rela\n", argv[0]);
		return 3;
	}

	num = (rela_end - rela_start) / sizeof(Elf32_Rela);

	debug("Number of entries: %u\n", num);

	for (i = 0; i < num; i++) {
		Elf32_Rela rela, swrela;
		Elf32_Sym symbols;
		uint32_t pos = rela_start + sizeof(Elf32_Rela) * i;
		uint32_t addr, pos_dyn;

		debug("\nPosition:\t%d/0x%x\n", i, pos);

		if (fseek(f, pos, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %" PRIx32
					" failed: %s\n",
				argv[0], argv[1], pos, strerror(errno));
		}

		if (fread(&rela, sizeof(rela), 1, f) != 1) {
			fprintf(stderr, "%s: %s: read rela failed at %"
					PRIx32 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		debug("Rela:\toffset:\t%" PRIx32 " r_info:\t%"
		      PRIu32 " r_addend:\t%" PRIx32 "\n",
		      rela.r_offset, rela.r_info, rela.r_addend);

		swrela.r_offset = elf32_to_cpu(rela.r_offset);
		swrela.r_info = elf32_to_cpu(rela.r_info);
		swrela.r_addend = elf32_to_cpu(rela.r_addend);

		debug("SWRela:\toffset:\t%" PRIx32 " r_info:\t%"
		      PRIu32 " r_addend:\t%" PRIx32 "\n",
		      swrela.r_offset, swrela.r_info, swrela.r_addend);

		if (!supported_rela32(&swrela, &type))
			continue;

		if (swrela.r_offset < text_base) {
			fprintf(stderr, "%s: %s: bad rela at %" PRIx32 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		addr = swrela.r_offset - text_base;

		debug("Addr:\t0x%" PRIx32 "\n", addr);

		if ((machine == EM_M68K && type == R_68K_RELATIVE) ||
		    (machine == EM_MICROBLAZE && type == R_MICROBLAZE_REL)) {
			if (fseek(f, addr, SEEK_SET) < 0) {
				fprintf(stderr, "%s: %s: seek to %"
					PRIx32 " failed: %s\n",
					argv[0], argv[1], addr, strerror(errno));
				return 5;
			}

			debug("Write addend\n");

			if (fwrite(&rela.r_addend, sizeof(rela.r_addend), 1, f) != 1) {
				fprintf(stderr, "%s: %s: write failed at %" PRIx32 "\n",
					argv[0], argv[1], addr);
				return 4;
			}
		} else if ((machine == EM_M68K &&
		            (type == R_68K_32 || type == R_68K_GLOB_DAT ||
			     type == R_68K_JMP_SLOT)) ||
			   (machine == EM_MICROBLAZE &&
			    (type == R_MICROBLAZE_32 ||
			     type == R_MICROBLAZE_GLOB_DAT))) {
			/* global symbols read it and add reloc offset */
			index = swrela.r_info >> 8;
			pos_dyn = dyn_start + sizeof(Elf32_Sym) * index;

			debug("Index:\t%d\n", index);
			debug("Pos_dyn:\t0x%x\n", pos_dyn);

			if (fseek(f, pos_dyn, SEEK_SET) < 0) {
				fprintf(stderr, "%s: %s: seek to %"
					PRIx32 " failed: %s\n",
					argv[0], argv[1], pos_dyn, strerror(errno));
				return 5;
			}

			if (fread(&symbols, sizeof(symbols), 1, f) != 1) {
				fprintf(stderr, "%s: %s: read symbols failed at %"
						PRIx32 "\n",
					argv[0], argv[1], pos_dyn);
				return 4;
			}

			debug("Symbol description:\n");
			debug(" st_name:\t0x%x\n", elf32_to_cpu(symbols.st_name));
			debug(" st_value:\t0x%x\n", elf32_to_cpu(symbols.st_value));
			debug(" st_size:\t0x%x\n", elf32_to_cpu(symbols.st_size));

			value = swrela.r_addend + elf32_to_cpu(symbols.st_value);

			debug("Value:\t0x%x\n", value);

			value = cpu_to_elf32(value);

			if (fseek(f, addr, SEEK_SET) < 0) {
				fprintf(stderr, "%s: %s: seek to %"
					PRIx32 " failed: %s\n",
					argv[0], argv[1], addr, strerror(errno));
				return 5;
			}

			if (fwrite(&value, sizeof(rela.r_addend), 1, f) != 1) {
				fprintf(stderr, "%s: %s: write failed at %" PRIx32 "\n",
					argv[0], argv[1], addr);
				return 4;
			}
		} else if (machine == EM_M68K && type == R_68K_NONE) {
			debug("R_68K_NONE - skip\n");
		} else if (machine == EM_MICROBLAZE && type == R_MICROBLAZE_NONE) {
			debug("R_MICROBLAZE_NONE - skip\n");
		} else {
			fprintf(stderr, "warning: unsupported relocation type %"
				PRIu32 " at %" PRIx32 "\n",
				type, rela.r_offset);
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	FILE *f;
	int ret;
	uint64_t file_size;

	if (argc != 3) {
		fprintf(stderr, "Statically apply ELF rela relocations\n");
		fprintf(stderr, "Usage: %s <bin file> <u-boot ELF>\n",
			argv[0]);
		return 1;
	}

	ret = decode_elf(argv);
	if (ret) {
		fprintf(stderr, "ELF decoding failed\n");
		return ret;
	}

	if (rela_start > rela_end || rela_start < text_base) {
		fprintf(stderr, "%s: bad rela bounds\n", argv[0]);
		return 3;
	}

	rela_start -= text_base;
	rela_end -= text_base;
	dyn_start -= text_base;

	f = fopen(argv[1], "r+b");
	if (!f) {
		fprintf(stderr, "%s: Cannot open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		return 2;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	if (rela_end > file_size) {
		// Most likely compiler inserted some section that didn't get
		// objcopy-ed into the final binary
		rela_end = file_size;
	}

	if (ei_class == 2)
		ret = rela_elf64(argv, f);
	else
		ret = rela_elf32(argv, f);

	if (fclose(f) < 0) {
		fprintf(stderr, "%s: %s: close failed: %s\n",
			argv[0], argv[1], strerror(errno));
		return 4;
	}

	return ret;
}
