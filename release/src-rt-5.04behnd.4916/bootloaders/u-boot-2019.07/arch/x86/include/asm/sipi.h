/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Gooogle, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _ASM_SIPI_H
#define _ASM_SIPI_H

#define AP_DEFAULT_BASE 0x30000
#define AP_DEFAULT_SIZE 0x10000

#ifndef __ASSEMBLER__

/**
 * struct sipi_params_16bit - 16-bit SIPI entry-point parameters
 *
 * These are set up in the same space as the SIPI 16-bit code so that each AP
 * can access the parameters when it boots.
 *
 * Each of these must be set up for the AP to boot, except @segment which is
 * set in the assembly code.
 *
 * @ap_start:		32-bit SIPI entry point for U-Boot
 * @segment:		Code segment for U-Boot
 * @pad:		Padding (not used)
 * @gdt_limit:		U-Boot GDT limit (X86_GDT_SIZE - 1)
 * @gdt:		U-Boot GDT (gd->arch.gdt)
 * @unused:		Not used
 */
struct __packed sipi_params_16bit {
	u32 ap_start;
	u16 segment;
	u16 pad;
	u16 gdt_limit;
	u32 gdt;
	u16 unused;
};

/**
 * struct sipi_params - 32-bit SIP entry-point parameters
 *
 * These are used by the AP init code and must be set up before the APs start.
 * The members must match with the sipi_params layout in sipi_vector.S.
 *
 * The stack area extends down from @stack_top, with @stack_size allocated
 * for each AP.
 *
 * @idt_ptr:		Interrupt descriptor table pointer
 * @stack_top:		Top of the AP stack area
 * @stack_size:		Size of each AP's stack
 * @microcode_lock:	Used to ensure only one AP loads microcode at once
 *			0xffffffff enables parallel loading.
 * @microcode_ptr:	Pointer to microcode, or 0 if none
 * @msr_table_ptr:	Pointer to saved MSRs, a list of struct saved_msr
 * @msr_count:		Number of saved MSRs
 * @c_handler:		C function to call once early init is complete
 * @ap_count:		Shared atomic value to allocate CPU indexes
 */
struct sipi_params {
	u32 idt_ptr;
	u32 stack_top;
	u32 stack_size;
	u32 microcode_lock;
	u32 microcode_ptr;
	u32 msr_table_ptr;
	u32 msr_count;
	u32 c_handler;
	atomic_t ap_count;
};

/* 16-bit AP entry point */
void ap_start16(void);

/* end of 16-bit code/data, marks the region to be copied to SIP vector */
void ap_start16_code_end(void);

/* 32-bit AP entry point */
void ap_start(void);

extern char sipi_params_16bit[];
extern char sipi_params[];

#endif /* __ASSEMBLER__ */

#endif
