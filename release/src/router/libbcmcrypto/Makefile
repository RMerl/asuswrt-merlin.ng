# libbcmcrypto
include $(TOP)/.config

vpath %.c $(SRCBASE)/bcmcrypto
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
 ifeq ($(HND_ROUTER_AX),y)
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.hnd_ax
 else
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.hnd
 endif
else
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.arm
endif
else ifeq ($(RTCONFIG_RALINK),y)
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.mipsel
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.qca_arm
else ifeq ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074)),y)
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.qca.ipq806x
else ifeq ($(RTCONFIG_QCA),y)
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.qca	# QCA MIPS
else ifeq ($(RTCONFIG_ALPINE),y)
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.alpine
else ifeq ($(RTCONFIG_LANTIQ),y)
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.lantiq
else
vpath %.o $(SRCBASE)/router/libbcmcrypto/prebuilt.mipsel
endif

ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
OBJS := aes.o aeskeywrap.o rijndael-alg-fst.o dh.o bn.o passhash.o md5.o rc4.o random.o sha2.o sha2x.o
else
OBJS := aes.o aeskeywrap.o rijndael-alg-fst.o dh.o bn.o sha1.o passhash.o prf.o md5.o hmac.o rc4.o random.o
endif
OBJS += sha256.o hmac_sha256.o

# Find all ELF binaries (including extra libraries)
BINARIES := $(shell find $(TARGETDIR) -path $(TARGETDIR)/lib -prune -o -type f -print | file -f - | grep ELF | cut -d':' -f1)
MAP := .map
UNR := .unresolved
SYM := .symbols

all: libbcmcrypto.so

install: all
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 libbcmcrypto.so $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/libbcmcrypto.so

clean:
	rm -f *.o *.so *.a $(MAP) $(UNR) $(SYM)

libbcmcrypto.so: $(OBJS)
	$(LD) -shared -o $@ $^
