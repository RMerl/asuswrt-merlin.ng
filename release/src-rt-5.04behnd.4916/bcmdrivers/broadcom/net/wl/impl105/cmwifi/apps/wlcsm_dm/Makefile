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
OUTPUT_LIB_NAME:=libwlcsm_dm.so

GENERATE = $(CURR_DIR)/generate_from_dm_wl.pl
HDRADJUST = $(CURR_DIR)/header_adjust.pl
DMFILE2 := lattice-dm-wifi.xml
MAPPERFILE := stringintegermapper.xml
export PERL5LIB=$(BUILD_DIR)/data-model

DM_GENERATED_FILES := $(CURR_DIR)/src/wlcsm_dm_nvram_mngr_mapping.c
DM_GENERATED_FILES += $(CURR_DIR)/src/wlcsm_dm_mngr_strmapper.c
DM_GENERATED_FILES += $(CURR_DIR)/src/wlcsm_dm_mngr_structoff.c
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_generic.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_mngr_structs_name.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_mngr_structoff.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_nvram_mngr_mapping.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_mngr_strmapper.h

DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/src/wlcsm_dm_tr181_mngr_mapping.c
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/src/wlcsm_dm_tr181_oid_mapper.c
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/include/wlcsm_dm_tr181_mngr_mapping.h
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/include/wlcsm_dm_tr181_oid_mapping.h
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/lattice/src/wlcsm_dm_lattice_mngr_mapping.c
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/lattice/include/wlcsm_dm_lattice_mngr_mapping.h
DM_GENERATED_FILES += $(DM_TR181_GENERATED_FILES)

WLCSM_FILES := wlcsm_lib_dm.c
WLCSM_FILES += wlcsm_dm_mngr_strmapper.c
WLCSM_FILES += wlcsm_dm_mngr_structoff.c
WLCSM_FILES += wlcsm_dm_nvram_mngr_mapping.c
#WLCSM_FILES = $(foreach x, $(ls $(CURR_DIR)/src) ,$(notdir $(x)))
WLCSM_OBJS = $(foreach x, $(WLCSM_FILES),$(x:.c=.o))
OBJS += $(WLCSM_OBJS)

WLCSM_DM_IMPLS := $(foreach x, $(shell ls $(CURR_DIR)/dm_impls) ,$(notdir $(x)))

#WLCSM_ONLY_FILES = $(foreach x, $(wildcard $(CURR_DIR)/src/*.c) ,$(notdir $(x)))
#WLCSM_ONLY_OBJS = $(foreach x, $(WLCSM_ONLY_FILES),$(x:.c=.o)) #$(error $(WLCSM_ONLY_OBJS))

ifneq ($(SUPPORT_NO_WIFI_CMS),y)
LIBS = -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -l cms_cli -lcms_msg $(CMS_COMMON_LIBS) -ldl $(CMS_CORE_LIBS)
endif

all: regenerate $(OUTPUT_LIB_NAME)
ifneq ($(SUPPORT_NO_WIFI_CMS),y)
	cd $(CURR_DIR)/dm_impls/tr181; $(MAKE) all
endif
	cd $(CURR_DIR)/dm_impls/lattice; $(MAKE) all;

DM_MISSING_FILES := $(shell for fl in $(DM_GENERATED_FILES); do { test ! -e $$fl && echo $$fl; }; done)

ifneq ($(DM_MISSING_FILES),)
regenerate: $(BUILD_DIR)/data-model/$(DMFILE2) $(HDRADJUST) $(GENERATE)
	@for fl in $(DM_MISSING_FILES); do ( echo "### Missing $$fl"; ); done
	-for fl in $(DM_GENERATED_FILES); do ( rm -rf $$fl; ); done
	$(GENERATE) wlcsm $(WL_CMWIFI_DIR) $(DMFILE2) $(DMFILE2)
	$(GENERATE) wlcsm_mngr $(WL_CMWIFI_DIR) $(DMFILE2) $(DMFILE2)
	$(GENERATE) strmapper $(WL_CMWIFI_DIR) $(MAPPERFILE) $(MAPPERFILE)
	$(HDRADJUST)
else
regenerate:
	@echo "### No need to regenerate DM c/h files!"
endif

clean: generic_clean
	-rm -f *.o *.map *.d  *.d.*
	-rm -rf *.so
	-rm -rf doc
	-for d in $(WLCSM_DM_IMPLS); do ( cd $(CURR_DIR)/dm_impls/$$d; $(MAKE) clean); done

dmclean: clean
	-for fl in $(DM_GENERATED_FILES); do ( rm -rf $$fl; ); done
	-for d in $(WLCSM_DM_IMPLS); do ( cd $(CURR_DIR)/dm_impls/$$d; $(MAKE) dmclean); done

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

#ifeq ($(BRCM_WAPI),y)
#OBJS += wlmngr_wapi_cert_mdm.o wlmngr_wapi_cert_mgmt.o
#endif // endif

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
                         -I./include/tr181 \
                         -I../include

ifneq ($(SUPPORT_NO_WIFI_CMS),y)
ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core/linux
endif

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

ALLOWED_INCLUDE_PATHS += -I$(WL_CMWIFI_DIR)/mods/wlcsm_ext/include -I$(WL_CMWIFI_DIR)/libs/wlcsm/include -I$(WL_CMWIFI_DIR)/apps/181wlmngr

ifeq ($(DSLCPE_WLCSM_EXT),1)
CFLAGS += -g -DDSLCPE_WLCSM_EXT
ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include
endif
CFLAGS += -DCMWIFI $(USR_CFLAGS)
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

ifeq ($(EXT_WLCONF),y)
vpath %.c $(WIRELESS_DRIVER_PATH)/router/wlconf/
vpath %.c $(WIRELESS_DRIVER_PATH)/router/shared/
vpath %.c $(WIRELESS_DRIVER_PATH)/shared/bcmwifi/src/
endif
vpath %.c $(CURR_DIR)/src

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

# Default for RDK
WIFI_DM = lattice-dm-wifi.xml

ifneq ($(findstring -lattice,$(PLAT)),)
WIFI_DM = openbfc/lattice-dm-wifi.xml
endif

$(OUTPUT_LIB_NAME): $(OBJS) $(DM_GENERATED_FILES)
	$(CC) -g -shared -o $(OUTPUT_LIB_NAME) $(OBJS)
	install -m 755 $(OUTPUT_LIB_NAME) $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/etc/lattice

install: all
	install -m 755 $(OUTPUT_LIB_NAME) $(INSTALL_DIR)/lib
ifneq ($(SUPPORT_NO_WIFI_CMS),y)
	cd $(CURR_DIR)/dm_impls/tr181; $(MAKE) install;
endif
	cd $(CURR_DIR)/dm_impls/lattice; $(MAKE) install;
	cp $(WL_CMWIFI_DIR)/data-model/$(WIFI_DM) $(INSTALL_DIR)/etc/lattice

help:
	doxygen

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)
