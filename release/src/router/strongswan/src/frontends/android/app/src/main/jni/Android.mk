LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# use "bring your own device" (BYOD) features (also see USE_BYOD in
# MainActivity.java)
strongswan_USE_BYOD := true

strongswan_CHARON_PLUGINS := android-log openssl fips-prf random nonce pubkey \
	chapoly curve25519 pkcs1 pkcs8 pem xcbc hmac socket-default revocation \
	eap-identity eap-mschapv2 eap-md5 eap-gtc eap-tls x509

ifneq ($(strongswan_USE_BYOD),)
strongswan_BYOD_PLUGINS := eap-ttls eap-tnc tnc-imc tnc-tnccs tnccs-20
endif

strongswan_PLUGINS := $(strongswan_CHARON_PLUGINS) \
	$(strongswan_BYOD_PLUGINS)

strongswan_DIR := ../../../../../../../

# includes
strongswan_PATH := $(LOCAL_PATH)/$(strongswan_DIR)
openssl_PATH := $(LOCAL_PATH)/openssl/include

include $(strongswan_PATH)/Android.common.mk

# CFLAGS (partially from a configure run using droid-gcc)
strongswan_CFLAGS := \
	-Wall \
	-Wextra \
	-Wno-format \
	-Wno-pointer-sign \
	-Wno-pointer-arith \
	-Wno-sign-compare \
	-Wno-strict-aliasing \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-Wno-self-assign \
	-DHAVE___BOOL \
	-DHAVE_STDBOOL_H \
	-DHAVE_ALLOCA_H \
	-DHAVE_ALLOCA \
	-DHAVE_CLOCK_GETTIME \
	-DHAVE_DLADDR \
	-DHAVE_PRCTL \
	-DHAVE_LINUX_UDP_H \
	-DHAVE_STRUCT_SADB_X_POLICY_SADB_X_POLICY_PRIORITY \
	-DHAVE_IPSEC_MODE_BEET \
	-DHAVE_IPSEC_DIR_FWD \
	-DHAVE_IN6ADDR_ANY \
	-DHAVE_NETINET_IP6_H \
	-DCONFIG_H_INCLUDED \
	-DMONOLITHIC \
	-DUSE_IKEV1 \
	-DUSE_IKEV2 \
	-DUSE_BUILTIN_PRINTF \
	-DDEBUG \
	-DCHARON_UDP_PORT=0 \
	-DCHARON_NATT_PORT=0 \
	-DVERSION=\"$(strongswan_VERSION)\" \
	-DDEV_RANDOM=\"/dev/random\" \
	-DDEV_URANDOM=\"/dev/urandom\"

ifneq ($(strongswan_USE_BYOD),)
strongswan_CFLAGS += -DUSE_BYOD
endif

strongswan_BUILD := \
	openssl \
	libandroidbridge \
	$(strongswan_DIR)/src/libipsec \
	$(strongswan_DIR)/src/libcharon \
	$(strongswan_DIR)/src/libstrongswan

ifneq ($(strongswan_USE_BYOD),)
strongswan_BUILD += \
	$(strongswan_DIR)/src/libtnccs \
	$(strongswan_DIR)/src/libtncif \
	$(strongswan_DIR)/src/libimcv \
	$(strongswan_DIR)/src/libtpmtss
endif

include $(addprefix $(LOCAL_PATH)/,$(addsuffix /Android.mk, \
		$(strongswan_BUILD)))
