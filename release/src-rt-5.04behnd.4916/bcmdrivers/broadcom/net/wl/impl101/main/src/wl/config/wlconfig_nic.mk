# Config makefile for NIC builds.

# Copyright (C) 2023, Broadcom. All Rights Reserved.
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
# <<Broadcom-WL-IPTag/Open:>>

# Identify this configuration.
CHIP = NIC

# Ucode parameters.
UC_TYPES := all
UC_DO_D11CONFS := 1
UC_PREBUILT := 1

# This comment block must stay at the bottom of the file.
# No reason for tabs in this file since no recipes.
# Local Variables:
# mode: GNUmakefile
# fill-column: 80
# indent-tabs-mode: nil
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 et
