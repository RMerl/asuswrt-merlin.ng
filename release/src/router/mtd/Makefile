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

CFLAGS += -Wno-error=unused-parameter

ifeq ($(strip $(BUILD_MTDUTILS)),y)
build: 
	#tar xkzf mtd-utils-56840a1.tar.gz 2>/dev/null || true
	make -C mtd-utils-56840a1
	cp `find mtd-utils-56840a1/opt/ -type f -perm -700` $(INSTALL_DIR)/bin/
else
build:
	@echo "Skipping nand utils  (not configured)"
endif

clean: 
	-PATH=/bin:/usr/bin ; find mtd-utils-56840a1/opt/ -type f -perm -700 -execdir rm -f $(INSTALL_DIR)/bin/"{}" ";"
	-make -C mtd-utils-56840a1 clean

# Shell target permits creating an interacive shell for debugging
shell:
	bash -i


