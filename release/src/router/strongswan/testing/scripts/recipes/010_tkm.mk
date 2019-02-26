#!/usr/bin/make

PKG = tkm
SRC = http://git.codelabs.ch/git/$(PKG).git
REV = 53d224a7312124516aa6220743355c896be6345a

export ADA_PROJECT_PATH=/usr/local/ada/lib/gnat

all: install

.$(PKG)-cloned:
	[ -d $(PKG) ] || git clone $(SRC) $(PKG)
	@touch $@

.$(PKG)-checkout-$(REV): .$(PKG)-cloned
	cd $(PKG) && git fetch && git checkout $(REV)
	@touch $@

.$(PKG)-built-$(REV): .$(PKG)-checkout-$(REV)
	cd $(PKG) && make
	@touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make install
