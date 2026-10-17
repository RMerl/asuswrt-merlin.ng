LIB := libfdt.so

libfdt_src_name := dtc-1.7.2

all install: $(LIB)


check_untar:
	if [ ! -e $(libfdt_src_name)/Makefile ]; then \
	echo "Untarring original $(libfdt_src_name) source"; \
		unzip ../../$(libfdt_src_name).zip; \
		touch $(libfdt_src_name)/Makefile; \
	fi;

$(LIB): check_untar
	mkdir -p $(BCM_FSBUILD_DIR)/public/lib
	(export PKG_CONFIG_LIBDIR=$(BCM_FSBUILD_DIR)/public/lib; \
	export PKG_CONFIG_PATH=$(BCM_FSBUILD_DIR)/public/lib/pkgconfig; \
	cd $(libfdt_src_name); \
	$(MAKE) NO_PYTHON=1 PREFIX=$(BCM_FSBUILD_DIR)/public/ libfdt; \
	$(MAKE) NO_PYTHON=1 PREFIX=$(BCM_FSBUILD_DIR)/public/ INSTALL="$(INSTALL)" install-lib ; \
	$(MAKE) NO_PYTHON=1 PREFIX=$(BCM_FSBUILD_DIR)/public/ INSTALL="$(INSTALL)" install-includes; )
ifneq ($(strip $(BUILD_DTC)),)
	mkdir -p $(BCM_FSINSTALL_DIR)/bin
	cp $(libfdt_src_name)/dtc $(BCM_FSINSTALL_DIR)/bin/
endif


native_clean:
	if [ -e $(libfdt_src_name)/Makefile ]; then \
		cd $(libfdt_src_name); $(MAKE) clean; \
	fi;

clean: native_clean
	rm -f $(BCM_FSBUILD_DIR)/public/lib/libfdt*.so*
	rm -f $(BCM_FSINSTALL_DIR)/bin/dtc
