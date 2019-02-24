#!/usr/bin/make

PV  = $(SWANVERSION)
PKG = strongswan-$(PV)
TAR = $(PKG).tar.bz2
SRC = http://download.strongswan.org/$(TAR)

# can be passed to load sources from a directory instead of a tarball
ifneq ($(origin SRCDIR), undefined)
DIR = $(SRCDIR)
BUILDDIR ?= $(SRCDIR)
endif
DIR ?= .
# can be passed if not building in the source directory
BUILDDIR ?= $(PKG)

NUM_CPUS := $(shell getconf _NPROCESSORS_ONLN)

CONFIG_OPTS = \
	--sysconfdir=/etc \
	--with-strongswan-conf=/etc/strongswan.conf.testing \
	--with-random-device=/dev/urandom \
	--disable-load-warning \
	--enable-curl \
	--enable-soup \
	--enable-ldap \
	--enable-eap-aka \
	--enable-eap-aka-3gpp2 \
	--enable-eap-sim \
	--enable-eap-sim-file \
	--enable-eap-simaka-sql \
	--enable-eap-md5 \
	--enable-md4 \
	--enable-eap-mschapv2 \
	--enable-eap-identity \
	--enable-eap-radius \
	--enable-eap-dynamic \
	--enable-eap-tls \
	--enable-eap-ttls \
	--enable-eap-peap \
	--enable-eap-tnc \
	--enable-tnc-ifmap \
	--enable-tnc-pdp \
	--enable-tnc-imc \
	--enable-tnc-imv \
	--enable-tnccs-11 \
	--enable-tnccs-20 \
	--enable-tnccs-dynamic \
	--enable-imc-test \
	--enable-imv-test \
	--enable-imc-scanner \
	--enable-imv-scanner \
	--enable-imc-os \
	--enable-imv-os \
	--enable-imc-attestation \
	--enable-imv-attestation \
	--enable-imc-swima \
	--enable-imv-swima \
	--enable-imc-hcd \
	--enable-imv-hcd \
	--enable-sql \
	--enable-sqlite \
	--enable-attr-sql \
	--enable-mediation \
	--enable-botan \
	--enable-openssl \
	--enable-blowfish \
	--enable-kernel-pfkey \
	--enable-integrity-test \
	--enable-leak-detective \
	--enable-load-tester \
	--enable-test-vectors \
	--enable-gcrypt \
	--enable-socket-default \
	--enable-socket-dynamic \
	--enable-dhcp \
	--enable-farp \
	--enable-connmark \
	--enable-forecast \
	--enable-addrblock \
	--enable-ctr \
	--enable-ccm \
	--enable-gcm \
	--enable-cmac \
	--enable-chapoly \
	--enable-ha \
	--enable-af-alg \
	--enable-whitelist \
	--enable-xauth-generic \
	--enable-xauth-eap \
	--enable-pkcs8 \
	--enable-unity \
	--enable-unbound \
	--enable-ipseckey \
	--enable-dnscert \
	--enable-acert \
	--enable-cmd \
	--enable-libipsec \
	--enable-kernel-libipsec \
	--enable-tkm \
	--enable-ntru \
	--enable-lookip \
	--enable-bliss \
	--enable-sha3 \
	--enable-newhope \
	--enable-systemd \
	--enable-counters \
	--enable-save-keys

export ADA_PROJECT_PATH=/usr/local/ada/lib/gnat

all: install

$(TAR):
	wget $(SRC)

$(PKG): $(TAR)
	tar xfj $(TAR)
	echo "$(SWANVERSION)" > /root/shared/.strongswan-version

configure: $(BUILDDIR)
	cd $(BUILDDIR) && $(DIR)/configure $(CONFIG_OPTS)

build: configure
	cd $(BUILDDIR) && make -j $(NUM_CPUS)

install: build
	cd $(BUILDDIR) && make -j install
