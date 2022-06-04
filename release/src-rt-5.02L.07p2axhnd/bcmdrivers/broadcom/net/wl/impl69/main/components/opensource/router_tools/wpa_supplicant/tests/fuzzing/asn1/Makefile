all: asn1
include ../rules.include

OBJS += $(SRC)/utils/common.o
OBJS += $(SRC)/utils/os_unix.o
OBJS += $(SRC)/utils/wpa_debug.o
OBJS += $(SRC)/utils/wpabuf.o
OBJS += $(SRC)/tls/asn1.o

asn1: asn1.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS) $(ELIBS)

clean:
	$(MAKE) -C $(SRC) clean
	$(MAKE) -C $(WPAS_SRC) clean
	rm -f asn1 *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
