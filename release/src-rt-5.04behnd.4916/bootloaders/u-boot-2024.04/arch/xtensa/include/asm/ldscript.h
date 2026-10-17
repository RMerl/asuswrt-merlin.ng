/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007 Tensilica, Inc.
 * (C) Copyright 2014 - 2016 Cadence Design Systems Inc.
 */

#ifndef _XTENSA_LDSCRIPT_H
#define _XTENSA_LDSCRIPT_H

/*
 * This linker script is pre-processed with CPP to avoid hard-coding
 * addresses that depend on the Xtensa core configuration, because
 * this FPGA board can be used with a huge variety of Xtensa cores.
 */

#include <asm/arch/core.h>
#include <asm/addrspace.h>

#define ALIGN_LMA	4
#define LMA_EQ_VMA
#define FORCE_OUTPUT	. = .
#define FOLLOWING(sec)							\
	AT(((LOADADDR(sec) + SIZEOF(sec) + ALIGN_LMA-1)) & ~(ALIGN_LMA-1))

/*
 * Specify an output section that will be added to the ROM store table
 * (PACKED_SECTION) or one that will be resident in ROM (RESIDENT_SECTION).
 * 'symname' is a base name for section boundary symbols *_start & *_end.
 * 'lma' is the load address at which a section will be packed in ROM.
 * 'region' is the basename identifying a memory region and program header.
 * 'keep' prevents removal of empty sections (must be 'KEEP' or 'NOKEEP').
 */

#define RELOCATE1(_sec_)						\
	LONG(_##_sec_##_start);						\
	LONG(_##_sec_##_end);						\
	LONG(LOADADDR(.##_sec_));

#define RELOCATE2(_sym_, _sec_)						\
	LONG(_##_sym_##_##_sec_##_start);				\
	LONG(_##_sym_##_##_sec_##_end);					\
	LONG(LOADADDR(.##_sym_##.##_sec_));

#define RELOCATE_USER1(_sec_)						\
	LONG(_##_sec_##_start);						\
	LONG(_##_sec_##_end);						\
	LONG(LOADADDR(_sec_));

#define SECTION_VECTOR(_sym_, _sec_, _vma_, _lma_)			\
.##_sym_##.##_sec_ _vma_ : _lma_					\
{									\
	. = ALIGN(4);							\
	_##_sym_##_##_sec_##_start = ABSOLUTE(.);			\
	KEEP(*(.##_sym_##.##_sec_))					\
	_##_sym_##_##_sec_##_end = ABSOLUTE(.);				\
}

/* In MMU configs there are two aliases of SYSROM, cached and uncached.
 * For various reasons it is simpler to use the uncached mapping for load
 * addresses, so ROM sections end up contiguous with the reset vector and
 * we get a compact binary image. However we can gain performance by doing
 * the unpacking from the cached ROM mapping. So we adjust all the load
 * addresses in the ROM store table with an offset to the cached mapping,
 * including the symbols referring to the ROM store table itself.
 */

#define SECTION_ResetVector(_vma_, _lma_)				\
	.ResetVector.text _vma_ : _lma_					\
	{								\
		FORCE_OUTPUT;						\
		KEEP(*(.ResetVector.text));				\
		KEEP(*(.reset.literal .reset.text))			\
	}

#define SECTION_text(_vma_, _lma_)					\
	.text _vma_ : _lma_						\
	{								\
		_text_start = ABSOLUTE(.);				\
		*(.literal .text)					\
		*(.literal.* .text.* .stub)				\
		*(.gnu.warning .gnu.linkonce.literal.*)			\
		*(.gnu.linkonce.t.*.literal .gnu.linkonce.t.*)		\
		*(.fini.literal)					\
		*(.fini)						\
		*(.gnu.version)						\
		_text_end = ABSOLUTE(.);				\
	}

#define SECTION_rodata(_vma_, _lma_)					\
	.rodata _vma_ : _lma_						\
	{								\
		_rodata_start = ABSOLUTE(.);				\
		*(.rodata)						\
		*(.rodata.*)						\
		*(.dtb.init.rodata)					\
		*(.gnu.linkonce.r.*)					\
		*(.rodata1)						\
		__XT_EXCEPTION_TABLE__ = ABSOLUTE(.);			\
		*(.xt_except_table)					\
		*(.gcc_except_table)					\
		*(.gnu.linkonce.e.*)					\
		*(.gnu.version_r)					\
		. = ALIGN(16);						\
		 _rodata_end = ABSOLUTE(.);				\
	}

#define SECTION_u_boot_list(_vma_, _lma_)				\
	__u_boot_list _vma_ : _lma_					\
	{								\
		___u_boot_list_start = ABSOLUTE(.);			\
		KEEP(*(SORT(__u_boot_list*)));				\
		___u_boot_list_end = ABSOLUTE(.);			\
	}

#define SECTION_data(_vma_, _lma_)					\
	.data _vma_ : _lma_						\
	{								\
		_data_start = ABSOLUTE(.);				\
		*(.data)						\
		*(.data.*)						\
		*(.gnu.linkonce.d.*)					\
		*(.data1)						\
		*(.sdata)						\
		*(.sdata.*)						\
		*(.gnu.linkonce.s.*)					\
		*(.sdata2)						\
		*(.sdata2.*)						\
		*(.gnu.linkonce.s2.*)					\
		*(.jcr)							\
		*(.eh_frame)						\
		*(.dynamic)						\
		*(.gnu.version_d)					\
		_data_end = ABSOLUTE(.);				\
	}

#define SECTION_lit4(_vma_, _lma_)					\
	.lit4 _vma_ : _lma_						\
	{								\
		_lit4_start = ABSOLUTE(.);				\
		*(*.lit4)						\
		*(.gnu.linkonce.lit4.*)					\
		_lit4_end = ABSOLUTE(.);				\
	}

#define SECTION_bss(_vma_, _lma_)					\
	.bss _vma_ : _lma_						\
	{								\
		. = ALIGN(8);						\
		_bss_start = ABSOLUTE(.);				\
		__bss_start = ABSOLUTE(.);				\
		*(.dynsbss)						\
		*(.sbss)						\
		*(.sbss.*)						\
		*(.gnu.linkonce.sb.*)					\
		*(.scommon)						\
		*(.sbss2)						\
		*(.sbss2.*)						\
		*(.gnu.linkonce.sb2.*)					\
		*(.dynbss)						\
		*(.bss)							\
		*(.bss.*)						\
		*(.gnu.linkonce.b.*)					\
		*(COMMON)						\
		*(.sram.bss)						\
		. = ALIGN(8);						\
		_bss_end = ABSOLUTE(.);					\
		__bss_end = ABSOLUTE(.);				\
		_end = ALIGN(0x8);					\
		PROVIDE(end = ALIGN(0x8));				\
		_stack_sentry = ALIGN(0x8);				\
	}

#define SECTION_debug							\
	.debug           0 :  { *(.debug) }				\
	.line            0 :  { *(.line) }				\
	.debug_srcinfo   0 :  { *(.debug_srcinfo) }			\
	.debug_sfnames   0 :  { *(.debug_sfnames) }			\
	.debug_aranges   0 :  { *(.debug_aranges) }			\
	.debug_pubnames  0 :  { *(.debug_pubnames) }			\
	.debug_info      0 :  { *(.debug_info) }			\
	.debug_abbrev    0 :  { *(.debug_abbrev) }			\
	.debug_line      0 :  { *(.debug_line) }			\
	.debug_frame     0 :  { *(.debug_frame) }			\
	.debug_str       0 :  { *(.debug_str) }				\
	.debug_loc       0 :  { *(.debug_loc) }				\
	.debug_macinfo   0 :  { *(.debug_macinfo) }			\
	.debug_weaknames 0 :  { *(.debug_weaknames) }			\
	.debug_funcnames 0 :  { *(.debug_funcnames) }			\
	.debug_typenames 0 :  { *(.debug_typenames) }			\
	.debug_varnames  0 :  { *(.debug_varnames) }

#define SECTION_xtensa							\
	.xt.insn 0 :							\
	{								\
		KEEP (*(.xt.insn))					\
		KEEP (*(.gnu.linkonce.x.*))				\
	}								\
	.xt.prop 0 :							\
	{								\
		KEEP (*(.xt.prop))					\
		KEEP (*(.xt.prop.*))					\
		KEEP (*(.gnu.linkonce.prop.*))				\
	}								\
	.xt.lit 0 :							\
	{								\
		KEEP (*(.xt.lit))					\
		KEEP (*(.xt.lit.*))					\
		KEEP (*(.gnu.linkonce.p.*))				\
	}								\
	.xt.profile_range 0 :						\
	{								\
		KEEP (*(.xt.profile_range))				\
		KEEP (*(.gnu.linkonce.profile_range.*))			\
	}								\
	.xt.profile_ranges 0 :						\
	{								\
		KEEP (*(.xt.profile_ranges))				\
		KEEP (*(.gnu.linkonce.xt.profile_ranges.*))		\
	}								\
	.xt.profile_files 0 :						\
	{								\
		KEEP (*(.xt.profile_files))				\
		KEEP (*(.gnu.linkonce.xt.profile_files.*))		\
	}

#endif	/* _XTENSA_LDSCRIPT_H */
