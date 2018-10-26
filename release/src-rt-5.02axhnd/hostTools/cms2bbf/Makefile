#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
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
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

APP        := cms2bbf

.PHONY: check_untar_patch_configure conditional_build check_versions clean distclean

ifneq ($(strip $(BUILD_PROFILE_SUPPORTED_DATA_MODEL)),)
conditional_build: $(APP)

$(APP):
	ant -f build.xml dist;
	@echo "Done building $(APP)"

clean:
	rm -rf build;

else

conditional_build: sanity_check
	@echo "skipping $(APP) (not configured)"

clean:
	@echo "skipping $(APP) (not configured)"

endif

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean

bcm_dorel_distclean: distclean

shell:
	bash -i
