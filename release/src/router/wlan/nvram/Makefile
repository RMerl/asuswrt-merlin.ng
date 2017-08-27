#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
WLCSM_DEBUG :=  0
DBG_OBJS = wlcsmdbg.o wldbg.o wltest.o
KERNEL_NVRAM_FILE_NAME := kernel_nvram.setting

include $(BUILD_DIR)/make.common

ifneq ("$(wildcard $(KERNEL_DIR)/.config)","")
include $(KERNEL_DIR)/.config
endif
ifneq ($(strip $(BUILD_BRCM_CMS)),)
LIBS = -lcms_msg $(CMS_COMMON_LIBS) -ldl -lnvram
else
LIBS = -ldl -lnvram
endif
LIBS += -L$(TOP)/shared -lshared

ifneq ($(strip $(BUILD_BRCM_HNDROUTER)),)
CFLAGS += -DBCA_HNDROUTER
endif

ifneq ($(strip $(BUILD_UNIFIED_WLMNGR)),)
CFLAGS +=-DSUPPORT_UNIFIED_WLMNGR
endif

ifneq ($(strip $(CONFIG_MTD_NAND)),)
CFLAGS += -DNAND_SYS
endif
IMPL=$(shell readlink $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP) | sed 's/[^0-9]//g')
ifeq ($(IMPL),)
# the symbolic link of bcm9$(BRCM_CHIP) has been removed/cleaned, set IMPL to an integer to avoid warning message
IMPL=0
endif
WL_SRC_BASE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo /main/src; fi ;)
WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
COMM_INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/common/include

#
# Private apps and libs are allowed to include header files from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/apps/wlan/wlmngr

# obviously, this app needs to access kernel headers
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/broadcom/include/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
                         -I$(INC_WIRELESS_DRIVER_PATH) \
                         -I$(COMM_INC_WIRELESS_DRIVER_PATH) \
                         -I$(WIRELESS_DRIVER_PATH)/bcmcrypto \
                         -I$(WIRELESS_DRIVER_PATH)/shared \
                         -I$(WIRELESS_DRIVER_PATH)/router/shared

CFLAGS += -DDSLCPE_WLCSM_EXT -DKERNEL_NVRAM_FILE_NAME='"/data/.$(KERNEL_NVRAM_FILE_NAME)"'
ALLOWED_INCLUDE_PATHS += -I$(TOP)/wlcsm/include 
WLCSM_LIB := -L$(INSTALL_DIR)/lib -lwlcsm
LIBS += -lwlcsm
ifneq ($(findstring CONFIG_DUMP_PREV_OOPS_MSG=, $(shell cat $(LINUXDIR)/.config)),)
CFLAGS += -DDUMP_PREV_OOPS_MSG
endif
#
# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private


%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

ifeq ($(strip $(WLCSM_DEBUG)),1)
OBJS = nvram_shared.o main.o brcm_generic_lock.o wlcsmdbg.o wldbg.o wltest.o
else
OBJS = nvram_shared.o main.o brcm_generic_lock.o
endif


ifeq ($(strip $(WLCSM_DEBUG)),1)
all:  libnvram nvram  wlcsmdbg wldbg wltest $(KERNEL_NVRAM_FILE_NAME)
else
ifneq ($(PREBUILT_BCMBIN),1)
all:  libnvram nvram 
else
all:
endif
endif
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/init.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/rc3.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/wlan
	install -m 755 nvram $(INSTALL_DIR)/bin	
	install -m 755 libnvram.so $(INSTALL_DIR)/lib
ifneq ($(strip $(BCA_HNDROUTER)),)
	install -m 755 ./scripts/hndnvram.sh $(INSTALL_DIR)/rom/rom/etc/init.d
	sed -e s/KERNEL_NVRAM_FILE_NAME/$(KERNEL_NVRAM_FILE_NAME)/g ./scripts/hndnvram.sh > $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S40hndnvram; ln -s ../init.d/hndnvram.sh S40hndnvram)
	install -m 755 ./scripts/hndmfg.sh $(INSTALL_DIR)/rom/rom/etc/init.d
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S05hndmfg; ln -s ../init.d/hndmfg.sh S05hndmfg)
else
	install -m 755 kernel_nvram.setting $(INSTALL_DIR)/rom/rom/etc/wlan
	sed -e s/KERNEL_NVRAM_FILE_NAME/$(KERNEL_NVRAM_FILE_NAME)/g ./scripts/wlnvram.sh > $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh.1
	sed -e s/LINUX_VER_STR/$(LINUX_VER_STR)/g  $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh.1 > $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh
	chmod +x  $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh
	rm -rf $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh.1
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S40wlnvram_mount_fs; ln -s ../init.d/wlnvram.sh S40wlnvram_mount_fs)
endif

install:
	@echo done

dynamic: all

libnvram: nvram_shared.o brcm_generic_lock.o
	$(LD) -shared -soname libnvram.so -o libnvram.so nvram_shared.o brcm_generic_lock.o  -L$(LIBDIR) $(WLCSM_LIB) -lc 
	install -m 755 libnvram.so $(INSTALL_DIR)/lib

nvram: libnvram main.o
ifneq ($(strip $(BUILD_BRCM_CMS)),)
	$(CC) -o $@ main.o -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s $(LIBS)
else
	$(CC) -o $@ main.o -s -Os -fomit-frame-pointer -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s $(LIBS) $(WLCSM_LIB)
endif

clean:
	-rm -f libnvram.so nvram $(OBJS)

ifeq ($(strip $(WLCSM_DEBUG)),1)

wlcsmdbg:  wlcsmdbg.o
	$(CC) -o $@ wlcsmdbg.o -fomit-frame-pointer -L$(INSTALL_DIR)/lib -lwlcsm -lc -lgcc_s 
	install -m 755 wlcsmdbg $(INSTALL_DIR)/bin

wldbg:  wldbg.o
	$(CC) -o $@ wldbg.o -fomit-frame-pointer -L$(INSTALL_DIR)/lib -lwlcsm -lc -lgcc_s 
	install -m 755 wldbg $(INSTALL_DIR)/bin

wltest:  wltest.o
	$(CC) -o $@ wltest.o -fomit-frame-pointer -L$(INSTALL_DIR)/lib -lwlcsm -lc -lgcc_s 
	install -m 755 wltest $(INSTALL_DIR)/bin
endif

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

#-include $(OBJS:.o=.d)
