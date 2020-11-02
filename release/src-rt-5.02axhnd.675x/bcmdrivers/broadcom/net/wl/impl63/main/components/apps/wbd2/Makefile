#
# Linux WBD Makefile
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
# $Id: Makefile 778661 2019-09-06 12:28:01Z $
#

dirs = shared master slave cli
WBDINSTALLDIR = $(INSTALLDIR)/wbd
PREBUILTDIR = prebuilt
LIBWBDSHARED = $(PREBUILTDIR)/libwbdshared.so
WBDMASTER = $(PREBUILTDIR)/wbd_master
WBDSLAVE = $(PREBUILTDIR)/wbd_slave
WBDCLI = $(PREBUILTDIR)/wb_cli

ifeq ($(PLC_WBD),1)
export CFLAGS += -DPLC_WBD
endif

CFLAGS  += -DWL11AC_160

# Enable this flag for MultiAP R2
# CFLAGS += -DMULTIAPR2

# For Plugfest branch only. TODO : Remove in production branch
# CFLAGS += -DMULTIAP_PLUGFEST

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
	$(STRIP) $(WBDINSTALLDIR)/usr/sbin/wbd_master
# Install WBD Slave App
	install $(WBDSLAVE) $(WBDINSTALLDIR)/usr/sbin/
	$(STRIP) $(WBDINSTALLDIR)/usr/sbin/wbd_slave
# Install WBD CLI App
	install $(WBDCLI) $(WBDINSTALLDIR)/usr/sbin/
	$(STRIP) $(WBDINSTALLDIR)/usr/sbin/wb_cli

.PHONY: doc
doc:
	doxygen $(DOXY_FILE)

.PHONY: clean
clean:
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i clean ; \
	done
	rm -rf $(DOXY_PATH)
