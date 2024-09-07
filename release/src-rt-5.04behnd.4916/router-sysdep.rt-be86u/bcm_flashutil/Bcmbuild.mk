LIB := libbcm_flashutil.so

default: all


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


ARCH=$(PROFILE_ARCH)
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include
ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(BCM_FSBUILD_DIR)/include \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BCM_FSBUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
                         -I$(BCM_FSBUILD_DIR)/bcmdrivers/include


CFLAGS += -DCHIP_FAMILY_ID_HEX=0x$(BRCM_CHIP)

ifneq ($(strip $(BUILD_EMMC_REPART)),)
CFLAGS += -DBRCM_EMMC_REPART
endif

ifneq ($(strip $(BRCM_NO_AUTOCOMMIT_IMAGE)),)
CFLAGS += -DNO_AUTOCOMMIT_IMAGE
endif

ifeq ($(strip $(CFE_DDR_TYPE_CHECK)),y)
CFLAGS += -DDDR_TYPE_CHECK
endif

ifeq ($(strip $(DISABLE_NOR_RAW_PARTITION)),y)
CFLAGS += -DDISABLE_NOR_RAW_PARTITION
endif

ifeq ($(strip $(BRCM_FLASH_BUILD_NOR)),y)
CFLAGS += -DBCM_FLASH_SPINOR
endif

ifeq ($(strip $(BUILD_VFBIO_IMG)),y)
export BUILD_VFBIO_IMG
CFLAGS += -DBCM_FLASH_VFBIO
endif

# Special target just for installing pkgtb keys.  Does not build the lib.
# Maybe this target should be moved to where the keys are generated...in $(PKGTB_KEY_DIR) ?
ifeq ($(strip $(BRCM_CHECK_PKGTB_SIG)),y)
CFLAGS += -DCHECK_PKGTB_SIG

all: pkg_keys

pkg_keys:
	mkdir -p $(EXT_DEVICEFS_DIR)/etc/authorized_pkg_keys
	cp $(BUILD_DIR)/$(PKGTB_KEY_DIR)/authorized_pkg_keys/*.pem $(EXT_DEVICEFS_DIR)/etc/authorized_pkg_keys/

endif #BRCM_CHECK_PKGTB_SIG


ifeq ($(strip $(DESKTOP_LINUX)),)
# Do not define this flag in DESKTOP_LINUX mode, code does not support it.
ifneq ($(strip $(BRCM_INCREMENTAL_IMAGE_LOAD)),)
CFLAGS += -DSUPPORT_INCREMENTAL_FLASHING
endif
endif

export ARCH CFLAGS LIB_INSTALL_DIR HEADER_INSTALL_DIR \
       BRCM_NO_AUTOCOMMIT_IMAGE


# Final location of LIB for system image.  Only the BRCM build system needs to
# know about this.
FINAL_LIB_INSTALL_DIR := $(EXT_DEVICEFS_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
FINAL_ETC_INSTALL_DIR := $(EXT_DEVICEFS_DIR)/etc
export FINAL_ETC_INSTALL_DIR


all install:
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_LIB_INSTALL_DIR)
	cp -upf $(LIB_INSTALL_DIR)/$(LIB) $(FINAL_LIB_INSTALL_DIR)
ifneq ($(strip $(BRCM_NO_AUTOCOMMIT_IMAGE)),)
	@echo "INSTALLING COMMIT AFTER REBOOT SCRIPT (FINAL)"
	mkdir -p $(FINAL_ETC_INSTALL_DIR)/init.d
	mkdir -p $(FINAL_ETC_INSTALL_DIR)/rc3.d
	install -p -m 0755 -t $(FINAL_ETC_INSTALL_DIR)/init.d $(ETC_INSTALL_DIR)/init.d/commit.sh
	(cd $(FINAL_ETC_INSTALL_DIR)/rc3.d; rm -f S68commit; ln -s ../init.d/commit.sh S68commit)
endif


clean:
	rm -f $(FINAL_LIB_INSTALL_DIR)/$(LIB)
	rm -f $(FINAL_ETC_INSTALL_DIR)/init.d/commit.sh
	rm -f $(FINAL_ETC_INSTALL_DIR)/rc3.d/S68commit
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs


shell:
	bash -i
