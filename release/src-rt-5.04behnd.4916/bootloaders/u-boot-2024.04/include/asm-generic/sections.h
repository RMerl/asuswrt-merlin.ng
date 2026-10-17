/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

/* Taken from Linux kernel, commit f56c3196 */

#ifndef _ASM_GENERIC_SECTIONS_H_
#define _ASM_GENERIC_SECTIONS_H_

#include <linux/types.h>

/* References to section boundaries */

extern char _text[], _stext[], _etext[];
extern char _data[], _sdata[], _edata[];
extern char __bss_start[], __bss_stop[];
extern char __init_begin[], __init_end[];
extern char _sinittext[], _einittext[];
extern char _end[], _init[];
extern char __per_cpu_load[], __per_cpu_start[], __per_cpu_end[];
extern char __kprobes_text_start[], __kprobes_text_end[];
extern char __entry_text_start[], __entry_text_end[];
extern char __initdata_begin[], __initdata_end[];
extern char __start_rodata[], __end_rodata[];
extern char __efi_helloworld_begin[];
extern char __efi_helloworld_end[];
extern char __efi_var_file_begin[];
extern char __efi_var_file_end[];

/* Private data used by of-platdata devices/uclasses */
extern char __priv_data_start[], __priv_data_end[];

/* Start and end of .ctors section - used for constructor calls. */
extern char __ctors_start[], __ctors_end[];

/* function descriptor handling (if any).  Override
 * in asm/sections.h */
#ifndef dereference_function_descriptor
#define dereference_function_descriptor(p) (p)
#endif

/* random extra sections (if any).  Override
 * in asm/sections.h */
#ifndef arch_is_kernel_text
static inline int arch_is_kernel_text(unsigned long addr)
{
	return 0;
}
#endif

#ifndef arch_is_kernel_data
static inline int arch_is_kernel_data(unsigned long addr)
{
	return 0;
}
#endif

/* U-Boot-specific things begin here */

/* Start of U-Boot text region */
extern char __text_start[];

/* This marks the text region which must be relocated */
extern char __image_copy_start[], __image_copy_end[];

extern char __bss_end[];
extern char __rel_dyn_start[], __rel_dyn_end[];
extern char _image_binary_end[];

/*
 * This is the U-Boot entry point - prior to relocation it should be same
 * as __text_start
 */
extern void _start(void);

#endif /* _ASM_GENERIC_SECTIONS_H_ */
