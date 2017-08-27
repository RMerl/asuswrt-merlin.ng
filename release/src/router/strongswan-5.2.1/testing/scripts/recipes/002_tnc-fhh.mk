#!/usr/bin/make

PKG = fhhtnc
SRC = git://github.com/trustatfhh/tnc-fhh.git

NUM_CPUS := $(shell getconf _NPROCESSORS_ONLN)

CONFIG_OPTS = \
	-DCOMPONENT=all \
	-DNAL=8021x

all: install

.$(PKG)-cloned:
	git clone $(SRC) $(PKG)
	mkdir $(PKG)/build
	@touch $@

.$(PKG)-configured: .$(PKG)-cloned
	cd $(PKG)/build && cmake $(CONFIG_OPTS) ../
	@touch $@

.$(PKG)-built: .$(PKG)-configured
	cd $(PKG)/build && make -j $(NUM_CPUS)
	@touch $@

install: .$(PKG)-built
	cd $(PKG)/build && make install
