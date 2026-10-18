LIB := libfdt.so

all install: $(LIB)


ifeq ($(BCM_MODULAR_BUILD),)
  # Old way: infer location of make.common based on pwd.
  CURR_DIR := $(shell pwd)
  BUILD_DIR:=$(HND_SRC)
  include $(BUILD_DIR)/make.common
else
  # New Modular Build way: EXT_BUILD_DIR must be set.
  # Also point BUILD_DIR to EXT_BUILD_DIR
  BUILD_DIR := $(EXT_BUILD_DIR)
  include $(EXT_BUILD_DIR)/make.common
endif

# BRCM_SUPPORTS_MULTIARCH_BUILD
ARCH=$(PROFILE_ARCH)
LOCAL_BUILD_SUBDIR    := objs/$(ARCH)
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include
ALLOWED_INCLUDE_PATHS := -I . \
                         -I $(BCM_FSBUILD_DIR)/include \
                         -I $(BCM_FSBUILD_DIR)/public/include

ALLOWED_LIB_DIRS := /lib:/public/lib


ifeq ($(strip $(DESKTOP_LINUX)),y)
EXTRA_CFLAGS := $(BCM_LD_FLAGS)
EXTRA_CFLAGS += -Wno-sign-compare
LDFLAGS := $(BCM_LD_FLAGS)
export LDFLAGS
else
EXTRA_CFLAGS := -Wno-sign-compare
endif


export LINUX_VER_STR TOOLCHAIN_PREFIX EXTRA_CFLAGS
export ARCH CFLAGS BCM_LIB_PATH
export LIB_INSTALL_DIR HEADER_INSTALL_DIR
export INSTALL
export BUILD_DTC


# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(EXT_DEVICEFS_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)


$(LIB):
	mkdir -p $(LOCAL_BUILD_SUBDIR)
	$(MAKE) -C $(LOCAL_BUILD_SUBDIR) -f ../../Makefile install
	mkdir -p  $(FINAL_LIB_INSTALL_DIR)
	cp -fav $(LIB_INSTALL_DIR)/libfdt*.so* $(FINAL_LIB_INSTALL_DIR)


clean:
	rm -f $(FINAL_LIB_INSTALL_DIR)/libfdt*.so*
	-mkdir -p $(LOCAL_BUILD_SUBDIR)
	-$(MAKE) -C $(LOCAL_BUILD_SUBDIR) -f ../../Makefile clean
	rm -rf objs


# The source code in this directory is released to everyone,
# so binaryonly_prepare is the same as clean
CONSUMER_RELEASE_BINARYONLY_PREPARE: clean


shell:
	@echo "Entering makefile debug shell (type exit to exit) >>>"
	@bash -i
	@echo "exiting debug shell."
