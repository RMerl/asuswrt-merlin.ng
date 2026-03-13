#!/usr/bin/make

PKG = tkm
SRC = https://git.codelabs.ch/git/$(PKG).git
ifeq (,$(filter $(BASEIMG),bullseye bookworm))
REV = 85d49c9255360663c344ccf3175092524dc66eae
else
REV = v0.3
endif

export ADA_PROJECT_PATH=/usr/local/ada/lib/gnat

all: install

.$(PKG)-cloned:
	[ -d $(PKG) ] || git clone $(SRC) $(PKG)
	@touch $@

.$(PKG)-checkout-$(REV): .$(PKG)-cloned
	cd $(PKG) && git fetch && git checkout $(REV)
	@rm -f .$(PKG)-checkout-* && touch $@

.$(PKG)-built-$(REV): .$(PKG)-checkout-$(REV)
	cd $(PKG) && make tests && make
	@rm -f .$(PKG)-built-* && touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make install
