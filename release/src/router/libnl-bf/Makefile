# Convert asuswrt build environment variable to libnl-bf
TARGET_CC=$(CROSS_COMPILE)gcc
LINUX_VERSION=$(LINUX_KERNEL)
PKG_BUILD_DIR=$(shell pwd)/source

all: $(PKG_BUILD_DIR)/Makefile
	$(MAKE) -C $(PKG_BUILD_DIR)

$(PKG_BUILD_DIR)/Makefile: $(PKG_BUILD_DIR)/configure
	$(MAKE) $(PKG_BUILD_DIR)-configure

$(PKG_BUILD_DIR)/configure:
	( cd $(PKG_BUILD_DIR) ; ./autogen.sh )

$(PKG_BUILD_DIR)-configure:
	( cd $(PKG_BUILD_DIR) ; $(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib --sysconfdir=/etc )

install: stage
	[ -d $(INSTALLDIR)/usr/lib ] || install -d $(INSTALLDIR)/usr/lib
	[ -d $(INSTALLDIR)/../rom/rom/etc/libnl ] || install -d $(INSTALLDIR)/../rom/rom/etc/libnl
	install -m 755 $(STAGEDIR)/usr/lib/libnl*.so* $(INSTALLDIR)/usr/lib/
	install -m 644 $(STAGEDIR)/etc/libnl/classid $(INSTALLDIR)/../rom/rom/etc/libnl/

clean:
	find $(PKG_BUILD_DIR) -type f -name "*.o" -o -name "stage" | xargs rm -f

stage:
	$(MAKE) -C $(PKG_BUILD_DIR) DESTDIR="$(STAGEDIR)" install
	@touch $@
