EXE := bcm_boot_launcher

default: all


ifeq ($(BCM_MODULAR_BUILD),)
# Old way: infer location of make.common based on pwd.
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common
else
# New Modular Build way: EXT_BUILD_DIR must be set.
# Also point BUILD_DIR to EXT_BUILD_DIR
BUILD_DIR := $(EXT_BUILD_DIR)
include $(EXT_BUILD_DIR)/make.common
endif


ARCH=$(PROFILE_ARCH)
# Install the EXE directly to EXT_DEVICEFS_DIR/INSTALL_DIR, no need to
# install it in EXT_BUILD_DIR/FSBUILD_DIR first and then copy over.
EXE_INSTALL_DIR       := $(EXT_DEVICEFS_DIR)/bin
ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(BCM_FSBUILD_DIR)/include \
                         -I$(BCM_FSBUILD_DIR)/public/include
ALLOWED_LIB_DIRS      := /lib:/public/lib

export ARCH CFLAGS BCM_LD_FLAGS CMS_LIB_PATH CMS_OPTION_RPATH CMS_LIB_RPATH EXE_INSTALL_DIR


all install:
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install


clean:
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs


shell:
	bash -i
