include $(TOPDIR)/rules.mk

PKG_NAME:=modem-com
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

STRIP:=:

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DEPENDS:=argp-standalone

define Package/modem-com
    SECTION:=utils
    CATEGORY:=Utilities
    TITLE:=modem-com
    DEPENDS:= +cJSON
endef

TARGET_LDFLAGS += -largp

define Package/modem-com/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/modem-com $(1)/usr/bin/
endef

$(eval $(call BuildPackage,modem-com))
