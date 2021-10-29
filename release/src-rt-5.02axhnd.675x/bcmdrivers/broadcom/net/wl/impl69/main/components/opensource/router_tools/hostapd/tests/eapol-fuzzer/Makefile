all: eapol-fuzzer

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

eapol-fuzzer: eapol-fuzzer.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ -Wl,--start-group $(LIBS) -Wl,--end-group

clean:
	$(MAKE) -C $(SRC) clean
	rm -f eapol-fuzzer *~ *.o *.d

-include $(OBJS:%.o=%.d)
