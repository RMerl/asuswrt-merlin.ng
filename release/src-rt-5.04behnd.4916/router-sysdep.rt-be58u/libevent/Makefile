#***********************************************************************
#
#  Copyright (c) 2012  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

all dynamic install: conditional_build

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ALLOWED_INCLUDE_PATHS := -I .\
			 -I$(BCM_FSBUILD_DIR)/public/include

ALLOWED_LIB_DIRS := /lib:/public/lib

LDFLAGS := $(BCM_LD_FLAGS) $(BCM_LIB_PATH)
PTHREAD_LIBS := -lpthread
export CFLAGS LDFLAGS PTHREAD_LIBS

UNTAR_COMPLETE_FILE = .untar_complete
CONFIG_COMPLETE_FILE = config_complete

APP = libevent-2.1.12-stable

ifneq ($(strip $(BUILD_LIBEVENT)),)

$(APP)/$(UNTAR_COMPLETE_FILE): $(APP).tar.gz Makefile
	rm -rf $(APP)
	(tar xkf $(APP).tar.gz 2> /dev/null || true);
	patch -p1 -b -N -s -d $(APP) <$(APP).patch;
	@echo "$(APP) is untarred"
	touch $(APP)/$(UNTAR_COMPLETE_FILE)

$(APP)/$(CONFIG_COMPLETE_FILE): $(APP)/$(UNTAR_COMPLETE_FILE) 
	(cd $(APP); ./configure \
	    --host=$(TOOLCHAIN_PREFIX) \
	    --enable-openssl=yes --enable-shared=yes \
	    --prefix=$(BCM_FSBUILD_DIR)/public \
	    --sbindir=$(BCM_FSBUILD_DIR)/public/bin \
	    --libdir=$(BCM_FSBUILD_DIR)/public/lib \
	    || exit 1);
	@echo "$(APP) is configured"
	touch $(APP)/$(CONFIG_COMPLETE_FILE)

conditional_build: $(APP)/$(CONFIG_COMPLETE_FILE) sanity_check
	$(MAKE) -C $(APP)
	$(MAKE) -C $(APP) install
	cp $(BCM_FSBUILD_DIR)/public/lib/libevent_* $(INSTALL_DIR)/lib/
	cp $(BCM_FSBUILD_DIR)/public/lib/libevent-2.1.so.7 $(INSTALL_DIR)/lib/
	echo "Done building $(APP)"
else
conditional_build: sanity_check
	@echo "skipping $(APP) (not configured)"
endif

# NOTE: make clean from within app does not do a proper job, so wiping out
# entire directory to ensure consistency.
clean: 
	rm -f $(INSTALL_DIR)/lib/libevent-2.1.so.7
	rm -f $(INSTALL_DIR)/lib/libevent-2.1.so.7.0.1
	@if [ -e $(APP)/Makefile ]; then \
		$(MAKE) -C $(APP) uninstall; \
		$(MAKE) -C $(APP) distclean; \
	fi
	rm -rf $(APP)

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean

shell:
	bash

