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

APP := mtd-utils-2.1.5
APP_TARBALL := mtd-utils-2.1.5.tar.gz

CFLAGS += -Wno-error=unused-parameter

$(APP)/configure: $(APP_TARBALL)
	@if [ ! -e ./$(APP)/Makefile ];then \
	tar zxvf $(APP_TARBALL) 2> /dev/null || true; \
	cd $(APP); \
	autoreconf --force --install; \
	patch -p1 -b -s -f < ../$(APP).patch || true; \
	fi
	@echo "$(APP) is untarred and patched!"

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
	-rm -rf $(APP)

# Shell target permits creating an interacive shell for debugging
shell:
	bash -i



