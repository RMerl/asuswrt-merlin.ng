all: eap-mschapv2-peer
include ../rules.include

CFLAGS += -DIEEE8021X_EAPOL

OBJS += $(SRC)/eap_peer/eap_mschapv2.o
OBJS += $(SRC)/eap_peer/mschapv2.o
OBJS += $(SRC)/eap_common/eap_common.o
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/utils/libutils.a

eap-mschapv2-peer: eap-mschapv2-peer.o $(OBJS) $(LIBS)
	$(Q)$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS) $(ELIBS)
	@$(E) "  LD " $@

clean:
	$(MAKE) -C $(SRC) clean
	rm -f eap-mschapv2-peer *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
