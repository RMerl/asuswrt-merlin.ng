#
# Since version 1.6 the asynchronous mode has been
# disabled by default. To re-enable it uncomment the
# corresponding CFLAG.
#
CRYPTODEV_CFLAGS ?= #-DENABLE_ASYNC
KBUILD_CFLAGS += -I$(src) $(CRYPTODEV_CFLAGS) -Wvla
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
VERSION = 1.12

prefix ?= /usr/local
includedir = $(prefix)/include

cryptodev-objs = ioctl.o main.o cryptlib.o authenc.o zc.o util.o

obj-m += cryptodev.o

KERNEL_MAKE_OPTS := -C $(KERNEL_DIR) M=$(CURDIR)
ifneq ($(ARCH),)
KERNEL_MAKE_OPTS += ARCH=$(ARCH)
endif
ifneq ($(CROSS_COMPILE),)
KERNEL_MAKE_OPTS += CROSS_COMPILE=$(CROSS_COMPILE)
endif
ifneq ($(INSTALL_MOD_PATH),)
KERNEL_MAKE_OPTS += INSTALL_MOD_PATH=$(INSTALL_MOD_PATH)
endif

build: version.h
	$(MAKE) $(KERNEL_MAKE_OPTS) modules

version.h: Makefile
	@echo "#define VERSION \"$(VERSION)\"" > version.h

install: modules_install

modules_install:
	$(MAKE) $(KERNEL_MAKE_OPTS) modules_install
	install -m 644 -D crypto/cryptodev.h $(DESTDIR)/$(includedir)/crypto/cryptodev.h

bcm_headers_install:
	echo CRYPTODEV installing header
	install -m 644 -D crypto/cryptodev.h $(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/crypto/cryptodev.h

install_tests: tests
	$(MAKE) -C tests install DESTDIR=$(PREFIX)

clean:
	-$(MAKE) $(KERNEL_MAKE_OPTS) clean
	-CFLAGS=$(CRYPTODEV_CFLAGS) KERNEL_DIR=$(KERNEL_DIR) $(MAKE) -C tests clean
	@echo CRYPTODEV deleting header
	-rm -f $(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/crypto/cryptodev.h

check:
	CFLAGS=$(CRYPTODEV_CFLAGS) KERNEL_DIR=$(KERNEL_DIR) $(MAKE) -C tests check

tests:
	KERNEL_DIR=$(KERNEL_DIR) $(MAKE) -C tests

.PHONY: install modules_install tests install_tests

CPOPTS =
ifneq ($(SHOW_TYPES),)
CPOPTS += --show-types
endif
ifneq ($(IGNORE_TYPES),)
CPOPTS += --ignore $(IGNORE_TYPES)
endif

checkpatch:
	$(KERNEL_DIR)/scripts/checkpatch.pl $(CPOPTS) --file *.c *.h

VERSIONTAG = refs/tags/cryptodev-linux-$(VERSION)
FILEBASE = cryptodev-linux-$(VERSION)
OUTPUT = $(FILEBASE).tar.gz

dist: clean
	@echo Packing
	@rm -f *.tar.gz
	@git archive --format=tar.gz --prefix=$(FILEBASE)/ --output=$(OUTPUT) $(VERSIONTAG)
	@echo Signing $(OUTPUT)
	@gpg --output $(OUTPUT).sig -sb $(OUTPUT)
	@gpg --verify $(OUTPUT).sig $(OUTPUT)
