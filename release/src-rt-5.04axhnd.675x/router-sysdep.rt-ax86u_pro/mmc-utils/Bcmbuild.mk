default: conditional_build 


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

ifneq ($(strip $(BRCM_DRIVER_EMMC)),)
conditional_build: all
else
conditional_build:
	@echo "BRCM_DRIVER_EMMC not configured, skipping mmc-utils"
endif

ARCH=$(PROFILE_ARCH)
PREFIX=$(INSTALL_DIR)
ALLOWED_INCLUDE_PATHS  := -I.\
                          -I$(INC_BCMDRIVER_PATH)/include/ \
			  -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                          -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                          -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                          -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)


export ARCH PREFIX CFLAGS 


all: 
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install

clean:
	@if [ -d objs ]; then \
		$(MAKE) -C objs -f ../Makefile clean; \
		rm -rf objs; \
	fi;

CONSUMER_RELEASE_BINARYONLY_PREPARE: binaryonly_prepare

binaryonly_prepare:
	mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile binaryonly_prepare


shell:
	bash -i

