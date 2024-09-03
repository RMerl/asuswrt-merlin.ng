FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

#------------------------------------------------------
# Additional for RDKM
#------------------------------------------------------

LDFLAGS += "-lcjson"
CFLAGS += "-DWIFI_HAL_VERSION_3"
CFLAGS += "-Wno-error=unused-but-set-variable"

# SWRDKB-465 - fix for wifi_restartHostApd
SRC_URI += "file://SWRDKB-465_wifi_restartHostApd.patch"

# SWRDKB-497 - fix missing wifi_api
SRC_URI += "file://SWRDKB-497_apshealth.patch"

# SWRDKB-1037: WiFi network is disabled in Web UI, even with Broadcom WiFi driver.
SRC_URI += "file://SWRDKB-1037_ccsp_wifi_agent.patch"

# fix for wifi_monitor
SRC_URI += "file://ccsp_wifi_agent_wifi_monitor.patch"

# fix for wifi_data_plane
SRC_URI += "file://ccsp_wifi_agent_wifi_data_plane.patch"

