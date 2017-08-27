#!/usr/bin/make

PV  = 2.0
PKG = wpa_supplicant-$(PV)
TAR = $(PKG).tar.gz
SRC = http://hostap.epitest.fi/releases/$(TAR)

NUM_CPUS := $(shell getconf _NPROCESSORS_ONLN)

CONFIG_OPTS =

PATCHES = \
	wpa_supplicant-eap-tnc

SUBDIR = wpa_supplicant

all: install

$(TAR):
	wget $(SRC)

.$(PKG)-unpacked: $(TAR)
	tar xfz $(TAR)
	@touch $@

.$(PKG)-patches-applied: .$(PKG)-unpacked
	cd $(PKG) && cat $(addprefix ../patches/, $(PATCHES)) | patch -p1
	@touch $@

.$(PKG)-configured: .$(PKG)-patches-applied
	cp $(PKG)/$(SUBDIR)/defconfig $(PKG)/$(SUBDIR)/.config
	@touch $@

.$(PKG)-built: .$(PKG)-configured
	cd $(PKG)/$(SUBDIR) && make -j $(NUM_CPUS)
	@touch $@

install: .$(PKG)-built
	cd $(PKG)/$(SUBDIR) && make install
