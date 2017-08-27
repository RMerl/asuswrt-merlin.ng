#***********************************************************************
#
#  Copyright (c) 2006-2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************

LIB = libbcm_flashutil.so
OBJS = bcm_flashutil.o bcm_imgif.o bcm_imgutil.o bcm_imgutil_api.o bcm_imgcombo.o
INCLUDE_DIR = $(BCM_FSBUILD_DIR)/public/include
HEADER = bcm_flashutil.h

all: sanity_check $(LIB) generic_public_lib_install fsbuild_install

.PHONY: crc_softlink

clean: generic_clean
	rm -f $(INSTALL_DIR)/lib/public/$(LIB)
	rm -f $(INCLUDE_DIR)/$(HEADER)

install fsbuild_install: generic_public_lib_install
	mkdir -p $(INCLUDE_DIR)
	install -m 444 $(HEADER) $(INCLUDE_DIR)
   
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

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
CFLAGS += -DCHIP_FAMILY_ID_HEX=0x$(BRCM_CHIP)


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
ifneq ($(PREBUILT_BCMBIN),1)
$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -lgcc_s -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive
else
$(LIB): 
	-cp -f $(TOP)/$(shell pwd | sed 's/\(.*router\/\)//')/$(PRBM_DIR)/$@ ./
	-cp -f $(TOP)/$(shell pwd | sed 's/\(.*router\/\)//')/$(PRBM_DIR)/$(HEADER) ./
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
