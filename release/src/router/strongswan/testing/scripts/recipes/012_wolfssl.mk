#!/usr/bin/make

PKG = wolfssl
SRC = https://github.com/wolfSSL/$(PKG).git
REV = v5.6.4-stable

NUM_CPUS := $(shell getconf _NPROCESSORS_ONLN)

CFLAGS = \
	-DWOLFSSL_PUBLIC_MP \
	-DWOLFSSL_DES_ECB \
	-DHAVE_AES_ECB \
	-DHAVE_ECC_BRAINPOOL \
	-DWOLFSSL_MIN_AUTH_TAG_SZ=8

CONFIG_OPTS = \
	--disable-crypttests \
	--disable-examples \
	--enable-silent-rules \
	--enable-aesccm \
	--enable-aesctr \
	--enable-aescfb \
	--enable-camellia \
	--enable-curve25519 \
	--enable-curve448 \
	--enable-des3 \
	--enable-ecccustcurves \
	--enable-ed25519 \
	--enable-ed448 \
	--enable-keygen \
	--enable-max-rsa-bits=8192 \
	--enable-md4 \
	--enable-rsapss \
	--enable-sha3 \
	--enable-shake256

all: install

.$(PKG)-cloned:
	[ -d $(PKG) ] || git clone $(SRC) $(PKG)
	@touch $@

.$(PKG)-checkout-$(REV): .$(PKG)-cloned
	cd $(PKG) && git fetch --tags && git checkout $(REV)
	@rm -f .$(PKG)-checkout-* && touch $@

.$(PKG)-built-$(REV): .$(PKG)-checkout-$(REV)
	cd $(PKG) && ./autogen.sh && ./configure C_FLAGS="$(CFLAGS)" $(CONFIG_OPTS) && make -j $(NUM_CPUS)
	@rm -f .$(PKG)-built-* && touch $@

install: .$(PKG)-built-$(REV)
	cd $(PKG) && make install && ldconfig
