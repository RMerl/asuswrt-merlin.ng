#!/usr/bin/make

PKG = swidGenerator
REV = v1.1.0
DIR = $(PKG)-$(REV)
TAR = $(PKG)-$(REV).tar.gz
SRC = https://github.com/strongswan/$(PKG)/archive/$(REV).tar.gz

all: install

$(TAR):
	wget --ca-directory="/usr/share/ca-certificates/mozilla" $(SRC) -O $(TAR)

.$(PKG)-unpacked-$(REV): $(TAR)
	# a tag's "v" prefix is not reflected in the directory name in the archive
	[ -d $(DIR) ] || (mkdir -p $(DIR); tar -xf $(TAR) --strip-components=1 -C $(DIR))
	@touch $@

install: .$(PKG)-unpacked-$(REV)
	cd $(DIR) && SETUPTOOLS_USE_DISTUTILS=stdlib python3 setup.py install
