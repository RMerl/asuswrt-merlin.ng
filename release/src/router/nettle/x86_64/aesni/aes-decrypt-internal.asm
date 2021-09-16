C x86_64/aesni/aes-decrypt-internal.asm


ifelse(`
   Copyright (C) 2015, 2018 Niels Möller

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
define(`ROUNDS', `%rdi')
define(`KEYS',	`%rsi')
C define(`TABLE',	`%rdx') C Unused here
define(`LENGTH',`%rcx')
define(`DST',	`%r8')
define(`SRC',	`%r9')

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
define(`KEY11', `%xmm11')
define(`KEY12', `%xmm12')
define(`KEY13', `%xmm13')
define(`KEYLAST', `%xmm14')
define(`BLOCK', `%xmm15')

	.file "aes-decrypt-internal.asm"

	C _aes_decrypt(unsigned rounds, const uint32_t *keys,
	C	       const struct aes_table *T,
	C	       size_t length, uint8_t *dst,
	C	       uint8_t *src)
	.text
	ALIGN(16)
PROLOGUE(_nettle_aes_decrypt)
	W64_ENTRY(6, 16)
	shr	$4, LENGTH
	test	LENGTH, LENGTH
	jz	.Lend

	movups	(KEYS), KEY0
	movups	16(KEYS), KEY1
	movups	32(KEYS), KEY2
	movups	48(KEYS), KEY3
	movups	64(KEYS), KEY4
	movups	80(KEYS), KEY5
	movups	96(KEYS), KEY6
	movups	112(KEYS), KEY7
	movups	128(KEYS), KEY8
	movups	144(KEYS), KEY9
	lea	160(KEYS), KEYS
	sub	$10, XREG(ROUNDS)	C Also clears high half
	je	.Lkey_last

	movups	(KEYS), KEY10
	movups	16(KEYS), KEY11
	lea	(KEYS, ROUNDS, 8), KEYS
	lea	(KEYS, ROUNDS, 8), KEYS

	cmpl	$2, XREG(ROUNDS)
	je	.Lkey_last
	movups	-32(KEYS), KEY12
	movups	-16(KEYS), KEY13

.Lkey_last:
	movups	(KEYS), KEYLAST

.Lblock_loop:
	movups	(SRC), BLOCK
	pxor	KEY0, BLOCK
	aesdec	KEY1, BLOCK
	aesdec	KEY2, BLOCK
	aesdec	KEY3, BLOCK
	aesdec	KEY4, BLOCK
	aesdec	KEY5, BLOCK
	aesdec	KEY6, BLOCK
	aesdec	KEY7, BLOCK
	aesdec	KEY8, BLOCK
	aesdec	KEY9, BLOCK
	testl	XREG(ROUNDS), XREG(ROUNDS)
	je	.Lblock_end
	aesdec	KEY10, BLOCK
	aesdec	KEY11, BLOCK
	cmpl	$2, XREG(ROUNDS)
	je	.Lblock_end

	aesdec	KEY12, BLOCK
	aesdec	KEY13, BLOCK

.Lblock_end:
	aesdeclast KEYLAST, BLOCK

	movups	BLOCK, (DST)
	add	$16, SRC
	add	$16, DST
	dec	LENGTH
	jnz	.Lblock_loop

.Lend:
	W64_EXIT(6, 16)
	ret
EPILOGUE(_nettle_aes_decrypt)
