include $(TOPDIR)/rules.mk

PKG_NAME:=libnl-bf
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://github.com/tgraf/libnl.git
PKG_BRANCH:=master
PKG_REV:=7d47666eb3c414feb8901970d35b96461214c2bf
PKG_VERSION:=g$(PKG_REV)
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_MIRROR:=none

PATCH_DIR:=./patches

PKG_FIXUP:=libtool

include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk

define Package/libnl-bf
  SECTION:=Bigfoot
  CATEGORY:=Bandwidth Control
  TITLE:=NetLink socket library with BF modifications
  DEPENDS:=+libpthread
  MAINTAINER:=Ryan Nowakowski <rnowakow@qca.qualcomm.com>
  PROVIDES:=libnl
endef

define Package/libnl-bf/description
 This package contains a library for applications dealing with netlink sockets.
 The standard libnl has been modified to suit Bigfoot purposes.
endef

TARGET_CFLAGS += -ffunction-sections $(FPIC)

CONFIGURE_ARGS += --enable-cli

define Build/Configure
	( cd $(PKG_BUILD_DIR); ./autogen.sh );
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(call Build/Compile/Default)
	make -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
	$(INSTALL_DIR) $(1)/usr/include/libnl3
	$(CP) $(PKG_BUILD_DIR)/include/netlink-* $(1)/usr/include/libnl3/
	$(CP) $(PKG_BUILD_DIR)/include/linux $(1)/usr/include/libnl3/
endef

define Package/libnl-bf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl*so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/libnl
	$(CP) $(PKG_INSTALL_DIR)/etc/libnl/classid $(1)/etc/libnl/
endef

$(eval $(call BuildPackage,libnl-bf))
