CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
WIFI_API :=wifi_api
WIFI_HAL :=libwifi_hal.a
WIFIHAL_TEST :=wifihal_test
default: install

include $(BUILD_DIR)/make.common

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
   LIBS += -lwlmdm
  endif
 endif
else
 LIBS += -lwlsysutil
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread -lbcm_util -lsys_util
 endif
endif

ifeq ($(strip $(BCA_CPEROUTER)),)
LIBS += -lwlctl -lnvram -lm  -lwlbcmshared
else
LIBS += -lnvram -lm -lshared
CFLAGS+= -DDSLCPE_WLCONF_CMD
endif
LIBS += -lbcm_flashutil -lbcm_boardctl -lgen_util -lbcm_util -lsys_util


ifneq ($(strip $(RDK_BUILD)),)
CFLAGS += -DRDK_BUILD -DBCA_CPEROUTER_RDK
endif

CFLAGS = -I./wlcsm/include -I. -DBUILD_RDKWIFI -Wno-unused-variable -Wno-unused-function -Wno-return-type
CFLAGS += -DRDKB_WLDM
CFLAGS += -DSKY_WLMNGR_SUPPORT_ENABLED
CFLAGS +=-s -Os -fomit-frame-pointer -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)  -I./core  -I.
CFLAGS += -DWLAN_BCG_CM_LINUX  -DWLAN_BCG_CM_RDKB
CFLAGS += -I$(TOOLCHAIN)/include  -lgcc_s -I./bsp_wlcsm/include -I./include
CFLAGS += -fno-strict-aliasing -fPIC
LDFLAGS=-Os -Wl,-allow-shlib-undefined  -lpthread
CFLAGS_EXTRA =
ifeq ($(ARCH), arm)
EXTRA_LDFLAGS += -z muldefs
endif
CFLAGS += -I$(TOOLCHAIN)/include  -lgcc_s -I./include
CFLAGS += -fno-strict-aliasing -fPIC
LDFLAGS=-Os -Wl,-allow-shlib-undefined  -lpthread $(LIBS)
CFLAGS_EXTRA =
ifeq ($(ARCH), arm)
EXTRA_LDFLAGS += -z muldefs
endif
CFLAGS += $(SSP_TYP_COMPILER_OPTS) $(SSP_TYP_LIBS) $(SSP_LIB_INC)
EXTRA_LDFLAGS += $(SSP_TYP_LIBS)  $(SSP_LIB_INC)
EXTRA_LDFLAGS += -L$(INSTALL_DIR)/lib -lwlcsm
WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/sys/src
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
SHARED_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/shared
SHARED_WIRELESS_ROUTER_PATH=$(WIRELESS_DRIVER_PATH)/../../main/src/router/shared
BCMWIFI_WIRELESS_ROUTER_PATH=$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/src
WLPPR_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/wl/ppr/src
COMM_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/../components/shared
CFLAGS +=-Wall -I$(INC_WIRELESS_DRIVER_PATH) -I$(COMM_WIRELESS_DRIVER_PATH) -I$(COMM_WIRELESS_DRIVER_PATH)/proto -DDSLCPE -DD11AC_IOTYPES -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include -I$(WIRELESS_DRIVER_PATH)/wl/ppr/include -DPPR_API -DDSLCPE_ENDIAN
CFLAGS += -I$(WIRELESS_DRIVER_PATH)/../../main/components/proto/include
CFLAGS += -I$(SHARED_WIRELESS_ROUTER_PATH) -I$(WIRELESS_DRIVER_PATH)/../../main/src/include
CFLAGS_EXTRA = -fpic
CFLAGS += -DDSLCPE -DDSLCPE_SCBLIST

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

WIFIHAL_TEST_FILES = ./wifi_hal_test.c
WIFIHAL_TEST_OBJS := $(foreach x, $(WIFIHAL_TEST_FILES),$(x:.c=.o))

WIFI_API_FILES = ./wifi_hal/wifi_api.c
WIFI_API_OBJS := $(foreach x, $(WIFI_API_FILES),$(x:.c=.o))
WIFI_API_FILES = ./wifi_hal/wifi_api.c
WIFI_API_OBJS := $(foreach x, $(WIFI_API_FILES),$(x:.c=.o))
$(WIFI_API_OBJS) $(WIFI_HAL_OBJS):./wifi_hal/%.o:./wifi_hal/%.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) $(WIFI_API_INCLUDE) -o $@ $<

$(WIFI_API): $(WIFI_API_OBJS)  $(WIFI_HAL)
	 $(CC) -o $(WIFI_API) $(WIFI_API_OBJS) $(WIFI_HAL_OBJS) -s -Os -fomit-frame-pointer -L$(LIBDIR) $(EXTRA_LDFLAGS) $(WIFI_API_LIBS) -lc -lgcc_s   $(LIBS) $(LDFLAGS)

$(WIFIHAL_TEST): $(WIFIHAL_TEST_OBJS)  $(WIFI_HAL)
	 $(CC) -o $(WIFIHAL_TEST) $(WIFIHAL_TEST_OBJS) $(WIFI_HAL_OBJS) -s -Os -fomit-frame-pointer -L$(LIBDIR) $(EXTRA_LDFLAGS) $(WIFI_API_LIBS) -lc -lgcc_s  $(LDFLAGS)
#------------------------------------------------------------------------------------------------

install: all
	install -m 755 $(WIFI_API) $(INSTALL_DIR)/bin
	install -m 755 $(WIFIHAL_TEST) $(INSTALL_DIR)/bin
	#link /usr/local/etc/wlan to /etc/wlan for RDKB wifi_setup.sh 
	mkdir -p $(INSTALL_DIR)/usr/local/etc
	ln -sf /etc/wlan  $(INSTALL_DIR)/usr/local/etc/wlan

all: $(WIFI_API)  $(WIFIHAL_TEST)

clean:
	rm -f $(WLCSM_OBJS) *.a
	rm -f $(WIFI_API_OBJS) $(WLDM_OBJS) $(WIFIHAL_TEST_OBJS) $(WIFI_HAL_OBJS)
	rm -f $(WIFIHAL_TEST_OBJS)
	rm -f $(WIFI_API) $(WIFIHAL_TEST)
