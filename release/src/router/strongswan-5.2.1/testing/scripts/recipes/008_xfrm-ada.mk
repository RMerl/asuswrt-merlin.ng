#!/usr/bin/make

PKG = xfrm-ada
SRC = http://git.codelabs.ch/git/$(PKG).git
REV = v0.1

PREFIX = /usr/local/ada

export ADA_PROJECT_PATH=$(PREFIX)/lib/gnat

all: install

$(PKG):
	git clone $(SRC) $(PKG)

.$(PKG)-cloned-$(REV): $(PKG)
	cd $(PKG) && git fetch && git checkout $(REV)
	@touch $@

.$(PKG)-built-$(REV): .$(PKG)-cloned-$(REV)
	cd $(PKG) && make
	@touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make PREFIX=$(PREFIX) install
