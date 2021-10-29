all: wnm-fuzzer

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
CFLAGS += -DCONFIG_WNM
CFLAGS += -DCONFIG_INTERWORKING
CFLAGS += -DCONFIG_GAS
CFLAGS += -DCONFIG_HS20
CFLAGS += -DIEEE8021X_EAPOL

$(SRC)/utils/libutils.a:
	$(MAKE) -C $(SRC)/utils

$(SRC)/common/libcommon.a:
	$(MAKE) -C $(SRC)/common

$(SRC)/crypto/libcrypto.a:
	$(MAKE) -C $(SRC)/crypto

$(SRC)/tls/libtls.a:
	$(MAKE) -C $(SRC)/tls

$(SRC)/rsn_supp/librsn_supp.a:
	$(MAKE) -C $(SRC)/rsn_supp

$(SRC)/eapol_supp/libeapol_supp.a:
	$(MAKE) -C $(SRC)/eapol_supp

$(SRC)/eap_peer/libeap_peer.a:
	$(MAKE) -C $(SRC)/eap_peer

$(SRC)/eap_common/libeap_common.a:
	$(MAKE) -C $(SRC)/eap_common

$(SRC)/l2_packet/libl2_packet.a:
	$(MAKE) -C $(SRC)/l2_packet

LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/tls/libtls.a
LIBS += $(SRC)/rsn_supp/librsn_supp.a
LIBS += $(SRC)/eapol_supp/libeapol_supp.a
LIBS += $(SRC)/eap_peer/libeap_peer.a
LIBS += $(SRC)/eap_common/libeap_common.a
LIBS += $(SRC)/l2_packet/libl2_packet.a
LIBS += $(SRC)/utils/libutils.a

ELIBS += $(SRC)/crypto/libcrypto.a
ELIBS += $(SRC)/tls/libtls.a

CFLAGS += -I$(SRC)/utils
OBJS += ../../wpa_supplicant/wnm_sta.o
OBJS += ../../wpa_supplicant/bss.o
OBJS += ../../wpa_supplicant/scan.o
OBJS += ../../wpa_supplicant/notify.o
OBJS += ../../wpa_supplicant/wpa_supplicant.o
OBJS += ../../wpa_supplicant/config.o
OBJS += ../../wpa_supplicant/config_file.o
OBJS += ../../wpa_supplicant/blacklist.o
OBJS += ../../wpa_supplicant/events.o
OBJS += ../../wpa_supplicant/wpas_glue.o
OBJS += ../../wpa_supplicant/wmm_ac.o
OBJS += ../../wpa_supplicant/eap_register.o
OBJS += ../../wpa_supplicant/gas_query.o
OBJS += ../../wpa_supplicant/offchannel.o
OBJS += ../../wpa_supplicant/interworking.o
OBJS += ../../wpa_supplicant/hs20_supplicant.o
OBJS += ../../wpa_supplicant/rrm.o
OBJS += ../../wpa_supplicant/op_classes.o
OBJS += $(SRC)/drivers/drivers.o
OBJS += $(SRC)/drivers/driver_common.o

wnm-fuzzer: wnm-fuzzer.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS) $(ELIBS)

clean:
	$(MAKE) -C $(SRC) clean
	$(MAKE) -C ../../wpa_supplicant clean
	rm -f wnm-fuzzer *~ *.o *.d

-include $(OBJS:%.o=%.d)
