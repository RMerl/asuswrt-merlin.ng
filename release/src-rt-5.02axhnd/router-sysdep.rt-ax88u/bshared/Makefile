#
# Binary shared library Makefile for Linux router
#
# Copyright (C) 2020, Broadcom. All Rights Reserved.
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
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id$
#

dirs = src
PREBUILTDIR = prebuilt
BSHARED = $(PREBUILTDIR)/libbshared.so

.PHONY: all
all:
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i || exit $$? ; \
	done

.PHONY: install
install:
	install -d $(PREBUILTDIR)
# Call individual Makefiles if the directory exists which will copy to prebuilt
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i install || exit $$? ; \
	done
# Now install to router install directory from prebuilt
	install -d $(INSTALLDIR)/usr/lib
# Install Binary Shared Library
	install -m 755 $(BSHARED) $(INSTALLDIR)/usr/lib

.PHONY: clean
clean:
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i clean ; \
	done
