#Linux Makefile
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(word 1, $(HND_SRC))
WL_SRC_BASE := /main/src
SRCBASE=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
WLCSM_EXT_BASE=$(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/impl1
KERNEL_NVRAM_FILE_NAME := kernel_nvram.setting

default_bld:all

include $(BUILD_DIR)/make.common

ifneq ("$(wildcard $(KERNEL_DIR)/.config)","")
	include $(KERNEL_DIR)/.config
endif

ifeq ($(strip $(BUILD_BRCM_CMS)),)
BUILD_NO_CMS=y
endif
BUILD_MNGR_CHANNEL = $(BUILD_TR181_WLMNGR)

ifneq ($(strip $(CONFIG_MTD_NAND)),)
	CFLAGS += -DNAND_SYS -DKERNEL_NVRAM_FILE_NAME='"/data/.$(KERNEL_NVRAM_FILE_NAME)"'
endif

INCLUDE += -I$(WLCSM_EXT_BASE)/include -I./include -I../wlmngr
ifneq ($(strip $(HNDAPPS_IN_COMP)),)
INCLUDE += -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
else
endif
INCLUDE += -I. -I.. -I$(SRCBASE)/include -I$(SRCBASE)/common/include -I$(SRCBASE)/shared -I$(SRCBASE)/router/shared -I$(SRCBASE)/wl/exe
INCLUDE += -I$(SRCBASE)/common/include/devctrl_if
INCLUDE += -I$(SRCBASE)/security/secfrw/include -I$(SRCBASE)/supp/include -I$(SRCBASE)/shared/bcmwifi/include

INCLUDE += -I$(BUILD_DIR)/userspace/public/include/linux
INCLUDE += -I$(BUILD_DIR)/userspace/public/include

CFLAGS += $(INCLUDE)
CFLAGS += -Wall
ifneq ($(strip $(BUILD_MNGR_CHANNEL)),)
CFLAGS += -DWITH_MNGR_CHANNEL
endif

ifneq ($(strip $(BCA_HNDROUTER)),)
CFLAGS += -DBCA_HNDROUTER
endif
CFLAGS += -D$(MODEL)
ifneq ($(findstring CONFIG_DUMP_PREV_OOPS_MSG=, $(shell cat $(LINUXDIR)/.config)),)
CFLAGS += -DDUMP_PREV_OOPS_MSG
endif

DSLCPE_WLCSM_DEBUG :=0
ifeq ($(strip $(DSLCPE_WLCSM_DEBUG)),1)
CFLAGS  += -g
else
CFLAGS  += -s
endif
CFLAGS += -DDSLCPE -Werror

LDFLAGS += -L. -L$(INSTALL_DIR)/lib  

LDFLAGS += -L$(TOP)/shared -lshared

vpath %.c $(CURR_DIR)/src


WLCSM_FILES = $(foreach x, $(shell find ./src -name \*.c) ,$(notdir $(x)))

ifneq ($(strip $(BUILD_BRCM_HNDROUTER_ALONE)),)
CSM_FILES := wlcsm_lib_hspot.c wlcsm_lib_wl.c
WLCSM_FILES := $(filter-out $(CSM_FILES), $(WLCSM_FILES))
endif
WLCSM_OBJS := $(foreach x, $(WLCSM_FILES),$(x:.c=.o))

OBJS += $(WLCSM_OBJS)
STATIC_OBJS := $(filter-out wlcsm_lib_hspot.o, $(OBJS))
all: pre libwlcsm.so
	ln -sf $(BUILD_DIR)/userspace/private/libs/wlcsm/include/wlcsm_defs.h  $(BUILD_DIR)/userspace/private/include/wlcsm_defs.h  
	ln -sf $(BUILD_DIR)/userspace/private/libs/wlcsm/include/wlcsm_lib_api.h  $(BUILD_DIR)/userspace/private/include/wlcsm_lib_api.h  

ifneq ($(PREBUILT_BCMBIN),1)
ifneq ($(strip $(WLCSM_FILES)),)
libwlcsm.so:$(OBJS) $(WLCSM_EXT_BASE)/include/wlcsm_linux.h
	ln -sf $(WLCSM_EXT_BASE)/include/wlcsm_linux.h  $(CURR_DIR)/include/wlcsm_linux.h
ifeq ($(strip $(DSLCPE_WLCSM_DEBUG)),1)
	$(CC) -fPIC -shared -o libwlcsm.so $(OBJS) -lpthread 
else
	$(CC) -s -Os -fPIC -shared -o libwlcsm.so $(OBJS) -lpthread 
endif
else
libwlcsm.so: 
	cp -f libwlcsm-$(EXT_CPU_ARCH_NAME).so libwlcsm.so
endif
else
libwlcsm.so:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif
	install -m 755 libwlcsm.so $(INSTALL_DIR)/lib

pre:
	rm -rf $(CURR_DIR)/include/wlcsm_linux.h
	ln -sf $(WLCSM_EXT_BASE)/include/wlcsm_linux.h  $(CURR_DIR)/include/wlcsm_linux.h

install: all
	install -m 755 libwlcsm.so $(INSTALL_DIR)/lib

distclean clean:
	-rm -f *.o *.map *.d  *.d.* *.a
	-rm -f libwlcsm.so
	-rm -rf doc
	-rm -rf $(CURR_DIR)/include/wlcsm_linux.h
	-rm -rf $(BUILD_DIR)/userspace/private/include/wlcsm_lib_api.h
	-rm -rf $(BUILD_DIR)/userspace/private/include/wlcsm_defs.h
	-rm -f $(INSTALL_DIR)/lib/libwlcsm.so
#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)
