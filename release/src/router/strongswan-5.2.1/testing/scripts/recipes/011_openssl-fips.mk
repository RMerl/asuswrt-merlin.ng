#!/usr/bin/make

PV  = 2.0.3
PKG = openssl-fips-$(PV)
TAR = $(PKG).tar.gz
SRC = http://www.openssl.org/source/$(TAR)

all: install

$(TAR):
	wget $(SRC)

$(PKG): $(TAR)
	tar xfz $(TAR)

configure: $(PKG)
	cd $(PKG) && ./config

build: configure
	cd $(PKG) && make

install: build
	cd $(PKG) && make install
