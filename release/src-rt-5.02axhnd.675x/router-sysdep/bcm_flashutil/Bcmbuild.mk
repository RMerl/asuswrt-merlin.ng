LIB := libbcm_flashutil.so

default: all


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

ARCH=$(PROFILE_ARCH)
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include
ETC_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/etc
ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(HEADER_INSTALL_DIR) \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMSHARED_PUB_PATH)/../flash  \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) 

CFLAGS += -DCHIP_FAMILY_ID_HEX=0x$(BRCM_CHIP) 

ifneq ($(strip $(BRCM_NO_AUTOCOMMIT_IMAGE)),)
CFLAGS += -DNO_AUTOCOMMIT_IMAGE
endif

ifeq ($(strip $(CFE_DDR_TYPE_CHECK)),y)
CFLAGS += -DDDR_TYPE_CHECK
endif

ifeq ($(strip $(DISABLE_NOR_RAW_PARTITION)),y)
CFLAGS += -DDISABLE_NOR_RAW_PARTITION
endif

ifeq ($(strip $(DESKTOP_LINUX)),)
# Do not define this flag in DESKTOP_LINUX mode, code does not support it.
ifneq ($(strip $(BRCM_INCREMENTAL_IMAGE_LOAD)),)
CFLAGS += -DSUPPORT_INCREMENTAL_FLASHING
endif
endif

export ARCH CFLAGS LIB_INSTALL_DIR HEADER_INSTALL_DIR ETC_INSTALL_DIR \
       BRCM_NO_AUTOCOMMIT_IMAGE


# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
FINAL_ETC_INSTALL_DIR := $(INSTALL_DIR)/etc


all install: clean_legacy
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_LIB_INSTALL_DIR)
	cp -p $(LIB_INSTALL_DIR)/$(LIB) $(FINAL_LIB_INSTALL_DIR)
ifneq ($(strip $(BRCM_NO_AUTOCOMMIT_IMAGE)),)
	@echo "INSTALLING COMMIT AFTER REBOOT SCRIPT (FINAL)"
	mkdir -p $(FINAL_ETC_INSTALL_DIR)/init.d
	mkdir -p $(FINAL_ETC_INSTALL_DIR)/rc3.d
	install -p -m 0755 -t $(FINAL_ETC_INSTALL_DIR)/init.d $(ETC_INSTALL_DIR)/init.d/commit.sh
	(cd $(FINAL_ETC_INSTALL_DIR)/rc3.d; rm -f S68commit; ln -s ../init.d/commit.sh S68commit)
endif


clean: clean_legacy
	rm -f $(FINAL_LIB_INSTALL_DIR)/$(LIB)
	rm -f $(FINAL_ETC_INSTALL_DIR)/init.d/commit.sh
	rm -f $(FINAL_ETC_INSTALL_DIR)/rc3.d/S68commit
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs

# delete objects left over from old Makefile. (Not needed for new directory
# which started with dual makefiles.)
clean_legacy:
	rm -f *.o *.d $(LIB)


shell:
	bash -i
