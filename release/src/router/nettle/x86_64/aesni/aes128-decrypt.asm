C x86_64/aesni/aes128-decrypt.asm

ifelse(`
   Copyright (C) 2015, 2018, 2021 Niels Möller

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

C Input argument
define(`CTX',	`%rdi')
define(`LENGTH',`%rsi')
define(`DST',	`%rdx')
define(`SRC',	`%rcx')

define(`KEY0', `%xmm0')
define(`KEY1', `%xmm1')
define(`KEY2', `%xmm2')
define(`KEY3', `%xmm3')
define(`KEY4', `%xmm4')
define(`KEY5', `%xmm5')
define(`KEY6', `%xmm6')
define(`KEY7', `%xmm7')
define(`KEY8', `%xmm8')
define(`KEY9', `%xmm9')
define(`KEY10', `%xmm10')
define(`X', `%xmm11')
define(`Y', `%xmm12')

	.file "aes128-decrypt.asm"

	C nettle_aes128_decrypt(const struct aes128_ctx *ctx,
	C                       size_t length, uint8_t *dst,
	C                       const uint8_t *src);

	.text
	ALIGN(16)
PROLOGUE(nettle_aes128_decrypt)
	W64_ENTRY(4, 13)
	shr	$4, LENGTH
	test	LENGTH, LENGTH
	jz	.Lend

	movups	(CTX), KEY0
	movups	16(CTX), KEY1
	movups	32(CTX), KEY2
	movups	48(CTX), KEY3
	movups	64(CTX), KEY4
	movups	80(CTX), KEY5
	movups	96(CTX), KEY6
	movups	112(CTX), KEY7
	movups	128(CTX), KEY8
	movups	144(CTX), KEY9
	movups	160(CTX), KEY10
	shr	LENGTH
	jnc	.Lblock_loop

	movups	(SRC), X
	pxor	KEY0, X
	aesdec	KEY1, X
	aesdec	KEY2, X
	aesdec	KEY3, X
	aesdec	KEY4, X
	aesdec	KEY5, X
	aesdec	KEY6, X
	aesdec	KEY7, X
	aesdec	KEY8, X
	aesdec	KEY9, X
	aesdeclast KEY10, X

	movups	X, (DST)
	add	$16, SRC
	add	$16, DST
	test	LENGTH, LENGTH
	jz	.Lend

.Lblock_loop:
	movups	(SRC), X
	movups	16(SRC), Y
	pxor	KEY0, X
	pxor	KEY0, Y
	aesdec	KEY1, X
	aesdec	KEY1, Y
	aesdec	KEY2, X
	aesdec	KEY2, Y
	aesdec	KEY3, X
	aesdec	KEY3, Y
	aesdec	KEY4, X
	aesdec	KEY4, Y
	aesdec	KEY5, X
	aesdec	KEY5, Y
	aesdec	KEY6, X
	aesdec	KEY6, Y
	aesdec	KEY7, X
	aesdec	KEY7, Y
	aesdec	KEY8, X
	aesdec	KEY8, Y
	aesdec	KEY9, X
	aesdec	KEY9, Y
	aesdeclast KEY10, X
	aesdeclast KEY10, Y

	movups	X, (DST)
	movups	Y, 16(DST)
	add	$32, SRC
	add	$32, DST
	dec	LENGTH
	jnz	.Lblock_loop

.Lend:
	W64_EXIT(4, 13)
	ret
EPILOGUE(nettle_aes128_decrypt)
