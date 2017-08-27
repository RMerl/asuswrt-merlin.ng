#!/usr/bin/make

PV  = 2.2.1
PKG = freeradius-server-$(PV)
TAR = $(PKG).tar.bz2
SRC = ftp://ftp.freeradius.org/pub/freeradius/old/$(TAR)

NUM_CPUS := $(shell getconf _NPROCESSORS_ONLN)

CONFIG_OPTS = \
	--with-raddbdir=/etc/freeradius \
	--sysconfdir=/etc \
	--with-logdir=/var/log/freeradius \
	--enable-developer \
	--with-experimental-modules

PATCHES = \
	freeradius-eap-sim-identity \
	freeradius-tnc-fhh

all: install

$(TAR):
	wget $(SRC)

.$(PKG)-unpacked: $(TAR)
	tar xfj $(TAR)
	@touch $@

.$(PKG)-patches-applied: .$(PKG)-unpacked
	cd $(PKG) && cat $(addprefix ../patches/, $(PATCHES)) | patch -p1
	@touch $@

.$(PKG)-configured: .$(PKG)-patches-applied
	cd $(PKG) && ./configure $(CONFIG_OPTS)
	@touch $@

.$(PKG)-built: .$(PKG)-configured
	cd $(PKG) && make -j $(NUM_CPUS)
	@touch $@

install: .$(PKG)-built
	cd $(PKG) && make install
