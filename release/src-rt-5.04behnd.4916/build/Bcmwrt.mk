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
DEVKIT := $(shell echo $(OPENWRT_TAGVER))

undefine DO_BUILD_OPENSSL
undefine BUILD_CERT
BCM_COND_HAVE_LIBCURL := y
#undefine DO_BUILD_EXPAT
#undefine BUILD_WEB_SOCKETS
#undefine BUILD_LIBXML2
COND_HAVE_OPENSSL := y
BCM_COND_HAVE_LIBWEBSOCKETS := y

undefine BUILD_VANILLA_BUSYBOX
BCM_COND_HAVE_VANILLA_BUSYBOX := y

undefine BUILD_LIBJSONC
BCM_COND_HAVE_LIBJSONC := y

undefine BUILD_LIBJANSSON
BCM_COND_HAVE_LIBJANSSON := y

undefine BUILD_LIBNL
BCM_COND_HAVE_LIBNL := y

undefine BUILD_LIBMNL
BCM_COND_HAVE_LIBMNL := y

undefine BUILD_GPTFDISK
BCM_COND_HAVE_GPTFDISK := y

undefine BUILD_DNSMASQ
BCM_COND_HAVE_DNSMASQ := y

undefine BUILD_MICRO_HTTPD

ifneq ($(findstring v23, $(strip $(DEVKIT))),)
BCM_COND_HAVE_NFTABLES := y
endif

undefine BUILD_UBUS
BCM_COND_HAVE_UBUS := y

undefine BUILD_LIBUBOX
BCM_COND_HAVE_LIBUBOX := y

undefine BUILD_LIBUUID
BCM_COND_HAVE_LIBUUID := y

# In the mixed 32/64bits usersapce, we need 64bits zlib for elfutils/iproute2.
# The openwrt only supplies the 32bits zlib.
# Only do this with pure 64bits or 32bits userspace.
ifeq ($(PROFILE_KARCH),$(ORIG_PROFILE_ARCH))
undefine BUILD_ZLIB
BCM_COND_HAVE_ZLIB := y
endif
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
undefine BUILD_LIBPCAP
BCM_COND_HAVE_LIBPCAP := y

undefine BUILD_E2FSPROGS
BCM_COND_HAVE_E2FSPROGS :=y

undefine BUILD_OBUSPA
BCM_COND_HAVE_OBUSPA :=y

undefine BUILD_MOSQUITTO
BCM_COND_HAVE_MOSQUITTO :=y

undefine BUILD_LIBSQLITE
BCM_COND_HAVE_LIBSQLITE :=y

undefine BUILD_LIBNETFILTER_QUEUE
BCM_COND_HAVE_LIBNETFILTER_QUEUE :=y

undefine BUILD_LIBNETFILTER_CONNTRACK
BCM_COND_HAVE_LIBNETFILTER_CONNTRACK :=y

undefine BUILD_LIBNFNETLINK
BCM_COND_HAVE_LIBNFNETLINK :=y

# For apps/libs that are provided by PRPL, we should not build those from  BDK side.
# Includig tirpc, iptables and ebtables
BCM_COND_HAVE_PRPL_LIBS :=y
endif

