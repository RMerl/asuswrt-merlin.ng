#
# curl Makefile
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
# $Id: $
#

CURL_SRC = curl-7.35.0

.PHONY: all
all: curl

.PHONY: configure
configure:
	cd $(CURL_SRC) && ./configure \
	    --host=arm-linux \
	    --disable-shared \
	    --without-ssl \
	    --disable-ares \
	    --disable-cookies \
	    --disable-crypto-auth \
	    --disable-ipv6 \
	    --disable-proxy \
	    --disable-verbose \
	    --enable-hidden-symbols \
	    --without-libidn \
	    --without-librtmp \
	    --without-zlib

.PHONY: install
install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install $(CURDIR)/$(CURL_SRC)/src/curl $(INSTALLDIR)/usr/sbin/

.PHONY: curl
curl: configure
	+$(MAKE) -C $(CURL_SRC)

.PHONY: clean
clean:
	+[ ! -f $(CURL_SRC)/Makefile ] || $(MAKE) -C $(CURL_SRC) clean
