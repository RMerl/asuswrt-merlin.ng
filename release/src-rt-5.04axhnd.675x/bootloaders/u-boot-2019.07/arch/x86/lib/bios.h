/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From Coreboot file device/oprom/realmode/x86.h
 *
 * Copyright (C) 2007 Advanced Micro Devices, Inc.
 * Copyright (C) 2009-2010 coresystems GmbH
 */

#ifndef _X86_LIB_BIOS_H
#define _X86_LIB_BIOS_H

#include <linux/linkage.h>

#define REALMODE_BASE		0x600

#ifdef __ASSEMBLY__

#define PTR_TO_REAL_MODE(x)	(x - asm_realmode_code + REALMODE_BASE)

#else

/* Convert a symbol address to our real mode area */
#define PTR_TO_REAL_MODE(sym)\
	(void *)(REALMODE_BASE + ((char *)&(sym) - (char *)&asm_realmode_code))

/*
 * The following symbols cannot be used directly. They need to be fixed up
 * to point to the correct address location after the code has been copied
 * to REALMODE_BASE. Absolute symbols are not used because those symbols are
 * relocated by U-Boot.
 */
extern unsigned char asm_realmode_call, __realmode_interrupt;
extern unsigned char asm_realmode_buffer;

#define DOWNTO8(A) \
	union { \
		struct { \
			union { \
				struct { \
					uint8_t A##l; \
					uint8_t A##h; \
				} __packed; \
				uint16_t A##x; \
			} __packed; \
			uint16_t h##A##x; \
		} __packed; \
		uint32_t e##A##x; \
	} __packed;

#define DOWNTO16(A) \
	union { \
		struct { \
			uint16_t A; \
			uint16_t h##A; \
		} __packed; \
		uint32_t e##A; \
	} __packed;

struct eregs {
	DOWNTO8(a);
	DOWNTO8(c);
	DOWNTO8(d);
	DOWNTO8(b);
	DOWNTO16(sp);
	DOWNTO16(bp);
	DOWNTO16(si);
	DOWNTO16(di);
	uint32_t vector;
	uint32_t error_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
};

struct realmode_idt {
	u16 offset, cs;
};

void x86_exception(struct eregs *info);

/* From x86_asm.S */
extern unsigned char __idt_handler;
extern unsigned int __idt_handler_size;
extern unsigned char asm_realmode_code;
extern unsigned int asm_realmode_code_size;

asmlinkage void (*realmode_call)(u32 addr, u32 eax, u32 ebx, u32 ecx, u32 edx,
				 u32 esi, u32 edi);

asmlinkage void (*realmode_interrupt)(u32 intno, u32 eax, u32 ebx, u32 ecx,
				      u32 edx, u32 esi, u32 edi);

int int10_handler(void);
int int12_handler(void);
int int16_handler(void);
int int1a_handler(void);
#endif /*__ASSEMBLY__ */

#endif
