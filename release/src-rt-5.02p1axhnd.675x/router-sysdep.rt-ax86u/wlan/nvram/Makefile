#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
DBG_OBJS = wlcsmdbg.o wldbg.o wltest.o
KERNEL_NVRAM_FILE_NAME := kernel_nvram.setting
USER_NVRAM_FILE_NAME := user_nvram.setting

default: all

include $(BUILD_DIR)/make.common

include $(TOP)/.config

ifneq ("$(wildcard $(KERNEL_DIR)/.config)","")
include $(KERNEL_DIR)/.config
endif
ifneq ($(strip $(BUILD_BRCM_CMS)),)
LIBS = -lcms_msg $(CMS_COMMON_LIBS) -ldl
else
LIBS = -ldl -lnvram
endif
LIBS += -L$(TOP)/shared -lshared

ifeq ($(RTCONFIG_AMAS),y)
LIBS += -L$(TOP)/json-c/.libs -ljson-c
endif

ifneq ($(strip $(WLCSM_DEBUG_TRACE)),)
WLCSM_DEBUG :=  1
else
WLCSM_DEBUG :=  0
endif

ifneq ($(strip $(BUILD_BRCM_HNDROUTER_ALONE)),)
CFLAGS += -DBCA_HNDROUTER
endif

ifneq ($(strip $(BUILD_UNIFIED_WLMNGR)),)
CFLAGS +=-DSUPPORT_UNIFIED_WLMNGR
endif

ifneq ($(strip $(CONFIG_MTD_NAND)),)
CFLAGS += -DNAND_SYS
endif

# need to revise the macro
ifeq ($(strip $(DISABLE_NOR_RAW_PARTITION)),y)
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
ALLOWED_INCLUDE_PATHS += -I$(TOP_PLATFORM)/wlcsm/include
WLCSM_LIB := $(TOP_PLATFORM)/wlcsm/libwlcsm.a
SHARED_LIB := -L$(TOP)/shared -lshared
LIBS += $(WLCSM_LIB)
ifneq ($(findstring CONFIG_DUMP_PREV_OOPS_MSG=, $(shell cat $(LINUXDIR)/.config)),)
CFLAGS += -DDUMP_PREV_OOPS_MSG
endif
LIBDIR = $(INSTALL_DIR)/lib
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
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
ifneq ($(PREBUILT_BCMBIN),1)
all:  libnvram nvram 
else
all:
endif
else
all:
endif
endif
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/init.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/rc3.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/wlan
	mkdir -p $(INSTALL_DIR)/usr/sbin
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
	install -m 755 nvram $(INSTALL_DIR)/bin	
	install -m 755 libnvram.so $(INSTALL_DIR)/lib
endif
	install -m 755 ./scripts/hndnvram.sh $(INSTALL_DIR)/rom/rom/etc/init.d
	install -m 755 ./scripts/hnddm.sh $(INSTALL_DIR)/usr/sbin
	install -m 755 ./scripts/wifi.sh $(INSTALL_DIR)/rom/rom/etc/init.d
	sed -e s/KERNEL_NVRAM_FILE_NAME/$(KERNEL_NVRAM_FILE_NAME)/g ./scripts/hndnvram.sh > $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)_$(BUILD_BRCM_CMS)),y_)
# fill in the content of "user_nvram_file" in hndnvram.sh for UNFWLCFG+NOCMS builds
	sed -i s/user_nvram_file=\"\"/user_nvram_file=\"\\/data\\/.$(USER_NVRAM_FILE_NAME)\"/g $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
endif
ifneq ($(strip $(BUILD_BRCM_HNDROUTER_ALONE)),)
	sed -i s/HNDROUTER_ALONE=n/HNDROUTER_ALONE=y/g $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
endif
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S40hndnvram; ln -s ../init.d/hndnvram.sh S40hndnvram)
	install -m 755 ./scripts/hndmfg.sh $(INSTALL_DIR)/rom/rom/etc/init.d
	sed -e s/KERNEL_NVRAM_FILE_NAME/$(KERNEL_NVRAM_FILE_NAME)/g ./scripts/hndmfg.sh > $(INSTALL_DIR)/rom/rom/etc/init.d/hndmfg.sh
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S39hndmfg; ln -s ../init.d/hndmfg.sh S39hndmfg)
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
	install -m 755 kernel_nvram.setting $(INSTALL_DIR)/rom/rom/etc/wlan
else
# give empty content as original kernel_nvram.setting
	touch $(INSTALL_DIR)/rom/rom/etc/wlan/$(KERNEL_NVRAM_FILE_NAME)
endif

ifneq ($(strip $(CONFIG_BCM96846)),)
# dhda_rnr_offloverride - Override offload support setting, bit[1:0] with each bit position corresponding to a radio
#   possible values
#   - 0 (no offload), 1 (offload dhd0), 2 (offload dhd1), 3 (offload dhd0)
	echo "dhd_rnr_offload_override=1" >> $(INSTALL_DIR)/etc/wlan/kernel_nvram.setting
endif
ifeq ($(strip $(BCA_HNDROUTER)),)
	sed -e s/KERNEL_NVRAM_FILE_NAME/$(KERNEL_NVRAM_FILE_NAME)/g ./scripts/wlnvram.sh > $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh.1
	sed -e s/LINUX_VER_STR/$(LINUX_VER_STR)/g  $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh.1 > $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh
	chmod +x  $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh
	rm -rf $(INSTALL_DIR)/rom/rom/etc/init.d/wlnvram.sh.1
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S42wlnvram_mount_fs; ln -s ../init.d/wlnvram.sh S42wlnvram_mount_fs)
endif
ifneq ($(strip $(BRCM_NVRAM_INCLUDE_IMAGE)),)
	mkdir -p $(INSTALL_DIR)/etc/wlan/nvram/
	-cp -f $(BUILD_DIR)/targets/nvram/$(BRCM_CHIP)/*.nvm $(INSTALL_DIR)/etc/wlan/nvram/
endif

install:
	@echo done

dynamic: all

libnvram: nvram_shared.o brcm_generic_lock.o $(WLCSM_LIB)
	$(LD) -shared -soname libnvram.so -o libnvram.so nvram_shared.o brcm_generic_lock.o  -L$(LIBDIR) $(WLCSM_LIB) -lc -lpthread
	install -m 755 libnvram.so $(INSTALL_DIR)/lib

nvram: libnvram main.o $(WLCSM_LIB)
ifneq ($(strip $(BUILD_BRCM_CMS)),)
	$(CC) -o $@ main.o -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s -lpthread $(LIBS)
else
	$(CC) -o $@ main.o -s -Os -fomit-frame-pointer -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s -lpthread $(LIBS) $(WLCSM_LIB)
endif

clean:
	-rm -f *.o
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
