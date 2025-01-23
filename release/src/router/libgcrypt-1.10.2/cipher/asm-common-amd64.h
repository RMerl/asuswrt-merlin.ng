/* asm-common-amd64.h  -  Common macros for AMD64 assembly
 *
 * Copyright (C) 2018 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#ifndef GCRY_ASM_COMMON_AMD64_H
#define GCRY_ASM_COMMON_AMD64_H

#include <config.h>

#ifdef HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS
# define ELF(...) __VA_ARGS__
#else
# define ELF(...) /*_*/
#endif

#ifdef __PIC__
#  define rRIP (%rip)
#else
#  define rRIP
#endif

#ifdef __PIC__
#  define RIP %rip
#else
#  define RIP
#endif

#ifdef __PIC__
#  define ADD_RIP +rip
#else
#  define ADD_RIP
#endif

#if defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS) || !defined(__PIC__)
#  define GET_EXTERN_POINTER(name, reg) movabsq $name, reg
#else
#  ifdef __code_model_large__
#    define GET_EXTERN_POINTER(name, reg) \
	       pushq %r15; \
	       pushq %r14; \
	    1: leaq 1b(%rip), reg; \
	       movabsq $_GLOBAL_OFFSET_TABLE_-1b, %r14; \
	       movabsq $name@GOT, %r15; \
	       addq %r14, reg; \
	       popq %r14; \
	       movq (reg, %r15), reg; \
	       popq %r15;
#  else
#    define GET_EXTERN_POINTER(name, reg) movq name@GOTPCREL(%rip), reg
#  endif
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

# define CFI_PUSH(reg) \
	CFI_ADJUST_CFA_OFFSET(8); CFI_REL_OFFSET(reg, 0)
# define CFI_POP(reg) \
	CFI_ADJUST_CFA_OFFSET(-8); CFI_RESTORE(reg)
# define CFI_POP_TMP_REG() \
	CFI_ADJUST_CFA_OFFSET(-8);
# define CFI_LEAVE() \
	CFI_ADJUST_CFA_OFFSET(-8); CFI_DEF_CFA_REGISTER(%rsp)

/* CFA expressions are used for pointing CFA and registers to
 * %rsp relative offsets. */
# define DW_REGNO_rax 0
# define DW_REGNO_rdx 1
# define DW_REGNO_rcx 2
# define DW_REGNO_rbx 3
# define DW_REGNO_rsi 4
# define DW_REGNO_rdi 5
# define DW_REGNO_rbp 6
# define DW_REGNO_rsp 7
# define DW_REGNO_r8  8
# define DW_REGNO_r9  9
# define DW_REGNO_r10 10
# define DW_REGNO_r11 11
# define DW_REGNO_r12 12
# define DW_REGNO_r13 13
# define DW_REGNO_r14 14
# define DW_REGNO_r15 15

# define DW_REGNO(reg) DW_REGNO_ ## reg

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
	  0x77, /* DW_OP_breg7, rsp + constant */ \
	    DW_SLEB128_28BIT(rsp_offs), \
	  0x06, /* DW_OP_deref */ \
	  0x23, /* DW_OP_plus_constu */ \
	    DW_SLEB128_28BIT((cfa_depth)+8)

# define CFI_REG_ON_STACK(reg,rsp_offs) \
	.cfi_escape \
	  0x10, /* DW_CFA_expression */ \
	    DW_SLEB128_7BIT(DW_REGNO(reg)), \
	    DW_SLEB128_7BIT(5), /* length */ \
	  0x77, /* DW_OP_breg7, rsp + constant */ \
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

# define CFI_PUSH(reg)
# define CFI_POP(reg)
# define CFI_POP_TMP_REG()
# define CFI_LEAVE()

# define CFI_CFA_ON_STACK(rsp_offs,cfa_depth)
# define CFI_REG_ON_STACK(reg,rsp_offs)
#endif

#ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
# define ENTER_SYSV_FUNC_PARAMS_0_4 \
	pushq %rdi; \
	CFI_PUSH(%rdi); \
	pushq %rsi; \
	CFI_PUSH(%rsi); \
	movq %rcx, %rdi; \
	movq %rdx, %rsi; \
	movq %r8, %rdx; \
	movq %r9, %rcx; \

# define ENTER_SYSV_FUNC_PARAMS_5 \
	ENTER_SYSV_FUNC_PARAMS_0_4; \
	movq 0x38(%rsp), %r8;

# define ENTER_SYSV_FUNC_PARAMS_6 \
	ENTER_SYSV_FUNC_PARAMS_5; \
	movq 0x40(%rsp), %r9;

# define EXIT_SYSV_FUNC \
	popq %rsi; \
	CFI_POP(%rsi); \
	popq %rdi; \
	CFI_POP(%rdi);
#else
# define ENTER_SYSV_FUNC_PARAMS_0_4
# define ENTER_SYSV_FUNC_PARAMS_5
# define ENTER_SYSV_FUNC_PARAMS_6
# define EXIT_SYSV_FUNC
#endif

/* 'ret' instruction replacement for straight-line speculation mitigation */
#define ret_spec_stop \
	ret; int3;

#endif /* GCRY_ASM_COMMON_AMD64_H */
