LIB = libgen_util.so


all:
	@cp -f prebuilt/$(LIB) .

install:
	mkdir -p $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	install -m 755 $(LIB) $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	mkdir -p $(BCM_FSBUILD_DIR)/private/lib$(BCM_INSTALL_SUFFIX_DIR)
	install -m 755 $(LIB) $(BCM_FSBUILD_DIR)/private/lib$(BCM_INSTALL_SUFFIX_DIR)

clean:

dynamic: all install

