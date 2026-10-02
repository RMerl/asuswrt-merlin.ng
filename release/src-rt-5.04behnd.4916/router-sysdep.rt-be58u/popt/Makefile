
libpopt: conditional_build 

# BRCM_SUPPORTS_MULTIARCH_BUILD

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

export LINUX_VER_STR TOOLCHAIN_PREFIX

ifneq ($(strip $(BUILD_LIBPOPT)),)
conditional_build: all
else
conditional_build:
	@echo "skipping libpopt (not configured)"
endif

LIBNAME=popt-1.16
$(LIBNAME)/configure: $(LIBNAME).tar.gz
	tar -xzf $(LIBNAME).tar.gz
	patch -p1 -b -N -s -d$(LIBNAME) < $(LIBNAME).patch 
	touch -c $(LIBNAME)/configure
	@echo "$(LIBNAME) is untarred"

check_config: objs/$(PROFILE_ARCH)/Makefile

objs/$(PROFILE_ARCH)/Makefile: $(LIBNAME)/configure
	mkdir -p objs/$(PROFILE_ARCH)
	cd objs/$(PROFILE_ARCH) ; ac_cv_linux_vers=$(LINUX_VER_STR)  ../../$(LIBNAME)/configure --host=$(TOOLCHAIN_PREFIX) --with-popt=linux --prefix=$(BCM_FSBUILD_DIR)/public/

all: check_config
	mkdir -p $(INSTALL_DIR)/lib/public/
	cd objs/$(PROFILE_ARCH) ; $(MAKE)
	cd objs/$(PROFILE_ARCH) ; $(MAKE) install
	mkdir -p  $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	cp -d $(BCM_FSBUILD_DIR)/public/lib/libpopt.so* $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

clean:
	rm -f $(INSTALL_DIR)/lib/public/libpopt.so*
	-[ ! -e objs/$(PROFILE_ARCH)/Makefile ] || $(MAKE) -C objs/$(PROFILE_ARCH) uninstall 
	rm -rf objs
	rm -rf $(LIBNAME)

install:
	cd objs/$(PROFILE_ARCH) ; $(MAKE) install
	mkdir -p  $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	cp -d $(BCM_FSBUILD_DIR)/public/lib/libpopt.so* $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

bcm_dorel_distclean: distclean

distclean: clean

shell:
	bash -i

