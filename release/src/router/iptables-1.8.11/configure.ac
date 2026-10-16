
AC_INIT([iptables], [1.8.11])

# See libtool.info "Libtool's versioning system"
libxtables_vcurrent=19
libxtables_vage=7

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INSTALL
AM_INIT_AUTOMAKE([-Wall])
AC_PROG_CC
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([disable-static])

AC_ARG_WITH([kernel],
	AS_HELP_STRING([--with-kernel=PATH],
	[Path to kernel source/build directory]),
	[kbuilddir="$withval"; ksourcedir="$withval";])
AC_ARG_WITH([kbuild],
	AS_HELP_STRING([--with-kbuild=PATH],
	[Path to kernel build directory [[/lib/modules/CURRENT/build]]]),
	[kbuilddir="$withval"])
AC_ARG_WITH([ksource],
	AS_HELP_STRING([--with-ksource=PATH],
	[Path to kernel source directory [[/lib/modules/CURRENT/source]]]),
	[ksourcedir="$withval"])
AC_ARG_WITH([xtlibdir],
	AS_HELP_STRING([--with-xtlibdir=PATH],
	[Path where to install Xtables extensions [[LIBEXECDIR/xtables]]]),
	[xtlibdir="$withval"],
	[xtlibdir="${libdir}/xtables"])
AC_ARG_ENABLE([ipv4],
	AS_HELP_STRING([--disable-ipv4], [Do not build iptables]),
	[enable_ipv4="$enableval"], [enable_ipv4="yes"])
AC_ARG_ENABLE([ipv6],
	AS_HELP_STRING([--disable-ipv6], [Do not build ip6tables]),
	[enable_ipv6="$enableval"], [enable_ipv6="yes"])
AC_ARG_ENABLE([largefile],
	AS_HELP_STRING([--disable-largefile], [Do not build largefile support]),
	[enable_largefile="$enableval"],
	[enable_largefile="yes"])
AS_IF([test "$enable_largefile" = "yes"], [largefile_cppflags='-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64'])

AC_ARG_ENABLE([devel],
	AS_HELP_STRING([--enable-devel],
	[Install Xtables development headers]),
	[enable_devel="$enableval"], [enable_devel="yes"])
AC_ARG_ENABLE([libipq],
	AS_HELP_STRING([--enable-libipq], [Build and install libipq]),
	[enable_libipq="$enableval"], [enable_libipq="no"])
AC_ARG_ENABLE([bpf-compiler],
	AS_HELP_STRING([--enable-bpf-compiler], [Build bpf compiler]),
	[enable_bpfc="$enableval"], [enable_bpfc="no"])
AC_ARG_ENABLE([nfsynproxy],
	AS_HELP_STRING([--enable-nfsynproxy], [Build SYNPROXY configuration tool]),
	[enable_nfsynproxy="$enableval"], [enable_nfsynproxy="no"])
AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=PATH],
	[Path to the pkgconfig directory [[LIBDIR/pkgconfig]]]),
	[pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_ARG_ENABLE([nftables],
	AS_HELP_STRING([--disable-nftables], [Do not build nftables compat]),
	[enable_nftables="$enableval"], [enable_nftables="yes"])
AC_ARG_ENABLE([libnfnetlink],
    AS_HELP_STRING([--disable-libnfnetlink], [Do not use netfilter netlink library]),
    [enable_libnfnetlink="$enableval"], [enable_libnfnetlink="auto"])
AC_ARG_ENABLE([connlabel],
	AS_HELP_STRING([--disable-connlabel],
	[Do not build libnetfilter_conntrack]),
	[enable_connlabel="$enableval"], [enable_connlabel="yes"])
AC_ARG_WITH([xt-lock-name], AS_HELP_STRING([--with-xt-lock-name=PATH],
	[Path to the xtables lock [[/run/xtables.lock]]]),
	[xt_lock_name="$withval"],
	[xt_lock_name="/run/xtables.lock"])
AC_ARG_ENABLE([profiling],
	AS_HELP_STRING([--enable-profiling], [build for use of gcov/gprof]),
	[enable_profiling="$enableval"], [enable_profiling="no"])

AC_MSG_CHECKING([whether $LD knows -Wl,--no-undefined])
saved_LDFLAGS="$LDFLAGS";
LDFLAGS="-Wl,--no-undefined";
AC_LINK_IFELSE([AC_LANG_SOURCE([int main(void) {}])],
	[noundef_LDFLAGS="$LDFLAGS"; AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])]
)
LDFLAGS="$saved_LDFLAGS";

blacklist_modules=""
blacklist_x_modules=""
blacklist_b_modules=""
blacklist_a_modules=""
blacklist_4_modules=""
blacklist_6_modules=""

AC_CHECK_HEADERS([linux/dccp.h linux/ip_vs.h linux/magic.h linux/proc_fs.h linux/bpf.h])
if test "$ac_cv_header_linux_dccp_h" != "yes"; then
	blacklist_modules="$blacklist_modules dccp";
fi;
if test "$ac_cv_header_linux_ip_vs_h" != "yes"; then
	blacklist_modules="$blacklist_modules ipvs";
fi;

AC_CHECK_SIZEOF([struct ip6_hdr], [], [#include <netinet/ip6.h>])

AM_CONDITIONAL([ENABLE_STATIC], [test "$enable_static" = "yes"])
AM_CONDITIONAL([ENABLE_SHARED], [test "$enable_shared" = "yes"])
AM_CONDITIONAL([ENABLE_IPV4], [test "$enable_ipv4" = "yes"])
AM_CONDITIONAL([ENABLE_IPV6], [test "$enable_ipv6" = "yes"])
AM_CONDITIONAL([ENABLE_LARGEFILE], [test "$enable_largefile" = "yes"])
AM_CONDITIONAL([ENABLE_DEVEL], [test "$enable_devel" = "yes"])
AM_CONDITIONAL([ENABLE_LIBIPQ], [test "$enable_libipq" = "yes"])
AM_CONDITIONAL([ENABLE_BPFC], [test "$enable_bpfc" = "yes"])
AM_CONDITIONAL([ENABLE_SYNCONF], [test "$enable_nfsynproxy" = "yes"])
AM_CONDITIONAL([ENABLE_NFTABLES], [test "$enable_nftables" = "yes"])
AM_CONDITIONAL([ENABLE_CONNLABEL], [test "$enable_connlabel" = "yes"])

# If specified explicitly on the command line, error out when library was not found
# Otherwise, disable and continue
AS_IF([test "x$enable_libnfnetlink" = "xyes"],
	[PKG_CHECK_MODULES([libnfnetlink], [libnfnetlink >= 1.0],
			   [nfnetlink=1])],
      [test "x$enable_libnfnetlink" = "xauto"],
	[PKG_CHECK_MODULES([libnfnetlink], [libnfnetlink >= 1.0],
			   [nfnetlink=1], [nfnetlink=0])])
AM_CONDITIONAL([HAVE_LIBNFNETLINK], [test "$nfnetlink" = 1])

if test "x$enable_bpfc" = "xyes" || test "x$enable_nfsynproxy" = "xyes"; then
	PKG_CHECK_MODULES([libpcap], [libpcap], [], [
		AC_MSG_ERROR(missing libpcap library required by bpf compiler or nfsynproxy tool)])
fi

if test "x$enable_nftables" = "xyes"; then
	PKG_CHECK_MODULES([libmnl], [libmnl >= 1.0], [mnl=1], [mnl=0])

	if test "$mnl" = 0;
	then
		echo "*** Error: No suitable libmnl found. ***"
		echo "    Please install the 'libmnl' package"
		echo "    Or consider --disable-nftables to skip"
		echo "    iptables-compat over nftables support."
		exit 1
	fi

	PKG_CHECK_MODULES([libnftnl], [libnftnl >= 1.2.6], [nftables=1], [nftables=0])

	if test "$nftables" = 0;
	then
		echo "*** Error: no suitable libnftnl found. ***"
		echo "    Please install the 'libnftnl' package"
		echo "    Or consider --disable-nftables to skip"
		echo "    iptables-compat over nftables support."
		exit 1
	fi
fi

AM_CONDITIONAL([HAVE_LIBMNL], [test "$mnl" = 1])
AM_CONDITIONAL([HAVE_LIBNFTNL], [test "$nftables" = 1])

if test "$nftables" != 1; then
	blacklist_b_modules="$blacklist_b_modules limit mark nflog mangle"
	blacklist_a_modules="$blacklist_a_modules mangle"
fi

if test "x$enable_connlabel" = "xyes"; then
	PKG_CHECK_MODULES([libnetfilter_conntrack],
		[libnetfilter_conntrack >= 1.0.6],
		[nfconntrack=1], [nfconntrack=0])

	if test "$nfconntrack" -ne 1; then
		blacklist_modules="$blacklist_modules connlabel";
		echo "WARNING: libnetfilter_conntrack not found, connlabel match will not be built";
		enable_connlabel="no";
	fi;
else
	blacklist_modules="$blacklist_modules connlabel";
fi;

AM_CONDITIONAL([HAVE_LIBNETFILTER_CONNTRACK], [test "$nfconntrack" = 1])

AC_SUBST([blacklist_modules])
AC_SUBST([blacklist_x_modules])
AC_SUBST([blacklist_b_modules])
AC_SUBST([blacklist_a_modules])
AC_SUBST([blacklist_4_modules])
AC_SUBST([blacklist_6_modules])

regular_CFLAGS="-Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Wlogical-op \
	-Winline -pipe";
regular_CPPFLAGS="${largefile_cppflags} -D_REENTRANT \
	-DXTABLES_LIBDIR=\\\"\${xtlibdir}\\\" -DXTABLES_INTERNAL";
kinclude_CPPFLAGS="";
if [[ -n "$kbuilddir" ]]; then
	kinclude_CPPFLAGS="$kinclude_CPPFLAGS -I$kbuilddir/include/uapi -I$kbuilddir/include";
fi;
if [[ -n "$ksourcedir" ]]; then
	kinclude_CPPFLAGS="$kinclude_CPPFLAGS -I$ksourcedir/include/uapi -I$ksourcedir/include";
fi;
pkgdatadir='${datadir}/xtables';

if test "x$enable_profiling" = "xyes"; then
	regular_CFLAGS+=" -fprofile-arcs -ftest-coverage"
	regular_LDFLAGS+=" -lgcov --coverage"
fi

AC_MSG_CHECKING([whether the build is using musl-libc])
enable_musl_build=""

AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([[#include <netinet/if_ether.h>]],
	[[
	#if ! defined(__UAPI_DEF_ETHHDR) || __UAPI_DEF_ETHHDR != 0
		#error error trying musl...
	#endif
	]]
	)],
	[enable_musl_build="yes"],[enable_musl_build="no"]
)
AC_MSG_RESULT([${enable_musl_build}])

if test "x$enable_musl_build" = "xyes"; then
	regular_CFLAGS+=" -D__UAPI_DEF_ETHHDR=0"
fi

define([EXPAND_VARIABLE],
[$2=[$]$1
if test $prefix = 'NONE'; then
	prefix="/usr/local"
fi
while true; do
  case "[$]$2" in
    *\[$]* ) eval "$2=[$]$2" ;;
    *) break ;;
  esac
done
eval "$2=[$]$2"
])dnl EXPAND_VARIABLE

AC_SUBST([regular_CFLAGS])
AC_SUBST([regular_CPPFLAGS])
AC_SUBST([noundef_LDFLAGS])
AC_SUBST([regular_LDFLAGS])
AC_SUBST([kinclude_CPPFLAGS])
AC_SUBST([kbuilddir])
AC_SUBST([ksourcedir])
AC_SUBST([xtlibdir])
AC_SUBST([pkgconfigdir])
AC_SUBST([pkgdatadir])
AC_SUBST([libxtables_vcurrent])
AC_SUBST([libxtables_vage])
libxtables_vmajor=$(($libxtables_vcurrent - $libxtables_vage));
AC_SUBST([libxtables_vmajor])

AC_DEFINE_UNQUOTED([XT_LOCK_NAME], "${xt_lock_name}",
	[Location of the iptables lock file])
AC_SUBST([XT_LOCK_NAME], "${xt_lock_name}")

AC_CONFIG_FILES([Makefile extensions/GNUmakefile include/Makefile
	iptables/Makefile iptables/xtables.pc
	iptables/iptables.8 iptables/iptables-extensions.8.tmpl
	iptables/iptables-save.8 iptables/iptables-restore.8
	iptables/iptables-apply.8 iptables/iptables-xml.1
	libipq/Makefile libipq/libipq.pc
	libiptc/Makefile libiptc/libiptc.pc
	libiptc/libip4tc.pc libiptc/libip6tc.pc
	libxtables/Makefile utils/Makefile
	include/xtables-version.h
	iptables/xtables-monitor.8
	utils/nfnl_osf.8
	utils/nfbpf_compile.8])
AC_OUTPUT


EXPAND_VARIABLE(xtlibdir, e_xtlibdir)
EXPAND_VARIABLE(pkgconfigdir, e_pkgconfigdir)

echo "
Iptables Configuration:
  IPv4 support:				${enable_ipv4}
  IPv6 support:				${enable_ipv6}
  Devel support:			${enable_devel}
  IPQ support:				${enable_libipq}
  Large file support:			${enable_largefile}
  BPF utils support:			${enable_bpfc}
  nfsynproxy util support:		${enable_nfsynproxy}
  nftables support:			${enable_nftables}
  connlabel support:			${enable_connlabel}
  profiling support:			${enable_profiling}

Build parameters:
  Put plugins into executable (static):	${enable_static}
  Support plugins via dlopen (shared):	${enable_shared}
  Installation prefix (--prefix):	${prefix}
  Xtables extension directory:		${e_xtlibdir}
  Pkg-config directory:			${e_pkgconfigdir}
  Xtables lock file:			${xt_lock_name}
  Build against musl-libc:		${enable_musl_build}"

if [[ -n "$ksourcedir" ]]; then
	echo "  Kernel source directory:		${ksourcedir}"
fi;
if [[ -n "$kbuilddir" ]]; then
	echo "  Kernel build directory:		${kbuilddir}"
fi;

echo "  Host:					${host}
  GCC binary:				${CC}"

test x"$blacklist_modules" = "x" || echo "
Iptables modules that will not be built: $blacklist_modules"
