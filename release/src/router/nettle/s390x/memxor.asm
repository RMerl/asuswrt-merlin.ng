C s390x/memxor.asm

ifelse(`
   Copyright (C) 2021 Mamone Tarsha
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

.file "memxor.asm"
.machine "z10"

.text

C void * memxor(void *dst, const void *src, size_t n)

PROLOGUE(nettle_memxor)
    srlg           %r5,%r4,8
    clgije         %r5,0,Llen
L256_loop:
    xc             0(256,%r2),0(%r3)
    aghi           %r2,256
    aghi           %r3,256
    brctg          %r5,L256_loop
Llen:
    risbg          %r5,%r4,56,191,0
    jz             Ldone
    XOR_LEN(%r2,%r3,%r5,%r1)
Ldone:
    br             RA
EPILOGUE(nettle_memxor)
