
all dynamic: conditional_build

libfdt_src_name := dtc-1.7.0

# BRCM_SUPPORTS_MULTIARCH_BUILD

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

ifeq ($(strip $(DESKTOP_LINUX)),y)
EXTRA_CFLAGS := $(BCM_LD_FLAGS)
EXTRA_CFLAGS += -Wno-sign-compare
LDFLAGS := $(BCM_LD_FLAGS)
export LDFLAGS
else
EXTRA_CFLAGS := -Wno-sign-compare
endif
export LINUX_VER_STR TOOLCHAIN_PREFIX EXTRA_CFLAGS

check_untar:
	if [ ! -e $(libfdt_src_name)/Makefile ]; then \
	echo "Untarring original $(libfdt_src_name) source"; \
		unzip $(libfdt_src_name).zip; \
		touch $(libfdt_src_name)/Makefile; \
	fi;

libfdt: check_untar
	mkdir -p $(INSTALL_DIR)/lib/public/
	(export PKG_CONFIG_LIBDIR=$(BCM_FSBUILD_DIR)/public/lib; \
	export PKG_CONFIG_PATH=$(BCM_FSBUILD_DIR)/public/lib/pkgconfig; \
	cd $(libfdt_src_name); \
	$(MAKE) NO_PYTHON=1 libfdt PREFIX=$(BCM_FSBUILD_DIR)/public/; \
	$(MAKE) NO_PYTHON=1 libfdt PREFIX=$(BCM_FSBUILD_DIR)/public/ INSTALL="$(INSTALL)" install-lib ; \
	$(MAKE) NO_PYTHON=1 libfdt PREFIX=$(BCM_FSBUILD_DIR)/public/ INSTALL="$(INSTALL)" install-includes; )
	mkdir -p  $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	cp -d $(BCM_FSBUILD_DIR)/public/lib/libfdt*.so* $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

conditional_build: libfdt
ifneq ($(strip $(BUILD_DTC)),)
	mkdir -p $(INSTALL_DIR)/bin
	cp $(libfdt_src_name)/dtc $(INSTALL_DIR)/bin/
endif


clean:
	rm -f $(INSTALL_DIR)/lib/public/libfdt.so*
	rm -f $(INSTALL_DIR)/bin/dtc
	if [ -e $(libfdt_src_name)/Makefile ]; then \
		cd $(libfdt_src_name); $(MAKE) clean; \
	fi;
	rm -rf $(libfdt_src_name)

bcm_dorel_distclean: distclean

distclean: clean

shell:
	bash -i

