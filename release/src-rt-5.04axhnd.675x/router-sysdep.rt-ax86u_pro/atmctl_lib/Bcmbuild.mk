default: conditional_build 


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

ifneq ($(strip $(BUILD_XTMCTL)),)
conditional_build: all
else
conditional_build:
	@echo "skipping atmctl (not configured)"
endif

ARCH=$(PROFILE_ARCH)
PREFIX=$(BCM_FSBUILD_DIR)/private/
ALLOWED_INCLUDE_PATHS  := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR)

ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)


export ARCH PREFIX OALDIR CFLAGS DESKTOP_LINUX


all: 
	mkdir -p $(INSTALL_DIR)/lib/private/
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p  $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	cp -d $(BCM_FSBUILD_DIR)/private/lib/libatmctl.so* $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

clean:
	rm -f $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)/libatmctl.so*
	-mkdir -p objs/linux
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs

CONSUMER_RELEASE_BINARYONLY_PREPARE: binaryonly_prepare

binaryonly_prepare:
	mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile binaryonly_prepare


shell:
	bash -i

