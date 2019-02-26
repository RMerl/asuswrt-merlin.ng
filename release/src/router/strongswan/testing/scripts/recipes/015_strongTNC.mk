#!/usr/bin/make

PKG = strongTNC
ZIP = $(PKG)-master.zip
SRC = https://github.com/strongswan/$(PKG)/archive/master.zip
DEPS = $(PKG)-deps

all: install

$(ZIP):
	wget --ca-directory=/usr/share/ca-certificates/mozilla/ $(SRC) -O $(ZIP)

$(PKG)-master: $(ZIP)
	unzip -u $(ZIP)

$(DEPS): $(PKG)-master
	mkdir -p $(DEPS)
	pip install --download $(DEPS) -r $(PKG)-master/requirements.txt

install: $(DEPS)
	pip install --no-index --find-links=file://`pwd`/$(DEPS) -r $(PKG)-master/requirements.txt
	cp -r $(PKG)-master /var/www/tnc && chgrp -R www-data /var/www/tnc && chmod g+sw /var/www/tnc
