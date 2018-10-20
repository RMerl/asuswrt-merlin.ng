# Convert asuswrt build environment variable to samba-3.6.x
include ../common.mak
MODULES=
SMB_USE_MULTICALL=1
PKG_BUILD_DIR=$(shell pwd)/source

SMBCFLAGS = $(EXTRACFLAGS) -O3 -ffunction-sections -fdata-sections -I$(TOP)/libiconv-1.14/include
SMBLDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libiconv-1.14/lib/.libs

ifneq ($(CONFIG_LINUX26),y)
SMBCFLAGS += -DMAX_DEBUG_LEVEL="-1"
endif

ifeq ($(RTCONFIG_BCMWL6), y)
ifeq ($(RTCONFIG_BCMARM), y)
SMBLDFLAGS += -lgcc_s
HOST = arm

ifeq ($(HND_ROUTER),y)
FUNC_GETADDRINFO=yes
FUNC_GETIFADDRS=yes
LIB_RESOLV=yes
else
FUNC_GETADDRINFO=yes
FUNC_GETIFADDRS=no
DEF_FCNTL=-Dfcntl=fcntl64
LIB_RESOLV=no
endif
else
HOST = mips
endif
endif

ifeq ($(RTCONFIG_QCA),y)
SMBCFLAGS += -Wl,-z,relro,-z,now -Wno-error=unused-but-set-variable
SMBCFLAGS += $(if $(RTCONFIG_SOC_IPQ8064),-mfloat-abi=softfp)
endif
CONFIGURE_VARS =
CONFIGURE_ARGS =

#CONFIGURE_VARS += \
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

#CONFIGURE_ARGS += \
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


all: $(PKG_BUILD_DIR)/source3/Makefile
	$(MAKE) -C $(PKG_BUILD_DIR)/source3 include/proto.h && $(MAKE) -j 8 -C $(PKG_BUILD_DIR)/source3 all

$(PKG_BUILD_DIR)/source3/Makefile:
	cd $(PKG_BUILD_DIR)/source3 && \
	export samba_cv_CC_NEGATIVE_ENUM_VALUES=yes ; \
	export libreplace_cv_READDIR_GETDIRENTRIES=no ; \
	export libreplace_cv_READDIR_GETDENTS=no ; \
	export libreplace_cv_HAVE_GETADDRINFO=$(FUNC_GETADDRINFO) ; \
	export libreplace_cv_HAVE_GETIFADDRS=$(FUNC_GETIFADDRS) ; \
	export linux_getgrouplist_ok=no ; \
	export samba_cv_REPLACE_READDIR=no ; \
	export samba_cv_HAVE_WRFILE_KEYTAB=yes ; \
	export samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes ; \
	export samba_cv_HAVE_KERNEL_CHANGE_NOTIF=yes ; \
	export samba_cv_HAVE_KERNEL_SHARE_MODES=yes ; \
	export samba_cv_HAVE_IFACE_IFCONF=yes ; \
	export samba_cv_USE_SETRESUID=yes ; \
	export samba_cv_have_longlong=yes ; \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	ac_cv_lib_resolv___dn_expand=$(LIB_RESOLV) \
	ac_cv_lib_resolv_dn_expand=$(LIB_RESOLV) \
	ac_cv_header_uuid_uuid_h=no \
	ac_cv_func_uuid_generate=no \
	ac_cv_func_prctl=no \
	samba_cv_HAVE_INO64_T=yes \
	samba_cv_HAVE_OFF64_T=yes \
	samba_cv_HAVE_STRUCT_FLOCK64=yes \
	samba_cv_SIZEOF_OFF_T=yes \
	samba_cv_HAVE_MMAP=yes \
	samba_cv_HAVE_FTRUNCATE_EXTEND=yes \
	samba_cv_HAVE_BROKEN_LINUX_SENDFILE=no \
	samba_cv_have_setresgid=yes \
	samba_cv_have_setresuid=yes \
	samba_cv_USE_SETREUID=yes \
	samba_cv_REALPATH_TAKES_NULL=no \
	samba_cv_HAVE_FCNTL_LOCK=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_NATIVE_ICONV=no \
	samba_cv_HAVE_BROKEN_FCNTL64_LOCKS=no \
	samba_cv_HAVE_BROKEN_GETGROUPS=no \
	samba_cv_HAVE_BROKEN_READDIR_NAME=no \
	samba_cv_HAVE_C99_VSNPRINTF=yes \
	samba_cv_HAVE_DEV64_T=no \
	samba_cv_HAVE_DEVICE_MAJOR_FN=yes \
	samba_cv_HAVE_DEVICE_MINOR_FN=yes \
	samba_cv_HAVE_IFACE_AIX=no \
	samba_cv_HAVE_KERNEL_CHANGE_NOTIFY=yes \
	samba_cv_HAVE_MAKEDEV=yes \
	samba_cv_HAVE_TRUNCATED_SALT=no \
	samba_cv_HAVE_UNSIGNED_CHAR=no \
	samba_cv_HAVE_WORKING_AF_LOCAL=yes \
	samba_cv_HAVE_Werror=yes \
	samba_cv_REPLACE_INET_NTOA=no \
	samba_cv_SIZEOF_DEV_T=yes \
	samba_cv_SIZEOF_INO_T=yes \
	samba_cv_SIZEOF_TIME_T=no \
	CPPFLAGS="-DNDEBUG -DSHMEM_SIZE=524288 $(DEF_FCNTL) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_LARGE_FILES=1" \
	CFLAGS="$(SMBCFLAGS)" LDFLAGS="$(SMBLDFLAGS)" CC=$(CC) LD=$(LD) AR=$(AR) RANLIB=$(RANLIB) \
	$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libdir=/etc \
		--localstatedir=/var \
		--host=$(HOST)-linux \
		--with-configdir=/etc \
		--with-rootsbindir=/usr/sbin \
		--with-piddir=/var/run/samba \
		--with-privatedir=/etc/samba \
		--with-lockdir=/var/lock \
		--with-included-popt=no \
		--with-krb5=no \
		--with-libiconv=$(TOP)/libiconv-1.14 \
		--with-shared-modules=$(MODULES) \
		--disable-shared-libs \
		--disable-static \
		--disable-cups \
		--disable-iprint \
		--disable-pie \
		--disable-fam \
		--disable-dmalloc \
		--disable-krb5developer \
		--disable-developer \
		--disable-debug \
		--without-ads \
		--without-krb5 \
		--without-acl-support \
		--without-ldap \
		--without-cluster-support \
		--without-utmp \
		--without-winbind \
		--without-quotas \
		--without-libtalloc \
		--without-sys-quotas \
		--without-libtdb \
		--without-libnetapi \
		--without-libsmbsharemodes \
		--without-libtevent \
		--without-sys-quotas \
		--without-dnsupdate \
		--disable-swat \
		--with-syslog \
		--with-codepagedir=/usr/share/samba/codepages

install:
	@install -d $(INSTALLDIR)/usr/bin/
ifeq ($(SMB_USE_MULTICALL), 1)
	@install -D $(PKG_BUILD_DIR)/source3/bin/samba_multicall $(INSTALLDIR)/usr/sbin/samba_multicall
	@ln -sf ../sbin/samba_multicall $(INSTALLDIR)/usr/sbin/smbd
	@ln -sf ../sbin/samba_multicall $(INSTALLDIR)/usr/sbin/nmbd
	@ln -sf ../sbin/samba_multicall $(INSTALLDIR)/usr/bin/smbpasswd
	$(STRIP) $(INSTALLDIR)/usr/sbin/samba_multicall
else
	install -D $(PKG_BUILD_DIR)/source3/bin/smbd $(INSTALLDIR)/usr/sbin/smbd
	$(STRIP) $(INSTALLDIR)/usr/sbin/smbd
	install -D $(PKG_BUILD_DIR)/source3/bin/nmbd $(INSTALLDIR)/usr/sbin/nmbd
	$(STRIP) $(INSTALLDIR)/usr/sbin/nmbd
	install -D $(PKG_BUILD_DIR)/source3/bin/smbpasswd $(INSTALLDIR)/usr/sbin/smbpasswd
	$(STRIP) $(INSTALLDIR)/usr/sbin/smbpasswd
endif
	install -D $(PKG_BUILD_DIR)/codepages/lowcase.dat $(INSTALLDIR)/usr/share/samba/codepages/lowcase.dat
	install -D $(PKG_BUILD_DIR)/codepages/upcase.dat $(INSTALLDIR)/usr/share/samba/codepages/upcase.dat
	install -D $(PKG_BUILD_DIR)/codepages/valid.dat $(INSTALLDIR)/usr/share/samba/codepages/valid.dat
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/sysroot/lib/libresolv.so.2 $(INSTALLDIR)/usr/lib/libresolv.so.2
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libresolv.so.0 $(INSTALLDIR)/usr/lib/libresolv.so.0
endif
# Modules
#	install -D $(PKG_BUILD_DIR)/source3/bin/acl_tdb.so $(INSTALLDIR)/usr/lib/acl_tdb.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/acl_xattr.so $(INSTALLDIR)/usr/lib/acl_xattr.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/audit.so $(INSTALLDIR)/usr/lib/audit.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/autorid.so $(INSTALLDIR)/usr/lib/autorid.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/cap.so $(INSTALLDIR)/usr/lib/cap.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/catia.so $(INSTALLDIR)/usr/lib/catia.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/CP437.so $(INSTALLDIR)/usr/lib/CP437.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/CP850.so $(INSTALLDIR)/usr/lib/CP850.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/crossrename.so $(INSTALLDIR)/usr/lib/crossrename.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/default_quota.so $(INSTALLDIR)/usr/lib/default_quota.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/dirsort.so $(INSTALLDIR)/usr/lib/dirsort.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/expand_msdfs.so $(INSTALLDIR)/usr/lib/expand_msdfs.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/extd_audit.so $(INSTALLDIR)/usr/lib/extd_audit.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/fake_perms.so $(INSTALLDIR)/usr/lib/fake_perms.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/full_audit.so $(INSTALLDIR)/usr/lib/full_audit.so

# Libraries
#	install -D $(PKG_BUILD_DIR)/source3/bin/libnetapi.so.0 $(INSTALLDIR)/usr/lib/libnetapi.so.0
#	@ln -sf libnetapi.so.0 $(INSTALLDIR)/usr/lib/libnetapi.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/libsmbclient.so.0 $(INSTALLDIR)/usr/lib/libsmbclient.so.0
#	@ln -sf libsmbclient.so.0 $(INSTALLDIR)/usr/lib/libsmbclient.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/libsmbsharemodes.so $(INSTALLDIR)/usr/lib/libsmbsharemodes.so
#	@ln -sf libsmbsharemodes.so.0 $(INSTALLDIR)/usr/lib/libsmbsharemodes.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/libtalloc.so.2 $(INSTALLDIR)/usr/lib/libtalloc.so.2
#	@ln -sf libtalloc.so.2.0.5 $(INSTALLDIR)/usr/lib/libtalloc.so
#	@ln -sf libtalloc.so.2.0.5 $(INSTALLDIR)/usr/lib/libtalloc.so.2
#	install -D $(PKG_BUILD_DIR)/source3/bin/libtdb.so $(INSTALLDIR)/usr/lib/libtdb.so.1.2.9
#	@ln -sf libtdb.so.1.2.9 $(INSTALLDIR)/usr/lib/libtdb.so.1
#	@ln -sf libtdb.so.1.2.9 $(INSTALLDIR)/usr/lib/libtdb.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/libtevent.so $(INSTALLDIR)/usr/lib/libtevent.so.0.9.11
#	@ln -sf libtevent.so.0.9.11 $(INSTALLDIR)/usr/lib/libtevent.so.0
#	@ln -sf libtevent.so.0.9.11 $(INSTALLDIR)/usr/lib/libtevent.so

# Extensions
#	install -D $(PKG_BUILD_DIR)/source3/bin/linux_xfs_sgid.so $(INSTALLDIR)/usr/lib/linux_xfs_sgid.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/netatalk.so $(INSTALLDIR)/usr/lib/netatalk.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/preopen.so $(INSTALLDIR)/usr/lib/preopen.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/readahead.so $(INSTALLDIR)/usr/lib/readahead.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/readonly.so $(INSTALLDIR)/usr/lib/readonly.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/recycle.so $(INSTALLDIR)/usr/lib/recycle.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/scannedonly.so $(INSTALLDIR)/usr/lib/scannedonly.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/script.so $(INSTALLDIR)/usr/lib/script.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/shadow_copy2.so $(INSTALLDIR)/usr/lib/shadow_copy2.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/shadow_copy.so $(INSTALLDIR)/usr/lib/shadow_copy.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/smb_traffic_analyzer.so $(INSTALLDIR)/usr/lib/smb_traffic_analyzer.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/streams_depot.so $(INSTALLDIR)/usr/lib/streams_depot.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/streams_xattr.so $(INSTALLDIR)/usr/lib/streams_xattr.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/syncops.so $(INSTALLDIR)/usr/lib/syncops.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/time_audit.so $(INSTALLDIR)/usr/lib/time_audit.so
#	install -D $(PKG_BUILD_DIR)/source3/bin/xattr_tdb.so $(INSTALLDIR)/usr/lib/xattr_tdb.so

clean:
	[ ! -f $(PKG_BUILD_DIR)/source3/Makefile ] || $(MAKE) -C $(PKG_BUILD_DIR)/source3 distclean
	rm -f $(PKG_BUILD_DIR)/source3/Makefile

distclean: clean
	@find $(PKG_BUILD_DIR)/source3 -name config.h | xargs rm -f
	@find $(PKG_BUILD_DIR)/source3 -name Makefile | xargs rm -f
	@find $(PKG_BUILD_DIR)/source3 -name config.status | xargs rm -f
	@find $(PKG_BUILD_DIR)/source3 -name config.cache | xargs rm -f

