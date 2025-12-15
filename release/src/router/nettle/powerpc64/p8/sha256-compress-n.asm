C x86_64/sha256-compress-n.asm

ifelse(`
   Copyright (C) 2024 Eric Richter, IBM Corporation

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

.file "sha256-compress-n.asm"

C Parameters in
define(`SP', `r1')
define(`STATE', `r3')
define(`K', `r4')
define(`NUMBLOCKS', `r5')
define(`INPUT', `r6')

define(`T0', `r7')
define(`T1', `r8')
define(`TK', `r9')
define(`COUNT', `r10')
define(`TC0', `0')	C Index instructions allow literal 0 instead of a GPR
define(`TC16', `r11')

C State registers
define(`VSA', `v0')
define(`VSB', `v1')
define(`VSC', `v2')
define(`VSD', `v3')
define(`VSE', `v4')
define(`VSF', `v5')
define(`VSG', `v6')
define(`VSH', `v7')

C Previous state value registers stored in VSX
define(`VSXA', `vs0')
define(`VSXB', `vs1')
define(`VSXC', `vs2')
define(`VSXD', `vs3')
define(`VSXE', `vs4')
define(`VSXF', `vs5')
define(`VSXG', `vs6')
define(`VSXH', `vs7')

C Current K values
define(`VK', `v8')

C Temp registers for math
define(`VT0', `v9')
define(`VT1', `v10')
define(`VT2', `v11')
define(`VT3', `v12')
define(`VT4', `v13')

C Convenience named registers for sigma(a) and sigma(e)
define(`SIGA', `v14')
define(`SIGE', `v15')

C Registers v16-v31 are used for input words W[0] through W[15]

C Convert an index for W[i] to the corresponding vector register v[16 + i]
define(`IV', `m4_unquote(v`'eval((($1) % 16) + 16))')

C ROUND(A B C D E F G H R)
define(`ROUND', `

	vadduwm	VT1, VK, IV($9)               C VT1: k+W
	vadduwm	VT4, $8, VT1                  C VT4: H+k+W

	lxvw4x	VSR(VK), TK, K                C Load Key
	addi	TK, TK, 4	              C Increment Pointer to next key

	vadduwm	VT2, $4, $8	              C VT2: H+D
	vadduwm	VT2, VT2, VT1                 C VT2: H+D+k+W

	vshasigmaw	SIGE, $5, 1, 0b1111   C Sigma(E)  Se
	vshasigmaw	SIGA, $1, 1, 0        C Sigma(A)  Sa

	vxor	VT3, $2, $3                   C VT3: b^c
	vsel	VT0, $7, $6, $5	              C VT0: Ch.
	vsel	VT3, $3, $1, VT3              C VT3: Maj(a,b,c)

	vadduwm	VT4, VT4, VT0                 C VT4: Hkw + Ch.
	vadduwm	VT3, VT3, VT4                 C VT3: HkW + Ch. + Maj.

	vadduwm	VT0, VT0, VT2                 C VT0: Ch. + DHKW
	vadduwm	$8, SIGE, SIGA                C Anext: Se + Sa
	vadduwm	$4, VT0, SIGE                 C Dnext: Ch. + DHKW + Se
	vadduwm	$8, $8, VT3                   C Anext: Se+Sa+HkW+Ch.+Maj.
')

C Extend W[i]
define(`EXTEND', `
	vshasigmaw	SIGE, IV($1 + 14), 0, 0b1111
	vshasigmaw	SIGA, IV($1 + 1), 0, 0b0000
	vadduwm		IV($1), IV($1), SIGE
	vadduwm		IV($1), IV($1), SIGA
	vadduwm		IV($1), IV($1), IV($1 + 9)
')

define(`EXTENDROUND',	`
	ROUND($1, $2, $3, $4, $5, $6, $7, $8, $9)
	C Schedule (data) for 16th round in future
	EXTEND($9)
')
define(`NOEXTENDROUND',	`ROUND($1, $2, $3, $4, $5, $6, $7, $8, $9)')

define(`NOEXTENDROUNDS', `
	NOEXTENDROUND(VSA, VSB, VSC, VSD, VSE, VSF, VSG, VSH, 0)
	NOEXTENDROUND(VSH, VSA, VSB, VSC, VSD, VSE, VSF, VSG, 1)
	NOEXTENDROUND(VSG, VSH, VSA, VSB, VSC, VSD, VSE, VSF, 2)
	NOEXTENDROUND(VSF, VSG, VSH, VSA, VSB, VSC, VSD, VSE, 3)

	NOEXTENDROUND(VSE, VSF, VSG, VSH, VSA, VSB, VSC, VSD, 4)
	NOEXTENDROUND(VSD, VSE, VSF, VSG, VSH, VSA, VSB, VSC, 5)
	NOEXTENDROUND(VSC, VSD, VSE, VSF, VSG, VSH, VSA, VSB, 6)
	NOEXTENDROUND(VSB, VSC, VSD, VSE, VSF, VSG, VSH, VSA, 7)

	NOEXTENDROUND(VSA, VSB, VSC, VSD, VSE, VSF, VSG, VSH, 8)
	NOEXTENDROUND(VSH, VSA, VSB, VSC, VSD, VSE, VSF, VSG, 9)
	NOEXTENDROUND(VSG, VSH, VSA, VSB, VSC, VSD, VSE, VSF, 10)
	NOEXTENDROUND(VSF, VSG, VSH, VSA, VSB, VSC, VSD, VSE, 11)

	NOEXTENDROUND(VSE, VSF, VSG, VSH, VSA, VSB, VSC, VSD, 12)
	NOEXTENDROUND(VSD, VSE, VSF, VSG, VSH, VSA, VSB, VSC, 13)
	NOEXTENDROUND(VSC, VSD, VSE, VSF, VSG, VSH, VSA, VSB, 14)
	NOEXTENDROUND(VSB, VSC, VSD, VSE, VSF, VSG, VSH, VSA, 15)
')

define(`EXTENDROUNDS', `
	EXTENDROUND(VSA, VSB, VSC, VSD, VSE, VSF, VSG, VSH, 0)
	EXTENDROUND(VSH, VSA, VSB, VSC, VSD, VSE, VSF, VSG, 1)
	EXTENDROUND(VSG, VSH, VSA, VSB, VSC, VSD, VSE, VSF, 2)
	EXTENDROUND(VSF, VSG, VSH, VSA, VSB, VSC, VSD, VSE, 3)

	EXTENDROUND(VSE, VSF, VSG, VSH, VSA, VSB, VSC, VSD, 4)
	EXTENDROUND(VSD, VSE, VSF, VSG, VSH, VSA, VSB, VSC, 5)
	EXTENDROUND(VSC, VSD, VSE, VSF, VSG, VSH, VSA, VSB, 6)
	EXTENDROUND(VSB, VSC, VSD, VSE, VSF, VSG, VSH, VSA, 7)

	EXTENDROUND(VSA, VSB, VSC, VSD, VSE, VSF, VSG, VSH, 8)
	EXTENDROUND(VSH, VSA, VSB, VSC, VSD, VSE, VSF, VSG, 9)
	EXTENDROUND(VSG, VSH, VSA, VSB, VSC, VSD, VSE, VSF, 10)
	EXTENDROUND(VSF, VSG, VSH, VSA, VSB, VSC, VSD, VSE, 11)

	EXTENDROUND(VSE, VSF, VSG, VSH, VSA, VSB, VSC, VSD, 12)
	EXTENDROUND(VSD, VSE, VSF, VSG, VSH, VSA, VSB, VSC, 13)
	EXTENDROUND(VSC, VSD, VSE, VSF, VSG, VSH, VSA, VSB, 14)
	EXTENDROUND(VSB, VSC, VSD, VSE, VSF, VSG, VSH, VSA, 15)
')

define(`LOAD', `
	IF_BE(`lxvw4x	VSR(IV($1)), $2, INPUT')
	IF_LE(`
		lxvd2x	VSR(IV($1)), $2, INPUT
		vperm	IV($1), IV($1), IV($1), VT0
	')
')

define(`DOLOADS', `
	IF_LE(`DATA_LOAD_VEC(VT0, .load_swap, T1)')
	LOAD(0, TC0)
	vsldoi	IV(1), IV(0), IV(0), 4
	vsldoi	IV(2), IV(0), IV(0), 8
	vsldoi	IV(3), IV(0), IV(0), 12
	addi	INPUT, INPUT, 16
	LOAD(4, TC0)
	vsldoi	IV(5), IV(4), IV(4), 4
	vsldoi	IV(6), IV(4), IV(4), 8
	vsldoi	IV(7), IV(4), IV(4), 12
	addi	INPUT, INPUT, 16
	LOAD(8, TC0)
	vsldoi	IV(9), IV(8), IV(8), 4
	vsldoi	IV(10), IV(8), IV(8), 8
	vsldoi	IV(11), IV(8), IV(8), 12
	addi	INPUT, INPUT, 16
	LOAD(12, TC0)
	vsldoi	IV(13), IV(12), IV(12), 4
	vsldoi	IV(14), IV(12), IV(12), 8
	vsldoi	IV(15), IV(12), IV(12), 12
	addi	INPUT, INPUT, 16
')

.text
PROLOGUE(_nettle_sha256_compress_n)
	cmpwi	0, NUMBLOCKS, 0
	ble	0, .done
	mtctr	NUMBLOCKS

	C Store non-volatile registers

	ALIGN(16)	C Appears necessary for optimal stores
	li	TC16, 16
	li	T0, -16
	li	T1, -32
	stvx	v20, T0, SP
	stvx	v21, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	stvx	v22, T0, SP
	stvx	v23, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	stvx	v24, T0, SP
	stvx	v25, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	stvx	v26, T0, SP
	stvx	v27, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	stvx	v28, T0, SP
	stvx	v29, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	stvx	v30, T0, SP
	stvx	v31, T1, SP

	ALIGN(16)	C Appears necessary for optimal loads

	C Load state values
	lxvw4x	VSR(VSA), 0, STATE	C VSA contains A,B,C,D
	lxvw4x	VSR(VSE), TC16, STATE	C VSE contains E,F,G,H

	C "permute" state from VSA containing A,B,C,D into VSA,VSB,VSC,VSD

	vsldoi	VSB, VSA, VSA, 4
	vsldoi	VSF, VSE, VSE, 4

	vsldoi	VSC, VSA, VSA, 8
	vsldoi	VSG, VSE, VSE, 8

	vsldoi	VSD, VSA, VSA, 12
	vsldoi	VSH, VSE, VSE, 12

.loop:
	xxlor	VSXA, VSR(VSA), VSR(VSA)
	xxlor	VSXB, VSR(VSB), VSR(VSB)
	xxlor	VSXC, VSR(VSC), VSR(VSC)
	xxlor	VSXD, VSR(VSD), VSR(VSD)
	xxlor	VSXE, VSR(VSE), VSR(VSE)
	xxlor	VSXF, VSR(VSF), VSR(VSF)
	xxlor	VSXG, VSR(VSG), VSR(VSG)
	xxlor	VSXH, VSR(VSH), VSR(VSH)

	li	TK, 0
	lxvw4x	VSR(VK), TK, K
	addi	TK, TK, 4

	DOLOADS

	EXTENDROUNDS
	EXTENDROUNDS
	EXTENDROUNDS
	NOEXTENDROUNDS

	C Reload initial state from VSX registers
	xxlor	VSR(VT0), VSXA, VSXA
	xxlor	VSR(VT1), VSXB, VSXB
	xxlor	VSR(VT2), VSXC, VSXC
	xxlor	VSR(VT3), VSXD, VSXD
	xxlor	VSR(VT4), VSXE, VSXE
	xxlor	VSR(SIGA), VSXF, VSXF
	xxlor	VSR(SIGE), VSXG, VSXG
	xxlor	VSR(VK), VSXH, VSXH

	vadduwm	VSA, VSA, VT0
	vadduwm	VSB, VSB, VT1
	vadduwm	VSC, VSC, VT2
	vadduwm	VSD, VSD, VT3
	vadduwm	VSE, VSE, VT4
	vadduwm	VSF, VSF, SIGA
	vadduwm	VSG, VSG, SIGE
	vadduwm	VSH, VSH, VK

	bdnz .loop

	C Repack VSA,VSB,VSC,VSD into VSA,VSE for storing
	vmrghw	VSA, VSA, VSB
	vmrghw	VSC, VSC, VSD
	vmrghw	VSE, VSE, VSF
	vmrghw	VSG, VSG, VSH

	xxmrghd	VSR(VSA), VSR(VSA), VSR(VSC)
	xxmrghd	VSR(VSE), VSR(VSE), VSR(VSG)

	stxvw4x	VSR(VSA), 0, STATE
	stxvw4x	VSR(VSE), TC16, STATE


	C Restore nonvolatile registers
	li	T0, -16
	li	T1, -32
	lvx	v20, T0, SP
	lvx	v21, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	lvx	v22, T0, SP
	lvx	v23, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	lvx	v24, T0, SP
	lvx	v25, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	lvx	v26, T0, SP
	lvx	v27, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	lvx	v28, T0, SP
	lvx	v29, T1, SP
	subi	T0, T0, 32
	subi	T1, T1, 32
	lvx	v30, T0, SP
	lvx	v31, T1, SP

.done:
	mr r3, INPUT

	blr
EPILOGUE(_nettle_sha256_compress_n)

IF_LE(`
.data
.align 4
.load_swap:
	.byte 8,9,10,11, 12,13,14,15, 0,1,2,3, 4,5,6,7
')
