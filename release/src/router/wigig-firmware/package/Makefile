#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wigig-firmware
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk

include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=QCA wigig(wil6210,11ad) firmware
endef

define Package/$(PKG_NAME)/description/Default
  This package contains the wigig(wil61210,11ad) Firmware binaries
endef

define Package/$(PKG_NAME)/install/Default
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/WIL6210/wil6210.fw \
	$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/WIL6210/wil6210.board \
	$(1)/lib/firmware/wil6210.brd
endef

Build/Compile=true

define Package/$(PKG_NAME)
$(call Package/$(PKG_NAME)/Default)
endef

define Package/$(PKG_NAME)/description
$(call Package/$(PKG_NAME)/description/Default)
endef

define Package/$(PKG_NAME)/install
$(call Package/$(PKG_NAME)/install/Default,$(1))
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

define Package/$(PKG_NAME)-ipdock
  $(call Package/$(PKG_NAME)/Default,ipdock)
  VARIANT:=ipdock
endef

define Package/$(PKG_NAME)-ipdock/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Package/$(PKG_NAME)-ipdock/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPDOCK/wil6210.fw \
	$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPDOCK/wil6210.board \
	$(1)/lib/firmware/wil6210.brd
endef

$(eval $(call BuildPackage,$(PKG_NAME)-ipdock))
