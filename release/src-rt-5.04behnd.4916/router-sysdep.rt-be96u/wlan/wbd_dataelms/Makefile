
#
# You probably don't need to modify anything in this Makefile.
#

.PHONY: install uninstall clean all scripts-install scripts-clean

default:all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan

LIB := libwbddataelms.so
OBJS += wbd_dataelms_i5dm.o wbd_dataelms_i5dm_tables.o wbd_dataelms_jsonc_utils.o wbd_dataelms_list.o wbd_dataelms_dm.o wbd_dataelms_dm_tables.o wbd_dataelms_utils.o

LIBS = -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/usr/lib -ldl -lpthread -lgen_util

LIBS += -ljson-c

CFLAGS += -DMULTIAP $(USR_CFLAGS) $(RDKB_CFLAGS)

#ifneq ($(strip $(BCA_CPEROUTER)),)
#LIBS += -lnvram -lm -lshared
#endif

all dynamic: $(LIB)

install: all
	mkdir -p $(BCM_FSBUILD_DIR)/private/include
	install -m 644 wbd_dataelms_dm.h $(BCM_FSBUILD_DIR)/private/include
	install -m 755 $(LIB) $(INSTALL_DIR)/lib

clean: generic_clean scripts-clean
	-rm -f $(BCM_FSBUILD_DIR)/private/include/wbd_dataelms_dm.h

scripts-install:

scripts-clean:

CFLAGS += -I$(HNDAPPS_DIR)/shared  -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
CFLAGS += -I$(HNDAPPS_DIR)/../math/include 

ALLOWED_INCLUDE_PATHS += -I. \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BCM_FSBUILD_DIR)/private/include \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(INC_BCMDRIVER_PATH)/include \
                         -I$(BCM_FSBUILD_DIR)/public/include/json-c

# obviously, this app needs to access wireless driver headers
ALLOWED_INCLUDE_PATHS += -I$(WIRELESS_DRIVER_PATH)/include \
                         -I$(WIRELESS_DRIVER_PATH)/shared \
                         -I$(WIRELESS_DRIVER_PATH)/router/shared \
                         -I$(WIRELESS_DRIVER_PATH)/../components/bcmwifi/include \
                         -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include \
                         -I$(WIRELESS_DRIVER_PATH)/wl

# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/public/lib:/private/lib:/usr/lib
#

ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/ieee1905
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/wbd2
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/wbd2/shared
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/wbd2/blanket
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/router/bshared/include
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/opensource/router_tools/wpa_supplicant/src/common


LIB_RPATH = $(INSTALL_DIR)$(subst :,:$(INSTALL_DIR),$(ALLOWED_LIB_DIRS))
LDFLAGS += $(BCM_RPATH_LINK_OPTION) -Wl,-rpath-link=$(LIB_RPATH)

%.o: %.c
	$(CC) -g -c $(CFLAGS) -o $@ $<

$(LIB): $(OBJS) 
	$(CC) $(CFLAGS) $(BCM_BLT32_FLAGS) -shared -Wl,-soname,$(LIB) -o $(LIB) $(OBJS) -L$(LIBDIR) $(CMS_LIB_PATH) -lc $(LIBS)

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
