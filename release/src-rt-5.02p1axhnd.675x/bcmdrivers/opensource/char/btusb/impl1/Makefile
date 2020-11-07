
# Specify Include folders
EXTRA_CFLAGS += -I$(SUBDIRS)
EXTRA_CFLAGS += -I$(SUBDIRS)/inc
EXTRA_CFLAGS += -I$(SUBDIRS)/src
EXTRA_CFLAGS += -I$(SUBDIRS)/src/gki
EXTRA_CFLAGS += -DEXPORT_SYMTAB

LITE ?= FALSE
ifeq ($(strip $(LITE)),TRUE)
EXTRA_CFLAGS += -DBUILDCFG
EXTRA_CFLAGS += -DBTUSB_LITE
EXTRA_CFLAGS += -I$(SUBDIRS)/../btpcm
EXTRA_CFLAGS += -I$(SUBDIRS)/../btsbc
COMPONENTS_PATH := $(SUBDIRS)/../../../../../Components
EXTRA_CFLAGS += -I$(COMPONENTS_PATH)/bta/include
EXTRA_CFLAGS += -I$(COMPONENTS_PATH)/stack/include
EXTRA_CFLAGS += -I$(COMPONENTS_PATH)/hcis
endif

SEC ?= FALSE
ifeq ($(strip $(SEC)),TRUE)
EXTRA_CFLAGS += -DBTUSB_LITE_SEC
EXTRA_CFLAGS += -I$(SUBDIRS)/../btsec
endif

EXTRA_CFLAGS += -DEXPORT_SYMTAB

# BTUSB_VID and BTUSB_PID can be defined to support a single BT controller type
# The following definitions are examples for 20702A1 chip
#EXTRA_CFLAGS += -DBTUSB_VID=0x0A5C
#EXTRA_CFLAGS += -DBTUSB_PID=0x22BE

usbobjs := \
        src/btusb.o \
        src/btusb_dev.o \
        src/btusb_isoc.o \
        src/btusb_version.o

ifeq ($(CONFIG_PROC_FS),y)
usbobjs += \
        src/btusb_proc.o
endif

ifeq ($(strip $(LITE)),TRUE)
liteobjs := src/btusb_lite.o \
            src/btusb_lite_av.o \
            src/btusb_lite_avdt.o \
            src/btusb_lite_l2c.o \
            src/btusb_lite_hci.o
endif

gkiobjs := \
        src/gki/gki_buffer.o \
        src/gki/gki_klinux.o

obj-m += btusb.o
btusb-objs := $(usbobjs) $(gkiobjs) $(liteobjs)

# specify Kernel build location (can be overriden at command line)
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
ifeq ($(strip $(LITE)),TRUE)
	cat ../btpcm/Module.symvers > ./Module.symvers
	cat ../btsbc/Module.symvers >> ./Module.symvers
ifeq ($(strip $(SEC)),TRUE)
	cat ../btsec/Module.symvers >> ./Module.symvers
endif
endif
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install:
	cp -v btusb.ko /lib/modules/$(uname -r)/kernel/drivers/brcm/btusb.ko

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm -f Module.markers
	rm -f Module.symvers
	rm -f modules.order
	rm -f ./src/*.o
	rm -f ./src/.*.o.cmd
	rm -f ./src/gki/*.o
	rm -f ./src/gki/.*.o.cmd

# define the location of the Linux stable repository
PROJECTS_DIR ?= ~/projects
LINUX_STABLE = $(PROJECTS_DIR)/linux-stable

# rule to run a regression on a specific file
v2.% v3.%:
	@echo Running regression on: $@
	# clean the previous version before switching version
	cd $(LINUX_STABLE); make mrproper >/dev/null
	# switch version
	cd $(LINUX_STABLE); git checkout $@ >/dev/null
	# prepare to build modules
	cd $(LINUX_STABLE); make defconfig >/dev/null
	cd $(LINUX_STABLE); make modules_prepare >/dev/null
	make KDIR=$(LINUX_STABLE)

regression-install:
	cd $(PROJECTS_DIR)
	git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git

regression-update:
	cd $(LINUX_STABLE)
	git pull

regression: v2.6.28 v2.6.35 v2.6.36 v3.2.1 v3.12.1
	@echo Regression completed successfully!

.PHONY: help regression regression-update regression-install clean install

help:
	@echo "BTUSB module configurations and targets:"
	@echo "  LITE=FALSE         : No StackLite (Default)"
	@echo "  LITE=TRUE          : StackLite compiled"
	@echo "  SEC=FALSE          : No SEC Encoder Support(default)"
	@echo "  SEC=TRUE           : SEC Encoder Support. Requires StackLite"
	@echo "  clean              : remove the build output files"
	@echo "  install            : copy the driver in the current platform modules list"
	@echo "  regression         : run a regression on several Linux versions"



