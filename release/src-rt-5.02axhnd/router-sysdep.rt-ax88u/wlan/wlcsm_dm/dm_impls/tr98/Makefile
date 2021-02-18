#
# In most cases, you only need to modify this first section.
#

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
OUTPUT_LIB:= libwlcsm_dm_tr98.so


WLCSM_FILES := wlcsm_dm_tr98.c wlmngr.c wlmdm.c  wlutil.c wldsltr.c nvram_mdm.c  wlsyscall.c 

#$(info $(WLCSM_FILES))

WLCSM_OBJS = $(foreach x, $(WLCSM_FILES),$(x:.c=.o))
OBJS += $(WLCSM_OBJS)

LIBS = -lwlcsm -lbcmshared
LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -l cms_cli -lcms_msg $(CMS_COMMON_LIBS) -ldl $(CMS_CORE_LIBS)

all:$(OUTPUT_LIB) 
clean: generic_clean
	-rm -f *.o *.map *.d  *.d.*
	-rm -rf *.so
	
#
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common


WL_SRC_BASE := /main/src

WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
COMM_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/common/include
#
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
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include  \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core \
                         -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include \
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



ifneq ($(strip $(HNDAPPS_IN_COMP)),)
ALLOWED_INCLUDE_PATHS += -I$(HNDAPPS_DIR)/shared  -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
endif

CFLAGS +=  -DDSLCPE -DDSLCPE_WLCSM_EXT 
CFLAGS += -DDSLCPE_ENDIAN

ifeq (exists, $(shell [ -d $(WIRELESS_DRIVER_PATH)/router/eapd ]&& echo exists))
	    CFLAGS += -DBUILD_EAPD -DDSLCPE -I$(WIRELESS_DRIVER_PATH)/router/eapd
    endif

ifneq ($(strip $(DSLCPE_WPS_SEC_CLONE)),)
	CFLAGS += -DDSLCPE_WPS_SEC_CLONE
endif

ifneq ($(strip $(BCA_CPEROUTER)),)
CFLAGS +=-DWIRELESS -DSUPPORT_MIMO -DBCMWPA2
else
CFLAGS +=-DWIRELESS -DSUPPORT_MIMO -DDSLCPE_SHLIB -DBCMWPA2
endif
CFLAGS += -DWMF
CFLAGS += -DDUCATI
ifneq ($(strip $(BUILD_WL_IDLE_PWRSAVE)),)
	   CFLAGS += -DIDLE_PWRSAVE
   endif

ifeq ($(BUILD_WLHSPOT),y)
	CFLAGS += -DHSPOT_SUPPORT
endif

ifeq ($(EXT_ACS),y)
	CFLAGS += -DEXT_ACS
endif

ifeq ($(EXT_WPS_VER2),y)
	CFLAGS += -DWPS_V2
endif

ifneq ($(strip $(BUILD_WSC)), )
	CFLAGS += -DSUPPORT_WSC
ifeq ($(strip $(BUILD_WSC_CONFIG_UPNP_DISABLE)),1)
	CFLAGS += -DWSC_CONFIG_UPNP_DISABLE
endif
endif



ifeq ($(EXT_WL11AC),y)
	CFLAGS += -DWL11AC
endif

ifneq ($(strip $(MFP)),)
	CFLAGS += -DMFP
endif

                                                                                   


ifeq ($(DSLCPE_WLCSM_EXT),1)
	LIBS += -lwlcsm
	CFLAGS += -DDSLCPE_WLCSM_EXT
	ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include
endif

ifeq ($(DSLCPE_CONFIG_MIPS),1)
	CFLAGS+=-DCPU_MIPS
endif

CFLAGS += -DDSLCPE_DONGLEHOST_MFG


# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private


# treat all warnings as errors
#CUSTOM_CFLAGS += -Wno-error=unused-function -Wno-error=unused-but-set-variable -Werror -Wfatal-errors  # NEW_FORBID_WARNINGS
CUSTOM_CFLAGS += -Werror  -Wunused-function -Wunused-variable
vpath %.c $(CURR_DIR)/src 

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


$(OUTPUT_LIB): $(OBJS) 
	#$(CC) -s -Os -fomit-frame-pointer -shared -o $(OUTPUT_LIB) $(OBJS)
	$(CC)  -shared -o $(OUTPUT_LIB) $(OBJS)
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
