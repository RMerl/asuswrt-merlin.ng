all: x509
include ../rules.include

LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/tls/libtls.a
LIBS += $(SRC)/utils/libutils.a

ELIBS += $(SRC)/crypto/libcrypto.a
ELIBS += $(SRC)/tls/libtls.a

x509: x509.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS) $(ELIBS)

clean:
	$(MAKE) -C $(SRC) clean
	$(MAKE) -C $(WPAS_SRC) clean
	rm -f x509 *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
