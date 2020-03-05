#
# In most cases, you only need to modify this first section.
#

all: build

install:
	@echo done

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

CROSS := $(CROSS_COMPILE)
export CROSS

APP := mtd-utils-2.0.2
APP_TARBALL := mtd-utils-2.0.2+43369d4.tar.bz2

CFLAGS += -Wno-error=unused-parameter

$(APP)/configure: $(APP_TARBALL)
	tar xkjf $(APP_TARBALL) 2> /dev/null || true
	@echo "$(APP) is untarred"

$(APP)/Makefile: $(APP)/configure
	cd $(APP);\
	./configure \
		--without-ubifs \
		--without-lzo \
		--without-jffs \
		--disable-tests \
		--prefix $(CURR_DIR)/$(APP)/opt \
		--host $(TOOLCHAIN_PREFIX)

ifeq ($(strip $(BUILD_MTDUTILS)),y)
build: $(APP)/Makefile
	$(MAKE) -C $(APP)
	$(MAKE) -C $(APP) install
	cp `find $(APP)/opt/ -type f -perm -700` $(INSTALL_DIR)/bin/
else
build:
	@echo "Skipping nand utils  (not configured)"
endif

clean: 
	rm -rf $(APP)/opt
	-[ ! -e $(APP)/Makefile ] || $(MAKE) -C $(APP) distclean

# Shell target permits creating an interacive shell for debugging
shell:
	bash -i



