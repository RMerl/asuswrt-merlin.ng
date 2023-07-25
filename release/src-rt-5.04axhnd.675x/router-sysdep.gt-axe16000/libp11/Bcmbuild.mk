LIB=libp11-libp11-0.4.10

all install: conditional_build

ifeq ($(strip $(BCM_OPTEE)),y)
conditional_build: libp11
clean: libp11_clean
else
conditional_build clean: ;
endif

libp11:
	$(MAKE) -f Makefile install
	@cp -a $(LIB)/src/.libs/pkcs11.so $(INSTALL_DIR)/lib/
	@cp -a $(LIB)/src/.libs/libp11.so* $(INSTALL_DIR)/lib/

libp11_clean:
	$(MAKE) -f Makefile clean

