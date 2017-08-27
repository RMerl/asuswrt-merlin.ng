#!/usr/bin/make

PV  = 1.0.1e
PKG = openssl-$(PV)
SRC = http://download.strongswan.org/testing/openssl-fips/

all: install

$(PKG):
	wget -r $(SRC) --no-directories --directory-prefix $(PKG) --accept deb

install: $(PKG)
	cd $(PKG) && dpkg -i *.deb
