LIB := libsys_util.so

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
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include
ALLOWED_INCLUDE_PATHS := -I . \
                         -I $(BCM_FSBUILD_DIR)/include \
                         -I $(BCM_FSBUILD_DIR)/public/include

export ARCH CFLAGS LIB_INSTALL_DIR HEADER_INSTALL_DIR


# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(EXT_DEVICEFS_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

all install:
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_LIB_INSTALL_DIR)
	cp -p $(LIB_INSTALL_DIR)/$(LIB) $(FINAL_LIB_INSTALL_DIR)


clean:
	rm -f $(FINAL_LIB_INSTALL_DIR)/$(LIB)
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs


shell:
	bash -i
