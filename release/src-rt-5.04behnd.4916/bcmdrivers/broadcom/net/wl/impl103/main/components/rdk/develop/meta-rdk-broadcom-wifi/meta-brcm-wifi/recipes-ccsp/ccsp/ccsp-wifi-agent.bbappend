FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

#------------------------------------------------------
# Additional for RDKM
#------------------------------------------------------

LDFLAGS += "-lcjson"

# SWRDKB-465 - fix for wifi_restartHostApd
SRC_URI += "file://SWRDKB-465_wifi_restartHostApd.patch"

# SWRDKB-497 - fix missing wifi_api
SRC_URI += "file://SWRDKB-497_apshealth.patch"

# SWRDKB-1037: WiFi network is disabled in Web UI, even with Broadcom WiFi driver.
SRC_URI += "file://SWRDKB-1037_ccsp_wifi_agent.patch"

# fix for a compilation error for XB7
SRC_URI += "file://ccsp_wifi_agent_XB7_001.patch"

#------------------------------------------------------
# Additional CFLAGS
#------------------------------------------------------
CFLAGS += "-D_XB7_PRODUCT_REQ_"
