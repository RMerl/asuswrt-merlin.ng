#! /bin/sh

#
# gen-note-integrity.sh - Build tool to generate hmac hash section
#
# Copyright (C) 2022  g10 Code GmbH
#
# This file is part of libgcrypt.
#
# libgcrypt is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# libgcrypt is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, see <https://www.gnu.org/licenses/>.
#

set -e

#
# Following variables should be defined to invoke this script
#
#   READELF
#   AWK
#   ECHO_N
#

######## Emit ElfN_Nhdr for note.fdo.integrity ########

NOTE_NAME="FDO"

# n_namesz = 4 including NUL
printf '%b' '\004'
printf '%b' '\000'
printf '%b' '\000'
printf '%b' '\000'

# n_descsz = 32
printf '%b' '\040'
printf '%b' '\000'
printf '%b' '\000'
printf '%b' '\000'

# n_type: NT_FDO_INTEGRITY=0xCAFE2A8E
printf '%b' '\312'
printf '%b' '\376'
printf '%b' '\052'
printf '%b' '\216'

# the name
echo $ECHO_N $NOTE_NAME
printf '%b' '\000'

# Here comes the alignment.  As the size of name is 4, it's none.
# NO PADDING HERE.

######## Rest is to generate hmac hash ########

AWK_VERSION_OUTPUT=$($AWK 'BEGIN { print PROCINFO["version"] }')
if test -n "$AWK_VERSION_OUTPUT"; then
    # It's GNU awk, which supports PROCINFO.
    AWK_OPTION=--non-decimal-data
fi

FILE=.libs/libgcrypt.so

#
# Fixup the ELF header to clean up section information
#
BYTE002=$(printf '%b' '\002')
CLASS_BYTE=$(dd ibs=1 skip=4 count=1 if=$FILE status=none)
if test "$CLASS_BYTE" = "$BYTE002"; then
    CLASS=64
    HEADER_SIZE=64
else
    CLASS=32
    HEADER_SIZE=52
fi

if test $CLASS -eq 64; then
    dd ibs=1         count=40 if=$FILE     status=none
    dd ibs=1         count=8  if=/dev/zero status=none
    dd ibs=1 skip=48 count=10 if=$FILE     status=none
    dd ibs=1         count=6  if=/dev/zero status=none
else
    dd ibs=1         count=32 if=$FILE     status=none
    dd ibs=1         count=4  if=/dev/zero status=none
    dd ibs=1 skip=36 count=10 if=$FILE     status=none
    dd ibs=1         count=6  if=/dev/zero status=none
fi > header-fixed.bin

#
# Compute the end of segments, and emit the COUNT to read
# (For each segment in program headers, calculate the offset
#  and select the maximum)
#
# This require computation in hexadecimal, and GNU awk needs
# --non-decimal-data option
#
COUNT=$($READELF --wide --program-headers $FILE | \
         $AWK $AWK_OPTION \
"BEGIN { max_offset=0 }
/^\$/ { if (program_headers_start) program_headers_end=1 }
(program_headers_start && !program_headers_end) { offset = \$2 + \$5 }
(max_offset < offset) { max_offset = offset }
/^  Type/ { program_headers_start=1 }
END { print max_offset- $HEADER_SIZE }")

#
# Feed the header fixed and all segments to HMAC256
# to generate hmac hash of the FILE
#
(cat header-fixed.bin; \
 dd ibs=1 skip=$HEADER_SIZE count=$COUNT if=$FILE status=none) \
 | ./hmac256 --stdkey --binary

rm -f header-fixed.bin
