C x86_64/aesni/cbc-aes256-encrypt.asm

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
define(`IV',	`%rsi')
define(`LENGTH',`%rdx')
define(`DST',	`%rcx')
define(`SRC',	`%r8')

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
define(`BLOCK', `%xmm15')

	.file "cbc-aes256-encrypt.asm"

	C nettle_cbc_aes256_encrypt(struct cbc_aes256_ctx *ctx,
	C                           uint8_t *iv,
	C                           size_t length, uint8_t *dst,
	C                           const uint8_t *src);

	.text
	ALIGN(16)
PROLOGUE(nettle_cbc_aes256_encrypt)
	W64_ENTRY(5, 16)
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
	movups	(IV), X

.Lblock_loop:
	movups	(SRC), BLOCK	C Cleartext block
	pxor	KEY0_7, X
	movups	112(CTX), KEY0_7
	pxor	BLOCK, X
	aesenc	KEY1, X
	aesenc	KEY2, X
	aesenc	KEY3, X
	aesenc	KEY4, X
	aesenc	KEY5, X
	aesenc	KEY6, X
	aesenc	KEY0_7, X
	movups	(CTX), KEY0_7
	aesenc	KEY8, X
	aesenc	KEY9, X
	aesenc	KEY10, X
	aesenc	KEY11, X
	aesenc	KEY12, X
	aesenc	KEY13, X
	aesenclast KEY14, X

	movups	X, (DST)
	add	$16, SRC
	add	$16, DST

	dec	LENGTH
	jnz	.Lblock_loop

	movups	X, (IV)

.Lend:
	W64_EXIT(5, 16)
	ret
EPILOGUE(nettle_cbc_aes256_encrypt)
