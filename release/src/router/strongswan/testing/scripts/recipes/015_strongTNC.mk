#!/usr/bin/make

PKG = strongTNC
REV = 1.0.2
DIR = $(PKG)-$(REV)
ZIP = $(PKG)-$(REV).zip
SRC = https://github.com/strongswan/$(PKG)/archive/$(REV).zip
DEPS = $(PKG)-deps
VENV = /usr/local/venvs/tnc

all: install

$(ZIP):
	wget --ca-directory=/usr/share/ca-certificates/mozilla/ $(SRC) -O $(ZIP)

.$(PKG)-unpacked-$(REV): $(ZIP)
	[ -d $(DIR) ] || unzip $(ZIP)
	@touch $@

.$(PKG)-deps-$(REV): .$(PKG)-unpacked-$(REV)
	python3 -m venv $(VENV)
	$(VENV)/bin/pip download -d $(DEPS) -r $(DIR)/requirements.txt
	@touch $@

install: .$(PKG)-deps-$(REV)
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install --no-index --find-links=file://`pwd`/$(DEPS) -r $(DIR)/requirements.txt
	cp -r $(DIR) /var/www/tnc && chgrp -R www-data /var/www/tnc && chmod g+sw /var/www/tnc
