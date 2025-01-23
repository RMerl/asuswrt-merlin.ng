/* AMD64 (x86_64) lshift -- Left shift a limb vector and store
 *		  result in a second limb vector.
 *
 *      Copyright (C) 1992, 1994, 1995, 1998, 
 *                    2001, 2002, 2006 Free Software Foundation, Inc.
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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * Note: This code is heavily based on the GNU MP Library.
 *	 Actually it's the same code with only minor changes in the
 *	 way the data is stored; this is to support the abstraction
 *	 of an optional secure memory allocation which may be used
 *	 to avoid revealing of sensitive data due to paging etc.
 */


#include "sysdep.h"
#include "asm-syntax.h"

/*******************
 * mpi_limb_t
 * _gcry_mpih_lshift( mpi_ptr_t wp,	rdi
 *		   mpi_ptr_t up,	rsi
 *		   mpi_size_t usize,	rdx
 *		   unsigned cnt)	rcx
 */

.text
	.globl C_SYMBOL_NAME(_gcry_mpih_lshift)
C_SYMBOL_NAME(_gcry_mpih_lshift:)
	FUNC_ENTRY()
	/* Note: %xmm6 and %xmm7 not used for WIN64 ABI compatibility. */
	movq	-8(%rsi,%rdx,8), %xmm4
	movd	%ecx, %xmm1
	movl	$64, %eax
	subl	%ecx, %eax
	movd	%eax, %xmm0
	movdqa	%xmm4, %xmm3
	psrlq	%xmm0, %xmm4
	movd	%xmm4, %rax
	subq	$2, %rdx
	jl	.Lendo

	ALIGN(4)			/* minimal alignment for claimed speed */
.Loop:	movq	(%rsi,%rdx,8), %xmm5
	movdqa	%xmm5, %xmm2
	psrlq	%xmm0, %xmm5
	psllq	%xmm1, %xmm3
	por	%xmm5, %xmm3
	movq	%xmm3, 8(%rdi,%rdx,8)
	je	.Lende
	movq	-8(%rsi,%rdx,8), %xmm4
	movdqa	%xmm4, %xmm3
	psrlq	%xmm0, %xmm4
	psllq	%xmm1, %xmm2
	por	%xmm4, %xmm2
	movq	%xmm2, (%rdi,%rdx,8)
	subq	$2, %rdx
	jge	.Loop

.Lendo:	movdqa	%xmm3, %xmm2
.Lende:	psllq	%xmm1, %xmm2
	movq	%xmm2, (%rdi)
	FUNC_EXIT()
