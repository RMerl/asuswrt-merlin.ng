EXE  = seltctl
OBJS = seltctl.o
OBJS_BINARY = seltctl.o.$(PROFILE_ARCH).save

LIBS = -lm -l xdslctl  -l seltctl -lgcc

all dynamic install: conditional_build


clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)


ifneq ($(strip $(BUILD_DSL_SELT_TEST)),)
conditional_build: $(EXE)
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
else
conditional_build:
	@echo "skipping seltctl (not configured)"
endif


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

#
# Private apps and libs are allowed to include header files from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR)


# obviously, this app needs to access kernel headers
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
			 -I$(INC_BRCMDRIVER_PRIV_PATH)/../char/adsl/impl1/softdsl

#
# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/private:/lib/public

# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors

# additional CFLAGS, hmm, should this be in make.common, but only if we are
# doing adsl/vdsl, not GPON.
CFLAGS += -D$(BRCM_ADSL_STANDARD)
CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP)

ifneq ($(strip $(BUILD_TR69_XBRCM)),)
CFLAGS += -DADSL_MIBOBJ_PLN
endif

ifeq ($(strip $(BRCM_NTR_SUPPORT)),1)
CFLAGS += -DNTR_SUPPORT
endif

ifeq "$(BRCM_PHY_BONDING)" "y"
	CFLAGS += -DSUPPORT_DSL_BONDING
endif

# This is support for the static mode.  Do we still support this mode?
ifeq ($(strip $(BUILD_ADSLCTL)), static)
CFLAGS += -DBUILD_STATIC
endif

static: $(EXE).a

$(EXE).a: $(SOURCE_FILES)
	$(AR) rcs $(EXE).a $^

HAVESRC= $(wildcard seltctl.c)

ifeq ($(strip $(HAVESRC)),)
$(OBJS): $(OBJS_BINARY)
	cat $< >$@
endif

#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) -Wl,-rpath,$(CMS_LIB_RPATH) $(CMS_LIB_PATH) $(LIBS)

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)

