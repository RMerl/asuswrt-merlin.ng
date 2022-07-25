EXE := bcm_boot_launcher

default: all


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

ARCH=$(PROFILE_ARCH)
EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/bin
ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(BCM_FSBUILD_DIR)/public/include
ALLOWED_LIB_DIRS      := /lib:/lib/public

export ARCH CFLAGS BCM_LD_FLAGS CMS_LIB_PATH CMS_LIB_RPATH EXE_INSTALL_DIR


# Final location of EXE for system image.  Only the BRCM build system needs to
# know about this.
FINAL_EXE_INSTALL_DIR := $(INSTALL_DIR)/bin

all install: clean_legacy
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_EXE_INSTALL_DIR)
	cp -p $(EXE_INSTALL_DIR)/$(EXE) $(FINAL_EXE_INSTALL_DIR)


clean: clean_legacy
	rm -f $(FINAL_EXE_INSTALL_DIR)/$(EXE)
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs

# delete objects left over from old Makefile. (Not needed for new directory
# which started with dual makefiles.)
clean_legacy:
	rm -f *.o *.d $(EXE)


shell:
	bash -i
