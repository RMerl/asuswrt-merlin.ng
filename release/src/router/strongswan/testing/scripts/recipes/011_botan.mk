#!/usr/bin/make

PKG = botan
SRC = https://github.com/randombit/$(PKG).git
REV = 3.2.0

NUM_CPUS := $(shell getconf _NPROCESSORS_ONLN)

# the first two are necessary due to LD, the others to reduce the build time
CONFIG_OPTS = \
	--without-os-features=threads \
	--disable-modules=locking_allocator \
	--disable-modules=pkcs11,tls,x509,xmss \

all: install

.$(PKG)-cloned:
	[ -d $(PKG) ] || git clone $(SRC) $(PKG)
	@touch $@

.$(PKG)-checkout-$(REV): .$(PKG)-cloned
	cd $(PKG) && git fetch && git checkout $(REV)
	@rm -f .$(PKG)-checkout-* && touch $@

.$(PKG)-built-$(REV): .$(PKG)-checkout-$(REV)
	cd $(PKG) && python3 ./configure.py $(CONFIG_OPTS) && make -j $(NUM_CPUS)
	@rm -f .$(PKG)-built-* && touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make install && ldconfig
