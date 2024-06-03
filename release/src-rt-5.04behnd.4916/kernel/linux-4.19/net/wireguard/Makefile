# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

KERNELRELEASE ?= $(shell uname -r)
KERNELDIR ?= /lib/modules/$(KERNELRELEASE)/build
PREFIX ?= /usr
DESTDIR ?=
SRCDIR ?= $(PREFIX)/src
DKMSDIR ?= $(SRCDIR)/wireguard
DEPMOD ?= depmod
DEPMODBASEDIR ?= /

PWD := $(shell pwd)

all: module
debug: module-debug

ifneq ($(V),1)
MAKEFLAGS += --no-print-directory
endif

WIREGUARD_VERSION = $(patsubst v%,%,$(shell GIT_CEILING_DIRECTORIES="$(PWD)/../.." git describe --dirty 2>/dev/null))

module:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules

module-debug:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) V=1 CONFIG_WIREGUARD_DEBUG=y WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules

clean:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

module-install:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules_install
	$(DEPMOD) -b "$(DEPMODBASEDIR)" -a $(KERNELRELEASE)

install: module-install

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
DKMS_SOURCES := version.h Makefile Kbuild Kconfig dkms.conf $(filter-out version.h wireguard.mod.c tests/%,$(call rwildcard,,*.c *.h *.S *.pl *.include))
dkms-install: $(DKMS_SOURCES)
	@$(foreach f,$(DKMS_SOURCES),install -v -m0644 -D $(f) $(DESTDIR)$(DKMSDIR)/$(f);)

style:
	$(KERNELDIR)/scripts/checkpatch.pl -f --max-line-length=4000 --codespell --color=always $(filter-out wireguard.mod.c,$(wildcard *.c)) $(wildcard *.h) $(wildcard selftest/*.c)

check: clean
	scan-build --html-title=wireguard-linux-compat -maxloop 100 --view --keep-going $(MAKE) module CONFIG_WIREGUARD_DEBUG=y C=2 CF="-D__CHECK_ENDIAN__"

coccicheck: clean
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) CONFIG_WIREGUARD_DEBUG=y coccicheck MODE=report

cloc:
	@cloc --skip-uniqueness --by-file --extract-with="$$(readlink -f ../kernel-tree-scripts/filter-compat-defines.sh) >FILE< > \$$(basename >FILE<)" $(filter-out wireguard.mod.c,$(wildcard *.c)) $(wildcard *.h)

-include tests/debug.mk

.PHONY: all module module-debug module-install install dkms-install clean cloc check style
