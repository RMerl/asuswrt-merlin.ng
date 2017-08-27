#!/usr/bin/make

PV  = 1.25
PKG = libtnc-$(PV)
TAR = $(PKG).tar.gz
SRC = http://downloads.sourceforge.net/project/libtnc/libtnc/$(PV)/$(TAR)

NUM_CPUS := $(shell getconf _NPROCESSORS_ONLN)

CONFIG_OPTS = \
	--sysconfdir=/etc

all: install

$(TAR):
	wget $(SRC)

.$(PKG)-unpacked: $(TAR)
	tar xfz $(TAR)
	@touch $@

.$(PKG)-configured: .$(PKG)-unpacked
	cd $(PKG) && ./configure $(CONFIG_OPTS)
	@touch $@

.$(PKG)-built: .$(PKG)-configured
	cd $(PKG) && make -j $(NUM_CPUS)
	@touch $@

install: .$(PKG)-built
	cd $(PKG) && make install
