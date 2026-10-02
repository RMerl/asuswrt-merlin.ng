
civetweb: all 

LIB_TAR := civetweb-1.16.tar.gz
LIB_DIR := civetweb

# ASUSWRT
BUILD_DIR=$(SRCBASE)/../../../../../../../
#CURR_DIR := $(shell pwd)
#BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
#BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

PREFIX := $(BCM_FSBUILD_DIR)/public
HEADER_INSTALL_DIR := $(BCM_FSBUILD_DIR)/public/include

BCM_HOST := $(TOOLCHAIN_PREFIX)

untar: $(LIB_TAR)
	mkdir -p $(LIB_DIR); cd $(LIB_DIR); tar --strip-components=1 -xzf ../$(LIB_TAR)
	@echo "$(LIB_DIR) is untarred"


all: untar
	cd $(LIB_DIR) ; $(MAKE) lib && $(MAKE) install-lib PREFIX=$(PREFIX)
	cp -d $(LIB_DIR)/include/civetweb.h $(HEADER_INSTALL_DIR)

clean:
	[ ! -e $(LIB_DIR)/Makefile ] || $(MAKE) -C $(LIB_DIR) clean
	rm -f $(HEADER_INSTALL_DIR)/civetweb.h
	rm -f $(PREFIX)/public/lib/libcivetweb.a
	rm -rf $(LIB_DIR)

bcm_dorel_distclean: distclean

distclean: clean
