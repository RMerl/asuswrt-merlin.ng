C s390/vf/memxor3.asm

ifelse(`
   Copyright (C) 2020 Mamone Tarsha
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

.file "memxor3.asm"
.machine "z13"

.text

C void * memxor3(void *dst_in, const void *a_in, const void *b_in, size_t n)   

PROLOGUE(nettle_memxor3)
    agr            %r2,%r5
    agr            %r3,%r5
    agr            %r4,%r5
Lmod:
    risbg          %r1,%r5,60,191,0
    jz             L1x
    sgr            %r3,%r1
    sgr            %r4,%r1
    sgr            %r2,%r1
    aghi           %r1,-1                        C highest index
    vll            %v24,%r1,0(%r3)
    vll            %v28,%r1,0(%r4)
    vx             %v24,%v24,%v28
    vstl           %v24,%r1,0(%r2)
L1x:
    risbg          %r1,%r5,58,187,0
    jz             L4x
    srlg           %r1,%r1,4                     C 1-block loop count 'n / 16'
L1x_loop:
    aghi           %r3,-16
    aghi           %r4,-16
    aghi           %r2,-16
    vl             %v24,0(%r3),0
    vl             %v28,0(%r4),0
    vx             %v24,%v24,%v28
    vst            %v24,0(%r2),0
    brctg          %r1,L1x_loop
L4x:
    risbg          %r1,%r5,0,185,0
    jz             Ldone
    srlg           %r1,%r1,6                     C 4-blocks loop count 'n / (16 * 4)'
L4x_loop:
    aghi           %r3,-64
    aghi           %r4,-64
    aghi           %r2,-64
    vlm            %v24,%v27,0(%r3),0
    vlm            %v28,%v31,0(%r4),0
    vx             %v24,%v24,%v28
    vx             %v25,%v25,%v29
    vx             %v26,%v26,%v30
    vx             %v27,%v27,%v31
    vstm           %v24,%v27,0(%r2),0
    brctg          %r1,L4x_loop
Ldone:
    br             RA
EPILOGUE(nettle_memxor3)
