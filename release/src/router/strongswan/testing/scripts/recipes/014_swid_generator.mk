#!/usr/bin/make

PKG = swidGenerator
ZIP = $(PKG)-master.zip
SRC = https://github.com/strongswan/$(PKG)/archive/master.zip

all: install

$(ZIP):
	wget --ca-directory="/usr/share/ca-certificates/mozilla" $(SRC) -O $(ZIP)

$(PKG)-master: $(ZIP)
	unzip $(ZIP)

install: $(PKG)-master
	cd $(PKG)-master && python setup.py install
