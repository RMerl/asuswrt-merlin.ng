#***********************************************************************
#
#  Copyright (c) 2012  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

all dynamic : conditional_build

install:
	@echo done
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
export BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

APP = busybox-1.24.1


.PHONY: check_untar_patch_configure conditional_build check_versions

ifneq ($(strip $(BUILD_BUSYBOX)),)
conditional_build: $(APP)

CMS_BUILD=0
ifneq ($(strip $(BUILD_BRCM_CMS)),)
CMS_BUILD=1
endif


# Depend on tar file
# Use $(APP)/Makefile to mark tar progress is completed
$(APP)/.config:
	if [ ! -e $(APP) ]; then \
	    mkdir $(APP); cd $(APP); \
	    tar --strip-components=1 -xkf ../$(APP).tar.bz2; \
	    cd ..; \
	if [ $(CMS_BUILD) -eq 1 ]; then \
		echo "*********************Applying patches to $(APP)"; \
		patch -p1 -b -s -d$(APP) < $(APP).patch; \
	fi; \
	fi;
	@echo "$(APP) is untarred"

# Depend on $(APP)/configure so that tar progress should be done only once
$(APP)/Makefile: $(APP)/.config
	@echo "$(APP) is configured"

# Depend on $(APP)/Makefile so that configure progress should be done only once
check_config: $(APP)/Makefile

# Depend on check_config so that make progress can continue where it is stopped before
all $(APP): check_config
	for i in config.d/[0-9]*.conf ; do echo "config script $$i" ; perl $$i $(APP)/.config; done;
	$(MAKE) -C $(APP) include/autoconf.h
	cd $(APP) && rm -f Config.h && ln -sf include/autoconf.h Config.h && touch Config.h
	$(MAKE) -C $(APP) install
	@echo "Done building $(APP)"
	
else
conditional_build: sanity_check
	@echo "skipping $(APP) (not configured)"
endif


# NOTE: make clean from within app does not do a proper job, so wiping out
# entire directory to ensure consistency.
clean:
	$(MAKE) -C busybox-1.24.1 clean

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean

bcm_dorel_distclean: distclean

shell:
	bash -i
