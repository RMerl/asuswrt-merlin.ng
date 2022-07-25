#***********************************************************************
#
#  Copyright (c) 2006-2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************


#
# In most cases, you only need to modify this first section.
#
LIB = libcms_core.so

FULL_SOURCE_OBJS = management.o  phl_ene.o \
                   phl.o object.o odl.o \
                   locks.o remote.o phl_merge.o \
                   securitylog.o \
                   mdm_config.o mdm_configin.o \
                   mdm_binaryHelper.o mdm_dataModelHelper.o \
                   mdm_oidInfoArray.o \
                   mdm_init.o \
                   mdm_initdsl.o mdm_initlan.o mdm_initvpn.o \
                   mdm_initstorageservice.o \
                   mdm_initponwan.o mdm_initepon.o mdm_initoptical.o \
                   mdm_initbmu.o \
                   mdm2_init.o \
                   mdm2_initqos.o \
                   mdm2_initusb.o


OBJS = $(FULL_SOURCE_OBJS)

all install: conditional_build

clean: generic_oalclean generic_clean device2_clean
	rm -f $(INSTALL_DIR)/lib/private/$(LIB)



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

export BUILD_EPON_SDK_VOICE_OAM

ifneq ($(strip $(BRCM_VOICE_SUPPORT)),)
include $(BUILD_DIR)/make.voice
endif

ifeq ($(strip $(BUILD_CUSTOMER)),)
FULL_SOURCE_OBJS += mdm_initwlan.o
endif

ifeq ($(BCM_PON),y)
FULL_SOURCE_OBJS += mdm_initwanfilters.o 
endif                   

ifneq ($(strip $(BUILD_DM_TR104_V2)),)
    FULL_SOURCE_OBJS += mdm2_initvoice.o
else
    FULL_SOURCE_OBJS += mdm_initvoice.o
endif

ifneq ($(strip $(BCA_HND_EAP)),)
	CFLAGS += -DBCA_HND_EAP
endif

CFLAGS += $(SSP_TYP_COMPILER_OPTS) $(SSP_TYP_LIBS)  $(SSP_LIB_INC)


#
# Private apps and libs are allowed to include header files from the
# private and public directories
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BCM_FSBUILD_DIR)/private/include \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR)

ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/public/libs/bcm_util

ifneq ($(strip $(WIRELESS)),)
CFLAGS += -DSUPPORT_WIRELESS
ALLOWED_INCLUDE_PATHS += -I$(INC_BCMDRIVER_PATH)/include
endif

ifeq ($(strip $(BRCM_XDSL_DISTPOINT_USE_G9991)),y)
CFLAGS += -DG9991
endif

ifeq ($(strip $(DISABLE_NOR_RAW_PARTITION)),y)
CFLAGS += -DDISABLE_NOR_RAW_PARTITION
endif

#management.c needs cms_boardioctl.h, so we have to include these kernel header files
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(BCM_FSBUILD_DIR)/public/include

ifeq ($(BUILD_RDPA),y)
ALLOWED_INCLUDE_PATHS += $(INC_RDP_FLAGS)
endif

# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors # NEW_FORBID_WARNINGS


device2:
	$(MAKE) -C $(OALDIR)/device2

device2_clean:
	$(MAKE) -C $(OALDIR)/device2 clean


ifneq ($(strip $(BUILD_BRCM_CMS)),)
COND_BUILD_CMS_CORE := 1
endif
ifneq ($(strip $(BUILD_DISTRIBUTED_MDM)),)
COND_BUILD_CMS_CORE := 1
endif

ifeq ($(strip $(COND_BUILD_CMS_CORE)),1)

conditional_build: generic_private_lib_install

$(LIB): generic_oaldir $(OBJS) device2
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) \
          $(OALDIR)/oal.a $(OALDIR)/device2/device2.a -Wl,--no-whole-archive -ldl -lpthread 


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)

else
conditional_build:
	@echo "skipping $(LIB) (not configured)"
endif
