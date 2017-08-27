LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# the executables that should be installed on the final system have to be added
# to PRODUCT_PACKAGES in
#   build/target/product/core.mk
# possible executables are
#   starter - allows to control and configure the daemon from the command line
#   charon - the IKE daemon
#   scepclient - SCEP client

# if you enable starter or scepclient (see above) uncomment the proper
# lines here
# strongswan_BUILD_STARTER := true
# strongswan_BUILD_SCEPCLIENT := true

# this is the list of plugins that are built into libstrongswan and charon
# also these plugins are loaded by default (if not changed in strongswan.conf)
strongswan_CHARON_PLUGINS := android-log openssl fips-prf random nonce pubkey \
	pkcs1 pkcs8 pem xcbc hmac kernel-netlink socket-default android-dns \
	stroke eap-identity eap-mschapv2 eap-md5 eap-gtc

ifneq ($(strongswan_BUILD_SCEPCLIENT),)
# plugins loaded by scepclient
strongswan_SCEPCLIENT_PLUGINS := openssl curl fips-prf random pkcs1 pkcs7 pem
endif

strongswan_STARTER_PLUGINS := kernel-netlink

# list of all plugins - used to enable them with the function below
strongswan_PLUGINS := $(sort $(strongswan_CHARON_PLUGINS) \
			     $(strongswan_STARTER_PLUGINS) \
			     $(strongswan_SCEPCLIENT_PLUGINS))

include $(LOCAL_PATH)/Android.common.mk

# includes
strongswan_PATH := $(LOCAL_PATH)
libcurl_PATH := external/strongswan-support/libcurl/include
libgmp_PATH := external/strongswan-support/gmp
openssl_PATH := external/openssl/include

# some definitions
strongswan_DIR := "/system/bin"
strongswan_SBINDIR := "/system/bin"
strongswan_PIDDIR := "/data/misc/vpn"
strongswan_PLUGINDIR := "$(strongswan_IPSEC_DIR)/ipsec"
strongswan_CONFDIR := "/system/etc"
strongswan_STRONGSWAN_CONF := "$(strongswan_CONFDIR)/strongswan.conf"

# CFLAGS (partially from a configure run using droid-gcc)
strongswan_CFLAGS := \
	-Wno-format \
	-Wno-pointer-sign \
	-Wno-pointer-arith \
	-Wno-sign-compare \
	-Wno-strict-aliasing \
	-DHAVE___BOOL \
	-DHAVE_STDBOOL_H \
	-DHAVE_ALLOCA_H \
	-DHAVE_ALLOCA \
	-DHAVE_CLOCK_GETTIME \
	-DHAVE_DLADDR \
	-DHAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC \
	-DHAVE_PRCTL \
	-DHAVE_LINUX_UDP_H \
	-DHAVE_STRUCT_SADB_X_POLICY_SADB_X_POLICY_PRIORITY \
	-DHAVE_IPSEC_MODE_BEET \
	-DHAVE_IPSEC_DIR_FWD \
	-DOPENSSL_NO_EC \
	-DOPENSSL_NO_ECDSA \
	-DOPENSSL_NO_ECDH \
	-DOPENSSL_NO_ENGINE \
	-DCONFIG_H_INCLUDED \
	-DCAPABILITIES \
	-DCAPABILITIES_NATIVE \
	-DMONOLITHIC \
	-DUSE_IKEV1 \
	-DUSE_IKEV2 \
	-DUSE_BUILTIN_PRINTF \
	-DDEBUG \
	-DROUTING_TABLE=0 \
	-DROUTING_TABLE_PRIO=220 \
	-DVERSION=\"$(strongswan_VERSION)\" \
	-DPLUGINDIR=\"$(strongswan_PLUGINDIR)\" \
	-DIPSEC_DIR=\"$(strongswan_DIR)\" \
	-DIPSEC_PIDDIR=\"$(strongswan_PIDDIR)\" \
	-DIPSEC_CONFDIR=\"$(strongswan_CONFDIR)\" \
	-DSTRONGSWAN_CONF=\"$(strongswan_STRONGSWAN_CONF)\" \
	-DDEV_RANDOM=\"/dev/random\" \
	-DDEV_URANDOM=\"/dev/urandom\"

# only for Android 2.0+
strongswan_CFLAGS += \
	-DHAVE_IN6ADDR_ANY

strongswan_BUILD := \
	charon \
	libcharon \
	libhydra \
	libstrongswan \
	libtncif \
	libtnccs \
	libimcv

ifneq ($(strongswan_BUILD_STARTER),)
strongswan_BUILD += \
	starter \
	stroke \
	ipsec
endif

ifneq ($(strongswan_BUILD_SCEPCLIENT),)
strongswan_BUILD += \
	scepclient
endif

include $(addprefix $(LOCAL_PATH)/src/,$(addsuffix /Android.mk, \
		$(sort $(strongswan_BUILD))))
