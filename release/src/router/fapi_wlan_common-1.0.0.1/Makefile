# fapi_wlan source Makefile

ifeq ($(wildcard $(SRCBASE)/router/fapi_wlan_common-1.0.0.1/*.c),)
all:
	-cp -f prebuilt/libfapiwlancommon.so ./

install:

clean:
else
PKG_NAME := libfapiwlancommon

CFLAGS := $(strip $(subst -DPACKAGE_ID=\"fapi_wlan_common\",,$(CFLAGS))) -DPACKAGE_ID=\"fapi_wlan_common\"
CFLAGS := $(strip $(subst -DLOGGING_ID="fapi_wlan_common",,$(CFLAGS))) -DLOGGING_ID="fapi_wlan_common"

opt_no_flags := -Werror -Wcast-qual

COMMON_WAVE_BUILD = y
ifneq ($(COMMON_WAVE_BUILD),y)
	bins := libfapiwlancommon.so
	libfapiwlancommon.so_sources := fapi_wlan.c
	libfapiwlancommon.so_cflags  := -I./include -I$(TOP)/libhelper-1.4.0.2/include
	libfapiwlancommon.so_ldflags := -L./ -L$(TOP)/libhelper-1.4.0.2
else
	# bins := libfapiwlancommon.so fapi_wlan_cli fapi_wlan_beerock_cli fapi_wlan_dbXml
	bins := libfapiwlancommon.so fapi_wlan_cli fapi_wlan_dbXml
	libfapiwlancommon.so_sources := fapi_wlan.c fapi_wlan_beerock.c fapi_wlan_beerock_aml.c wlan_config_server_api.c fapi_wlan_tools.c
	libfapiwlancommon.so_cflags  := -I./include -I$(TOP)/libhelper-1.4.0.2/include -DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX
	libfapiwlancommon.so_ldflags := -L./ -L$(TOP)/libhelper-1.4.0.2

	fapi_wlan_cli_sources := fapi_wlan_cli.c
	fapi_wlan_cli_ldflags := -L./ -lfapiwlancommon -ldl -L$(TOP)/libhelper-1.4.0.2 -lhelper -lrt
	fapi_wlan_cli_cflags  := -I./include -I$(TOP)/libhelper-1.4.0.2/include

	fapi_wlan_beerock_cli_sources := ../hostapd-2.6/src/common/wpa_ctrl.c ../hostapd-2.6/src/utils/os_unix.c fapi_wlan_beerock_cli.c
	fapi_wlan_beerock_cli_ldflags := -L./ -lfapiwlancommon -ldl -lhelper -lcurses -lreadline -lrt
	fapi_wlan_beerock_cli_cflags  := -I./include -I../hostapd-2.6/src/common/ -I../hostapd-2.6/src/utils/ -DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX

	fapi_wlan_dbXml_sources := fapi_wlan_dbXml.c fapi_wlan_ezxml.c
	fapi_wlan_dbXml_ldflags := -L./  -lhelper -L$(TOP)/libhelper-1.4.0.2
	fapi_wlan_dbXml_cflags  := -I./include -I$(TOP)/libhelper-1.4.0.2/include
endif

include make.inc
endif
