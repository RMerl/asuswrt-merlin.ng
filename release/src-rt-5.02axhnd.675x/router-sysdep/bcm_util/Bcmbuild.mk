LIB := libbcm_util.so

default: all


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common


ARCH=$(PROFILE_ARCH)
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include
ALLOWED_INCLUDE_PATHS := -I . \
                         -I $(BUILD_DIR)/userspace/public/include \
                         -I $(HEADER_INSTALL_DIR)

export ARCH CFLAGS LIB_INSTALL_DIR HEADER_INSTALL_DIR


# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

all install: clean_legacy
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_LIB_INSTALL_DIR)
	cp -p $(LIB_INSTALL_DIR)/$(LIB) $(FINAL_LIB_INSTALL_DIR)


clean: clean_legacy
	rm -f $(FINAL_LIB_INSTALL_DIR)/$(LIB)
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs

# delete objects left over from old Makefile. (Not needed for new directory
# which started with dual makefiles.)
clean_legacy:
	rm -f *.o *.d $(LIB)


shell:
	bash -i
