LIB:=OpenSC-0.22.0
PCSC:=pcsc-lite-1.9.4
PCSC_PATH = $(BUILD_DIR)/userspace/public/libs/opensc/$(PCSC)/src/PCSC
UNTAR_COMPLETE_FILE = .untar_complete
CONFIG_FLAGS = \
	--host=$(TOOLCHAIN_PREFIX) \
	--prefix=/lib \
	PCSC_CFLAGS=-I$(PCSC_PATH) \
	OPENSSL_CFLAGS=" " \
	OPENSSL_LIBS=" "

$(LIB)/$(UNTAR_COMPLETE_FILE): $(LIB).tgz
	@echo "Untarring opensc source from $(LIB).tgz..."
	tar xzf $(LIB).tgz
	@echo "Untarring pcsc-lite source from $(PCSC).tar.bz2..."
	tar fx $(PCSC).tar.bz2
	touch $(LIB)/$(UNTAR_COMPLETE_FILE)
	(cd $(LIB); ./bootstrap;)
	(cd $(LIB); ./configure $(CONFIG_FLAGS);)

install: $(LIB)/$(UNTAR_COMPLETE_FILE)
	$(MAKE) -j1 -C $(LIB) V=1 INCLUDES=-I$(BCM_FSBUILD_DIR)/public/include CFLAGS=-L$(BCM_FSBUILD_DIR)/public/lib

clean:
	rm -f $(LIB)/$(UNTAR_COMPLETE_FILE)
	$(MAKE) -j1 -C $(LIB) clean
