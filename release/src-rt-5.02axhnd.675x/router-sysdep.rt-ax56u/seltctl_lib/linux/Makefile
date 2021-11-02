LIB = oal.a

FULL_SOURCE_OBJS = KissFft.o SeltBldProfileDep.o
BINARY_ONLY_OBJS = seltctl.o SeltUtils.o SeltAfeCalibration.o
OBJS = $(FULL_SOURCE_OBJS) $(BINARY_ONLY_OBJS)

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) -rcs $@ $(OBJS)

HAVESRC= $(wildcard seltctl.c)

clean: generic_clean


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

#
# Private apps and libs are allowed to include header files from the
# private and public directories
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I. -I..\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR)


# adsl.c needs cms_boardioctl.h, so we need to include these kernel header files
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
			 -I$(INC_BRCMDRIVER_PRIV_PATH)/../char/adsl/impl1/softdsl

#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 


#
# Generic rule makes the library oal.a
# see make.common
#


#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(FULL_SOURCE_OBJS:.o=.d)

ifeq ($(strip $(HAVESRC)),)
seltctl.o: seltctl.o.$(PROFILE_ARCH).save
	cat $< >$@

SeltUtils.o: SeltUtils.o.$(PROFILE_ARCH).save
	cat $< >$@

SeltAfeCalibration.o: SeltAfeCalibration.o.$(PROFILE_ARCH).save
	cat $< >$@

else
-include $(BINARY_ONLY_OBJS:.o=.d)
endif
