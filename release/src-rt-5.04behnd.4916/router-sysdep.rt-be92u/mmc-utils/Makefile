all dynamic install: conditional_build

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#

CURR_DIR := $(shell pwd)
# BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
# BUILD_DIR:=$(word 1, $(BUILD_DIR))
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

CHANGESET = b7e4d5a6ae9942d26a11de9b05ae7d52c0802802
APP = mmc-utils-$(CHANGESET)
EXE = mmc

.PHONY: check_untar_patch_configure conditional_build

ifneq ($(strip $(BRCM_DRIVER_EMMC)),$(strip $(BUILD_VFBIO_LVM)),)
check_untar_patch_configure: sanity_check
	@if [ ! -e $(APP)/Makefile ]; then \
    	    echo "Unzipping original $(APP) source"; \
	    (unzip $(APP).zip > /dev/null || true); \
	fi   

conditional_build: check_untar_patch_configure
	cd ./$(APP); $(MAKE);
	mkdir -p $(INSTALL_DIR)/bin;
	install -m 755 $(APP)/$(EXE) $(INSTALL_DIR)/bin;
else	
conditional_build: sanity_check
	@echo "skipping $(APP) (not configured)"
endif

# NOTE: make clean from within app does not do a proper job, so wiping out
# entire directory to ensure consistency.
clean:
	@rm -rf $(APP) ;
	@rm -f $(INSTALL_DIR)/bin/$(EXE) ;

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean
