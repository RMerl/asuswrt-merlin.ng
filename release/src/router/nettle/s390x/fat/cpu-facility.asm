C s390x/fat/cpu-facility.asm

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

.file "cpu-facility.asm"

.text

C void _nettle_stfle(uint64_t *facility, uint64_t facility_size)

PROLOGUE(_nettle_stfle)
    lgr            %r0,%r3                       C facility_size
    aghi           %r0,-1                        C facility_size - 1
    .long          0xb2b02000                    C stfle 0(%r2)
    br             RA
EPILOGUE(_nettle_stfle)

C void _nettle_km_status(uint64_t *status)

PROLOGUE(_nettle_km_status)
    lghi           %r0,0                         C query code
    lgr            %r1,%r2                       C status
    .long          0xb92e0022                    C km %r2,%r2. Operands are ignored
    br             RA
EPILOGUE(_nettle_km_status)

C void _nettle_kimd_status(uint64_t *status)

PROLOGUE(_nettle_kimd_status)
    lghi           %r0,0                         C query code
    lgr            %r1,%r2                       C status
    .long          0xb93e0002                    C kimd %r0,%r2. Operands are ignored
    br             RA
EPILOGUE(_nettle_kimd_status)
