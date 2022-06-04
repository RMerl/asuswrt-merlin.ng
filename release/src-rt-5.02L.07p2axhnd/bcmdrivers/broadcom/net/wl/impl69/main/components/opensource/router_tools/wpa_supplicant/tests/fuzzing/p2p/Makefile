all: p2p
include ../rules.include

LIBS += $(SRC)/utils/libutils.a
LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/p2p/libp2p.a
LIBS += $(SRC)/tls/libtls.a
LIBS += $(SRC)/wps/libwps.a

p2p: p2p.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(MAKE) -C $(SRC) clean
	rm -f p2p *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
