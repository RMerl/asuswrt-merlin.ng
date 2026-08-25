include ../common.mak

CFLAGS += -fPIC -Os -Wall $(EXTRACFLAGS) -Wno-deprecated-declarations \
	-D_FORTIFY_SOURCE=2 -fstack-protector-strong -fvisibility=hidden \
	-fno-semantic-interposition -Wformat -Wformat-security \
	-I$(TOP)/openssl-3.5/include
LIBS := -ldl
SHARED_FLAGS := $(EXTRALDFLAGS) -shared -Wl,--gc-sections -Wl,--as-needed \
	-Wl,-Bsymbolic-functions -Wl,-z,defs -Wl,-z,relro -Wl,-z,now \
	-Wl,-z,noexecstack

all: libcrypto.so.1.0.0 libssl.so.1.0.0

crypto_compat.o: crypto_compat.c
	$(CC) $(CFLAGS) -c $< -o $@

ssl_compat.o: ssl_compat.c
	$(CC) $(CFLAGS) -c $< -o $@

libcrypto.so.1.0.0: crypto_compat.o libcrypto.map
	$(CC) $(SHARED_FLAGS) -Wl,-soname,libcrypto.so.1.0.0 \
		-Wl,--version-script=libcrypto.map -o $@ crypto_compat.o $(LIBS)

libssl.so.1.0.0: ssl_compat.o libssl.map
	$(CC) $(SHARED_FLAGS) -Wl,-soname,libssl.so.1.0.0 \
		-Wl,--version-script=libssl.map -o $@ ssl_compat.o $(LIBS)

stage: all
	install -D libcrypto.so.1.0.0 $(DESTDIR)/usr/lib/libcrypto.so.1.0.0
	install -D libssl.so.1.0.0 $(DESTDIR)/usr/lib/libssl.so.1.0.0

install: all
	install -D libcrypto.so.1.0.0 $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libcrypto.so.1.0.0
	install -D libssl.so.1.0.0 $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libssl.so.1.0.0
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libcrypto.so.1.0.0)
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libssl.so.1.0.0)

clean:
	rm -f *.o libcrypto.so.1.0.0 libssl.so.1.0.0
