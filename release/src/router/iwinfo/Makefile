# Convert asuswrt build environment variable to iw
PKG_BUILD_DIR=$(shell pwd)/source
STAGING_DIR=$(STAGEDIR)
FPIC=-fpic

#IWINFO_BACKENDS := \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl),wl) \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl-mini),wl) \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl-mimo),wl) \
	$(if $(CONFIG_PACKAGE_kmod-madwifi),madwifi) \
	$(if $(CONFIG_PACKAGE_kmod-qca-wifi-perf),qcawifi) \
	$(if $(CONFIG_PACKAGE_kmod-qca-wifi-akronite-perf),qcawifi) \
	$(if $(CONFIG_PACKAGE_kmod-qca-wifi-10.4-akronite-perf),qcawifi) \
	$(if $(CONFIG_PACKAGE_kmod-mac80211),nl80211) \
	$(if $(CONFIG_PACKAGE_kmod-qca-mac80211),nl80211) \
	$(if $(CONFIG_PACKAGE_kmod-qca-wil6210),nl80211)

IWINFO_BACKENDS := qcawifi nl80211

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include \
	-D_GNU_SOURCE

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	BACKENDS="$(IWINFO_BACKENDS)"

all:
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS)

install:
	install -d $(INSTALLDIR)/usr/bin
	install -m 755 $(PKG_BUILD_DIR)/iwinfo $(INSTALLDIR)/usr/bin
	$(STRIP) $(INSTALLDIR)/usr/bin/*
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 $(PKG_BUILD_DIR)/libiwinfo.so $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/*
	#install -d $(INSTALLDIR)/usr/lib/lua
	#install -m 755 $(PKG_BUILD_DIR)/iwinfo.so $(INSTALLDIR)/usr/lib/lua
	#$(STRIP) $(INSTALLDIR)/usr/lib/lua/*

clean:
	find $(PKG_BUILD_DIR) -type f -name iwinfo \
		-o -name "*.so" \
		-o -name ".*.o.cmd" \
		-o -name ".*.ko.cmd" \
		-o -name "*.o" \
		-o -name "modules.order" \
		-o -name "*.ko" \
		-o -name ".tmp_versions" \
		-o -name "Module.symvers" \
		-o -name "*.mod.c" | xargs rm -fr

stage:
	install -d $(STAGEDIR)/usr/include/iwinfo
	cp -rf $(PKG_BUILD_DIR)/include/iwinfo.h $(STAGEDIR)/usr/include/
	cp -rf $(PKG_BUILD_DIR)/include/iwinfo/* $(STAGEDIR)/usr/include/iwinfo/
	install -d $(STAGEDIR)/usr/lib
	install -m 755 $(PKG_BUILD_DIR)/libiwinfo.so $(STAGEDIR)/usr/lib/libiwinfo.so
	#install -d $(STAGEDIR)/usr/lib/lua
	#install -m 755 $(PKG_BUILD_DIR)/iwinfo.so $(STAGEDIR)/usr/lib/lua/iwinfo.so
	@touch $@
