CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
WIFI_API :=wifi_api
WIFI_HAL :=libwifi_hal.a
default: install

include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan

ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
MDM_CORE_LIBS += -lmdm_cbk_wifi
else
MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif

LIBS = -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/usr/lib -ldl -lwldm -lrt

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
 LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_msg $(CMS_COMMON_LIBS) $(MDM_CORE_LIBS)
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread
  ifneq ($(strip $(BUILD_CUSTOMER)),)
   LIBS += -lwlmdm_customer
  else
   LIBS += -lwlmdm -lpcre2-8
  endif
 endif
else
 LIBS += -lwlsysutil
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread -lbcm_util -lsys_util
 endif
endif

ifneq ($(strip $(BCA_CPEROUTER)),)
LIBS += -lnvram -lm -lshared
endif
LIBS += -lbcm_flashutil -lbcm_boardctl -lgen_util -lbcm_util -lsys_util


ifneq ($(strip $(RDK_BUILD)),)
CFLAGS += -DBCA_CPEROUTER_RDK
endif

CFLAGS += -I./wlcsm/include -I. -DBUILD_RDKWIFI -Wno-unused-variable -Wno-unused-function -Wno-return-type -I$(USERSPACE_PUBLIC_LIBS_DIR)/../include
CFLAGS += -DRDKB_WLDM
CFLAGS += -DSKY_WLMNGR_SUPPORT_ENABLED
CFLAGS +=-s -Os -fomit-frame-pointer -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)  -I./core  -I.
CFLAGS += -DWLAN_BCG_CM_LINUX  -DWLAN_BCG_CM_RDKB
CFLAGS += -I$(TOOLCHAIN)/include  $(LIBS_CC_RT) -I./bsp_wlcsm/include -I./include
CFLAGS += -fno-strict-aliasing -fPIC
LDFLAGS=-Os -Wl,-allow-shlib-undefined  -lpthread
CFLAGS_EXTRA =
ifeq ($(USER_ARCH), arm)
EXTRA_LDFLAGS += -z muldefs
endif
CFLAGS += -I$(TOOLCHAIN)/include  $(LIBS_CC_RT) -I./include
CFLAGS += -fno-strict-aliasing -fPIC
LDFLAGS=-Os -Wl,-allow-shlib-undefined  -lpthread $(LIBS)
CFLAGS_EXTRA =
ifeq ($(USER_ARCH), arm)
EXTRA_LDFLAGS += -z muldefs
endif
CFLAGS += $(SSP_TYP_COMPILER_OPTS) $(SSP_TYP_LIBS) $(SSP_LIB_INC)
EXTRA_LDFLAGS += $(SSP_TYP_LIBS)  $(SSP_LIB_INC)
EXTRA_LDFLAGS += -L$(INSTALL_DIR)/lib -lwlcsm
SHARED_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/shared
SHARED_WIRELESS_ROUTER_PATH=$(WIRELESS_DRIVER_PATH)/../../main/src/router/shared
BCMWIFI_WIRELESS_ROUTER_PATH=$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/src
WLPPR_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/wl/ppr/src
CFLAGS +=-Wall -I$(INC_WIRELESS_DRIVER_PATH) -DDSLCPE -DD11AC_IOTYPES -I$(WIRELESS_DRIVER_PATH)/../components/bcmwifi/include -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include -I$(WIRELESS_DRIVER_PATH)/wl/ppr/include -DPPR_API
CFLAGS += -I$(COMPON_WIRELESS_DRIVER_PATH)/proto/include
CFLAGS += -I$(SHARED_WIRELESS_ROUTER_PATH) -I$(WIRELESS_DRIVER_PATH)/src/include
CFLAGS_EXTRA = -fpic $(USR_CFLAGS)

.PHONY: all $(WIFI_API)

### wifi_hal objs #########################
WIFI_HAL_FILES = ./wifi_hal/wifi_hal.c 
WIFI_HAL_OBJS := $(foreach x, $(WIFI_HAL_FILES),$(x:.c=.o))
#$(WIFI_HAL_OBJS):./wifi_hal/%.o:./wifi_hal/%.c
#	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) $(WIFI_API_INCLUDE) -o $@ $<
$(WIFI_HAL): $(WIFI_HAL_OBJS)

#---------------------wifi_api application -----------------------------------------------------
WIFI_API_LIBS := -lwlcsm -lnvram -lpthread
WIFI_API_INCLUDE := -I../components/apps/wldm


WIFI_API_FILES = ./wifi_hal/wifi_api.c
WIFI_API_OBJS := $(foreach x, $(WIFI_API_FILES),$(x:.c=.o))
WIFI_API_FILES = ./wifi_hal/wifi_api.c
WIFI_API_OBJS := $(foreach x, $(WIFI_API_FILES),$(x:.c=.o))
$(WIFI_API_OBJS) $(WIFI_HAL_OBJS):./wifi_hal/%.o:./wifi_hal/%.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) $(WIFI_API_INCLUDE) -o $@ $<

$(WIFI_API): $(WIFI_API_OBJS)  $(WIFI_HAL)
	 $(CC) -o $(WIFI_API) $(WIFI_API_OBJS) $(WIFI_HAL_OBJS) -s -Os -fomit-frame-pointer -L$(LIBDIR) $(EXTRA_LDFLAGS) $(WIFI_API_LIBS) -lc $(LIBS_CC_RT)   $(LIBS) $(LDFLAGS)

#------------------------------------------------------------------------------------------------

install: all
	install -m 755 $(WIFI_API) $(INSTALL_DIR)/bin
	#link /usr/local/etc/wlan to /etc/wlan for RDKB wifi_setup.sh 
	mkdir -p $(INSTALL_DIR)/usr/local/etc
	ln -sf /etc/wlan  $(INSTALL_DIR)/usr/local/etc/wlan

all: $(WIFI_API)

clean:
	rm -f $(WLCSM_OBJS) *.a
	rm -f $(WIFI_API_OBJS) $(WLDM_OBJS) $(WIFI_HAL_OBJS)
	rm -f $(WIFI_API)
