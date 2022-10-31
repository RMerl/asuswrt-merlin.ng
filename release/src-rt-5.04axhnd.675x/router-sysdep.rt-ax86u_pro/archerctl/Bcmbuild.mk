EXE    := archerctl
EXEALT := archer

all install: conditional_build


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common


ARCH                  := $(PROFILE_ARCH)
EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/bin
ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)

ALLOWED_LIB_DIRS := /lib:/lib/private:/lib/public

export ARCH CFLAGS BCM_LD_FLAGS CMS_LIB_PATH CMS_LIB_RPATH EXE_INSTALL_DIR


# Final location of EXE for system image.  Only the BRCM build system needs to
# know about this.
FINAL_EXE_INSTALL_DIR := $(INSTALL_DIR)/bin$(BCM_INSTALL_SUFFIX_DIR)

ifneq ($(strip $(BUILD_ARCHERCTL)),)

conditional_build:
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_EXE_INSTALL_DIR)
	cp -p $(EXE_INSTALL_DIR)/$(EXE) $(FINAL_EXE_INSTALL_DIR)
	(cd $(FINAL_EXE_INSTALL_DIR); ln -sf $(EXE) $(EXEALT))

else

conditional_build:
	@echo "Skipping $(EXE) (not configured)"

endif


clean: clean_legacy
	rm -f $(FINAL_EXE_INSTALL_DIR)/$(EXE) $(FINAL_EXE_INSTALL_DIR)/$(EXEALT)
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs

# delete objects left over from old Makefile. (Not needed for new directory
# which started with split makefiles.)
clean_legacy:
	rm -f *.o *.d $(EXE)


CONSUMER_RELEASE_BINARYONLY_PREPARE: binaryonly_prepare

binaryonly_prepare:
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile binaryonly_prepare
	rm -rf objs


shell:
	@echo "Entering makefile debug shell (type exit to exit) >>>"
	@bash -i
	@echo "exiting debug shell."
