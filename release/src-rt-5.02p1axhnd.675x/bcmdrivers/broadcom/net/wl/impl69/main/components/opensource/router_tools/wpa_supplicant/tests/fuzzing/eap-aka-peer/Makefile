all: eap-aka-peer
include ../rules.include

CFLAGS += -DIEEE8021X_EAPOL
CFLAGS += -DCONFIG_USIM_SIMULATOR

OBJS += $(SRC)/eap_peer/eap_aka.o
OBJS += $(SRC)/eap_common/eap_sim_common.o
OBJS += $(SRC)/eap_common/eap_common.o
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/utils/libutils.a

eap-aka-peer: eap-aka-peer.o $(OBJS) $(LIBS)
	$(Q)$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS) $(ELIBS)
	@$(E) "  LD " $@

clean:
	$(MAKE) -C $(SRC) clean
	rm -f eap-aka-peer *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
