
TOOL := pciutils
VER := 3.5.6
TOOL_VER := $(TOOL)-$(VER)
SRC_DIR := $(TOOL)-$(VER)

$(TOOL): conditional_build 

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

export LINUX_VER_STR TOOLCHAIN_PREFIX
export CFLAGS LDFLAGS
CFLAGS=-I$(BCM_FSBUILD_DIR)/public/include -I$(TOOLCHAIN_INCLUDE_DIR) $(TOOLCHAIN_INCLUDE_DIR) -I$(CURR_DIR)/$(SRC_DIR)
# -DNDEBUG=1 -O0
LDFLAGS=-L$(BCM_FSBUILD_DIR)/public/lib 



ifneq ($(strip $(BUILD_PCIUTILS)),)
conditional_build: all
else
conditional_build:
	@echo "skipping $(TOOL) (not configured)"
endif


check_config: $(TOOL_VER).tar.gz
	if [ ! -e $(SRC_DIR)/Makefile ]; then \
	(tar xkfz $(TOOL_VER).tar.gz 2> /dev/null || true); cd $(TOOL_VER); patch -p1 < ../patch.pciutils_cross; cd ..;  echo "$(TOOL) is untarred"; \
	fi

all: check_config
	$(MAKE) -C $(SRC_DIR) CROSS_COMPILE=$(CROSS_COMPILE) HOST=$(TOOLCHAIN_PREFIX) PREFIX=/etc/pci DNS=no ZLIB=no SHARED=no LIBKMOD=no HWDB=no all
	mkdir -p $(INSTALL_DIR)/bin
	install -m 755 ./$(SRC_DIR)/lspci ./$(SRC_DIR)/setpci $(INSTALL_DIR)/bin
#	mkdir -p $(INSTALL_DIR)/etc/pci/share
#	install -m 755 ./$(SRC_DIR)/pci.ids $(INSTALL_DIR)/etc/pci/share

clean:
	rm -rf $(INSTALL_DIR)/etc/pci
	rm -f $(INSTALL_DIR)/bin/lspci $(INSTALL_DIR)/bin/setpci
	rm -f $(INSTALL_DIR)/*/bin/lspci $(INSTALL_DIR)/*/bin/setpci
	rm -rf ./$(SRC_DIR)

bcm_dorel_distclean: distclean

distclean: clean

shell:
	bash -i

