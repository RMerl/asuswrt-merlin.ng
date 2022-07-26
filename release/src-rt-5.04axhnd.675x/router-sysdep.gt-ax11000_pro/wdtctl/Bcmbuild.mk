all install default: conditional_build


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common


ARCH=$(PROFILE_ARCH)
PREFIX=$(INSTALL_DIR)
ALLOWED_INCLUDE_PATHS  := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR)


export ARCH PREFIX CFLAGS 


ifneq ($(strip $(BUILD_FW_UPGRADE_WDT)),)
WDTD_FILE := wdtd-fwupgrade.sh
else
WDTD_FILE := wdtd-basic.sh
endif


ifneq ($(strip $(BUILD_WDTCTL)),)

conditional_build:
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	install -m 755 scripts/$(WDTD_FILE) $(INSTALL_DIR)/etc/init.d/wdtd.sh
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f S26wdtd; ln -s ../init.d/wdtd.sh S26wdtd)

else

conditional_build:
	@echo "Skipping $(EXE) (not configured)"

endif


clean:
	rm -f $(INSTALL_DIR)/etc/rc3.d/S26wdtd
	rm -f $(INSTALL_DIR)/etc/init.d/wdtd.sh
	@if [ -d objs ]; then \
		$(MAKE) -C objs -f ../Makefile clean; \
		rm -rf objs; \
	fi;


shell:
	bash -i

