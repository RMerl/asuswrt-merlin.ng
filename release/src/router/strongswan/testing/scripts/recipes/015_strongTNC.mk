#!/usr/bin/make

PKG = strongTNC
REV = 1.0
DIR = $(PKG)-$(REV)
ZIP = $(PKG)-$(REV).zip
SRC = https://github.com/strongswan/$(PKG)/archive/$(REV).zip
DEPS = $(PKG)-deps

all: install

$(ZIP):
	wget --ca-directory=/usr/share/ca-certificates/mozilla/ $(SRC) -O $(ZIP)

.$(PKG)-unpacked-$(REV): $(ZIP)
	[ -d $(DIR) ] || unzip $(ZIP)
	@touch $@

.$(PKG)-deps-$(REV): .$(PKG)-unpacked-$(REV)
	mkdir -p $(DEPS)
	pip3 download -d $(DEPS) -r $(DIR)/requirements.txt
	@touch $@

install: .$(PKG)-deps-$(REV)
	pip3 install --no-index --find-links=file://`pwd`/$(DEPS) -r $(DIR)/requirements.txt
	cp -r $(DIR) /var/www/tnc && chgrp -R www-data /var/www/tnc && chmod g+sw /var/www/tnc
