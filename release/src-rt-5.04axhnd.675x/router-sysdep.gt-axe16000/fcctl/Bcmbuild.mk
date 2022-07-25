EXE    := fcctl
EXEALT := fc

all dynamic install: conditional_build


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common



ARCH                  := $(PROFILE_ARCH)
EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/bin
ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)

ALLOWED_LIB_DIRS := /lib:/lib/private:/lib/public

export ARCH CFLAGS BCM_LD_FLAGS CMS_LIB_PATH CMS_LIB_RPATH EXE_INSTALL_DIR
# export TOOLCHAIN_PREFIX



# Final location of EXE for system image.  Only the BRCM build system needs to
# know about this.
FINAL_EXE_INSTALL_DIR := $(INSTALL_DIR)/bin


ifneq ($(strip $(BUILD_FCCTL)),)

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
	

clean:
	rm -f $(FINAL_EXE_INSTALL_DIR)/$(EXE)
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs


CONSUMER_RELEASE_BINARYONLY_PREPARE: binaryonly_prepare

binaryonly_prepare:
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile binaryonly_prepare
	rm -rf objs


shell:
	@echo "Entering makefile debug shell (type exit to exit) >>>"
	@bash -i
	@echo "exiting debug shell."
