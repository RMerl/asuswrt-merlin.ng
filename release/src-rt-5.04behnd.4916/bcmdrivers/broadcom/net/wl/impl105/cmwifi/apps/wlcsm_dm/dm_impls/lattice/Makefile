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

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
OUTPUT_LIB:= libwlcsm_dm_lattice.so
DM_GENERATED_FILES := $(CURR_DIR)/src/wlcsm_dm_lattice_mngr_mapping.c
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_lattice_mngr_mapping.h
WLCSM_FILES = $(foreach x, $(shell find ./src -name \*.c) ,$(notdir $(x)))
#$(info $(WLCSM_FILES))

WLCSM_OBJS = $(foreach x, $(WLCSM_FILES),$(x:.c=.o))
OBJS += $(WLCSM_OBJS)

LIBS = -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -l cms_cli -lcms_msg $(CMS_COMMON_LIBS) -ldl $(CMS_CORE_LIBS)

all: $(DM_GENERATED_FILES) $(OUTPUT_LIB)

$(DM_GENERATED_FILES):
#	$(error "no generated file, run make regenerate from wlcsm_dm")
	$(MAKE) -C $(CURR_DIR)/../../ regenerate

clean: generic_clean
	-rm -f *.o *.map *.d  *.d.*
	-rm -rf *.so

dmclean: clean
	-rm -rf include/wlcsm_dm_lattice_mngr_mapping.h
	-rm -rf src/wlcsm_dm_lattice_mngr_mapping.c
#
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

WIRELESS_DRIVER_PATH=$(WL_DIR)/bcm9$(BRCM_CHIP)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
COMM_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/common/include
#
# Private apps and libs are allowed to include header files from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I./include \
                         -I../../include \
                         -I$(WL_RG_APPS_PRIVATE_DIR)/include  \
                         -I$(WL_RG_APPS_PRIVATE_DIR)/include/$(OALDIR) \
                         -I$(WL_RG_APPS_PRIVATE_DIR)/libs/wlcsm/include \
                         -I$(WL_RG_APPS_PRIVATE_DIR)/apps/wlan/181wlmngr

# obviously, this app needs to access kernel headers
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/broadcom/include/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
                         -I$(INC_WIRELESS_DRIVER_PATH) \
                         -I$(COMM_WIRELESS_DRIVER_PATH) \
                         -I$(WIRELESS_DRIVER_PATH)/bcmcrypto \
                         -I$(WIRELESS_DRIVER_PATH)/shared \
                         -I$(WIRELESS_DRIVER_PATH)/router/shared \
                         -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include \
                         -I$(WIRELESS_DRIVER_PATH)/wl/wlcsm_ext/include

ALLOWED_INCLUDE_PATHS += -I$(WLSRCBASE)/include

ALLOWED_INCLUDE_PATHS += -I$(WL_CMWIFI_DIR)/mods/wlcsm_ext/include -I$(WL_CMWIFI_DIR)/libs/wlcsm/include -I$(WL_CMWIFI_DIR)/apps/181wlmngr

CFLAGS += -g -DDSLCPE_WLCSM_EXT -DCMWIFI $(USR_CFLAGS)
ifeq ($(SUPPORT_NO_WIFI_CMS),y)
 CFLAGS += -DSUPPORT_NO_WIFI_CMS
endif
ifeq ($(CMWIFI_NVRAM_ENC),y)
CFLAGS += -DCMWIFI_NVRAM_ENC
endif

# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private

# treat all warnings as errors
#CUSTOM_CFLAGS += -Wno-error=unused-function -Wno-error=unused-but-set-variable -Werror -Wfatal-errors  # NEW_FORBID_WARNINGS
vpath %.c $(CURR_DIR)/src

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OUTPUT_LIB): $(OBJS)
	$(CC) -g -shared -o $(OUTPUT_LIB) $(OBJS)
	install -m 755 $(OUTPUT_LIB) $(INSTALL_DIR)/lib
install: all
	install -m 755 $(OUTPUT_LIB) $(INSTALL_DIR)/lib

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)
