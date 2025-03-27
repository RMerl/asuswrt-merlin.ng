VER := 7.0.2
PKG := obuspa-$(VER)
SRC_DIR := obuspa
BINARY_DIR := src/vendor
PREFIX := obuspa-

CPPFLAGS += $(HEADER_INSTALL_DIR) $(CMS_COMPILE_FLAGS)
LDFLAGS += $(BCM_RPATH_LINK_OPTION) $(BCM_LIB_PATH)

PATCH_INCLUDE := $(SRC_DIR)_include.patch
PATCH_CORE_DATA_MODEL := $(SRC_DIR)_core_data_model.patch
PATCH_CORE_MESSAGE := $(SRC_DIR)_core_message.patch
PATCH_CORE_STOMP := $(SRC_DIR)_core_stomp.patch
PATCH_CORE_MQTT := $(SRC_DIR)_core_mqtt.patch
PATCH_CORE_WEBSOCKET := $(SRC_DIR)_core_websocket.patch
PATCH_CORE_MISC := $(SRC_DIR)_core_misc.patch
PATCH_AM := $(SRC_DIR)_am.patch
PATCH_VENDOR := $(SRC_DIR)_vendor.patch

AM_FILE := $(SRC_DIR)/Makefile.am
VENDOR_HEADER := $(SRC_DIR)/src/vendor/vendor_defs.h

ifeq ($(strip $(DESKTOP_LINUX)),y)
BCM_BLT32_FLAGS = CFLAGS='$(BCM_LD_FLAGS)'
TOOLCHAIN_PREFIX=$(ARCH)-none-linux
endif

ifeq ($(strip $(BUILD_LIBCURL_WITH_SSL)),y)
BCM_BLT32_FLAGS += CPPFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS)" LIBS="-lbcm_webproto -lbcm_sslconf $(MDM_CORE_LIBS) -lmdm_cbk_usp -lbcm_generic_hal -lcms_msg $(CMS_COMMON_LIBS) -lssl -lcrypto"
endif

ifeq ($(strip $(BUILD_BRCM_OPENWRT)),y)
CFLAGS += -DCONFIG_BRCM_OPENWRT
endif

handle_binary_obj:
	@if [ ! -e $(SRC_DIR)/$(BINARY_DIR)/bdk_main.c ]; then \
		( cd $(SRC_DIR)/$(BINARY_DIR); \
		cp $(PREFIX)bdk_main.o.$(PROFILE_ARCH).save $(PREFIX)bdk_main.o; \
		cp $(PREFIX)device_controller.o.$(PROFILE_ARCH).save $(PREFIX)device_controller.o; \
		cp $(PREFIX)device_ctrust.o.$(PROFILE_ARCH).save $(PREFIX)device_ctrust.o; \
		cp $(PREFIX)device_local_agent.o.$(PROFILE_ARCH).save $(PREFIX)device_local_agent.o; \
		cp $(PREFIX)device_mqtt.o.$(PROFILE_ARCH).save $(PREFIX)device_mqtt.o; \
		cp $(PREFIX)device_mtp.o.$(PROFILE_ARCH).save $(PREFIX)device_mtp.o; \
		cp $(PREFIX)device_security.o.$(PROFILE_ARCH).save $(PREFIX)device_security.o; \
		cp $(PREFIX)device_stomp.o.$(PROFILE_ARCH).save $(PREFIX)device_stomp.o; \
		cp $(PREFIX)device_subscription.o.$(PROFILE_ARCH).save $(PREFIX)device_subscription.o; \
		cp $(PREFIX)device_request.o.$(PROFILE_ARCH).save $(PREFIX)device_request.o; \
		cp $(PREFIX)device_bulkdata.o.$(PROFILE_ARCH).save $(PREFIX)device_bulkdata.o; \
		cp $(PREFIX)vendor_httpDownload.o.$(PROFILE_ARCH).save $(PREFIX)vendor_httpDownload.o; \
		cp $(PREFIX)vendor_httpUpload.o.$(PROFILE_ARCH).save $(PREFIX)vendor_httpUpload.o; \
		cp $(PREFIX)vendor_op_dsldiag.o.$(PROFILE_ARCH).save $(PREFIX)vendor_op_dsldiag.o; \
		cp $(PREFIX)vendor_op_firmware.o.$(PROFILE_ARCH).save $(PREFIX)vendor_op_firmware.o; \
		cp $(PREFIX)vendor_op_ipdiag.o.$(PROFILE_ARCH).save $(PREFIX)vendor_op_ipdiag.o; \
		cp $(PREFIX)vendor_op_configfile.o.$(PROFILE_ARCH).save $(PREFIX)vendor_op_configfile.o; \
		cp $(PREFIX)vendor_op_modsw.o.$(PROFILE_ARCH).save $(PREFIX)vendor_op_modsw.o; \
		cp $(PREFIX)vendor.o.$(PROFILE_ARCH).save $(PREFIX)vendor.o; ); \
	fi;

check_config: $(PKG).tar.gz Makefile
	@if [ ! -e $(SRC_DIR)/Makefile ]; then \
        echo "Untarring original $(EXE) source and configure"; \
        mkdir -p $(SRC_DIR); \
        ( cd $(SRC_DIR); tar --exclude .git --exclude .gitignore --strip-components=1 -xzkf ../$(PKG).tar.gz; \
        cd ..; \
        if [ ! -e $(SRC_DIR)/Makefile.am.orig ]; then \
            patch -p1 -b -s -d $(EXE) <$(PATCH_INCLUDE); \
            patch -p1 -b -s -d $(EXE) <$(PATCH_CORE_DATA_MODEL); \
            patch -p1 -b -s -d $(EXE) <$(PATCH_CORE_MESSAGE); \
            patch -p1 -b -s -d $(EXE) <$(PATCH_CORE_STOMP); \
            patch -p1 -b -s -d $(EXE) <$(PATCH_CORE_MQTT); \
            patch -p1 -b -s -d $(EXE) <$(PATCH_CORE_WEBSOCKET); \
            patch -p1 -b -s -d $(EXE) <$(PATCH_CORE_MISC); \
            patch -b $(AM_FILE) -i $(PATCH_AM); \
            patch -b $(VENDOR_HEADER) -i $(PATCH_VENDOR); \
        fi; \
        cd $(SRC_DIR); \
        autoreconf --force --install ); \
        (cd $(SRC_DIR); \
        ./configure --enable-coap=no --prefix=$(BCM_FSBUILD_DIR)/ --host=$(TOOLCHAIN_PREFIX) $(BCM_BLT32_FLAGS)); \
    fi;

install: handle_binary_obj check_config
	if [ ! -d $(SRC_DIR)/.libs ]; then \
		make -C $(SRC_DIR); \
	    make -C $(SRC_DIR) install; \
	fi;

clean:
	@if [ -e $(SRC_DIR)/Makefile ]; then \
	    make -C $(SRC_DIR) distclean; \
	    rm -rf $(SRC_DIR)/m4; \
	    rm -rf $(SRC_DIR)/autom4te.cache; \
	    rm -f $(SRC_DIR)/*.m4; \
	    rm -f $(SRC_DIR)/compile; \
	    rm -f $(SRC_DIR)/configure; \
	    rm -f $(SRC_DIR)/configure.ac; \
	    rm -f $(SRC_DIR)/config.guess; \
	    rm -f $(SRC_DIR)/config.sub; \
	    rm -f $(SRC_DIR)/depcomp; \
	    rm -f $(SRC_DIR)/install-sh; \
	    rm -f $(SRC_DIR)/ltmain.sh; \
	    rm -f $(SRC_DIR)/missing; \
	    rm -f $(SRC_DIR)/Makefile.in; \
	fi;
	-rm -f $(EXE_INSTALL_DIR)/$(EXE)

