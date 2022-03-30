/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2017 NVIDIA Corporation <www.nvidia.com>
 *
 * Derived from Linux kernel v4.14 files:
 *
 * arch/arm64/include/asm/assembler.h:
 * Based on arch/arm/include/asm/assembler.h, arch/arm/mm/proc-macros.S
 * Copyright (C) 1996-2000 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * arch/arm64/kernel/head.S:
 * Based on arch/arm/kernel/head.S
 * Copyright (C) 1994-2002 Russell King
 * Copyright (C) 2003-2012 ARM Ltd.
 * Authors:	Catalin Marinas <catalin.marinas@arm.com>
 *		Will Deacon <will.deacon@arm.com>
 *
 * arch/arm64/kernel/image.h:
 * Copyright (C) 2014 ARM Ltd.
 */

/*
 * There aren't any ELF relocations we can use to endian-swap values known only
 * at link time (e.g. the subtraction of two symbol addresses), so we must get
 * the linker to endian-swap certain values before emitting them.
 *
 * Note that, in order for this to work when building the ELF64 PIE executable
 * (for KASLR), these values should not be referenced via R_AARCH64_ABS64
 * relocations, since these are fixed up at runtime rather than at build time
 * when PIE is in effect. So we need to split them up in 32-bit high and low
 * words.
 */
#ifdef CONFIG_CPU_BIG_ENDIAN
#define DATA_LE32(data)				\
	((((data) & 0x000000ff) << 24) |	\
	 (((data) & 0x0000ff00) << 8)  |	\
	 (((data) & 0x00ff0000) >> 8)  |	\
	 (((data) & 0xff000000) >> 24))
#else
#define DATA_LE32(data) ((data) & 0xffffffff)
#endif

#define DEFINE_IMAGE_LE64(sym, data)				\
	sym##_lo32 = DATA_LE32((data) & 0xffffffff);		\
	sym##_hi32 = DATA_LE32((data) >> 32)

#define __MAX(a, b)		(((a) > (b)) ? (a) : (b))
#define __CODE_DATA_SIZE	(__bss_start - _start)
#define __BSS_SIZE		(__bss_end - __bss_start)
#ifdef CONFIG_SYS_INIT_SP_BSS_OFFSET
#define __MAX_EXTRA_RAM_USAGE	__MAX(__BSS_SIZE, CONFIG_SYS_INIT_SP_BSS_OFFSET)
#else
#define __MAX_EXTRA_RAM_USAGE	__BSS_SIZE
#endif
#define __MEM_USAGE		(__CODE_DATA_SIZE + __MAX_EXTRA_RAM_USAGE)

#ifdef CONFIG_CPU_BIG_ENDIAN
#define __HEAD_FLAG_BE		1
#else
#define __HEAD_FLAG_BE		0
#endif

#define __HEAD_FLAG_PAGE_SIZE	1 /* 4K hard-coded */

#define __HEAD_FLAG_PHYS_BASE	1

#define __HEAD_FLAGS		((__HEAD_FLAG_BE << 0) |	\
				 (__HEAD_FLAG_PAGE_SIZE << 1) |	\
				 (__HEAD_FLAG_PHYS_BASE << 3))

#define TEXT_OFFSET (CONFIG_SYS_TEXT_BASE - \
			CONFIG_LNX_KRNL_IMG_TEXT_OFFSET_BASE)

/*
 * These will output as part of the Image header, which should be little-endian
 * regardless of the endianness of the kernel. While constant values could be
 * endian swapped in head.S, all are done here for consistency.
 */
#define HEAD_SYMBOLS						\
	DEFINE_IMAGE_LE64(_kernel_size_le, __MEM_USAGE);	\
	DEFINE_IMAGE_LE64(_kernel_offset_le, TEXT_OFFSET);	\
	DEFINE_IMAGE_LE64(_kernel_flags_le, __HEAD_FLAGS);

	HEAD_SYMBOLS
