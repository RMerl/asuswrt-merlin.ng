SRC := cJSON-1.7.13
LIB := libcjson
LIB_DIR := cjson

HEADERS = cJSON.h

check_cmake: $(SRC).tar.gz
	@if [ ! -e $(LIB_DIR)/Makefile ]; then \
	    echo "Untarring original $(SRC) source and cmake"; \
		mkdir -p $(LIB_DIR); \
		( cd $(LIB_DIR); tar --strip-components=1 -xzf ../$(SRC).tar.gz; \
		cmake \
		-DCMAKE_RANLIB=$(RANLIB) \
		-DCMAKE_STRIP=$(STRIP) \
		-DCMAKE_NM:FILEPATH=$(NM) \
		-DCMAKE_OBJCOPY:FILEPATH=$(OBJCOPY) \
		-DCMAKE_OBJDUMP=$(OBJDUMP) \
		-DCMAKE_LINKER:FILEPATH="$(LD)" \
		-DCMAKE_AR:FILEPATH=$(AR) \
		-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
		-DCMAKE_FIND_ROOT_PATH:PATH="$(ROOT_PATH)" \
		-DCMAKE_PREFIX_PATH:PATH="$(PREFIX)" \
		-DCMAKE_INSTALL_LIBDIR=$(LIB_INSTALL_DIR) \
		-DCMAKE_INSTALL_PREFIX:PATH=$(PREFIX) ); \
	fi

install: check_cmake
	if [ ! -d $(LIB_DIR)/.libs ]; then \
		$(MAKE) -C $(LIB_DIR)/lib; \
	    $(MAKE) -C $(LIB_DIR) install; \
	fi

clean:
	rm -f $(LIB_INSTALL_DIR)/$(LIB).so*
	rm -rf $(HEADER_INSTALL_DIR)/$(LIB_DIR)
	rm -rf $(LIB_DIR)
