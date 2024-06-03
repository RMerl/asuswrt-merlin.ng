
default: install


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


HEADER_INSTALL_DIR := $(BCM_FSBUILD_DIR)/private/include

export HEADER_INSTALL_DIR INSTALL_HEADERS_WITH_CP


all install:
	$(MAKE) -f Makefile install

clean:
	$(MAKE) -f Makefile clean


shell:
	bash -i

