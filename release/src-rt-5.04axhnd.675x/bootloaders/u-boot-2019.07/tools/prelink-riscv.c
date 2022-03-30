// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology
 * Chih-Mao Chen <cmchen@andestech.com>
 *
 * Statically process runtime relocations on RISC-V ELF images
 * so that it can be directly executed when loaded at LMA
 * without fixup. Both RV32 and RV64 are supported.
 */

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Only little-endian host is supported"
#endif

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef EM_RISCV
#define EM_RISCV 243
#endif

#ifndef R_RISCV_32
#define R_RISCV_32 1
#endif

#ifndef R_RISCV_64
#define R_RISCV_64 2
#endif

#ifndef R_RISCV_RELATIVE
#define R_RISCV_RELATIVE 3
#endif

const char *argv0;

#define die(fmt, ...) \
	do { \
		fprintf(stderr, "%s: " fmt "\n", argv0, ## __VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while (0)

#define PRELINK_INC_BITS 32
#include "prelink-riscv.inc"
#undef PRELINK_INC_BITS

#define PRELINK_INC_BITS 64
#include "prelink-riscv.inc"
#undef PRELINK_INC_BITS

int main(int argc, const char *const *argv)
{
	argv0 = argv[0];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <u-boot>\n", argv0);
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_RDWR, 0);

	if (fd < 0)
		die("Cannot open %s: %s", argv[1], strerror(errno));

	struct stat st;

	if (fstat(fd, &st) < 0)
		die("Cannot stat %s: %s", argv[1], strerror(errno));

	void *data =
		mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (data == MAP_FAILED)
		die("Cannot mmap %s: %s", argv[1], strerror(errno));

	close(fd);

	unsigned char *e_ident = (unsigned char *)data;

	if (memcmp(e_ident, ELFMAG, SELFMAG) != 0)
		die("Invalid ELF file %s", argv[1]);

	bool is64 = e_ident[EI_CLASS] == ELFCLASS64;

	if (is64)
		prelink64(data);
	else
		prelink32(data);

	return 0;
}
