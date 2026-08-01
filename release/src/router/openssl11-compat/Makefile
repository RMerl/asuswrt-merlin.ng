include ../common.mak

CFLAGS += -fPIC -Os -Wall $(EXTRACFLAGS) -Wno-deprecated-declarations \
	-D_FORTIFY_SOURCE=2 -fstack-protector-strong -fvisibility=hidden \
	-fno-semantic-interposition -Wformat -Wformat-security \
	-I$(TOP)/openssl-3.5/include
LIBS := -ldl
SHARED_FLAGS := $(EXTRALDFLAGS) -shared -Wl,--gc-sections -Wl,--as-needed \
	-Wl,-Bsymbolic-functions -Wl,-z,defs -Wl,-z,relro -Wl,-z,now \
	-Wl,-z,noexecstack

all: libcrypto.so.1.1 libssl.so.1.1

crypto_compat.o: crypto_compat.c
	$(CC) $(CFLAGS) -c $< -o $@

ssl_compat.o: ssl_compat.c
	$(CC) $(CFLAGS) -c $< -o $@

libcrypto.so.1.1: crypto_compat.o libcrypto.map
	$(CC) $(SHARED_FLAGS) -Wl,-soname,libcrypto.so.1.1 -Wl,--version-script=libcrypto.map -o $@ crypto_compat.o $(LIBS)

libssl.so.1.1: ssl_compat.o libssl.map
	$(CC) $(SHARED_FLAGS) -Wl,-soname,libssl.so.1.1 -Wl,--version-script=libssl.map -o $@ ssl_compat.o $(LIBS)

abi_smoke: all abi_smoke.c
	$(CC) $(CFLAGS) -o $@ abi_smoke.c -L. -Wl,-rpath,'$$ORIGIN' -l:libssl.so.1.1 -l:libcrypto.so.1.1 -ldl

check: abi_smoke
	@if echo "$(CC)" | grep -q "arm-buildroot-linux-gnueabi"; then \
		echo "openssl11-compat: built abi_smoke for ARM; run it on the target image as the acceptance check."; \
	else \
		./abi_smoke; \
	fi

stage: check
	install -D libcrypto.so.1.1 $(DESTDIR)/usr/lib/libcrypto.so.1.1
	install -D libssl.so.1.1 $(DESTDIR)/usr/lib/libssl.so.1.1

install: all
	install -D libcrypto.so.1.1 $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libcrypto.so.1.1
	install -D libssl.so.1.1 $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libssl.so.1.1
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libcrypto.so.1.1)
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(INSTALLDIR)$(INSTALLSUBDIR)/usr/lib/libssl.so.1.1)

clean:
	rm -f *.o libcrypto.so.1.1 libssl.so.1.1 abi_smoke
