C x86_64/ghash-update.asm

ifelse(`
   Copyright (C) 2013, 2022 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
')

C Register usage:

define(`KEY', `%rdi')
define(`XP', `%rsi')
define(`BLOCKS', `%rdx')
define(`SRC', `%rcx')
define(`CNT', `%rax')
define(`KEY32', `%r8')
define(`X', `%xmm0')
define(`R', `%xmm1')
define(`M0', `%xmm2')
define(`M1', `%xmm3')
define(`M2', `%xmm4')
define(`M3', `%xmm5')
define(`T0', `%xmm6')
define(`T1', `%xmm7')

	.file "ghash-update.asm"

	C const uint8_t *_ghash_update (const struct gcm_key *key,
	C				union nettle_block16 *x,
	C				size_t blocks, const uint8_t *data)

	.text
	ALIGN(16)
PROLOGUE(_nettle_ghash_update)
	W64_ENTRY(4, 8)
	sub	$1, BLOCKS
	movups	(XP), X
	jc	.Ldone
	C Table offset corresponding to 32 bits.
	lea	1024(KEY), KEY32

ALIGN(16)
.Lblock_loop:
	C Unaligned input
	movups	(SRC), M0
	pxor	M0, X
	pxor	R, R
	mov	$992, CNT
ALIGN(16)
.Loop_bit:
	movdqa	X, M3
	psrad	$31, M3
	pshufd	$0x00, M3, M0
	pshufd	$0x55, M3, M1
	pshufd	$0xaa, M3, M2
	pshufd	$0xff, M3, M3
	pslld	$1, X
	C Tabulated values are only 8-byte aligned, and therefore
	C can't be used as memory operands to pand. It would be nice
	C if we could force 16-byte alignment on nettle_block16, using
	C C11 alignas.
	movups	(KEY, CNT), T0
	movups	(KEY32, CNT), T1
	pand	T0, M0
	pand	T1, M1
	movups	16(KEY, CNT), T0
	movups	16(KEY32, CNT), T1
	pand	T0, M2
	pand	T1, M3
	pxor	M0, M1
	pxor	M2, M3
	pxor	M1, R
	pxor	M3, R

	sub	$32, CNT
	jnc	.Loop_bit

	movaps	R, X

	add	$16, SRC
	sub	$1, BLOCKS
	jnc	.Lblock_loop

.Ldone:
	movups	X, (XP)
	mov	SRC, %rax
	W64_EXIT(4, 8)
	ret
EPILOGUE(_nettle_ghash_update)
