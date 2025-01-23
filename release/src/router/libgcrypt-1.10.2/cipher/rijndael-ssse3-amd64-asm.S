/* SSSE3 vector permutation AES for Libgcrypt
 * Copyright (C) 2014-2017 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 *
 *
 * The code is based on the public domain library libvpaes version 0.5
 * available at http://crypto.stanford.edu/vpaes/ and which carries
 * this notice:
 *
 *     libvpaes: constant-time SSSE3 AES encryption and decryption.
 *     version 0.5
 *
 *     By Mike Hamburg, Stanford University, 2009.  Public domain.
 *     I wrote essentially all of this code.  I did not write the test
 *     vectors; they are the NIST known answer tests.  I hereby release all
 *     the code and documentation here that I wrote into the public domain.
 *
 *     This is an implementation of AES following my paper,
 *       "Accelerating AES with Vector Permute Instructions
 *       CHES 2009; http://shiftleft.org/papers/vector_aes/
 */

#if defined(__x86_64__)
#include <config.h>
#if defined(HAVE_GCC_INLINE_ASM_SSSE3) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))

#include "asm-common-amd64.h"

.text

##
##  _gcry_aes_ssse3_enc_preload
##
ELF(.type _gcry_aes_ssse3_enc_preload,@function)
.globl _gcry_aes_ssse3_enc_preload
_gcry_aes_ssse3_enc_preload:
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4
	lea	.Laes_consts(%rip), %rax
	movdqa	          (%rax), %xmm9  # 0F
	movdqa	.Lk_inv   (%rax), %xmm10 # inv
	movdqa	.Lk_inv+16(%rax), %xmm11 # inva
	movdqa	.Lk_sb1   (%rax), %xmm13 # sb1u
	movdqa	.Lk_sb1+16(%rax), %xmm12 # sb1t
	movdqa	.Lk_sb2   (%rax), %xmm15 # sb2u
	movdqa	.Lk_sb2+16(%rax), %xmm14 # sb2t
	EXIT_SYSV_FUNC
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_aes_ssse3_enc_preload,.-_gcry_aes_ssse3_enc_preload)

##
##  _gcry_aes_ssse3_dec_preload
##
ELF(.type _gcry_aes_ssse3_dec_preload,@function)
.globl _gcry_aes_ssse3_dec_preload
_gcry_aes_ssse3_dec_preload:
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4
	lea	.Laes_consts(%rip), %rax
	movdqa	          (%rax), %xmm9   # 0F
	movdqa	.Lk_inv   (%rax), %xmm10  # inv
	movdqa	.Lk_inv+16(%rax), %xmm11  # inva
	movdqa	.Lk_dsb9   (%rax), %xmm13 # sb9u
	movdqa	.Lk_dsb9+16(%rax), %xmm12 # sb9t
	movdqa	.Lk_dsbd   (%rax), %xmm15 # sbdu
	movdqa	.Lk_dsbb   (%rax), %xmm14 # sbbu
	movdqa	.Lk_dsbe   (%rax), %xmm8  # sbeu
	EXIT_SYSV_FUNC
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_aes_ssse3_dec_preload,.-_gcry_aes_ssse3_dec_preload)

##
## Constant-time SSSE3 AES core implementation.
##
## By Mike Hamburg (Stanford University), 2009
## Public domain.
##

##
##  _aes_encrypt_core
##
##  AES-encrypt %xmm0.
##
##  Inputs:
##     %xmm0 = input
##     %xmm9-%xmm15 as in .Laes_preheat
##    (%rdi) = scheduled keys
##     %rsi  = nrounds
##
##  Output in %xmm0
##  Clobbers  %xmm1-%xmm4, %r9, %r11, %rax, %rcx, %rdx
##  Preserves %xmm6 - %xmm7 so you get some local vectors
##
##
.align 16
ELF(.type _gcry_aes_ssse3_encrypt_core,@function)
.globl _gcry_aes_ssse3_encrypt_core
_gcry_aes_ssse3_encrypt_core:
_aes_encrypt_core:
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4
	mov	%rdi,	%rdx
	leaq	-1(%rsi), %rax
	lea	.Laes_consts(%rip), %rcx
	leaq	.Lk_mc_backward(%rcx), %rdi
	mov	$16,	%rsi
	movdqa	.Lk_ipt   (%rcx), %xmm2 # iptlo
	movdqa	%xmm9,	%xmm1
	pandn	%xmm0,	%xmm1
	psrld	$4,	%xmm1
	pand	%xmm9,	%xmm0
	pshufb	%xmm0,	%xmm2
	movdqa	.Lk_ipt+16(%rcx), %xmm0 # ipthi
	pshufb	%xmm1,	%xmm0
	pxor	(%rdx),%xmm2
	pxor	%xmm2,	%xmm0
	add	$16,	%rdx
	jmp	.Laes_entry

.align 8
.Laes_loop:
	# middle of middle round
	movdqa  %xmm13,	%xmm4	# 4 : sb1u
	pshufb  %xmm2,	%xmm4   # 4 = sb1u
	pxor	(%rdx),	%xmm4	# 4 = sb1u + k
	movdqa  %xmm12,	%xmm0	# 0 : sb1t
	pshufb  %xmm3,	%xmm0	# 0 = sb1t
	pxor	%xmm4,	%xmm0	# 0 = A
	movdqa  %xmm15,	%xmm4	# 4 : sb2u
	pshufb	%xmm2,	%xmm4	# 4 = sb2u
	movdqa	.Lk_mc_forward-.Lk_mc_backward(%rsi,%rdi), %xmm1
	movdqa	%xmm14, %xmm2	# 2 : sb2t
	pshufb	%xmm3,  %xmm2	# 2 = sb2t
	pxor	%xmm4,  %xmm2	# 2 = 2A
	movdqa	%xmm0,  %xmm3	# 3 = A
	pshufb  %xmm1,  %xmm0	# 0 = B
	pxor	%xmm2,  %xmm0	# 0 = 2A+B
	pshufb	(%rsi,%rdi), %xmm3  # 3 = D
	lea	16(%esi),%esi	# next mc
	pxor	%xmm0,	%xmm3	# 3 = 2A+B+D
	lea	16(%rdx),%rdx	# next key
	pshufb  %xmm1,	%xmm0	# 0 = 2B+C
	pxor	%xmm3,	%xmm0	# 0 = 2A+3B+C+D
	and	$48, %rsi	# ... mod 4
	dec	%rax		# nr--

.Laes_entry:
	# top of round
	movdqa  %xmm9, 	%xmm1	# 1 : i
	pandn	%xmm0, 	%xmm1	# 1 = i<<4
	psrld	$4,    	%xmm1   # 1 = i
	pand	%xmm9, 	%xmm0   # 0 = k
	movdqa	%xmm11, %xmm2	# 2 : a/k
	pshufb  %xmm0,  %xmm2	# 2 = a/k
	pxor	%xmm1,	%xmm0	# 0 = j
	movdqa  %xmm10,	%xmm3  	# 3 : 1/i
	pshufb  %xmm1, 	%xmm3  	# 3 = 1/i
	pxor	%xmm2, 	%xmm3  	# 3 = iak = 1/i + a/k
	movdqa	%xmm10,	%xmm4  	# 4 : 1/j
	pshufb	%xmm0, 	%xmm4  	# 4 = 1/j
	pxor	%xmm2, 	%xmm4  	# 4 = jak = 1/j + a/k
	movdqa  %xmm10,	%xmm2  	# 2 : 1/iak
	pshufb  %xmm3,	%xmm2  	# 2 = 1/iak
	pxor	%xmm0, 	%xmm2  	# 2 = io
	movdqa  %xmm10, %xmm3   # 3 : 1/jak
	pshufb  %xmm4,  %xmm3   # 3 = 1/jak
	pxor	%xmm1,  %xmm3   # 3 = jo
	jnz	.Laes_loop

	# middle of last round
	movdqa	.Lk_sbo(%rcx), %xmm4	# 3 : sbou
	pshufb  %xmm2,  %xmm4   # 4 = sbou
	pxor	(%rdx), %xmm4   # 4 = sb1u + k
	movdqa	.Lk_sbo+16(%rcx), %xmm0	# 0 : sbot
	pshufb  %xmm3,	%xmm0	# 0 = sb1t
	pxor	%xmm4,	%xmm0	# 0 = A
	pshufb	.Lk_sr(%rsi,%rcx), %xmm0
	EXIT_SYSV_FUNC
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _aes_encrypt_core,.-_aes_encrypt_core)

##
##  Decryption core
##
##  Same API as encryption core.
##
.align 16
.globl _gcry_aes_ssse3_decrypt_core
ELF(.type _gcry_aes_ssse3_decrypt_core,@function)
_gcry_aes_ssse3_decrypt_core:
_aes_decrypt_core:
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4
	mov	%rdi,	%rdx
	lea	.Laes_consts(%rip), %rcx
	subl	$1,	%esi
	movl	%esi,   %eax
	shll	$4,	%esi
	xorl	$48,	%esi
	andl	$48,	%esi
	movdqa	.Lk_dipt   (%rcx), %xmm2 # iptlo
	movdqa	%xmm9,	%xmm1
	pandn	%xmm0,	%xmm1
	psrld	$4,	%xmm1
	pand	%xmm9,	%xmm0
	pshufb	%xmm0,	%xmm2
	movdqa	.Lk_dipt+16(%rcx), %xmm0 # ipthi
	pshufb	%xmm1,	%xmm0
	pxor	(%rdx),	%xmm2
	pxor	%xmm2,	%xmm0
	movdqa	.Lk_mc_forward+48(%rcx), %xmm5
	lea	16(%rdx), %rdx
	neg	%rax
	jmp	.Laes_dec_entry

.align 16
.Laes_dec_loop:
##
##  Inverse mix columns
##
	movdqa  %xmm13,	%xmm4		# 4 : sb9u
	pshufb	%xmm2,	%xmm4		# 4 = sb9u
	pxor	(%rdx),	%xmm4
	movdqa  %xmm12,	%xmm0		# 0 : sb9t
	pshufb	%xmm3,	%xmm0		# 0 = sb9t
	movdqa  .Lk_dsbd+16(%rcx),%xmm1	# 1 : sbdt
	pxor	%xmm4,	%xmm0		# 0 = ch
	lea	16(%rdx), %rdx		# next round key

	pshufb	%xmm5,	%xmm0		# MC ch
	movdqa  %xmm15,	%xmm4		# 4 : sbdu
	pshufb	%xmm2,	%xmm4		# 4 = sbdu
	pxor	%xmm0,	%xmm4		# 4 = ch
	pshufb	%xmm3,	%xmm1		# 1 = sbdt
	pxor	%xmm4,	%xmm1		# 1 = ch

	pshufb	%xmm5,	%xmm1		# MC ch
	movdqa  %xmm14,	%xmm4		# 4 : sbbu
	pshufb	%xmm2,	%xmm4		# 4 = sbbu
	inc     %rax                    # nr--
	pxor	%xmm1,	%xmm4		# 4 = ch
	movdqa  .Lk_dsbb+16(%rcx),%xmm0	# 0 : sbbt
	pshufb	%xmm3,	%xmm0		# 0 = sbbt
	pxor	%xmm4,	%xmm0		# 0 = ch

	pshufb	%xmm5,	%xmm0		# MC ch
	movdqa  %xmm8,	%xmm4		# 4 : sbeu
	pshufb	%xmm2,	%xmm4		# 4 = sbeu
	pshufd	$0x93,	%xmm5,	%xmm5
	pxor	%xmm0,	%xmm4		# 4 = ch
	movdqa  .Lk_dsbe+16(%rcx),%xmm0	# 0 : sbet
	pshufb	%xmm3,	%xmm0		# 0 = sbet
	pxor	%xmm4,	%xmm0		# 0 = ch

.Laes_dec_entry:
	# top of round
	movdqa  %xmm9, 	%xmm1	# 1 : i
	pandn	%xmm0, 	%xmm1	# 1 = i<<4
	psrld	$4,    	%xmm1   # 1 = i
	pand	%xmm9, 	%xmm0   # 0 = k
	movdqa	%xmm11, %xmm2	# 2 : a/k
	pshufb  %xmm0,  %xmm2	# 2 = a/k
	pxor	%xmm1,	%xmm0	# 0 = j
	movdqa  %xmm10,	%xmm3  	# 3 : 1/i
	pshufb  %xmm1, 	%xmm3  	# 3 = 1/i
	pxor	%xmm2, 	%xmm3  	# 3 = iak = 1/i + a/k
	movdqa	%xmm10,	%xmm4  	# 4 : 1/j
	pshufb	%xmm0, 	%xmm4  	# 4 = 1/j
	pxor	%xmm2, 	%xmm4  	# 4 = jak = 1/j + a/k
	movdqa  %xmm10,	%xmm2  	# 2 : 1/iak
	pshufb  %xmm3,	%xmm2  	# 2 = 1/iak
	pxor	%xmm0, 	%xmm2  	# 2 = io
	movdqa  %xmm10, %xmm3   # 3 : 1/jak
	pshufb  %xmm4,  %xmm3   # 3 = 1/jak
	pxor	%xmm1,  %xmm3   # 3 = jo
	jnz	.Laes_dec_loop

	# middle of last round
	movdqa	.Lk_dsbo(%rcx), %xmm4		# 3 : sbou
	pshufb  %xmm2,  %xmm4   # 4 = sbou
	pxor	(%rdx), %xmm4   # 4 = sb1u + k
	movdqa	.Lk_dsbo+16(%rcx), %xmm0	# 0 : sbot
	pshufb  %xmm3,	%xmm0	# 0 = sb1t
	pxor	%xmm4,	%xmm0	# 0 = A
	pshufb	.Lk_sr(%rsi,%rcx), %xmm0
	EXIT_SYSV_FUNC
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _aes_decrypt_core,.-_aes_decrypt_core)

########################################################
##                                                    ##
##                  AES key schedule                  ##
##                                                    ##
########################################################

.align 16
.globl _gcry_aes_ssse3_schedule_core
ELF(.type _gcry_aes_ssse3_schedule_core,@function)
_gcry_aes_ssse3_schedule_core:
_aes_schedule_core:
	# rdi = key
	# rsi = size in bits
	# rdx = buffer
	# rcx = direction.  0=encrypt, 1=decrypt
	# r8 = rotoffs
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_5

	# load the tables
	lea	.Laes_consts(%rip), %r10
	movdqa	          (%r10), %xmm9  # 0F
	movdqa	.Lk_inv   (%r10), %xmm10 # inv
	movdqa	.Lk_inv+16(%r10), %xmm11 # inva
	movdqa	.Lk_sb1   (%r10), %xmm13 # sb1u
	movdqa	.Lk_sb1+16(%r10), %xmm12 # sb1t
	movdqa	.Lk_sb2   (%r10), %xmm15 # sb2u
	movdqa	.Lk_sb2+16(%r10), %xmm14 # sb2t

	movdqa	.Lk_rcon(%r10), %xmm8	# load rcon
	movdqu	(%rdi),	%xmm0		# load key (unaligned)

	# input transform
	movdqu	%xmm0,	%xmm3
	lea	.Lk_ipt(%r10), %r11
	call	.Laes_schedule_transform
	movdqu	%xmm0,	%xmm7

	test	%rcx,	%rcx
	jnz	.Laes_schedule_am_decrypting

	# encrypting, output zeroth round key after transform
	movdqa	%xmm0,	(%rdx)
	jmp	.Laes_schedule_go

.Laes_schedule_am_decrypting:
	# decrypting, output zeroth round key after shiftrows
	pshufb  .Lk_sr(%r8,%r10),%xmm3
	movdqa	%xmm3,	(%rdx)
	xor	$48, 	%r8

.Laes_schedule_go:
	cmp	$192,	%rsi
	je	.Laes_schedule_192
	cmp	$256,	%rsi
	je	.Laes_schedule_256
	# 128: fall though

##
##  .Laes_schedule_128
##
##  128-bit specific part of key schedule.
##
##  This schedule is really simple, because all its parts
##  are accomplished by the subroutines.
##
.Laes_schedule_128:
	mov	$10, %rsi

.Laes_schedule_128_L:
	call 	.Laes_schedule_round
	dec	%rsi
	jz 	.Laes_schedule_mangle_last
	call	.Laes_schedule_mangle	# write output
	jmp 	.Laes_schedule_128_L

##
##  .Laes_schedule_192
##
##  192-bit specific part of key schedule.
##
##  The main body of this schedule is the same as the 128-bit
##  schedule, but with more smearing.  The long, high side is
##  stored in %xmm7 as before, and the short, low side is in
##  the high bits of %xmm6.
##
##  This schedule is somewhat nastier, however, because each
##  round produces 192 bits of key material, or 1.5 round keys.
##  Therefore, on each cycle we do 2 rounds and produce 3 round
##  keys.
##
.Laes_schedule_192:
	movdqu	8(%rdi),%xmm0		# load key part 2 (very unaligned)
	call	.Laes_schedule_transform	# input transform
	pshufd	$0x0E,	%xmm0,	%xmm6
	pslldq	$8,	%xmm6		# clobber low side with zeros
	mov	$4,	%rsi

.Laes_schedule_192_L:
	call	.Laes_schedule_round
	palignr	$8,%xmm6,%xmm0
	call	.Laes_schedule_mangle	# save key n
	call	.Laes_schedule_192_smear
	call	.Laes_schedule_mangle	# save key n+1
	call	.Laes_schedule_round
	dec	%rsi
	jz 	.Laes_schedule_mangle_last
	call	.Laes_schedule_mangle	# save key n+2
	call	.Laes_schedule_192_smear
	jmp	.Laes_schedule_192_L

##
##  .Laes_schedule_192_smear
##
##  Smear the short, low side in the 192-bit key schedule.
##
##  Inputs:
##    %xmm7: high side, b  a  x  y
##    %xmm6:  low side, d  c  0  0
##    %xmm13: 0
##
##  Outputs:
##    %xmm6: b+c+d  b+c  0  0
##    %xmm0: b+c+d  b+c  b  a
##
.Laes_schedule_192_smear:
	pshufd	$0x80,	%xmm6,	%xmm0	# d c 0 0 -> c 0 0 0
	pxor	%xmm0,	%xmm6		# -> c+d c 0 0
	pshufd	$0xFE,	%xmm7,	%xmm0	# b a _ _ -> b b b a
	pxor	%xmm6,	%xmm0		# -> b+c+d b+c b a
	pshufd	$0x0E,	%xmm0,	%xmm6
	pslldq	$8,	%xmm6		# clobber low side with zeros
	ret_spec_stop

##
##  .Laes_schedule_256
##
##  256-bit specific part of key schedule.
##
##  The structure here is very similar to the 128-bit
##  schedule, but with an additional 'low side' in
##  %xmm6.  The low side's rounds are the same as the
##  high side's, except no rcon and no rotation.
##
.Laes_schedule_256:
	movdqu	16(%rdi),%xmm0		# load key part 2 (unaligned)
	call	.Laes_schedule_transform	# input transform
	mov	$7, %rsi

.Laes_schedule_256_L:
	call	.Laes_schedule_mangle	# output low result
	movdqa	%xmm0,	%xmm6		# save cur_lo in xmm6

	# high round
	call	.Laes_schedule_round
	dec	%rsi
	jz 	.Laes_schedule_mangle_last
	call	.Laes_schedule_mangle

	# low round. swap xmm7 and xmm6
	pshufd	$0xFF,	%xmm0,	%xmm0
	movdqa	%xmm7,	%xmm5
	movdqa	%xmm6,	%xmm7
	call	.Laes_schedule_low_round
	movdqa	%xmm5,	%xmm7

	jmp	.Laes_schedule_256_L

##
##  .Laes_schedule_round
##
##  Runs one main round of the key schedule on %xmm0, %xmm7
##
##  Specifically, runs subbytes on the high dword of %xmm0
##  then rotates it by one byte and xors into the low dword of
##  %xmm7.
##
##  Adds rcon from low byte of %xmm8, then rotates %xmm8 for
##  next rcon.
##
##  Smears the dwords of %xmm7 by xoring the low into the
##  second low, result into third, result into highest.
##
##  Returns results in %xmm7 = %xmm0.
##  Clobbers %xmm1-%xmm4, %r11.
##
.Laes_schedule_round:
	# extract rcon from xmm8
	pxor	%xmm1,	%xmm1
	palignr	$15,	%xmm8,	%xmm1
	palignr	$15,	%xmm8,	%xmm8
	pxor	%xmm1,	%xmm7

	# rotate
	pshufd	$0xFF,	%xmm0,	%xmm0
	palignr	$1,	%xmm0,	%xmm0

	# fall through...

	# low round: same as high round, but no rotation and no rcon.
.Laes_schedule_low_round:
	# smear xmm7
	movdqa	%xmm7,	%xmm1
	pslldq	$4,	%xmm7
	pxor	%xmm1,	%xmm7
	movdqa	%xmm7,	%xmm1
	pslldq	$8,	%xmm7
	pxor	%xmm1,	%xmm7
	pxor	.Lk_s63(%r10), %xmm7

	# subbytes
	movdqa  %xmm9, 	%xmm1
	pandn	%xmm0, 	%xmm1
	psrld	$4,    	%xmm1		# 1 = i
	pand	%xmm9, 	%xmm0		# 0 = k
	movdqa	%xmm11, %xmm2		# 2 : a/k
	pshufb  %xmm0,  %xmm2		# 2 = a/k
	pxor	%xmm1,	%xmm0		# 0 = j
	movdqa  %xmm10,	%xmm3		# 3 : 1/i
	pshufb  %xmm1, 	%xmm3		# 3 = 1/i
	pxor	%xmm2, 	%xmm3		# 3 = iak = 1/i + a/k
	movdqa	%xmm10,	%xmm4		# 4 : 1/j
	pshufb	%xmm0, 	%xmm4		# 4 = 1/j
	pxor	%xmm2, 	%xmm4		# 4 = jak = 1/j + a/k
	movdqa  %xmm10,	%xmm2		# 2 : 1/iak
	pshufb  %xmm3,	%xmm2		# 2 = 1/iak
	pxor	%xmm0, 	%xmm2		# 2 = io
	movdqa  %xmm10, %xmm3		# 3 : 1/jak
	pshufb  %xmm4,  %xmm3		# 3 = 1/jak
	pxor	%xmm1,  %xmm3		# 3 = jo
	movdqa	.Lk_sb1(%r10), %xmm4	# 4 : sbou
	pshufb  %xmm2,  %xmm4		# 4 = sbou
	movdqa	.Lk_sb1+16(%r10), %xmm0	# 0 : sbot
	pshufb  %xmm3,	%xmm0		# 0 = sb1t
	pxor	%xmm4, 	%xmm0		# 0 = sbox output

	# add in smeared stuff
	pxor	%xmm7,	%xmm0
	movdqa	%xmm0,	%xmm7
	ret_spec_stop

##
##  .Laes_schedule_transform
##
##  Linear-transform %xmm0 according to tables at (%r11)
##
##  Requires that %xmm9 = 0x0F0F... as in preheat
##  Output in %xmm0
##  Clobbers %xmm1, %xmm2
##
.Laes_schedule_transform:
	movdqa	%xmm9,	%xmm1
	pandn	%xmm0,	%xmm1
	psrld	$4,	%xmm1
	pand	%xmm9,	%xmm0
	movdqa	(%r11), %xmm2 	# lo
	pshufb	%xmm0,	%xmm2
	movdqa	16(%r11), %xmm0 # hi
	pshufb	%xmm1,	%xmm0
	pxor	%xmm2,	%xmm0
	ret_spec_stop

##
##  .Laes_schedule_mangle
##
##  Mangle xmm0 from (basis-transformed) standard version
##  to our version.
##
##  On encrypt,
##    xor with 0x63
##    multiply by circulant 0,1,1,1
##    apply shiftrows transform
##
##  On decrypt,
##    xor with 0x63
##    multiply by 'inverse mixcolumns' circulant E,B,D,9
##    deskew
##    apply shiftrows transform
##
##
##  Writes out to (%rdx), and increments or decrements it
##  Keeps track of round number mod 4 in %r8
##  Preserves xmm0
##  Clobbers xmm1-xmm5
##
.Laes_schedule_mangle:
	movdqa	%xmm0,	%xmm4	# save xmm0 for later
	movdqa	.Lk_mc_forward(%r10),%xmm5
	test	%rcx, 	%rcx
	jnz	.Laes_schedule_mangle_dec

	# encrypting
	add	$16,	%rdx
	pxor	.Lk_s63(%r10),%xmm4
	pshufb	%xmm5,	%xmm4
	movdqa	%xmm4,	%xmm3
	pshufb	%xmm5,	%xmm4
	pxor	%xmm4,	%xmm3
	pshufb	%xmm5,	%xmm4
	pxor	%xmm4,	%xmm3

	jmp	.Laes_schedule_mangle_both

.Laes_schedule_mangle_dec:
	lea	.Lk_dks_1(%r10), %r11	# first table: *9
	call 	.Laes_schedule_transform
	movdqa	%xmm0,	%xmm3
	pshufb	%xmm5,	%xmm3

	add	$32, 	%r11		# next table:  *B
	call 	.Laes_schedule_transform
	pxor	%xmm0,	%xmm3
	pshufb	%xmm5,	%xmm3

	add	$32, 	%r11		# next table:  *D
	call 	.Laes_schedule_transform
	pxor	%xmm0,	%xmm3
	pshufb	%xmm5,	%xmm3

	add	$32, 	%r11		# next table:  *E
	call 	.Laes_schedule_transform
	pxor	%xmm0,	%xmm3
	pshufb	%xmm5,	%xmm3

	movdqa	%xmm4,	%xmm0		# restore %xmm0
	add	$-16,	%rdx

.Laes_schedule_mangle_both:
	pshufb	.Lk_sr(%r8,%r10),%xmm3
	add	$-16,	%r8
	and	$48,	%r8
	movdqa	%xmm3,	(%rdx)
	ret_spec_stop

##
##  .Laes_schedule_mangle_last
##
##  Mangler for last round of key schedule
##  Mangles %xmm0
##    when encrypting, outputs out(%xmm0) ^ 63
##    when decrypting, outputs unskew(%xmm0)
##
##  Always called right before return... jumps to cleanup and exits
##
.Laes_schedule_mangle_last:
	# schedule last round key from xmm0
	lea	.Lk_deskew(%r10),%r11	# prepare to deskew
	test	%rcx, 	%rcx
	jnz	.Laes_schedule_mangle_last_dec

	# encrypting
	pshufb	.Lk_sr(%r8,%r10),%xmm0	# output permute
	lea	.Lk_opt(%r10),	%r11	# prepare to output transform
	add	$32,	%rdx

.Laes_schedule_mangle_last_dec:
	add	$-16,	%rdx
	pxor	.Lk_s63(%r10),	%xmm0
	call	.Laes_schedule_transform # output transform
	movdqa	%xmm0,	(%rdx)		# save last key

	#_aes_cleanup
	pxor	%xmm0,  %xmm0
	pxor	%xmm1,  %xmm1
	pxor	%xmm2,  %xmm2
	pxor	%xmm3,  %xmm3
	pxor	%xmm4,  %xmm4
	pxor	%xmm5,  %xmm5
	pxor	%xmm6,  %xmm6
	pxor	%xmm7,  %xmm7
	pxor	%xmm8,  %xmm8
	EXIT_SYSV_FUNC
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_aes_ssse3_schedule_core,.-_gcry_aes_ssse3_schedule_core)

########################################################
##                                                    ##
##                     Constants                      ##
##                                                    ##
########################################################

.align 16
ELF(.type _aes_consts,@object)
.Laes_consts:
_aes_consts:
	# s0F
	.Lk_s0F = .-.Laes_consts
	.quad	0x0F0F0F0F0F0F0F0F
	.quad	0x0F0F0F0F0F0F0F0F

	# input transform (lo, hi)
	.Lk_ipt = .-.Laes_consts
	.quad	0xC2B2E8985A2A7000
	.quad	0xCABAE09052227808
	.quad	0x4C01307D317C4D00
	.quad	0xCD80B1FCB0FDCC81

	# inv, inva
	.Lk_inv = .-.Laes_consts
	.quad	0x0E05060F0D080180
	.quad	0x040703090A0B0C02
	.quad	0x01040A060F0B0780
	.quad	0x030D0E0C02050809

	# sb1u, sb1t
	.Lk_sb1 = .-.Laes_consts
	.quad	0xB19BE18FCB503E00
	.quad	0xA5DF7A6E142AF544
	.quad	0x3618D415FAE22300
	.quad	0x3BF7CCC10D2ED9EF


	# sb2u, sb2t
	.Lk_sb2 = .-.Laes_consts
	.quad	0xE27A93C60B712400
	.quad	0x5EB7E955BC982FCD
	.quad	0x69EB88400AE12900
	.quad	0xC2A163C8AB82234A

	# sbou, sbot
	.Lk_sbo = .-.Laes_consts
	.quad	0xD0D26D176FBDC700
	.quad	0x15AABF7AC502A878
	.quad	0xCFE474A55FBB6A00
	.quad	0x8E1E90D1412B35FA

	# mc_forward
	.Lk_mc_forward = .-.Laes_consts
	.quad	0x0407060500030201
	.quad	0x0C0F0E0D080B0A09
	.quad	0x080B0A0904070605
	.quad	0x000302010C0F0E0D
	.quad	0x0C0F0E0D080B0A09
	.quad	0x0407060500030201
	.quad	0x000302010C0F0E0D
	.quad	0x080B0A0904070605

	# mc_backward
	.Lk_mc_backward = .-.Laes_consts
	.quad	0x0605040702010003
	.quad	0x0E0D0C0F0A09080B
	.quad	0x020100030E0D0C0F
	.quad	0x0A09080B06050407
	.quad	0x0E0D0C0F0A09080B
	.quad	0x0605040702010003
	.quad	0x0A09080B06050407
	.quad	0x020100030E0D0C0F

	# sr
	.Lk_sr = .-.Laes_consts
	.quad	0x0706050403020100
	.quad	0x0F0E0D0C0B0A0908
	.quad	0x030E09040F0A0500
	.quad	0x0B06010C07020D08
	.quad	0x0F060D040B020900
	.quad	0x070E050C030A0108
	.quad	0x0B0E0104070A0D00
	.quad	0x0306090C0F020508

	# rcon
	.Lk_rcon = .-.Laes_consts
	.quad	0x1F8391B9AF9DEEB6
	.quad	0x702A98084D7C7D81

	# s63: all equal to 0x63 transformed
	.Lk_s63 = .-.Laes_consts
	.quad	0x5B5B5B5B5B5B5B5B
	.quad	0x5B5B5B5B5B5B5B5B

	# output transform
	.Lk_opt = .-.Laes_consts
	.quad	0xFF9F4929D6B66000
	.quad	0xF7974121DEBE6808
	.quad	0x01EDBD5150BCEC00
	.quad	0xE10D5DB1B05C0CE0

	# deskew tables: inverts the sbox's 'skew'
	.Lk_deskew = .-.Laes_consts
	.quad	0x07E4A34047A4E300
	.quad	0x1DFEB95A5DBEF91A
	.quad	0x5F36B5DC83EA6900
	.quad	0x2841C2ABF49D1E77

##
##  Decryption stuff
##  Key schedule constants
##
	# decryption key schedule: x -> invskew x*9
	.Lk_dks_1 = .-.Laes_consts
	.quad	0xB6116FC87ED9A700
	.quad	0x4AED933482255BFC
	.quad	0x4576516227143300
	.quad	0x8BB89FACE9DAFDCE

	# decryption key schedule: invskew x*9 -> invskew x*D
	.Lk_dks_2 = .-.Laes_consts
	.quad	0x27438FEBCCA86400
	.quad	0x4622EE8AADC90561
	.quad	0x815C13CE4F92DD00
	.quad	0x73AEE13CBD602FF2

	# decryption key schedule: invskew x*D -> invskew x*B
	.Lk_dks_3 = .-.Laes_consts
	.quad	0x03C4C50201C6C700
	.quad	0xF83F3EF9FA3D3CFB
	.quad	0xEE1921D638CFF700
	.quad	0xA5526A9D7384BC4B

	# decryption key schedule: invskew x*B -> invskew x*E + 0x63
	.Lk_dks_4 = .-.Laes_consts
	.quad	0xE3C390B053732000
	.quad	0xA080D3F310306343
	.quad	0xA0CA214B036982E8
	.quad	0x2F45AEC48CE60D67

##
##  Decryption stuff
##  Round function constants
##
	# decryption input transform
	.Lk_dipt = .-.Laes_consts
	.quad	0x0F505B040B545F00
	.quad	0x154A411E114E451A
	.quad	0x86E383E660056500
	.quad	0x12771772F491F194

	# decryption sbox output *9*u, *9*t
	.Lk_dsb9 = .-.Laes_consts
	.quad	0x851C03539A86D600
	.quad	0xCAD51F504F994CC9
	.quad	0xC03B1789ECD74900
	.quad	0x725E2C9EB2FBA565

	# decryption sbox output *D*u, *D*t
	.Lk_dsbd = .-.Laes_consts
	.quad	0x7D57CCDFE6B1A200
	.quad	0xF56E9B13882A4439
	.quad	0x3CE2FAF724C6CB00
	.quad	0x2931180D15DEEFD3

	# decryption sbox output *B*u, *B*t
	.Lk_dsbb = .-.Laes_consts
	.quad	0xD022649296B44200
	.quad	0x602646F6B0F2D404
	.quad	0xC19498A6CD596700
	.quad	0xF3FF0C3E3255AA6B

	# decryption sbox output *E*u, *E*t
	.Lk_dsbe = .-.Laes_consts
	.quad	0x46F2929626D4D000
	.quad	0x2242600464B4F6B0
	.quad	0x0C55A6CDFFAAC100
	.quad	0x9467F36B98593E32

	# decryption sbox final output
	.Lk_dsbo = .-.Laes_consts
	.quad	0x1387EA537EF94000
	.quad	0xC7AA6DB9D4943E2D
	.quad	0x12D7560F93441D00
	.quad	0xCA4B8159D8C58E9C
ELF(.size _aes_consts,.-_aes_consts)

#endif
#endif
