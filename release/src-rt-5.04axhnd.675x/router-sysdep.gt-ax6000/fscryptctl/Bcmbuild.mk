
dynamic install: conditional_build

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

APP = fscryptctl

EXE_INSTALL_DIR := $(INSTALL_DIR)/bin


.PHONY: conditional_build check_versions

ifneq ($(strip $(BRCM_BUILD_FSCRYPT)),)
conditional_build: $(APP)
else
conditional_build: 
endif

$(APP):
	$(MAKE) $(APP)
	$(STRIP) $(APP)
	mkdir -p $(EXE_INSTALL_DIR)
	cp -p $(APP) $(EXE_INSTALL_DIR)

clean:
	$(MAKE) clean

shell:
	bash -i

