# Convert asuswrt build environment variable to samba-3.6.x
-include ../.config
TARGET_CFLAGS = $(EXTRACFLAGS) -Os -pipe -fhonour-copts -msoft-float -ffunction-sections -fdata-sections
ifeq ($(RTCONFIG_QCA),y)
TARGET_CFLAGS += -Wl,-z,relro,-z,now -Wno-error=unused-but-set-variable
TARGET_CFLAGS += $(if $(RTCONFIG_SOC_IPQ8064),-mfloat-abi=softfp)
endif
TARGET_LDFLAGS = -ffunction-sections -fdata-sections
CONFIGURE_VARS =
CONFIGURE_ARGS =
PKG_BUILD_DIR=$(shell pwd)/source

# Define variables for samba-3.6.x
CONFIG_PACKAGE_SAMBA_MAX_DEBUG_LEVEL=-1
TARGET_CFLAGS += -DMAX_DEBUG_LEVEL=$(CONFIG_PACKAGE_SAMBA_MAX_DEBUG_LEVEL) -D__location__=\\\"\\\" -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

CONFIGURE_VARS += \
	ac_cv_lib_attr_getxattr=no \
	ac_cv_search_getxattr=no \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=yes \
	libreplace_cv_HAVE_IFACE_IFCONF=yes \
	LINUX_LFS_SUPPORT=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=yes \
	samba_cv_have_setreuid=yes \
	samba_cv_have_setresuid=yes \
	ac_cv_header_zlib_h=no \
	samba_cv_zlib_1_2_3=no \
	samba_cv_have_longlong=yes \
	samba_cv_HAVE_EXPLICIT_LARGEFILE_SUPPORT=yes \
	samba_cv_HAVE_KERNEL_CHANGE_NOTIFY=yes

CONFIGURE_ARGS += \
	--prefix=/usr \
	--bindir=/usr/bin \
	--sbindir=/usr/sbin \
	--libdir=/etc \
	--localstatedir=/var \
	--with-configdir=/etc \
	--with-rootsbindir=/usr/sbin \
	--with-piddir=/var/run/samba \
	--with-privatedir=/etc/samba \
	--with-lockdir=/var/lock \
	--with-syslog \
	--disable-avahi \
	--disable-cups \
	--disable-pie \
	--disable-relro \
	--disable-static \
	--disable-swat \
	--disable-shared-libs \
	--disable-iprint \
	--disable-fam \
	--disable-dmalloc \
	--disable-krb5developer \
	--disable-developer \
	--disable-debug \
	--enable-largefile \
	--with-codepagedir=/usr/share/samba/codepages \
	--with-included-iniparser \
	--with-included-popt=no \
	--with-logfilebase=/var/log \
	--with-nmbdsocketdir=/var/nmbd \
	--with-sendfile-support \
	--without-utmp \
	--without-quotas \
	--without-sys-quotas \
	--without-acl-support \
	--without-cluster-support \
	--without-ads \
	--without-krb5 \
	--without-ldap \
	--without-pam \
	--without-winbind \
	--without-libtdb \
	--without-libtalloc \
	--without-libnetapi \
	--without-libsmbclient \
	--without-libsmbsharemodes \
	--without-libtevent \
	--without-libaddns \
	--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_winbind,auth_wbc,auth_domain

MAKE_FLAGS += DYNEXP= PICFLAG= MODULES=

all: $(PKG_BUILD_DIR)/source3/Makefile
	$(MAKE) -C $(PKG_BUILD_DIR)/source3 include/proto.h && $(MAKE) -j 8 -C $(PKG_BUILD_DIR)/source3 $(MAKE_FLAGS)

$(PKG_BUILD_DIR)/source3/Makefile:
	$(MAKE) configure

configure:
	( cd $(PKG_BUILD_DIR)/source3 && \
		$(CONFIGURE_VARS) \
		CPPFLAGS="-DNDEBUG" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		$(CONFIGURE) $(CONFIGURE_ARGS) \
	)


install:
	install -D $(PKG_BUILD_DIR)/source3/bin/samba_multicall $(INSTALLDIR)/usr/sbin/samba_multicall
	$(STRIP) $(INSTALLDIR)/usr/sbin/samba_multicall
	ln -sf samba_multicall $(INSTALLDIR)/usr/sbin/smbd
	ln -sf samba_multicall $(INSTALLDIR)/usr/sbin/nmbd
	ln -sf samba_multicall $(INSTALLDIR)/usr/sbin/smbpasswd
	install -D $(PKG_BUILD_DIR)/codepages/lowcase.dat $(INSTALLDIR)/usr/share/samba/codepages/lowcase.dat
	install -D $(PKG_BUILD_DIR)/codepages/upcase.dat $(INSTALLDIR)/usr/share/samba/codepages/upcase.dat

clean:
	[ ! -f $(PKG_BUILD_DIR)/source3/Makefile ] || $(MAKE) -C $(PKG_BUILD_DIR)/source3 distclean
