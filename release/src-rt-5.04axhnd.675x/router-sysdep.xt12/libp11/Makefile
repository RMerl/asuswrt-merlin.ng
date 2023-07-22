LIB=libp11-libp11-0.4.10
UNTAR_COMPLETE_FILE = .untar_complete
CONFIG_FLAGS = \
	--host=$(TOOLCHAIN_PREFIX) \
	--prefix=/lib \
	OPENSSL_CFLAGS=" " \
	OPENSSL_LIBS="-lcrypto"

$(LIB)/$(UNTAR_COMPLETE_FILE): $(LIB).tgz
	@echo "Untarring libp11 source from $(LIB).tgz..."
	tar xzf $(LIB).tgz
	touch $(LIB)/$(UNTAR_COMPLETE_FILE)
	(cd $(LIB); ./bootstrap;)
	(cd $(LIB); ./configure $(CONFIG_FLAGS);)

install: $(LIB)/$(UNTAR_COMPLETE_FILE)
	$(MAKE) -j1 -C $(LIB) V=1 INCLUDES=-I$(BCM_FSBUILD_DIR)/public/include CFLAGS=-L$(BCM_FSBUILD_DIR)/public/lib


clean:
	rm -f $(LIB)/$(UNTAR_COMPLETE_FILE)
	$(MAKE) -j1 -C $(LIB) clean


