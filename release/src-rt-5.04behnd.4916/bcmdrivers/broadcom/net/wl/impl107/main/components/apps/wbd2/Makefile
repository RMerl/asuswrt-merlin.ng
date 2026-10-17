#
# Linux WBD Makefile
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
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: Makefile 848447 2024-11-21 11:37:07Z $
#

dirs = shared master slave cli hld
WBDINSTALLDIR = $(INSTALLDIR)/wbd
PREBUILTDIR = prebuilt
LIBWBDSHARED = $(PREBUILTDIR)/libwbdshared.so
WBDMASTER = $(PREBUILTDIR)/wbd_master
WBDSLAVE = $(PREBUILTDIR)/wbd_slave
WBDCLI = $(PREBUILTDIR)/wb_cli
WBDHLD = $(PREBUILTDIR)/wb_hld_demo
LIBWBDHLD = $(PREBUILTDIR)/libwbdhld.so

ifeq ($(PLC_WBD),1)
export CFLAGS += -DPLC_WBD
endif

CFLAGS  += -DWL11AC_160

# export WBD_USE_STUBS=y
ifeq ($(WBD_USE_STUBS), y)
CFLAGS += -DWBD_USE_STUBS
endif

CFLAGS += -I$(WPA_SUPPLICANT_DIR)/src/common

CFLAGS += -Wno-address-of-packed-member

# For Plugfest branch only. TODO : Remove in production branch
# CFLAGS += -DMULTIAP_PLUGFEST

ifeq ($(BUILD_ASAN), y)
CFLAGS := $(filter-out -O% -g% -s, $(CFLAGS)) -Og -g3
CFLAGS += -fsanitize=address -fsanitize-recover=address -U_FORTIFY_SOURCE -fno-omit-frame-pointer -fno-common \
	--param asan-instrumentation-with-call-threshold=0 -Wno-stringop-truncation
LD_FLAGS += -fsanitize=address
STRIP = /bin/touch
endif

DOXY_PATH = dox
DOXY_FILE = wbd.conf

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
	install -d $(WBDINSTALLDIR)/usr/lib
	install -d $(WBDINSTALLDIR)/usr/sbin/
# Install WBD Shared Library
	install -m 755 $(LIBWBDSHARED) $(WBDINSTALLDIR)/usr/lib
	$(STRIP) $(WBDINSTALLDIR)/usr/lib/libwbdshared.so
# Install WBD Master App
	install $(WBDMASTER) $(WBDINSTALLDIR)/usr/sbin/
# Install WBD Slave App
	install $(WBDSLAVE) $(WBDINSTALLDIR)/usr/sbin/
# Install WBD CLI App
	install $(WBDCLI) $(WBDINSTALLDIR)/usr/sbin/
# Install WBD HLD App
	install $(WBDHLD) $(WBDINSTALLDIR)/usr/sbin/
# Install WBD HLD Library
	install $(LIBWBDHLD) $(WBDINSTALLDIR)/usr/lib/

.PHONY: doc
doc:
	doxygen $(DOXY_FILE)

.PHONY: clean
clean:
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i clean ; \
	done
	rm -rf $(DOXY_PATH)
