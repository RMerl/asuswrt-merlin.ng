###############################################################################
#
#                          OPENWRT MAKE TARGETS
#
# Undefine some BUILD_xyz vars and define BCM_COND_HAVE_xyz.  
# We want to build the package on the Openwrt side,
# and use it on the BDK side (do not build another copy in the BDK side).
# In rare situations, if each side has specific version requirements, we may
# need to build both the Openwrt version and the BDK version.
# The eventual goal is to eliminate all duplicate package builds between
# Openwrt and BDK.
#
###############################################################################

ifneq ($(strip $(BUILD_BRCM_OPENWRT)),)

undefine DO_BUILD_OPENSSL
undefine BUILD_CERT
BCM_COND_HAVE_LIBCURL := y
#undefine DO_BUILD_EXPAT
#undefine BUILD_WEB_SOCKETS
undefine BUILD_LIBXML2
COND_HAVE_OPENSSL := y

undefine BUILD_VANILLA_BUSYBOX
BCM_COND_HAVE_VANILLA_BUSYBOX := y

undefine BUILD_LIBJSONC
BCM_COND_HAVE_LIBJSONC := y

undefine BUILD_UBUS
BCM_COND_HAVE_UBUS := y

undefine BUILD_LIBUBOX
BCM_COND_HAVE_LIBUBOX := y

endif  #BUILD_BRCM_OPENWRT


ifdef DEBUG_BCMWRT
$(info 1 BUILD_OPENWRT_NATIVE = $(BUILD_OPENWRT_NATIVE))
$(info 2 COND_HAVE_OPENSSL = $(COND_HAVE_OPENSSL))
endif


ifneq ($(strip $(BUILD_OPENWRT_NATIVE)),)
export BUILD_OPENWRT_NATIVE
undefine BUILD_DLNA
endif

ifneq ($(strip $(BUILD_PRPL_FEEDS)),)
export BUILD_PRPL_FEEDS
endif

