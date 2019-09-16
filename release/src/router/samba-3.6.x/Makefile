# Convert asuswrt build environment variable to samba-3.6.x
include ../common.mak


PKG_BUILD_DIR=$(shell pwd)/source

FUNC_GETIFADDRS=yes
SHARED_MODULES=--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_winbind,auth_wbc,auth_domain

TARGET_CFLAGS = $(EXTRACFLAGS) -O3 -ffunction-sections -fdata-sections -I$(TOP)/libiconv-1.14/include
ifneq ($(CONFIG_LINUX26),y)
# Define variables for samba-3.6.x
CONFIG_PACKAGE_SAMBA_MAX_DEBUG_LEVEL=-1
TARGET_CFLAGS += -DMAX_DEBUG_LEVEL=$(CONFIG_PACKAGE_SAMBA_MAX_DEBUG_LEVEL)
endif
ifeq ($(RTCONFIG_QCA),y)
TARGET_CFLAGS += -pipe -fhonour-copts -msoft-float -D__location__=\\\"\\\"
TARGET_CFLAGS += -Wl,-z,relro,-z,now -Wno-error=unused-but-set-variable
TARGET_CFLAGS += $(if $(RTCONFIG_SOC_IPQ8064),-mfloat-abi=softfp)

MAKE_FLAGS = DYNEXP= PICFLAG= MODULES=
endif


TARGET_LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libiconv-1.14/lib/.libs
ifeq ($(RTCONFIG_BCMARM), y)
TARGET_CFLAGS += -DNO_DEF_IFADDRS
TARGET_LDFLAGS += -lgcc_s

DEF_FCNTL = -DSHMEM_SIZE=524288 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_LARGE_FILES=1
FUNC_GETIFADDRS=no
SHARED_MODULES=
endif


CONFIGURE_VARS = \
	ac_cv_lib_attr_getxattr=no \
	ac_cv_search_getxattr=no \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=yes \
	libreplace_cv_HAVE_GETIFADDRS=$(FUNC_GETIFADDRS) \
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
	ac_cv_path_PYTHON="" \
	ac_cv_path_PYTHON_CONFIG="" \
	samba_cv_have_longlong=yes \
	samba_cv_HAVE_EXPLICIT_LARGEFILE_SUPPORT=yes \
	samba_cv_HAVE_KERNEL_CHANGE_NOTIFY=yes

CONFIGURE_ARGS = \
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
	--with-libiconv=$(TOP)/libiconv-1.14 \
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
	$(SHARED_MODULES)


all: $(PKG_BUILD_DIR)/source3/Makefile
	$(MAKE) -C $(PKG_BUILD_DIR)/source3 include/proto.h && $(MAKE) -j 8 -C $(PKG_BUILD_DIR)/source3 $(MAKE_FLAGS)

$(PKG_BUILD_DIR)/source3/Makefile:
	( cd $(PKG_BUILD_DIR)/source3 && \
		$(CONFIGURE_VARS) \
		CPPFLAGS="-DNDEBUG $(DEF_FCNTL)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CC=$(CC) LD=$(LD) AR=$(AR) RANLIB=$(RANLIB) \
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
	install -D $(PKG_BUILD_DIR)/codepages/valid.dat $(INSTALLDIR)/usr/share/samba/codepages/valid.dat
ifeq ($(RTCONFIG_BCMARM), y)
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libresolv.so.2 $(INSTALLDIR)/usr/lib/libresolv.so.2
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libresolv.so.0 $(INSTALLDIR)/usr/lib/libresolv.so.0
endif
endif

clean:
	[ ! -f $(PKG_BUILD_DIR)/source3/Makefile ] || $(MAKE) -C $(PKG_BUILD_DIR)/source3 distclean
	rm -f $(PKG_BUILD_DIR)/source3/Makefile

distclean: clean
	@find $(PKG_BUILD_DIR)/source3 -name config.h | xargs rm -f
	@find $(PKG_BUILD_DIR)/source3 -name Makefile | xargs rm -f
	@find $(PKG_BUILD_DIR)/source3 -name config.status | xargs rm -f
	@find $(PKG_BUILD_DIR)/source3 -name config.cache | xargs rm -f

