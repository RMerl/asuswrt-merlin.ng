dnl #############################################################
dnl #
dnl #  For information about autoconf, see:
dnl #
dnl #  http://www.gnu.org/software/autoconf/
dnl #
dnl #  The recommended order is:
dnl #
dnl #  AC_INIT(file)
dnl #  0. checks for compiler, libtool, and command line options
dnl #  1. checks for programs
dnl #  2. checks for libraries
dnl #  3. checks for header files
dnl #  4. checks for typedefs
dnl #  5. checks for structures and functions
dnl #  6. checks for compiler characteristics
dnl #  7. checks for library functions
dnl #  8. checks for system services
dnl #  AC_OUTPUT([file...])
dnl #
dnl #############################################################

AC_PREREQ([2.59])
export CFLAGS LIBS LDFLAGS CPPFLAGS

AC_INIT([freeradius],[$]Id[$],[http://bugs.freeradius.org],,[http://www.freeradius.org])
AC_CONFIG_SRCDIR([src/main/radiusd.c])
AC_CONFIG_HEADER([src/include/autoconf.h])
m4_include([m4/ax_cc.m4])

dnl #############################################################
dnl #
dnl #  Custom hackery to discover version at configure time
dnl #
dnl #############################################################

RADIUSD_MAJOR_VERSION=`cat VERSION | sed 's/\..*//'`
RADIUSD_MINOR_VERSION=`cat VERSION | sed 's/^[[^\.]]*\.//' | sed 's/\..*$//'`
RADIUSD_INCRM_VERSION=`cat VERSION | sed 's/^.*\..*\.//' | sed 's/[[\.-]].*$//'`

RADIUSD_VERSION=`echo | awk -v major="$RADIUSD_MAJOR_VERSION" \
-v minor="$RADIUSD_MINOR_VERSION" \
-v incrm="$RADIUSD_INCRM_VERSION" \
'{ printf "%02i%02i%02i", major, minor, incrm }'`

dnl #
dnl #  Still useful for custom builds
dnl #
RADIUSD_VERSION_STRING=`cat VERSION`

dnl #
dnl #  Add definitions to Make.inc as it's used by various build targets
dnl #
AC_SUBST([RADIUSD_VERSION_STRING])

dnl #
dnl #  Add definitions to autoconf.h, so that the headers that we install
dnl #  contain the version number of the server.
dnl #
AC_DEFINE_UNQUOTED([RADIUSD_VERSION], [${RADIUSD_VERSION}], [Version integer in format <ma><ma><mi><mi><in><in>])
AC_DEFINE_UNQUOTED([RADIUSD_VERSION_STRING], ["${RADIUSD_VERSION_STRING}"], [Raw version string from VERSION file])

dnl #############################################################
dnl #
dnl #  Override some of the default autoconf variables such as
dnl #  CFLAGS if were building in developer mode
dnl #
dnl #############################################################

dnl #
dnl #  Enable developer features like debugging symbols.
dnl #  These checks must be done before expanding the AC_PROG_CC
dnl #  and AC_PROG_CXX macros.
dnl #
AC_ARG_ENABLE(developer,
[  --enable-developer      enables features of interest to developers.],
[ case "$enableval" in
  no)
    developer=no
    ;;
  *)
    developer=yes
  esac ]
)

dnl #
dnl #  Turn on the developer flag when taken from a git checkout (not a release)
dnl #
if test -d $srcdir/.git; then
  if test "x$developer" != "xno"; then
    AC_MSG_NOTICE([found .git directory, enabling developer build implicitly, disable with --disable-developer])
    developer="yes"
  fi
fi

dnl #
dnl #  Autoconf sets -O2 and -g by default, but this is a PITA for debugging
dnl #  so we remove the defaults if were building in developer mode, and set
dnl #  -g3 so nice things like macro values are included. Other arguments are
dnl #  added later when we know what compiler were using.
dnl #
if test "x$developer" = "xyes"; then
  : ${CFLAGS=-g3}
fi

dnl #############################################################
dnl #
dnl #  0. Checks for compiler, libtool, and command line options.
dnl #
dnl #############################################################

dnl #
dnl #  Get system information
dnl #
AC_CANONICAL_SYSTEM

dnl #
dnl #  Check for GNU cc
dnl #
AC_PROG_CC
AC_PROG_CXX

dnl #
dnl #  check for AIX, to allow us to use some BSD functions
dnl #  must be before macros that call the compiler.
dnl #
AC_AIX

AC_PROG_GCC_TRADITIONAL
AC_PROG_CC_SUNPRO
AC_PROG_RANLIB

dnl #
dnl #  Definitive check for whether the compiler is clang
dnl #
AX_CC_IS_CLANG

dnl #
dnl #  Set Default CFLAGS for GCC compatible compilers
dnl #
if test "x$GCC" = "xyes"; then
  CFLAGS="$CFLAGS -Wall -D_GNU_SOURCE"
fi

dnl #
dnl #  -Qunused-arguments means the compiler won't complain about unsupported arguments
dnl #
AX_CC_QUNUSED_ARGUMENTS_FLAG
if test "x$ax_cv_cc_qunused_arguments_flag" = "xyes"; then
  CFLAGS="$CFLAGS -Qunused-arguments"
  LDFLAGS="$LDFLAGS -Qunused-arguments"
fi

dnl #
dnl #  Compile in large (2G+) file support.
dnl #
AC_SYS_LARGEFILE

dnl #
dnl #  check for system bytesex
dnl #  AC_DEFINES WORDS_BIGENDIAN
dnl #
AC_C_BIGENDIAN

dnl #
dnl #  Find GNU Make.
dnl #
AC_CHECK_PROG(GMAKE, gmake, yes, no)
if test $GMAKE = no; then
  AC_PATH_PROG(MAKE, make, /usr/local/bin/make)
else
  AC_PATH_PROG(MAKE, gmake, /usr/local/gnu/bin/make)
fi
makever=`$ac_cv_path_MAKE --version 2>&1 | grep "GNU Make"`
if test -z "$makever"; then
  AC_MSG_ERROR([GNU Make is not installed.  Please download and install it from ftp://prep.ai.mit.edu/pub/gnu/make/ before continuing.])
fi

dnl #
dnl #  autoconf explicitly sets MAKEFLAGS and MFLAGS to '' even though we
dnl #  didn't tell it to, so we have to use FR_MAKEFLAGS.
dnl #
dnl #  determine the number of cores available and set the number of build
dnl #  processes appropriately.
dnl #
AX_SYSTEM_CORES

dnl #  Temporarily disabled because test and installation targets do not
dnl #  have dependencies set up correctly for multiple build processes.
dnl if test "x$ax_cv_system_cores" != "x"; then
dnl  : ${FR_MAKEFLAGS=-j$ax_cv_system_cores}
dnl fi
AC_SUBST(FR_MAKEFLAGS)

dnl #
dnl #  See if we have Git.
dnl #
AC_CHECK_PROG(GIT, git, yes, no)

dnl Put this in later, when all distributed modules use autoconf.
dnl AC_ARG_WITH(disablemodulefoo,
dnl [  --without-rlm_foo         Disables module compilation.  Module list:]
dnl esyscmd([find src/modules -type d -name rlm_\* -print |\
dnl   sed -e 's%src/modules/.*/% (sub)- %; s%.*/%- %' |\
dnl   awk '{print "                            "$0}']))

AC_ARG_ENABLE(strict-dependencies,
[  --enable-strict-dependencies  fail configure on lack of module dependancy.])

AC_ARG_ENABLE(werror,
[  --enable-werror         causes the build to fail if any warnings are generated.],
[ case "$enableval" in
    no)
      werror=no
    ;;
    *)
      werror=yes
  esac ]
)

dnl #
dnl #  extra argument: --with-docdir
dnl #
docdir='${datadir}/doc/freeradius'
AC_MSG_CHECKING([docdir])
AC_ARG_WITH(docdir,
[  --with-docdir=DIR       directory for documentation [DATADIR/doc/freeradius] ],
[ case "$withval" in
  no)
    docdir=no
    ;;
  yes)
    ;;
  [[\\/$]]* | ?:[[\\/]]* )
    docdir="$withval"
    ;;
  *)
    AC_MSG_ERROR([expected an absolute directory name for --with-docdir: $withval])
    ;;
  esac ]
)
AC_SUBST(docdir)
AC_MSG_RESULT($docdir)
if test "x$docdir" = xno; then
  AC_MSG_WARN([Documentation files will NOT be installed.])
fi

dnl #
dnl #  extra argument: --with-logdir
dnl #
logdir='${localstatedir}/log/radius'
AC_MSG_CHECKING(logdir)
AC_ARG_WITH(logdir,
[  --with-logdir=DIR       directory for logfiles [LOCALSTATEDIR/log/radius] ],
[ case "$withval" in
  no)
    AC_MSG_ERROR([Need logdir])
    ;;
  yes)
    ;;
  [[\\/$]]* | ?:[[\\/]]* )
    logdir="$withval"
    ;;
  *)
    AC_MSG_ERROR([expected an absolute directory name for --with-logdir: $withval])
    ;;
  esac ]
)
AC_SUBST(logdir)
AC_MSG_RESULT($logdir)

dnl #
dnl #  extra argument: --with-radacctdir
dnl #
radacctdir='${logdir}/radacct'
AC_MSG_CHECKING(radacctdir)
AC_ARG_WITH(radacctdir,
[  --with-radacctdir=DIR   directory for detail files [LOGDIR/radacct] ],
[ case "$withval" in
  no)
    AC_MSG_ERROR([Need radacctdir])
    ;;
  yes)
    ;;
  [[\\/$]]* | ?:[[\\/]]* )
    radacctdir="$withval"
    ;;
  *)
    AC_MSG_ERROR([expected an absolute directory name for --with-radacctdir: $withval])
    ;;
  esac ]
)
AC_SUBST(radacctdir)
AC_MSG_RESULT($radacctdir)

dnl #
dnl #  extra argument: --with-raddbdir
dnl #
raddbdir='${sysconfdir}/raddb'
AC_MSG_CHECKING(raddbdir)
AC_ARG_WITH(raddbdir,
[  --with-raddbdir=DIR     directory for config files [SYSCONFDIR/raddb] ],
[ case "$withval" in
  no)
    AC_MSG_ERROR([Need raddbdir])
    ;;
  yes)
    ;;
  [[\\/$]]* | ?:[[\\/]]* )
    raddbdir="$withval"
    ;;
  *)
    AC_MSG_ERROR([expected an absolute directory name for --with-raddbdir: $withval])
    ;;
  esac ]
)
AC_SUBST(raddbdir)
AC_MSG_RESULT($raddbdir)

modconfdir='${raddbdir}/mods-config'
AC_SUBST(modconfdir)

dnl #
dnl #  extra argument: --with-ascend-binary
dnl #
WITH_ASCEND_BINARY=yes
AC_ARG_WITH(ascend-binary,
[  --with-ascend-binary    include support for Ascend binary filter attributes (default=yes)],
[ case "$withval" in
  yes)
    ;;
  *)
    WITH_ASCEND_BINARY=no
  esac ]
)
if test "x$WITH_ASCEND_BINARY" = "xyes"; then
  AC_DEFINE(WITH_ASCEND_BINARY, [1], [include support for Ascend binary filter attributes])
fi

dnl #
dnl #  extra argument: --with-threads
dnl #
WITH_THREADS=yes
AC_ARG_WITH(threads,
[  --with-threads          use threads, if available.  (default=yes) ],
[ case "$withval" in
  yes)
    ;;
  *)
    WITH_THREADS=no
  esac ]
)

dnl #
dnl #  extra argument: --with-tcp
dnl #
WITH_TCP=yes
AC_ARG_WITH(tcp,
[  --with-tcp              compile in TCP support. (default=yes)],
[ case "$withval" in
  yes)
    ;;
  *)
    WITH_TCP=no
  esac ]
)
if test "x$WITH_TCP" = "xyes"; then
  AC_DEFINE(WITH_TCP, [1], [define if you want TCP support (For RADSec et al)])
fi

dnl #
dnl #  extra argument: --with-vmps
dnl #
WITH_VMPS=yes
AC_ARG_WITH(vmps,
[  --with-vmps             compile in VMPS support. (default=yes)],
[ case "$withval" in
  yes)
    ;;
  *)
    WITH_VMPS=no
  esac ]
)
if test "x$WITH_VMPS" = "xyes"; then
  AC_DEFINE(WITH_VMPS, [1], [define if you want VMPS support])
fi

dnl #
dnl #  extra argument: --with-dhcp
dnl #
WITH_DHCP=yes
AC_ARG_WITH(dhcp,
[  --with-dhcp             compile in DHCP support. (default=yes)],
[ case "$withval" in
  yes)
    ;;
  *)
    WITH_DHCP=no
  esac ]
)
if test "x$WITH_DHCP" = "xyes"; then
  AC_DEFINE(WITH_DHCP, [1], [define if you want DHCP support])
fi

dnl #
dnl #  Allow the user to specify a list of modules to be linked
dnl #  statically to the server.
dnl #
STATIC_MODULES=
AC_ARG_WITH(static_modules,
[  --with-static-modules=QUOTED-MODULE-LIST],[
  for i in $withval; do
    STATIC_MODULES="$STATIC_MODULES -dlpreopen ../modules/rlm_$i/rlm_$i.la"
  done
])

USE_SHARED_LIBS=yes
AC_ARG_WITH(shared-libs,
[  --with-shared-libs               build dynamic libraries and link against them. (default=yes)],
[ case "$withval" in
  no)
    USE_SHARED_LIBS=no
    ;;
  *)
  esac
])

MODULES=
AC_ARG_WITH(modules,
[  --with-modules=QUOTED-MODULE-LIST],[
 for i in $withval; do
   MODULES="$MODULES $i"
 done
])

dnl #
dnl #  extra argument: --with-experimental-modules
dnl #
EXPERIMENTAL=
AC_ARG_WITH(experimental-modules,
[  --with-experimental-modules      use experimental and unstable modules. (default=no, unless --enable-developer=yes) ],
[ case "$withval" in
  yes)
    EXPERIMENTAL=yes
    ;;
  no)
    EXPERIMENTAL=no
    ;;
  *)
  esac ]
)

dnl #
dnl #  extra argument: --with-udpfromto
dnl #
WITH_UDPFROMTO=yes
AC_ARG_WITH(udpfromto,
[  --with-udpfromto                 compile in UDPFROMTO support. (default=yes)],
[ case "$withval" in
  yes)
    WITH_UDPFROMTO=yes
    ;;
  *)
    WITH_UDPFROMTO=no
  esac ]
)

if test "x$WITH_UDPFROMTO" = "xyes"; then
  AC_DEFINE(WITH_UDPFROMTO, [], [define if you want udpfromto])
fi

dnl #
dnl #  extra argument: --with-openssl
dnl #
WITH_OPENSSL=yes
AC_ARG_WITH(openssl,
[  --with-openssl                   use OpenSSL. (default=yes)],
[ case "$withval" in
  no)
    WITH_OPENSSL=no
    ;;
  *)
    WITH_OPENSSL=yes
    ;;
  esac ]
)

dnl #
dnl #  extra argument: --with-openssl-includes=dir
dnl #
OPENSSL_INCLUDE_DIR=
AC_ARG_WITH(openssl-includes,
[  --with-openssl-includes=DIR      directory to look for OpenSSL include files in],
[ case "$withval" in
  *) OPENSSL_INCLUDE_DIR="$withval"
    ;;
  esac ]
)

dnl #
dnl #  extra argument: --with-openssl-libraries=dir
dnl #
OPENSSL_LIB_DIR=
AC_ARG_WITH(openssl-libraries,
[  --with-openssl-libraries=DIR     directory to look for OpenSSL library files in],
[ case "$withval" in
  *) OPENSSL_LIB_DIR="$withval"
    ;;
  esac ]
)

dnl #
dnl #  These next two arguments don't actually do anything.  They're
dnl #  place holders so that the top-level configure script can tell
dnl #  the user how to configure lower-level modules
dnl #

dnl #
dnl #  extra argument: --with-rlm-FOO-lib-dir
dnl #
AC_ARG_WITH(rlm-FOO-lib-dir,
[  --with-rlm-FOO-lib-dir=DIR       directory to look for library files used by module FOO in],
[ case "$withval" in
  *)
    ;;
  esac ]
)

dnl #
dnl #  extra argument: --with-rlm-FOO-include-dir
dnl #
AC_ARG_WITH(rlm-FOO-include-dir,
[  --with-rlm-FOO-include-dir=DIR   directory to look for include files used by module FOO in],
[ case "$withval" in
  *)
    ;;
  esac ]
)

dnl #############################################################
dnl #
dnl #  1. Checks for programs
dnl #
dnl #############################################################

CHECKRAD=checkrad
AC_PATH_PROG(PERL, perl, /usr/local/bin/perl)
if test "x$ac_cv_path_PERL" = "x"; then
  AC_MSG_WARN([perl not found - Simultaneous-Use and checkrad may not work])
fi
AC_PATH_PROG(SNMPGET, snmpget)
if test "x$ac_cv_path_SNMPGET" = "x"; then
  AC_MSG_WARN([snmpget not found - Simultaneous-Use and checkrad may not work])
fi

AC_PATH_PROG(SNMPWALK, snmpwalk)
if test "x$ac_cv_path_SNMPWALK" = "x"; then
  AC_MSG_WARN([snmpwalk not found - Simultaneous-Use and checkrad may not work])
fi

AC_PATH_PROG(RUSERS, rusers, /usr/bin/rusers)

dnl #
dnl #  FIXME This is truly gross.
dnl #
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)

AC_PATH_PROG(LOCATE,locate)
AC_PATH_PROG(DIRNAME,dirname)
AC_PATH_PROG(GREP,grep)

dnl #############################################################
dnl #
dnl #  2. Checks for libraries
dnl #
dnl #############################################################

dnl #
dnl #  If using pthreads, check for -lpthread (posix) or -lc_r (*BSD)
dnl #
old_CFLAGS=$CFLAGS
if test "x$WITH_THREADS" = "xyes"; then
  if test $ac_cv_prog_suncc = "yes"; then
    CFLAGS="$CFLAGS -mt"
  fi

  AC_CHECK_HEADERS(pthread.h, [], [ WITH_THREADS="no" ])

  dnl #
  dnl #  pthread stuff is usually in -lpthread
  dnl #  or in -lc_r, on *BSD
  dnl #
  dnl #  On Some systems, we need extra pre-processor flags, to get them to
  dnl #  to do the threading properly.
  dnl #
  AC_CHECK_LIB(pthread, pthread_create,
    [
      CFLAGS="$CFLAGS -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
      LIBS="-lpthread $LIBS"
    ],
    [
      AC_CHECK_LIB(c_r, pthread_create,
        [ CFLAGS="$CFLAGS -pthread -D_THREAD_SAFE" ],
        [ WITH_THREADS="no" ]
      )
    ]
  )
fi

dnl #
dnl #  If we have NO pthread libraries, remove any knowledge of threads.
dnl #
if test "x$WITH_THREADS" != "xyes"; then
  CFLAGS=$old_CFLAGS
  ac_cv_header_pthread_h="no"
  WITH_THREADS=no
else
  dnl #
  dnl #  We need sem_init() and friends, as they're the friendliest
  dnl #  semaphore functions for threading.
  dnl #
  dnl #  HP/UX requires linking with librt, too, to get the sem_* symbols.
  dnl #  Some systems have them in -lsem
  dnl #  Solaris has them in -lposix4
  dnl #  NetBSD has them in -lsemaphore
  dnl #

  AC_SEARCH_LIBS(sem_init, pthread sem posix4 rt semaphore,
    [],
    [AC_MSG_ERROR([[-lsem not found.  You may want to download it from ftp://ftp.to.gd-es.com/pub/BSDI/libsem.tar.bz2 or ftp://ftp.freeradius.org/pub/radius/contrib/libsem.tar.gz]])]
  )
fi

if test "x$WITH_THREADS" = "xyes"; then
  AC_DEFINE(WITH_THREADS, [1], [define if you want thread support])
fi

dnl #
dnl #  Check if we need -lsocket
dnl #
AC_CHECK_LIB(dl, dlopen)

dnl #
dnl #  Check if we need -lsocket
dnl #
AC_CHECK_LIB(socket, getsockname)

dnl #
dnl #  Check for -lresolv
dnl #  This library may be needed later.
dnl #
AC_CHECK_LIB(resolv, inet_aton)

dnl #
dnl #  Check if we need -lnsl. Usually if we want to
dnl #  link against -lsocket we need to include -lnsl as well.
dnl #
AC_CHECK_LIB(nsl, inet_ntoa)
AC_CHECK_LIB(ws2_32, htonl)

dnl #
dnl #  Check the pcap library for the RADIUS sniffer.
dnl #
PCAP_LIBS=
AC_CHECK_LIB(pcap, pcap_open_live,
  [
    PCAP_LIBS="-lpcap"
    AC_DEFINE(HAVE_LIBPCAP, 1,
      [Define to 1 if you have the `pcap' library (-lpcap).]
    )
  ],
  [
    AC_MSG_WARN([pcap library not found, silently disabling the RADIUS sniffer.])
  ]
)

VL_LIB_READLINE

dnl #############################################################
dnl #
dnl #  3. Checks for header files
dnl #
dnl #############################################################

dnl #
dnl #  Interix requires us to set -D_ALL_SOURCE, otherwise
dnl #  getopt will be #included, but won't link.  <sigh>
dnl #
dnl #
case "$host" in
  *-interix*)
    CFLAGS="$CFLAGS -D_ALL_SOURCE"
    ;;
  *-darwin*)
    CFLAGS="$CFLAGS -DDARWIN"
    LIBS="-framework DirectoryService $LIBS"
    ;;
esac

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS( \
  dlfcn.h \
  unistd.h \
  crypt.h \
  errno.h \
  resource.h \
  sys/resource.h \
  getopt.h \
  malloc.h \
  utmp.h \
  utmpx.h \
  signal.h \
  sys/select.h \
  syslog.h \
  inttypes.h \
  stdint.h \
  stdio.h \
  netdb.h \
  semaphore.h \
  arpa/inet.h \
  netinet/in.h \
  sys/types.h \
  sys/socket.h \
  winsock.h \
  utime.h \
  sys/time.h \
  sys/wait.h \
  sys/security.h \
  fcntl.h \
  sys/fcntl.h \
  sys/prctl.h \
  sys/un.h \
  glob.h \
  prot.h \
  pwd.h \
  grp.h \
  stddef.h \
  fnmatch.h \
  sia.h \
  siad.h
)

dnl #
dnl #  FreeBSD requires sys/socket.h before net/if.h
dnl #
AC_CHECK_HEADERS(net/if.h, [], [],
[#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#  endif
])

dnl #
dnl #  other checks which require headers
dnl #
if test "x$ac_cv_header_sys_security_h" = "xyes" && test "x$ac_cv_header_prot_h" = "xyes"
then
  AC_DEFINE(OSFC2, [], [define if you have OSFC2 authentication])
fi

if test "x$ac_cv_header_sia_h" = "xyes" && test "x$ac_cv_header_siad_h" = "xyes"
then
  AC_DEFINE(OSFSIA, [], [define if you have OSFSIA authentication])
fi

dnl #
dnl #  Were we told to use OpenSSL, if we were and we find an error, call AC_MSG_FAILURE and exit
dnl #
if test "x$WITH_OPENSSL" = xyes; then
  old_LIBS=$LIBS
  old_LDFLAGS="$LDFLAGS"

  OPENSSL_INCLUDE="-DNO_OPENSSL"
  OPENSSL_LIBS=
  if test "x$OPENSSL_LIB_DIR" != "x"; then
    LDFLAGS="$LDFLAGS -L$OPENSSL_LIB_DIR"
  fi

  dnl #
  dnl #  Check we can link to libssl
  dnl #
  AC_CHECK_LIB(crypto, DH_new,
    [
      LIBS="-lcrypto $LIBS"
      AC_DEFINE(HAVE_LIBCRYPTO, 1, [Define to 1 if you have the `crypto' library (-lcrypto).])
      AC_CHECK_LIB(ssl, SSL_new,
        [
          AC_DEFINE(HAVE_LIBSSL, 1, [Define to 1 if you have the `ssl' library (-lssl).])
          if test "x$OPENSSL_LIB_DIR" != "x"; then
            OPENSSL_LIBS="-L$OPENSSL_LIB_DIR"
          fi
          OPENSSL_LIBS="$OPENSSL_LIBS -lcrypto -lssl -lcrypto"
        ],
        [
          AC_MSG_FAILURE([failed linking to libssl])
        ]
      )
    ],
    []
  )

  dnl #
  dnl #  Check we can find required headers
  dnl #
  old_CPPFLAGS=$CPPFLAGS
  if test "x$OPENSSL_INCLUDE_DIR" != "x"; then
    CPPFLAGS="$CPPFLAGS -I$OPENSSL_INCLUDE_DIR"
  fi

  dnl #
  dnl #  Stupid RedHat shit
  dnl #
  CPPFLAGS="$CPPFLAGS -DOPENSSL_NO_KRB5"
  AC_CHECK_HEADERS( \
    openssl/ssl.h \
    openssl/crypto.h \
    openssl/err.h \
    openssl/evp.h \
    openssl/md5.h \
    openssl/md4.h \
    openssl/sha.h \
    openssl/ocsp.h \
    openssl/engine.h,
    [],
    [
      AC_MSG_FAILURE([failed locating OpenSSL headers])
    ]
  )

  AC_MSG_CHECKING([for OpenSSL version >= 0.9.7])
  AC_EGREP_CPP(yes,
    [#include <openssl/crypto.h>
     #if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
     yes
     #endif
    ],
    [
      AC_MSG_RESULT(yes)
    ],
    [
      AC_MSG_RESULT(no)
      AC_MSG_FAILURE([OpenSSL version too old])
    ]
  )

  if test "x$OPENSSL_INCLUDE_DIR" != "x"; then
    OPENSSL_INCLUDE="-I$OPENSSL_INCLUDE_DIR -DOPENSSL_NO_KRB5"
  else
    OPENSSL_INCLUDE="-DOPENSSL_NO_KRB5"
  fi

  CPPFLAGS=$old_CPPFLAGS
  LIBS=$old_LIBS
  LDFLAGS="$old_LDFLAGS"
fi

AC_SUBST(OPENSSL_INCLUDE)
AC_SUBST(OPENSSL_LIBS)
export OPENSSL_LIBS

dnl #
dnl #  Check the pcap includes for the RADIUS sniffer.
dnl #
if test "x$PCAP_LIBS" = x; then
  AC_MSG_NOTICE([skipping test for pcap.h.])
else
  AC_CHECK_HEADER(pcap.h,
    [
      AC_DEFINE(HAVE_PCAP_H, 1, [Define to 1 if you have the <pcap.h> header file.])

      AC_CHECK_LIB(pcap, pcap_fopen_offline,
        [
          AC_DEFINE(HAVE_PCAP_FOPEN_OFFLINE, 1, [Define to 1 if you have the function pcap_fopen_offline.])
        ]
      )

      AC_CHECK_LIB(pcap, pcap_dump_fopen,
        [
          AC_DEFINE(HAVE_PCAP_DUMP_FOPEN, 1, [Define to 1 if you have the function pcap_dump_fopen.])
        ]
      )
    ],
    [
      PCAP_LIBS=
      AC_MSG_WARN([pcap.h not found, silently disabling the RADIUS sniffer.])
    ]
  )
fi
AC_SUBST(PCAP_LIBS)

dnl #############################################################
dnl #
dnl #  4. Checks for typedefs
dnl #
dnl #############################################################

dnl #
dnl #  Ensure that these are defined
dnl #
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

dnl #
dnl #  Check for socklen_t
dnl #
FR_CHECK_TYPE_INCLUDE(
  [
    #ifdef HAVE_SYS_TYPES_H
    #  include <sys/types.h>
    #endif

    #ifdef HAVE_SYS_SOCKET_H
    #  include <sys/socket.h>
    #endif
  ],
  socklen_t, int, [socklen_t is generally 'int' on systems which don't use it]
)

dnl #
dnl #  Check for uint8_t
dnl #
FR_CHECK_TYPE_INCLUDE(
  [
    #ifdef HAVE_INTTYPES_H
    #  include <inttypes.h>
    #endif

    #ifdef HAVE_STDINT_H
    #  include <stdint.h>
    #endif
  ],
  uint8_t, unsigned char, [uint8_t should be the canonical 'octet' for network traffic]
)

dnl #
dnl #  Check for uint16_t
dnl #
FR_CHECK_TYPE_INCLUDE(
  [
    #ifdef HAVE_INTTYPES_H
    #  include <inttypes.h>
    #endif

    #ifdef HAVE_STDINT_H
    #  include <stdint.h>
    #endif
  ],
  uint16_t, unsigned short, [uint16_t should be the canonical '2 octets' for network traffic]
)

dnl #
dnl #  Check for uint32_t
dnl #
FR_CHECK_TYPE_INCLUDE(
  [
    #ifdef HAVE_INTTYPES_H
    #  include <inttypes.h>
    #endif

    #ifdef HAVE_STDINT_H
    #  include <stdint.h>
    #endif
  ],
  uint32_t, unsigned int, [uint32_t should be the canonical 'network integer']
)

AC_CHECK_TYPE(struct in6_addr, AC_DEFINE(HAVE_STRUCT_IN6_ADDR, 1, [IPv6 address structure]), [],
  [
    #ifdef HAVE_NETINET_IN_H
    #  include <netinet/in.h>
    #endif
  ]
)

AC_CHECK_TYPE(struct sockaddr_storage, AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE, 1, [Generic socket addresses]), [],
  [
    #ifdef HAVE_NETINET_IN_H
    #  include <netinet/in.h>
    #endif

    #ifdef HAVE_SYS_SOCKET_H
    #  include <sys/socket.h>
    #endif
])

AC_CHECK_TYPE(struct sockaddr_in6, AC_DEFINE(HAVE_STRUCT_SOCKADDR_IN6, 1, [IPv6 socket addresses]), [],
  [
    #ifdef HAVE_NETINET_IN_H
    #  include <netinet/in.h>
    #endif
])

AC_CHECK_TYPE(struct addrinfo, AC_DEFINE(HAVE_STRUCT_ADDRINFO, 1, [Generic DNS lookups]), [],
  [
    #ifdef HAVE_SYS_TYPES_H
    #  include <sys/types.h>
    #endif

    #ifdef HAVE_SYS_SOCKET_H
    #  include <sys/socket.h>
    #endif

    #ifdef HAVE_NETDB_H
    #  include <netdb.h>
    #endif
  ]
)

dnl #############################################################
dnl #
dnl #  5. Checks for structures and functions
dnl #
dnl #############################################################
AC_CHECK_FUNCS( \
  getopt_long \
  fcntl \
  strsignal \
  sigaction \
  sigprocmask \
  pthread_sigmask \
  snprintf \
  vsnprintf \
  setsid \
  strncasecmp \
  strcasecmp \
  localtime_r \
  ctime_r \
  gmtime_r \
  strsep \
  inet_aton \
  inet_pton \
  inet_ntop \
  setlinebuf \
  setvbuf \
  getusershell \
  initgroups \
  getaddrinfo \
  getnameinfo \
  closefrom \
  gettimeofday \
  getpeereid \
  setuid \
  setresuid \
  getresuid \
  strlcat \
  strlcpy
)

AC_TYPE_SIGNAL

dnl #
dnl #  Check if we have utmpx.h
dnl #  if so, check if struct utmpx has entry ut_xtime
dnl #  if not, set it to define ut_xtime == ut_tv.tv_sec
dnl #
if test "x$ac_cv_header_utmpx_h" = "xyes"; then
 FR_CHECK_STRUCT_HAS_MEMBER([#include <utmpx.h>], [struct utmpx], ut_xtime)
 if test "x$ac_cv_type_struct_utmpx_has_ut_xtime" = "x"; then
   AC_DEFINE(ut_xtime, ut_tv.tv_sec, [define to something if you don't have ut_xtime in struct utmpx])
 fi
fi

dnl #
dnl #  struct ip_pktinfo
dnl #
FR_CHECK_STRUCT_HAS_MEMBER([#include <netinet/in.h>], [struct in_pktinfo], ipi_addr)
if test "x$ac_cv_type_struct_in_pktinfo_has_ipi_addr" = "xyes"; then
  AC_DEFINE(HAVE_IP_PKTINFO, [], [define if you have IP_PKTINFO (Linux)])
fi

dnl #
dnl #  struct in6_pktinfo
dnl #
FR_CHECK_STRUCT_HAS_MEMBER([#include <netinet/in.h>], [struct in6_pktinfo], ipi6_addr)
if test "x$ac_cv_type_struct_in6_pktinfo_has_ipi6_addr" = "xyes"; then
  AC_DEFINE(HAVE_IN6_PKTINFO, [], [define if you have IN6_PKTINFO (Linux)])
fi

dnl #############################################################
dnl #
dnl #  6. Checks for compiler characteristics
dnl #
dnl #############################################################

dnl #
dnl #  Ensure that these are defined
dnl #
AC_C_CONST

dnl #
dnl #  See if this is OS/2
dnl #
AC_MSG_CHECKING([type of OS])
OS=`uname -s`
AC_MSG_RESULT($OS)
if test "$OS" = "OS/2"; then
  LIBPREFIX=
else
  LIBPREFIX=lib
fi
AC_SUBST(LIBPREFIX)

if test "x$developer" = "xyes"; then
  AC_MSG_NOTICE([Setting additional developer CFLAGS])

  dnl #
  dnl #  Tell the compiler to parse doxygen documentation and verify it against function and variable declarations
  dnl #
  AX_CC_WDOCUMENTATION_FLAG
  if test "x$ax_cv_cc_wdocumentation_flag" = "xyes"; then
    devflags="-Wdocumentation"
  fi

  if test "x$GCC" = "xyes"; then
    devflags="$devflags -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -W -Wredundant-decls -Wundef -Wformat-y2k -Wno-format-extra-args -Wno-format-zero-length -Wno-cast-align -Wformat-nonliteral -Wformat-security -Wformat=2 -DWITH_VERIFY_PTR=1"
    INSTALLSTRIP=""
  fi

  AC_MSG_NOTICE([Developer CFLAGS are "$devflags"])

  CFLAGS="$CFLAGS $devflags"
  dnl #
  dnl #  Enable experimental modules (we want to know if code changes breaks one of them)
  dnl #
  if test "x$EXPERIMENTAL" != "xno"; then
    AC_MSG_NOTICE([is developer build, enabling experimental modules implicitly, disable with --without-experimental-modules])
    EXPERIMENTAL=yes
  fi
else
  devflags=""
  CFLAGS="$CFLAGS -DNDEBUG"
  INSTALLSTRIP=""
fi

dnl #
dnl #  May of been set outside of this configure script
dnl #
AC_MSG_CHECKING([if building with -DNDEBUG])
if echo "$CFLAGS" | grep '\-DNDEBUG' > /dev/null; then
  AC_MSG_RESULT([yes])
  AC_DEFINE([WITH_NDEBUG], [1], [define if the server was built with -DNDEBUG])
else
  AC_MSG_RESULT([no])
fi

export EXPERIMENTAL

dnl #
dnl #  append the current git hash onto the version string
dnl #
if test -d $srcdir/.git -a "x$GIT" = "xyes"; then
  RADIUSD_VERSION_COMMIT=`git log --pretty=format:'%h' -n 1`
  AC_DEFINE_UNQUOTED([RADIUSD_VERSION_COMMIT],["${RADIUSD_VERSION_COMMIT}"],[Commit HEAD at time of configuring])
fi

FR_TLS

dnl #############################################################
dnl #
dnl #  7. Checks for library functions
dnl #
dnl #############################################################

dnl Check for talloc
dnl extra argument: --with-talloc-include-dir=DIR
talloc_include_dir=
AC_ARG_WITH(talloc-include-dir,
  [  --with-talloc-include-dir=DIR    directory to look for talloc include files in],
  [case "$withval" in
    no)
      AC_MSG_ERROR([Need talloc-include-dir])
      ;;
    yes)
      ;;
    *)
      talloc_include_dir="$withval"
      ;;
  esac])

dnl Check for talloc
dnl extra argument: --with-talloc-lib-dir=DIR
talloc_lib_dir=
AC_ARG_WITH(talloc-lib-dir,
  [  --with-talloc-lib-dir=DIR        directory to look for talloc library files in],
  [case "$withval" in
    no)
      AC_MSG_ERROR([Need talloc-lib-dir])
      ;;
    yes)
      ;;
    *)
      talloc_lib_dir="$withval"
      ;;
  esac])

dnl #
dnl # Check for talloc header files
dnl #
smart_try_dir="$talloc_include_dir"
FR_SMART_CHECK_INCLUDE([talloc.h])
if test "x$ac_cv_header_talloc_h" != "xyes"; then
  AC_MSG_WARN([talloc headers not found. Use --with-talloc-include-dir=<path>.])
  AC_MSG_ERROR([FreeRADIUS requires libtalloc])
fi

smart_try_dir="$talloc_lib_dir"
FR_SMART_CHECK_LIB(talloc, _talloc)
if test "x$ac_cv_lib_talloc__talloc" != "xyes"; then
  AC_MSG_WARN([talloc library not found. Use --with-talloc-lib-dir=<path>.])
  AC_MSG_ERROR([FreeRADIUS requires libtalloc])
fi

dnl #
dnl # Check for libcrypt
dnl # We use crypt(3) which may be in libc, or in libcrypt (eg FreeBSD)
dnl #
AC_CHECK_LIB(crypt, crypt,
  CRYPTLIB="-lcrypt"
)

if test "$CRYPTLIB" != ""; then
  AC_DEFINE(HAVE_CRYPT, [], [Do we have the crypt function])
else
  AC_CHECK_FUNC(crypt, AC_DEFINE(HAVE_CRYPT, [], [Do we have the crypt function]))
fi

dnl Check for libcipher
AC_CHECK_LIB(cipher, setkey,
   CRYPTLIB="${CRYPTLIB} -lcipher"
)
AC_SUBST(CRYPTLIB)


dnl #
dnl #  Check for regular expression support, if were using PCRE it MUST be included
dnl #  before all others, else we seem to still pickup the posix symbols for regcomp
dnl #  and regexec, which results in crashes as soon as we call any posix regex
dnl #  functions.
dnl #
dnl extra argument: --with-pcre-lib-dir
pcre_lib_dir=
AC_ARG_WITH(rlm-pcre-lib-dir,
[  --with-pcre-lib-dir=DIR          directory to look for PCRE library files in],
[ case "$withval" in
    no)
	AC_MSG_ERROR(Need rlm-pcre-lib-dir)
	;;
    yes)
	;;
    *)
	pcre_lib_dir="$withval"
	;;
  esac ]
)

dnl extra argument: --with-pcre-include--dir
pcre_include_dir=
AC_ARG_WITH(rlm-pcre-include-dir,
[  --with-pcre-include-dir=DIR      directory to look for PCRE include files in],
[ case "$withval" in
    no)
	AC_MSG_ERROR(Need rlm-pcre-include-dir)
	;;
    yes)
	;;
    *)
	pcre_include_dir="$withval"
	;;
  esac ]
)

REGEX=no
REGEX_EXTENDED=no
REGEX_PCRE=no

dnl #
dnl #  First look for PCRE
dnl #
smart_try_dir=$pcre_include_dir
FR_SMART_CHECK_INCLUDE(pcreposix.h)
if test "x$ac_cv_header_pcreposix_h" = "xyes"; then
  AC_DEFINE(HAVE_REGEX_H, [1], [define if we have any regex])
  AC_DEFINE(HAVE_PCREPOSIX_H, [1], [define this if we have the <pcreposix.h> header file])
  REGEX=yes
  REGEX_EXTENDED=yes
  REGEX_PCRE=yes
  LIBS="-lpcre -lpcreposix $LIBS"

dnl #
dnl #  Then fallback to POSIX regular expressions
dnl #
else
  smart_try_dir=
  FR_SMART_CHECK_INCLUDE(regex.h)
  if test "x$ac_cv_header_regex_h" = "xyes"; then
    AC_DEFINE(HAVE_REGEX_H, [1], [define if we have any regex])
    REGEX=yes
    AC_EGREP_CPP(yes,
      [
        #include <regex.h>
        #ifdef REG_EXTENDED
        yes
        #endif
       ],
    [AC_DEFINE(HAVE_REG_EXTENDED, [1], [define this if we have REG_EXTENDED (from <regex.h>)]) REGEX_EXTENDED=yes]
    )
dnl #
dnl # Fixme, this is needed for mingw builds, but not available on some Linux systems
dnl # despite the presence of regex.h
dnl #
dnl # LIBS="$LIBS -lregex"
  fi
fi

AC_SUBST(REGEX)
AC_SUBST(REGEX_PCRE)
AC_SUBST(REGEX_EXTENDED)

dnl #
dnl #  Check the style of gethostbyaddr, in order of preference
dnl #  GNU (_r eight args)
dnl #
AC_DEFINE(GNUSTYLE, [1], [GNU-Style get*byaddr_r])

dnl #
dnl #  SYSV (_r six args)
dnl #
AC_DEFINE(SYSVSTYLE, [2], [SYSV-Style get*byaddr_r])

dnl #
dnl #  BSD (three args, may not be thread safe)
dnl #
AC_DEFINE(BSDSTYLE, [3], [BSD-Style get*byaddr_r])

dnl #
dnl #  Tru64 has BSD version, but it is thread safe
dnl #  http://h30097.www3.hp.com/docs/base_doc/DOCUMENTATION/V51B_HTML/MAN/MAN3/1739____.HTM
dnl #  We need #stdio.h to define NULL on FreeBSD (at least)
dnl #
gethostbyaddrrstyle=""
AC_MSG_CHECKING([gethostbyaddr_r() syntax])
case "$host" in
  *-freebsd*)
    dnl #
    dnl #  With FreeBSD, check if there's a prototype for gethostbyaddr_r.
    dnl #  Some versions (FreeBSD 5.1?) have a symbol but no prototype - so we
    dnl #  override this test to BSDSTYLE. FreeBSD 6.2 and up have proper GNU
    dnl #  style support.
    dnl #
    AC_CHECK_DECLS([gethostbyaddr_r], [],
      [
        AC_DEFINE(GETHOSTBYADDRRSTYLE, BSDSTYLE,
          [style of gethostbyaddr_r functions ])
        gethostbyaddrrstyle=BSD
        AC_MSG_WARN([FreeBSD overridden to BSD-style])
      ],
      [
        #ifdef HAVE_NETDB_H
        #include <netdb.h>
        #endif
      ])
    ;;
esac

if test "x$gethostbyaddrrstyle" = "x"; then
  AC_TRY_LINK(
    [
      #include <stdio.h>
      #include <netdb.h>
    ],
    [ gethostbyaddr_r(NULL, 0, 0, NULL, NULL, 0, NULL, NULL) ],
    [
      AC_DEFINE(GETHOSTBYADDRRSTYLE, GNUSTYLE, [style of gethostbyaddr_r functions ])
      gethostbyaddrrstyle=GNU
    ]
  )
fi

if test "x$gethostbyaddrrstyle" = "x"; then
  AC_TRY_LINK(
    [
      #include <stdio.h>
      #include <netdb.h>
    ],
    [ gethostbyaddr_r(NULL, 0, 0, NULL, NULL, 0, NULL) ] ,
    [
      AC_DEFINE(GETHOSTBYADDRRSTYLE, SYSVSTYLE, [style of gethostbyaddr_r functions ])
      gethostbyaddrrstyle=SYSV
    ]
  )
fi


if test "x$gethostbyaddrrstyle" = "x"; then
  AC_TRY_LINK(
    [
      #include <stdio.h>
      #include <netdb.h>
    ],
    [ gethostbyaddr(NULL, 0, 0)  ],
    [
      AC_DEFINE(GETHOSTBYADDRRSTYLE, BSDSTYLE, [style of gethostbyaddr_r functions ])
      gethostbyaddrrstyle=BSD
    ]
  )
fi

if test "x$gethostbyaddrrstyle" = "x"; then
  AC_MSG_RESULT([none!  It must not exist, here.])
else
  AC_MSG_RESULT([${gethostbyaddrrstyle}-style])
fi

if test "x$gethostbyaddrrstyle" = "xBSD"; then
  AC_MSG_WARN([ ****** BSD-style gethostbyaddr might NOT be thread-safe! ****** ])
fi

dnl #
dnl #  Check the style of gethostbyname, in order of preference
dnl #  GNU (_r seven args)
dnl #  SYSV (_r five args)
dnl #  BSD (two args, may not be thread safe)
dnl #  Tru64 has BSD version, but it _is_ thread safe
dnl #    http://h30097.www3.hp.com/docs/base_doc/DOCUMENTATION/V51B_HTML/MAN/MAN3/1946____.HTM
dnl #  We need #stdio.h to define NULL on FreeBSD (at least)
dnl #
gethostbynamerstyle=""
AC_MSG_CHECKING([gethostbyname_r() syntax])
AC_TRY_LINK(
  [
    #include <stdio.h>
    #include <netdb.h>
  ],
  [ gethostbyname_r(NULL, NULL, NULL, 0, NULL, NULL) ],
  [
    AC_DEFINE(GETHOSTBYNAMERSTYLE, GNUSTYLE, [style of gethostbyname_r functions ])
    gethostbynamerstyle=GNU
  ]
)

if test "x$gethostbynamerstyle" = "x"; then
  AC_TRY_LINK(
    [
      #include <stdio.h>
      #include <netdb.h>
    ],
    [ gethostbyname_r(NULL, NULL, NULL, 0, NULL) ] ,
    [
      AC_DEFINE(GETHOSTBYNAMERSTYLE, SYSVSTYLE, [style of gethostbyname_r functions ])
      gethostbynamerstyle=SYSV
    ]
  )
fi

if test "x$gethostbynamerstyle" = "x"; then
  AC_TRY_LINK(
    [
      #include <stdio.h>
      #include <netdb.h>
    ],
    [ gethostbyname(NULL) ],
    [
      AC_DEFINE(GETHOSTBYNAMERSTYLE, BSDSTYLE, [style of gethostbyname_r functions ])
      gethostbynamerstyle=BSD
    ]
  )
fi

if test "x$gethostbynamerstyle" = "x"; then
  AC_MSG_RESULT([none!  It must not exist, here.])
else
  AC_MSG_RESULT([${gethostbynamerstyle}-style])
fi

if test "x$gethostbynamerstyle" = "xBSD"; then
  AC_MSG_WARN([ ****** BSD-style gethostbyname might NOT be thread-safe! ****** ])
fi

dnl #
dnl #  Check for non-posix solaris ctime_r (extra buflen int arg)
dnl #
AC_DEFINE(POSIXSTYLE, [1], [Posix-Style ctime_r])
AC_DEFINE(SOLARISSTYLE, [2], [Solaris-Style ctime_r])
ctimerstyle=""
AC_MSG_CHECKING([ctime_r() syntax])
AC_TRY_LINK(
  [
    #include <time.h>
  ],
  [ ctime_r(NULL, NULL, 0) ],
  [
    AC_DEFINE(CTIMERSTYLE, SOLARISSTYLE, [style of ctime_r function])
    ctimerstyle="SOLARIS"
  ]
)

if test "x$ctimerstyle" = "x"; then
  AC_TRY_LINK(
    [
      #include <time.h>
    ],
    [ ctime_r(NULL, NULL) ],
    [
      AC_DEFINE(CTIMERSTYLE, POSIXSTYLE, [style of ctime_r function])
      ctimerstyle="POSIX"
    ]
  )
fi

if test "x$ctimerstyle" = "x"; then
    AC_MSG_RESULT([none!  It must not exist, here.])
else
    AC_MSG_RESULT([${ctimerstyle}-style])
fi

AC_SUBST(HOSTINFO, $host)

dnl #############################################################
dnl #
dnl #  8. Checks for system services
dnl #
dnl #############################################################

dnl #
dnl #  Figure out where libtool is located,
dnl #
top_builddir=`pwd`
export top_builddir
AC_MSG_RESULT([top_builddir=$top_builddir])
dnl #  AC_SUBST(top_builddir)

dnl #
dnl #  import libtool stuff
dnl #
dnl #############################################################
dnl #
dnl #  Configure in any module directories.
dnl #
dnl #############################################################

dnl ############################################################
dnl #  Remove any conflicting definitions if autoconf.h
dnl #  is being included by a module.
dnl #############################################################
AH_BOTTOM([#include <freeradius-devel/automask.h>])

dnl ############################################################
dnl #  make modules by list
dnl #############################################################
if test "x$EXPERIMENTAL" = "xyes"; then
  for foo in `ls -1 "${srcdir}"/src/modules | grep rlm_`; do
    MODULES="$MODULES $foo"
  done
else
   dnl #
   dnl #  make ONLY the stable modules
   dnl #
   for foo in `cat "${srcdir}"/src/modules/stable`; do
      MODULES="$MODULES $foo"
   done
fi

dnl ############################################################
dnl #  Add autoconf subdirs, based on the module list we
dnl #  previously created.
dnl #############################################################
mysubdirs=""
for bar in $MODULES; do
  if test -f "${srcdir}"/src/modules/$bar/configure; then
    mysubdirs="$mysubdirs src/modules/$bar"
  fi
done

dnl #
dnl #  Don't change the variable name here.  Autoconf goes bonkers
dnl #  if you do.
dnl #
AC_CONFIG_SUBDIRS($mysubdirs)
AC_SUBST(MODULES)

dnl #############################################################
dnl #
dnl #  Add -Werror last, so it doesn't interfere with autoconf's
dnl #  test programs.
dnl #
dnl #############################################################
if test "x$werror" == "xyes"; then
  CFLAGS="-Werror $CFLAGS"
fi

dnl #############################################################
dnl #
dnl #  And finally, output the results.
dnl #
dnl #############################################################
AC_CONFIG_COMMANDS([stamp-h], [echo timestamp > src/include/stamp-h])
AC_CONFIG_COMMANDS([build-radpaths-h], [(cd ./src/include && /bin/sh ./build-radpaths-h)])
AC_CONFIG_COMMANDS([main-chmod], [(cd ./src/main && chmod +x checkrad radlast radtest)])
AC_CONFIG_COMMANDS([scripts-chmod], [(cd ./scripts && chmod +x rc.radiusd radiusd.cron.daily radiusd.cron.monthly cryptpasswd)])

dnl #
dnl #  Substitute whatever libraries we found to be necessary
dnl #
AC_SUBST(LIBS)
AC_SUBST(INSTALLSTRIP)

AC_SUBST(USE_SHARED_LIBS)
USE_STATIC_LIBS="yes"
AC_SUBST(USE_STATIC_LIBS)
AC_SUBST(STATIC_MODULES)

AC_OUTPUT(\
  ./Make.inc \
  ./src/include/build-radpaths-h \
  ./src/main/radsniff.mk \
  ./src/main/checkrad \
  ./src/main/radlast \
  ./src/main/radtest \
  ./scripts/rc.radiusd \
  ./scripts/radiusd.cron.daily \
  ./scripts/radiusd.cron.monthly \
  ./scripts/cryptpasswd \
  ./raddb/dictionary \
  ./raddb/radrelay.conf \
  ./raddb/radiusd.conf
)
