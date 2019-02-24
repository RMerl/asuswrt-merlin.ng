#!/usr/bin/make

PKG = anet
SRC = http://git.codelabs.ch/git/$(PKG).git
REV = c9bdee807f2fcd2b6ec2ad8fe4c814e1abb71358

PREFIX = /usr/local/ada

all: install

.$(PKG)-cloned:
	[ -d $(PKG) ] || git clone $(SRC) $(PKG)
	@touch $@

.$(PKG)-checkout-$(REV): .$(PKG)-cloned
	cd $(PKG) && git fetch && git checkout $(REV)
	@touch $@

.$(PKG)-built-$(REV): .$(PKG)-checkout-$(REV)
	cd $(PKG) && make LIBRARY_KIND=static
	@touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make PREFIX=$(PREFIX) LIBRARY_KIND=static install
