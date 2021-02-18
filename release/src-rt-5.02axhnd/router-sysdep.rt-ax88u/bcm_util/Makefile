#***********************************************************************
#
#  Copyright (c) 2006-2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************

LIB = libbcm_crc.so
OBJS = bcm_crc.o
INCLUDE_DIR = $(BCM_FSBUILD_DIR)/public/include
HEADER = bcm_crc.h

all: sanity_check $(LIB) generic_public_lib_install fsbuild_install

.PHONY: crc_softlink

clean: generic_clean
	rm -f $(INSTALL_DIR)/lib/$(LIB)
	rm -f $(INCLUDE_DIR)/$(HEADER)

$(INCLUDE_DIR)/$(HEADER): $(LIB)
	mkdir -p $(dir $@)
	install -m 444 $(notdir $@) $@

fsbuild_install: $(INCLUDE_DIR)/$(HEADER) | generic_public_lib_install
install:
	@echo done
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
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR)

ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) 

# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
#
$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive



#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)
