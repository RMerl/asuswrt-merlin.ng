#
# In most cases, you only need to modify this first section.
#

default:all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

OUTPUT_LIB_NAME:=libwlcsm_dm.so

GENERATE = $(CURR_DIR)/generate_from_dm_wl.pl
HDRADJUST = $(CURR_DIR)/header_adjust.pl
DMFILE2 := cms-dm-tr181-wifi.xml
MAPPERFILE := stringintegermapper.xml
export PERL5LIB=$(BUILD_DIR)/data-model
IMPL=$(shell readlink $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP) | sed 's/[^0-9]//g')
WL_IMPL_PLUS= $(strip $(shell if [ "$(IMPL)" -ge 24 ]; then echo Y; fi ;))
WL_SRC_BASE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo /main/src; fi ;)
export WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
export INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
COMM_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/common/include

DM_GENERIC_FILE := $(CURR_DIR)/include/wlcsm_dm_generic.h
DM_GENERATED_FILES := $(CURR_DIR)/src/wlcsm_dm_nvram_mngr_mapping.c
DM_GENERATED_FILES += $(CURR_DIR)/src/wlcsm_dm_mngr_strmapper.c
DM_GENERATED_FILES += $(CURR_DIR)/src/wlcsm_dm_mngr_structoff.c
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_generic_orig.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_mngr_structs_name.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_mngr_structoff.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_nvram_mngr_mapping.h
DM_GENERATED_FILES += $(CURR_DIR)/include/wlcsm_dm_mngr_strmapper.h

DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/src/wlcsm_dm_tr181_mngr_mapping.c
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/src/wlcsm_dm_tr181_oid_mapper.c
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/include/wlcsm_dm_tr181_mngr_mapping.h 
DM_TR181_GENERATED_FILES += $(CURR_DIR)/dm_impls/tr181/include/wlcsm_dm_tr181_oid_mapping.h
DM_GENERATED_FILES += $(DM_TR181_GENERATED_FILES)

WLCSM_FILES := wlcsm_lib_dm.c
WLCSM_FILES += wlcsm_dm_mngr_strmapper.c
WLCSM_FILES += wlcsm_dm_mngr_structoff.c
WLCSM_FILES += wlcsm_dm_nvram_mngr_mapping.c
WLCSM_OBJS = $(foreach x, $(WLCSM_FILES),$(x:.c=.o))
OBJS += $(WLCSM_OBJS)

ifeq ($(BUILD_WLHSPOT),y)
CFLAGS += -DHSPOT_SUPPORT
endif

ifneq ($(strip $(HNDAPPS_IN_COMP)),)
CFLAGS += -I$(HNDAPPS_DIR)/shared  -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
endif

ifneq ($(strip $(BUILD_BRCM_CMS)),)
LIBS = -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -l cms_cli -lcms_msg $(CMS_COMMON_LIBS) -ldl $(CMS_CORE_LIBS)
ifneq ($(strip $(BUILD_PURE181_PROFILES)),)
WLCSM_DM_IMPLS := tr181
else
WLCSM_DM_IMPLS := tr98
endif
else
WLCSM_DM_IMPLS := nocms
endif

all:$(WLCSM_DM_IMPLS)

$(WLCSM_DM_IMPLS):$(OUTPUT_LIB_NAME)
	for d in $(WLCSM_DM_IMPLS); do ( if [ -e $(CURR_DIR)/dm_impls/$$d ]; then cd $(CURR_DIR)/dm_impls/$$d; $(MAKE) install; fi; ); done

ifneq ($(strip $(BUILD_BRCM_CMS)),)
$(DM_GENERIC_FILE):$(DM_GENERATED_FILES) $(HDRADJUST) 
else
$(DM_GENERIC_FILE): $(HDRADJUST) 
endif
	-rm -rf $(DM_GENERIC_FILE)
	$(HDRADJUST)

regenerate:$(DM_GENERATED_FILES) $(HDRADJUST) $(BUILD_DIR)/data-model/$(DMFILE2) $(GENERATE)

$(DM_GENERATED_FILES): regenerate_dm_files

regenerate_dm_files: $(BUILD_DIR)/data-model/$(DMFILE2) $(GENERATE)  $(MAPPERFILE) $(HDRADJUST)
	-for fl in $(DM_GENERATED_FILES); do ( rm -rf $$fl; ); done
	$(GENERATE) wlcsm $(BUILD_DIR) $(DMFILE2) $(DMFILE2)
	$(GENERATE) wlcsm_mngr $(BUILD_DIR) $(DMFILE2) $(DMFILE2)
	$(GENERATE) strmapper $(BUILD_DIR) $(MAPPERFILE) $(MAPPERFILE)
	@touch $@

clean: generic_clean
	-rm -f *.o *.map *.d  *.d.*
	-rm -rf *.so
	-rm -rf doc
	-rm -f regenerate_dm_files
	-for d in $(WLCSM_DM_IMPLS); do ( if [ -e $(CURR_DIR)/dm_impls/$$d ]; then cd $(CURR_DIR)/dm_impls/$$d; $(MAKE) clean; fi; ); done

dmclean: clean
	-for fl in $(DM_GENERATED_FILES); do ( rm -rf $$fl; ); done
	-for d in $(WLCSM_DM_IMPLS); do ( if [ -e $(CURR_DIR)/dm_impls/$$d ]; then cd $(CURR_DIR)/dm_impls/$$d; $(MAKE) clean; fi; ); done



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#

#ifeq ($(BRCM_WAPI),y)
#OBJS += wlmngr_wapi_cert_mdm.o wlmngr_wapi_cert_mgmt.o
#endif
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
                         -I../include \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core \
                         -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include \
                         -I$(BUILD_DIR)/userspace/private/apps/wlan/wlmngr \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core/linux 

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



ifeq ($(DSLCPE_WLCSM_EXT),1)
CFLAGS += -DDSLCPE_WLCSM_EXT -Werror
ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include
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

%.o: %.c $(DM_GENERIC_FILE)
	$(CC) -c $(CFLAGS) -o $@ $<


$(OBJS): $(DM_GENERIC_FILE)

$(OUTPUT_LIB_NAME): $(OBJS)
	$(CC) -s -Os -fomit-frame-pointer  -shared -o $(OUTPUT_LIB_NAME) $(OBJS)
	install -m 755 $(OUTPUT_LIB_NAME) $(INSTALL_DIR)/lib

install: all
	install -m 755 $(OUTPUT_LIB_NAME) $(INSTALL_DIR)/lib
	for d in $(WLCSM_DM_IMPLS); do ( if [ -e $(CURR_DIR)/dm_impls/$$d ]; then cd $(CURR_DIR)/dm_impls/$$d; $(MAKE) install; fi; ); done

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
