#
# In most cases, you only need to modify this first section.
#

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
vpath %.c $(CURR_DIR)/src 
OUTPUT_LIB:= libwlcsm_dm_nocms.so


WLCSM_FILES := wlcsm_dm_nocms.c

#$(info $(WLCSM_FILES))

WLCSM_OBJS = $(foreach x, $(WLCSM_FILES),$(x:.c=.o))
OBJS += $(WLCSM_OBJS)

LIBS = -lwlcsm -lbcmshared

all:$(OUTPUT_LIB) 
dmclean clean: generic_clean
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


IMPL=$(shell readlink $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP) | sed 's/[^0-9]//g')
WL_SRC_BASE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo /main/src; fi ;)
WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
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
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/linux  \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include 

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



CFLAGS +=  -DDSLCPE_WLCSM_EXT -Werror

# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private


# treat all warnings as errors
#CUSTOM_CFLAGS += -Wno-error=unused-function -Wno-error=unused-but-set-variable -Werror -Wfatal-errors  # NEW_FORBID_WARNINGS

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


$(OUTPUT_LIB): $(OBJS) 
	$(CC) -s -Os -fomit-frame-pointer -shared -o $(OUTPUT_LIB) $(OBJS)
	#$(CC)  -shared -o $(OUTPUT_LIB) $(OBJS)
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
