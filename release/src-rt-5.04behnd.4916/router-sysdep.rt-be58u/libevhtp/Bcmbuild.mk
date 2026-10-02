LIB := libevhtp

all install: conditional_build

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan

ARCH=$(PROFILE_ARCH)
LIB_INSTALL_DIR := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR := $(BCM_FSBUILD_DIR)/public/include
ALLOWED_INCLUDE_PATHS := -I . \
                         -I $(BUILD_DIR)/userspace/public/include \
                         -I $(HEADER_INSTALL_DIR)
ALLOWED_LIB_DIRS := /lib:/public/lib
PREFIX := $(BCM_FSBUILD_DIR)/public
ROOT_PATH := $(BCM_FSBUILD_DIR);$(BCM_FSBUILD_DIR)/public
LDFLAGS = $(BCM_LIB_PATH) $(BCM_LD_FLAGS)

CFLAGS += -Wno-maybe-uninitialized
export CFLAGS LDFLAGS ARCH LIB_INSTALL_DIR HEADER_INSTALL_DIR
export PREFIX ROOT_PATH


# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

ifneq ($(strip $(BUILD_LIBEVHTP)),)
conditional_build:
	$(MAKE) -f Makefile install
	mkdir -p $(FINAL_LIB_INSTALL_DIR)
	cp -d $(LIB_INSTALL_DIR)/$(LIB)*.so* $(FINAL_LIB_INSTALL_DIR)
else
conditional_build:
	@echo "skipping $(LIB) (not configured)"
endif

clean:
	rm -f $(FINAL_LIB_INSTALL_DIR)/$(LIB)*.so*
	-$(MAKE) -f Makefile clean

shell:
	bash -i
