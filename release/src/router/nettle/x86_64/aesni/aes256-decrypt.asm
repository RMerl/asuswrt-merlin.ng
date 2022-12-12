C x86_64/aesni/aes256-decrypt.asm

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

define(`KEY0_7', `%xmm0')
define(`KEY1', `%xmm1')
define(`KEY2', `%xmm2')
define(`KEY3', `%xmm3')
define(`KEY4', `%xmm4')
define(`KEY5', `%xmm5')
define(`KEY6', `%xmm6')
define(`KEY8', `%xmm7')
define(`KEY9', `%xmm8')
define(`KEY10', `%xmm9')
define(`KEY11', `%xmm10')
define(`KEY12', `%xmm11')
define(`KEY13', `%xmm12')
define(`KEY14', `%xmm13')
define(`X', `%xmm14')
define(`Y', `%xmm15')

	.file "aes256-decrypt.asm"

	C nettle_aes256_decrypt(const struct aes256_ctx *ctx,
	C                       size_t length, uint8_t *dst,
	C                       const uint8_t *src);

	.text
	ALIGN(16)
PROLOGUE(nettle_aes256_decrypt)
	W64_ENTRY(4, 16)
	shr	$4, LENGTH
	test	LENGTH, LENGTH
	jz	.Lend

	movups	(CTX), KEY0_7
	movups	16(CTX), KEY1
	movups	32(CTX), KEY2
	movups	48(CTX), KEY3
	movups	64(CTX), KEY4
	movups	80(CTX), KEY5
	movups	96(CTX), KEY6
	movups	128(CTX), KEY8
	movups	144(CTX), KEY9
	movups	160(CTX), KEY10
	movups	176(CTX), KEY11
	movups	192(CTX), KEY12
	movups	208(CTX), KEY13
	movups	224(CTX), KEY14

	shr	LENGTH
	jnc	.Lblock_loop

	movups	(SRC), X
	pxor	KEY0_7, X
	movups	112(CTX), KEY0_7
	aesdec	KEY1, X
	aesdec	KEY2, X
	aesdec	KEY3, X
	aesdec	KEY4, X
	aesdec	KEY5, X
	aesdec	KEY6, X
	aesdec	KEY0_7, X
	movups	(CTX), KEY0_7
	aesdec	KEY8, X
	aesdec	KEY9, X
	aesdec	KEY10, X
	aesdec	KEY11, X
	aesdec	KEY12, X
	aesdec	KEY13, X
	aesdeclast KEY14, X

	movups	X, (DST)
	add	$16, SRC
	add	$16, DST
	test	LENGTH, LENGTH
	jz	.Lend

.Lblock_loop:
	movups	(SRC), X
	movups	16(SRC), Y
	pxor	KEY0_7, X
	pxor	KEY0_7, Y
	movups	112(CTX), KEY0_7
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
	aesdec	KEY0_7, X
	aesdec	KEY0_7, Y
	movups	(CTX), KEY0_7
	aesdec	KEY8, X
	aesdec	KEY8, Y
	aesdec	KEY9, X
	aesdec	KEY9, Y
	aesdec	KEY10, X
	aesdec	KEY10, Y
	aesdec	KEY11, X
	aesdec	KEY11, Y
	aesdec	KEY12, X
	aesdec	KEY12, Y
	aesdec	KEY13, X
	aesdec	KEY13, Y
	aesdeclast KEY14, X
	aesdeclast KEY14, Y

	movups	X, (DST)
	movups	Y, 16(DST)
	add	$32, SRC
	add	$32, DST
	dec	LENGTH
	jnz	.Lblock_loop

.Lend:
	W64_EXIT(4, 16)
	ret
EPILOGUE(nettle_aes256_decrypt)
