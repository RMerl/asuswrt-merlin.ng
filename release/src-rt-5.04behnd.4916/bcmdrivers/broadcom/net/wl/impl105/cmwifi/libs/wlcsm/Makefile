CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

default_bld:all
-include $(BUILD_DIR)/make.common

export TOP=$(WLSRCBASE)/router
export LINUXDIR=$(KERNEL_DIR)

INCLUDE += -I$(WL_CMWIFI_DIR)/mods/wlcsm_ext/include
INCLUDE += -I$(WLSRCBASE)/../components/wlioctl/include
INCLUDE += -I$(WLSRCBASE)/../components/proto/include
INCLUDE += -I$(WLSRCBASE)/wl/wlcsm_ext/include -I./include
INCLUDE += -I. -I.. -I$(WLSRCBASE)/include -I$(WLSRCBASE)/common/include -I$(WLSRCBASE)/shared -I$(WLSRCBASE)/router/shared -I$(WLSRCBASE)/wl/exe -I$(WLSRCBASE)/../components/shared
INCLUDE += -I$(WLSRCBASE)/security/secfrw/include -I$(WLSRCBASE)/supp/include -I$(WLSRCBASE)/shared/bcmwifi/include

ifneq ($(SUPPORT_NO_WIFI_CMS),y)
INCLUDE += -I$(BUILD_DIR)/userspace/public/include/linux
INCLUDE += -I$(BUILD_DIR)/userspace/public/include
endif
ifneq ($(strip $(BUILD_RDKWIFI)),)
INCLUDE += $(CFLAGS)
CFLAGS := $(INCLUDE)
else
CFLAGS += $(INCLUDE)
endif
CFLAGS += -Wall
CFLAGS += -fPIC

DSLCPE_WLCSM_DEBUG :=0
# enable WLCSM DEBUG also requires to enable WLCSM_DEBUG in kernel file header wlcsm_ext/wlcsm_linux.h
ifeq ($(strip $(DSLCPE_WLCSM_DEBUG)),1)
CFLAGS  += -g
endif
CFLAGS += -DCMWIFI $(USR_CFLAGS)
CFLAGS += -DBCM_CSIMON_AP

LDFLAGS += -L. -L$(INSTALL_DIR)/lib -Wl,-rpath,$(CMS_LIB_RPATH) $(CMS_LIB_PATH) $(USR_LDFLAGS)

vpath %.c $(CURR_DIR)/src
vpath %.c $(WLSRCBASE)/shared/bcmwifi/src/ $(WLSRCBASE)/shared/ $(WLSRCBASE)/../components/bcmhal/src/

WLCSM_FILES = $(foreach x, $(shell find ./src -name \*.c) ,$(notdir $(x)))
WLCSM_OBJS = $(foreach x, $(WLCSM_FILES),$(x:.c=.o))

ifneq ($(strip $(BUILD_RDKWIFI)),)
OBJS = $(filter-out $(BSP_WLCSM_OBJS), $(WLCSM_OBJS)) bcmwifi_channels.o bcm_app_utils.o bcmxtlv.o
else
OBJS += $(WLCSM_OBJS) bcmwifi_channels.o bcm_app_utils.o
endif

STATIC_OBJS := $(filter-out wlcsm_lib_hspot.o, $(OBJS))
all: libwlcsm.so  libwlcsm.a

ifneq ($(strip $(WLCSM_FILES)),)
libwlcsm.so: $(OBJS)
ifeq ($(strip $(BUILD_RDKWIFI)),)
	$(CC) -Os -fPIC -shared -o libwlcsm.so $(OBJS) -lpthread
else
	$(AR) rv libwlcsm_cm.a $(OBJS)
	$(RANLIB) libwlcsm_cm.a
	$(CC) -fPIC -shared -o libwlcsm.so -Wl,--whole-archive libwlcsm.a  libwlcsm_cm.a -Wl,--no-whole-archive -lpthread
	#$(CC) -fPIC -shared -o libwlcsm.so -Wl,--whole-archive libwlcsm_all.a -Wl,--no-whole-archive -lpthread
endif
else
libwlcsm.so:
	cp -f libwlcsm-$(EXT_CPU_ARCH_NAME).so libwlcsm.so
endif
	install -m 755 libwlcsm.so $(INSTALL_DIR)/lib

install: all
	install -m 755 libwlcsm.so $(INSTALL_DIR)/lib

libwlcsm.a: $(STATIC_OBJS)
ifeq ($(strip $(BUILD_RDKWIFI)),)
	$(AR) -cvq $@ $(STATIC_OBJS)
else
	$(info "not needed")
endif

distclean clean:
	-rm -f *.o *.map *.d  *.d.* *.a
	-rm -f libwlcsm.so make.deprules
	-rm -rf doc
	-rm -rf $(CURR_DIR)/include/wlcsm_linux.h
	-rm -rf $(BUILD_DIR)/userspace/private/include/wlcsm_lib_api.h
	-rm -rf $(BUILD_DIR)/userspace/private/include/wlcsm_defs.h
	-rm -f $(INSTALL_DIR)/lib/libwlcsm.so
#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

-include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)
