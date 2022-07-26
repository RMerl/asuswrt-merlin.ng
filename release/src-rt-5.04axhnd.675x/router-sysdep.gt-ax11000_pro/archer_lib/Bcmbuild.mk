LIB = libarcher.so

default: conditional_build


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

ifneq ($(strip $(BUILD_ARCHERCTL)),)
conditional_build: all
else
conditional_build:
	@echo "Skipping $(LIB)"
endif

ARCH=$(PROFILE_ARCH)
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/lib
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(HND_SRC)/userspace/private/include  \
                         -I$(BUILD_DIR)/shared/opensource/include/bcm963xx \
                         -I$(BUILD_DIR)/shared/private/include/$(OALDIR) \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)



export ARCH CFLAGS LIB_INSTALL_DIR CFLAGS DESKTOP_LINUX BRCM_RELEASETAG

# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

all:
	mkdir -p $(INSTALL_DIR)/lib/private/
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_LIB_INSTALL_DIR)
	cp -p $(LIB_INSTALL_DIR)/$(LIB)* $(FINAL_LIB_INSTALL_DIR)

clean:
	rm -f $(FINAL_LIB_INSTALL_DIR)/$(LIB)*
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs

CONSUMER_RELEASE_BINARYONLY_PREPARE: binaryonly_prepare

binaryonly_prepare:
	mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile binaryonly_prepare


shell:
	bash -i

