all: sae
include ../rules.include

CFLAGS += -DCONFIG_SHA256
CFLAGS += -DCONFIG_ECC

LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/utils/libutils.a

OBJS += $(SRC)/crypto/crypto_openssl.o
LIBS += -lcrypto
OBJS += $(SRC)/crypto/dh_groups.o
OBJS += $(SRC)/crypto/sha256-prf.o
OBJS += $(SRC)/crypto/sha256-kdf.o
OBJS += $(SRC)/common/dragonfly.o

sae: sae.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(MAKE) -C $(SRC) clean
	rm -f sae *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
