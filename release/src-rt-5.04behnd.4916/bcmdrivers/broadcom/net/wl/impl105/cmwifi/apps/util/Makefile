#***********************************************************************
#
#  Copyright (c) 2006  Broadcom Corporation
#  All Rights Reserved
#
#  <:license-private
#
#***********************************************************************/

#
# In most cases, you only need to modify this first section.
#
EXE  = nvramUpdate
OBJS = nvram_util.o

all dynamic: $(EXE)
install: all
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(EXE)
	install -m 755 nvram_params $(INSTALL_DIR)/etc/wlan

clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common
CFLAGS += -Werror

WIRELESS_DRIVER_PATH:=$(WLSRCBASE)
INC_WIRELESS_DRIVER_PATH:=$(WIRELESS_DRIVER_PATH)/include

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
