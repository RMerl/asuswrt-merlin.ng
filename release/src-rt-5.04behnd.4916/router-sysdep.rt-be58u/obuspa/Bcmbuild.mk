EXE := obuspa

all install: conditional_build


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common


ARCH=$(PROFILE_ARCH)
EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/bin
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib

ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(INC_BRCMSHARED_PUB_PATH)/bcm963xx  \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/lib/include  \
                         -I$(BUILD_DIR)/userspace/public/include/linux  \
                         -I$(BUILD_DIR)/userspace/private/include

HEADER_INSTALL_DIR := $(ALLOWED_INCLUDE_PATHS) -I$(BCM_FSBUILD_DIR)/public/include -I$(BCM_FSBUILD_DIR)/public/lib/include -I$(BCM_FSBUILD_DIR)/private/include

ALLOWED_LIB_DIRS      := /lib:/private/lib:/public/lib:/lib/public
LIB_RPATH        = $(BCM_FSBUILD_DIR)$(subst :,:$(BCM_FSBUILD_DIR),$(ALLOWED_LIB_DIRS))

zlib_CFLAGS = -I$(LIB_INSTALL_DIR) -I$(LIB_INSTALL_DIR)/lib
zlib_LIBS = -L$(LIB_INSTALL_DIR) -L$(LIB_INSTALL_DIR)/lib -lz

export ARCH CFLAGS BCM_LD_FLAGS BCM_FSBUILD_DIR BCM_RPATH_LINK_OPTION BCM_LIB_PATH
export EXE_INSTALL_DIR LIB_RPATH HEADER_INSTALL_DIR CMS_COMMON_LIBS EXE
export DESKTOP_LINUX BUILD_LIBCURL_WITH_SSL BUILD_LIBCURL_WITH_HTTP2
export BUILD_WEB_SOCKETS BUILD_BRCM_OPENWRT
export zlib_CFLAGS zlib_LIBS

export LINUX_VER_STR TOOLCHAIN_PREFIX
export PKG_CONFIG=$(TOOLCHAIN_TOP)/$(TOOLCHAIN_USR_DIR)/bin/pkg-config
export PKG_CONFIG_LIBDIR=$(LIB_INSTALL_DIR)
export PKG_CONFIG_PATH=$(LIB_INSTALL_DIR)/pkgconfig

# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))

export MDM_CORE_LIBS


# Final location of EXE for system image.  Only the BRCM build system needs to
# know about this.
FINAL_EXE_INSTALL_DIR := $(INSTALL_DIR)/bin

ifeq ($(strip $(DESKTOP_LINUX)),y)
conditional_build:
	@echo "skipping $(EXE) (not supported in desktop build)"
else
ifneq ($(strip $(BUILD_OBUSPA)),)
conditional_build:
	$(MAKE) -f Makefile install
	mkdir -p $(FINAL_EXE_INSTALL_DIR)
	cp -p $(EXE_INSTALL_DIR)/$(EXE) $(FINAL_EXE_INSTALL_DIR)
else
conditional_build:
	@echo "skipping $(EXE) (not configured)"
endif
endif

clean:
	rm -f $(FINAL_EXE_INSTALL_DIR)/$(EXE)
	-$(MAKE) -f Makefile clean

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean

shell:
	bash -i
