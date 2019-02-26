#!/usr/bin/make

PKG = x509-ada
SRC = http://git.codelabs.ch/git/$(PKG).git
REV = v0.1.1

PREFIX = /usr/local/ada

all: install

.$(PKG)-cloned:
	[ -d $(PKG) ] || git clone $(SRC) $(PKG)
	@touch $@

.$(PKG)-checkout-$(REV): .$(PKG)-cloned
	cd $(PKG) && git fetch && git checkout $(REV)
	@touch $@

.$(PKG)-built-$(REV): .$(PKG)-checkout-$(REV)
	cd $(PKG) && make tests && make
	@touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make PREFIX=$(PREFIX) install
