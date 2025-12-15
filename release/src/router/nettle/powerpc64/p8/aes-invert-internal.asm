C powerpc64/p8/aes-invert-internal.asm

ifelse(`
   Copyright (C) 2024 Niels Möller
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

define(`SP', `r1')
define(`TOCP', `r2')

define(`ROUNDS', `r3')
define(`DST', `r4')
define(`SRC', `r5')

define(`KEY', `v1')

.file "aes-invert-internal.asm"

.text

 C _aes_invert(unsigned rounds, uint32_t *dst, const uint32_t *src)

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_aes_invert)
	C Since decrypt wants the same subkeys, just copy, or do
	C nothing if SRC == DST.
	cmpld	SRC, DST
	beq	.Ldone

	sldi	ROUNDS, ROUNDS, 4
.Loop:
	lxvd2x	VSR(KEY),ROUNDS,SRC
	stxvd2x	VSR(KEY),ROUNDS,DST
	subic.	ROUNDS, ROUNDS, 0x10
	bge	.Loop
.Ldone:
	blr
EPILOGUE(_nettle_aes_invert)
