all: eapol-key-supp
include ../rules.include

CFLAGS += -DCONFIG_IEEE80211R_AP
CFLAGS += -DCONFIG_IEEE80211R
CFLAGS += -DCONFIG_TDLS

LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/tls/libtls.a
LIBS += $(SRC)/rsn_supp/librsn_supp.a
LIBS += $(SRC)/eapol_supp/libeapol_supp.a
LIBS += $(SRC)/eap_peer/libeap_peer.a
LIBS += $(SRC)/eap_common/libeap_common.a
LIBS += $(SRC)/l2_packet/libl2_packet.a
LIBS += $(SRC)/utils/libutils.a

eapol-key-supp: eapol-key-supp.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ -Wl,--start-group $(LIBS) -Wl,--end-group

clean:
	$(MAKE) -C $(SRC) clean
	rm -f eapol-key-supp *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
