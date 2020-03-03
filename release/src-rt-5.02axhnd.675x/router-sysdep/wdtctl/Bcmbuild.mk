default: conditional_build 


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

conditional_build: all

ARCH=$(PROFILE_ARCH)
PREFIX=$(INSTALL_DIR)
ALLOWED_INCLUDE_PATHS  := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR)


export ARCH PREFIX CFLAGS 


all: 
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install

clean:
	@if [ -d objs ]; then \
		$(MAKE) -C objs -f ../Makefile clean; \
		rm -rf objs; \
	fi;

CONSUMER_RELEASE_BINARYONLY_PREPARE: binaryonly_prepare

binaryonly_prepare:
	mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile binaryonly_prepare


shell:
	bash -i

