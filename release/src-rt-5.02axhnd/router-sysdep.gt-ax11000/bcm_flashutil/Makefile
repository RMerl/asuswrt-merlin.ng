#***********************************************************************
#
#  Copyright (c) 2018  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************

LIB = libbcm_flashutil.so
OBJS = bcm_flashutil.o bcm_flashutil_nand.o bcm_imgutil.o bcm_imgutil_api.o bcm_imgcombo.o bcm_imgif.o

INCLUDE_DIR = $(BCM_FSBUILD_DIR)/public/include
HEADER = bcm_flashutil.h

all: sanity_check $(LIB) generic_public_lib_install fsbuild_install

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common


.PHONY: crc_softlink

clean: generic_clean
	rm -f $(INSTALL_DIR)/lib/public/$(LIB)
	rm -f $(INCLUDE_DIR)/$(HEADER)
	rm -f $(INSTALL_DIR)/etc/rc3.d/S68commit
	rm -f $(INSTALL_DIR)/etc/init.d/commit.sh

$(INCLUDE_DIR)/$(HEADER): $(LIB)
	mkdir -p $(dir $@)
	install -m 444 $(notdir $@) $@
ifneq ($(strip $(BRCM_NO_AUTOCOMMIT_IMAGE)),)
	@echo "INSTALLING COMMIT AFTER REBOOT SCRIPT"
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	install -m 0755 commit.sh $(INSTALL_DIR)/etc/init.d
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f S68commit; ln -s ../init.d/commit.sh S68commit)
endif

install fsbuild_install: $(INCLUDE_DIR)/$(HEADER) | generic_public_lib_install



#
# Public libs are only allowed to include header files from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(INCLUDE_DIR) \
                         -I$(CURR_DIR)/../bcm_util \

ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMSHARED_PUB_PATH)/../flash  \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) 

# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors
CFLAGS += -DCHIP_FAMILY_ID_HEX=0x$(BRCM_CHIP) -Wno-attributes

ifeq ($(strip $(DESKTOP_LINUX)),)
OBJS += bcm_flashutil_emmc.o  bcm_imgif_nand.o bcm_imgif_emmc.o 

ifneq ($(strip $(BRCM_INCREMENTAL_IMAGE_LOAD)),)
    CFLAGS += -DSUPPORT_INCREMENTAL_FLASHING
endif

ifneq ($(strip $(BRCM_NO_AUTOCOMMIT_IMAGE)),)
    CFLAGS += -DNO_AUTOCOMMIT_IMAGE
endif
endif


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
#
ifneq ($(PREBUILT_BCMBIN),1)
$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive
else
$(LIB): 
	-cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
	-cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$(HEADER) ./
endif


#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)
