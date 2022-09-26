#!/usr/bin/make

PKG = spark-crypto
SRC = https://git.codelabs.ch/spark-crypto.git
REV = c97939b6cdd5e5f19847cf8d1abb7575e1c01df7

DESTDIR = /usr/local/ada/lib/gnat

all: install

.$(PKG)-cloned:
	[ -d $(PKG) ] || git clone $(SRC) $(PKG)
	@touch $@

.$(PKG)-checkout-$(REV): .$(PKG)-cloned
	cd $(PKG) && git fetch && git checkout $(REV)
	@rm -f .$(PKG)-checkout-* && touch $@

.$(PKG)-built-$(REV): .$(PKG)-checkout-$(REV)
	cd $(PKG) && make NO_SPARK=1 NO_TESTS=1 NO_APIDOC=1
	@rm -f .$(PKG)-built-* && touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make NO_SPARK=1 NO_TESTS=1 NO_APIDOC=1 DESTDIR=$(DESTDIR) install
