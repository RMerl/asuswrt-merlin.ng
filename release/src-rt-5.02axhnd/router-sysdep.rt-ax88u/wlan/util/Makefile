#
# In most cases, you only need to modify this first section.
#
EXE  = nvramUpdate
OBJS = nvram_util.o

all dynamic: $(EXE)
ifneq ($(PREBUILT_BCMBIN),1)
install: all
else
install:
endif
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
	install -m 755 nvram_params $(INSTALL_DIR)/rom/rom/etc/wlan

clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

IMPL=$(shell readlink $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP) | sed 's/[^0-9]//g')
ifeq ($(IMPL),)
# the symbolic link of bcm9$(BRCM_CHIP) has been removed/cleaned, set IMPL to an integer to avoid warning message
IMPL=0
endif
WL_SRC_BASE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo /main/src; fi ;)

WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include

ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core


# obviously, this app needs to access kernel headers
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/broadcom/include/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
                         -I$(INC_WIRELESS_DRIVER_PATH) 

ALLOWED_LIB_DIRS := /lib:/lib/private:/lib/public

CFLAGS +=-DWIRELESS -DSUPPORT_MIMO -DDSLCPE_SHLIB -DBCMWPA2

ifneq ($(strip $(BUILD_WSC)), )
CFLAGS += -DSUPPORT_WSC
endif
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


$(EXE):  nvram_util.o
	$(CC) -o $@ nvram_util.o -s -Os -fomit-frame-pointer -Wl,-allow-shlib-undefined -Wl,-rpath,$(CMS_LIB_RPATH) $(CMS_LIB_PATH)


include $(BUILD_DIR)/make.deprules

#-include $(OBJS:.o=.d)





