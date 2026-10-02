EXE    := mcpctl
EXEALT := mcp

all install: conditional_build


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common


ARCH                  := $(PROFILE_ARCH)
EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/bin
ALLOWED_INCLUDE_PATHS := -I$(BUILD_DIR)/userspace/private/include


export ARCH CFLAGS BCM_LD_FLAGS EXE_INSTALL_DIR


# Final location of EXE for system image.  Only the BRCM build system needs to
# know about this.
FINAL_EXE_INSTALL_DIR := $(BCM_FSINSTALL_DIR)/bin


ifneq ($(strip $(BUILD_MCAST_PROXY)),)

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
	rm -f $(FINAL_EXE_INSTALL_DIR)/$(EXE)
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
