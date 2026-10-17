#
# CFLAGS used for compilation
#
# Copyright (C) 2025, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual:>>

CFLAGS += -Wall \
	-Werror \
	-Wextra \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wpointer-arith \
	-Wshadow \
	-Wcast-qual \
	-Wfloat-equal \
	-Wno-unused-parameter \
	-Wno-strict-aliasing \
	-Wno-address-of-packed-member \
	-Wno-array-bounds \
	-Wno-misleading-indentation \
	-Wframe-larger-than=8192

# This comment block must stay at the bottom of the file.
# Local Variables:
# mode: makefile-gmake
# fill-column: 80
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 noet
