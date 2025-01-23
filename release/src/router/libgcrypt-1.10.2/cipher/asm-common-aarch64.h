/* asm-common-aarch64.h  -  Common macros for AArch64 assembly
 *
 * Copyright (C) 2018 Martin Storsjö <martin@martin.st>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GCRY_ASM_COMMON_AARCH64_H
#define GCRY_ASM_COMMON_AARCH64_H

#include <config.h>

#ifdef HAVE_GCC_ASM_ELF_DIRECTIVES
# define ELF(...) __VA_ARGS__
#else
# define ELF(...) /*_*/
#endif

#ifdef __APPLE__
#define GET_DATA_POINTER(reg, name) \
	adrp    reg, name@GOTPAGE ; \
	add     reg, reg, name@GOTPAGEOFF ;
#elif defined(_WIN32)
#define GET_DATA_POINTER(reg, name) \
	adrp    reg, name ; \
	add     reg, reg, #:lo12:name ;
#else
#define GET_DATA_POINTER(reg, name) \
	adrp    reg, :got:name ; \
	ldr     reg, [reg, #:got_lo12:name] ;
#endif

#ifdef HAVE_GCC_ASM_CFI_DIRECTIVES
/* CFI directives to emit DWARF stack unwinding information. */
# define CFI_STARTPROC()            .cfi_startproc
# define CFI_ENDPROC()              .cfi_endproc
# define CFI_REMEMBER_STATE()       .cfi_remember_state
# define CFI_RESTORE_STATE()        .cfi_restore_state
# define CFI_ADJUST_CFA_OFFSET(off) .cfi_adjust_cfa_offset off
# define CFI_REL_OFFSET(reg,off)    .cfi_rel_offset reg, off
# define CFI_DEF_CFA_REGISTER(reg)  .cfi_def_cfa_register reg
# define CFI_REGISTER(ro,rn)        .cfi_register ro, rn
# define CFI_RESTORE(reg)           .cfi_restore reg

/* CFA expressions are used for pointing CFA and registers to
 * SP relative offsets. */
# define DW_REGNO_SP 31

/* Fixed length encoding used for integers for now. */
# define DW_SLEB128_7BIT(value) \
	0x00|((value) & 0x7f)
# define DW_SLEB128_28BIT(value) \
	0x80|((value)&0x7f), \
	0x80|(((value)>>7)&0x7f), \
	0x80|(((value)>>14)&0x7f), \
	0x00|(((value)>>21)&0x7f)

# define CFI_CFA_ON_STACK(rsp_offs,cfa_depth) \
	.cfi_escape \
	  0x0f, /* DW_CFA_def_cfa_expression */ \
	    DW_SLEB128_7BIT(11), /* length */ \
	  0x8f, /* DW_OP_breg31, rsp + constant */ \
	    DW_SLEB128_28BIT(rsp_offs), \
	  0x06, /* DW_OP_deref */ \
	  0x23, /* DW_OP_plus_constu */ \
	    DW_SLEB128_28BIT((cfa_depth)+8)

# define CFI_REG_ON_STACK(regno,rsp_offs) \
	.cfi_escape \
	  0x10, /* DW_CFA_expression */ \
	    DW_SLEB128_7BIT(regno), \
	    DW_SLEB128_7BIT(5), /* length */ \
	  0x8f, /* DW_OP_breg31, rsp + constant */ \
	    DW_SLEB128_28BIT(rsp_offs)

#else
# define CFI_STARTPROC()
# define CFI_ENDPROC()
# define CFI_REMEMBER_STATE()
# define CFI_RESTORE_STATE()
# define CFI_ADJUST_CFA_OFFSET(off)
# define CFI_REL_OFFSET(reg,off)
# define CFI_DEF_CFA_REGISTER(reg)
# define CFI_REGISTER(ro,rn)
# define CFI_RESTORE(reg)

# define CFI_CFA_ON_STACK(rsp_offs,cfa_depth)
# define CFI_REG_ON_STACK(reg,rsp_offs)
#endif

/* 'ret' instruction replacement for straight-line speculation mitigation */
#define ret_spec_stop \
	ret; dsb sy; isb;

#endif /* GCRY_ASM_COMMON_AARCH64_H */
