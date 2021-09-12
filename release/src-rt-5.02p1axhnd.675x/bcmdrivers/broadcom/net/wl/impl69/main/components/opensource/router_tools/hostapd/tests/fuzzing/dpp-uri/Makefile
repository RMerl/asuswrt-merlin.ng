all: dpp-uri
include ../rules.include

CFLAGS += -DCONFIG_DPP
CFLAGS += -DCONFIG_DPP2
CFLAGS += -DCONFIG_SHA256
CFLAGS += -DCONFIG_SHA384
CFLAGS += -DCONFIG_SHA512
CFLAGS += -DCONFIG_ECC
CFLAGS += -DCONFIG_OPENSSL_CMAC

LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/utils/libutils.a

OBJS += $(SRC)/crypto/crypto_openssl.o
LIBS += -lcrypto

OBJS += $(SRC)/crypto/aes-ctr.o
OBJS += $(SRC)/crypto/aes-siv.o
OBJS += $(SRC)/crypto/sha256-kdf.o
OBJS += $(SRC)/crypto/sha384-kdf.o
OBJS += $(SRC)/crypto/sha512-kdf.o
OBJS += $(SRC)/tls/asn1.o
OBJS += $(SRC)/common/dpp.o

dpp-uri: dpp-uri.o $(OBJS) $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(MAKE) -C $(SRC) clean
	rm -f dpp-uri *~ *.o *.d ../*~ ../*.o ../*.d

-include $(OBJS:%.o=%.d)
