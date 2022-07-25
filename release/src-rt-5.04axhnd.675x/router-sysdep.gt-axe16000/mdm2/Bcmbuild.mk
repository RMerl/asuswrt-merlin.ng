LIB := libmdm2.so

all install: conditional_build


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

ifneq ($(strip $(BRCM_VOICE_SUPPORT)),)
include $(BUILD_DIR)/make.voice
endif


# BRCM_SUPPORTS_MULTIARCH_BUILD
ARCH                  := $(PROFILE_ARCH)
LOCAL_BUILD_SUBDIR    := objs/$(ARCH)
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/lib$(BCM_LIB_INSTALL_SUFFIX)
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/private/include
ALLOWED_INCLUDE_PATHS := -I . \
                         -I $(BCM_FSBUILD_DIR)/public/include \
                         -I $(BCM_FSBUILD_DIR)/private/include \
                         -I $(BUILD_DIR)/userspace/public/include  \
                         -I $(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I $(BUILD_DIR)/userspace/private/include  \
                         -I $(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I $(BUILD_DIR)/userspace/private/libs/cms_core \
                         -I $(BUILD_DIR)/userspace/private/libs/cms_core/linux


export ARCH CFLAGS LIB_INSTALL_DIR HEADER_INSTALL_DIR


# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(INSTALL_DIR)/lib$(BCM_LIB_INSTALL_SUFFIX)


ifeq ($(strip $(BUILD_BRCM_BASE_SHELL)),)
ifneq ($(strip $(BUILD_DM_PURE181)),)
COND_BUILD_MDM2 := 1
endif
ifneq ($(strip $(BUILD_DM_DETECT)),)
COND_BUILD_MDM2 := 1
endif
endif

ifeq ($(strip $(COND_BUILD_MDM2)),1)

conditional_build:
	mkdir -p $(LOCAL_BUILD_SUBDIR)
	$(MAKE) -C $(LOCAL_BUILD_SUBDIR) -f ../../Makefile install
	mkdir -p  $(FINAL_LIB_INSTALL_DIR)
	cp -p $(LIB_INSTALL_DIR)/$(LIB) $(FINAL_LIB_INSTALL_DIR)

else

conditional_build:
	@echo "Skipping $(LIB) (not configured)"

endif


clean:
	rm -f $(FINAL_LIB_INSTALL_DIR)/$(LIB)
	-mkdir -p $(LOCAL_BUILD_SUBDIR)
	-$(MAKE) -C $(LOCAL_BUILD_SUBDIR) -f ../../Makefile clean
	rm -rf objs


CONSUMER_RELEASE_BINARYONLY_PREPARE: delete_source

save_binaries:
ifneq ($(wildcard objs/arm),)
	-$(MAKE) -C objs/arm -f ../../Makefile ARCH=arm save_binaries
endif
ifneq ($(wildcard objs/aarch64),)
	-$(MAKE) -C objs/aarch64 -f ../../Makefile ARCH=aarch64 save_binaries
endif
ifneq ($(wildcard objs/i386),)
	-$(MAKE) -C objs/i386 -f ../../Makefile ARCH=i386 save_binaries
endif

delete_source: save_binaries
	-$(MAKE) -f Makefile delete_source
	rm -rf objs


shell:
	@echo "Entering makefile debug shell (type exit to exit) >>>"
	@bash -i
	@echo "exiting debug shell."
