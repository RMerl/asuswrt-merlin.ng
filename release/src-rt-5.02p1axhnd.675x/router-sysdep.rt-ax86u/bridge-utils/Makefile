
bridge-utils: conditional_build 

.PHONY: all distclean clean conditional_build bridge-utils 

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

APP=bridge-utils-1.5
PATCH_FILE = bcm_patches/$(APP).patch

ifneq ($(strip $(BUILD_BRCTL)),)
conditional_build: all
else
conditional_build:
	@echo "skipping $(APP) (not configured)"
endif

APPCFLAGS:=$(BRCM_COMMON_CFLAGS)
APPCFLAGS += -Wno-cpp -I$(KERNEL_LINKS_DIR)
ifneq ($(strip $(BUILD_VLAN_AGGR)),)
APPCFLAGS += -DCONFIG_BCM_VLAN_AGGREGATION -DCONFIG_BCM_KF_VLAN_AGGREGATION
endif

$(APP)/configure: $(PATCH_FILE) $(APP).tar.gz
	rm -rf $(APP)
	tar xzf $(APP).tar.gz
	patch -p1 -d$(APP) < $(PATCH_FILE)
	(cd $(APP); autoconf -o configure configure.in)
	@echo "$(APP) is untarred"

objs/$(PROFILE_ARCH)/config_complete: $(APP)/configure
	mkdir -p objs/$(PROFILE_ARCH)
	(cd objs/$(PROFILE_ARCH); \
     ../../$(APP)/configure --host=$(TOOLCHAIN_PREFIX) \
         --prefix=$(BCM_FSBUILD_DIR)/gpl \
         --with-linux-headers=$(TOOLCHAIN_INCLUDE_DIR) \
         LDFLAGS=-L$(INSTALL_DIR)/lib \
         CFLAGS="$(APPCFLAGS)")
	touch objs/$(PROFILE_ARCH)/config_complete

all: objs/$(PROFILE_ARCH)/config_complete
	mkdir -p $(INSTALL_DIR)/bin
	$(MAKE) -C objs/$(PROFILE_ARCH)
	$(MAKE) -C objs/$(PROFILE_ARCH) install
	mkdir -p $(INSTALL_DIR)/bin$(BCM_INSTALL_SUFFIX_DIR)
	cp -d $(BCM_FSBUILD_DIR)/gpl/sbin/brctl $(INSTALL_DIR)/bin$(BCM_INSTALL_SUFFIX_DIR)

install:
	@echo done

clean:
	rm -f $(INSTALL_DIR)/bin/brctl
	rm -f $(INSTALL_DIR)/*/bin/brctl
	rm -rf objs
	rm -rf $(APP)

distclean: clean

bcm_dorel_distclean: distclean

shell:
	bash -i

