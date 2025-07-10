# Python bindings for Linux SMBus access through i2c-dev
#
# Copyright (C) 2009  Mike Frysinger <vapier@gentoo.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

PY_SMBUS_DIR := py-smbus

PYTHON ?= python
DISTUTILS := \
	cd $(PY_SMBUS_DIR) && \
	$(PYTHON) setup.py

all-python: $(INCLUDE_DIR)/i2c/smbus.h
	$(DISTUTILS) build

clean-python:
	$(DISTUTILS) clean
	rm -rf py-smbus/build

install-python:
	$(DISTUTILS) install

all: all-python

clean: clean-python

install: install-python
