all: ap-mgmt-fuzzer

ifndef CC
CC=gcc
endif

ifndef LDO
LDO=$(CC)
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

SRC=../../src

CFLAGS += -I$(SRC)
CFLAGS += -I$(SRC)/utils
CFLAGS += -DCONFIG_WNM
CFLAGS += -DCONFIG_INTERWORKING
CFLAGS += -DCONFIG_GAS
CFLAGS += -DCONFIG_HS20
CFLAGS += -DIEEE8021X_EAPOL
CFLAGS += -DNEED_AP_MLME
CFLAGS += -DCONFIG_AIRTIME_POLICY

$(SRC)/utils/libutils.a:
	$(MAKE) -C $(SRC)/utils

$(SRC)/common/libcommon.a:
	$(MAKE) -C $(SRC)/common

$(SRC)/crypto/libcrypto.a:
	$(MAKE) -C $(SRC)/crypto

$(SRC)/tls/libtls.a:
	$(MAKE) -C $(SRC)/tls

$(SRC)/wps/libwps.a:
	$(MAKE) -C $(SRC)/wps

$(SRC)/eap_common/libeap_common.a:
	$(MAKE) -C $(SRC)/eap_common

$(SRC)/eap_server/libeap_server.a:
	$(MAKE) -C $(SRC)/eap_server

$(SRC)/l2_packet/libl2_packet.a:
	$(MAKE) -C $(SRC)/l2_packet

$(SRC)/eapol_auth/libeapol_auth.a:
	$(MAKE) -C $(SRC)/eapol_auth

$(SRC)/ap/libap.a:
	$(MAKE) -C $(SRC)/ap

$(SRC)/radius/libradius.a:
	$(MAKE) -C $(SRC)/radius

LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/tls/libtls.a
LIBS += $(SRC)/wps/libwps.a
LIBS += $(SRC)/eap_server/libeap_server.a
LIBS += $(SRC)/eap_common/libeap_common.a
LIBS += $(SRC)/l2_packet/libl2_packet.a
LIBS += $(SRC)/ap/libap.a
LIBS += $(SRC)/eapol_auth/libeapol_auth.a
LIBS += $(SRC)/radius/libradius.a
LIBS += $(SRC)/utils/libutils.a

ELIBS += $(SRC)/crypto/libcrypto.a
ELIBS += $(SRC)/tls/libtls.a

ap-mgmt-fuzzer: ap-mgmt-fuzzer.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS) $(ELIBS)

clean:
	$(MAKE) -C $(SRC) clean
	rm -f ap-mgmt-fuzzer *~ *.o *.d

-include $(OBJS:%.o=%.d)
