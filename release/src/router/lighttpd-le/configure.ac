dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT([lighttpd], [1.4.45], [contact@lighttpd.net])
AC_CONFIG_SRCDIR([src/server.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

dnl HOW FUCKING BROKEN IS AUTOMAKE...
dnl serial-tests is not recognized before 1.12, and required for serial tests with 1.13, sane version checks "impossible"
dnl if you don't use the default (symlinked) automake on your box, set AUTOMAKE to the path you're using
m4_define([serial_tests], [
    m4_esyscmd([case `${AUTOMAKE:-automake} --version | head -n 1` in
      *1.11.*|*1.10.*|*1.9.*);;
      *) echo serial-tests;;
      esac])
])

AM_INIT_AUTOMAKE([-Wall -Wno-portability -Wno-override foreign dist-xz tar-ustar] serial_tests)

dnl enable with --enable-silent-rules or make V=0 (needs automake >= 1.11)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])


dnl @synopsis TRY_CFLAGS [compiler flags]
dnl @summary check whether compiler supports given flags and adds them to CFLAGS
AC_DEFUN([TRY_CFLAGS],
[dnl
  AC_MSG_CHECKING([if $CC supports $1])
  AC_LANG_PUSH([C])
  ac_try_cflags_saved_cflags="${CFLAGS}"
  CFLAGS="${CFLAGS} $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])],
    [
      AC_MSG_ERROR([no])
      CFLAGS="${ac_try_cflags_saved_cflags}"
    ]
  )
  AC_LANG_POP([C])
])

dnl @synopsis TRY_LDFLAGS [linker flags]
dnl @summary check whether linker supports given flags and adds them to LDFLAGS
AC_DEFUN([TRY_LDFLAGS],
[dnl
  AC_MSG_CHECKING([if $LD supports $1])
  AC_LANG_PUSH([C])
  ac_try_ldflags_saved_ldflags="${LDFLAGS}"
  LDFLAGS="${LDFLAGS} $1"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])],
    [
      AC_MSG_ERROR([no])
      LDFLAGS="${ac_try_ldflags_saved_cflags}"
    ]
  )
  AC_LANG_POP([C])
])

dnl Checks for programs.
AC_PROG_CC
AX_PROG_CC_FOR_BUILD
AM_PROG_CC_C_O
AC_PROG_LD
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_CPP
dnl AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl AM_PROG_AR is needed for some linker stuff
dnl AC_USE_SYSTEM_EXTENSIONS requires autoconf 2.60
m4_ifdef([AC_USE_SYSTEM_EXTENSIONS], [AC_USE_SYSTEM_EXTENSIONS])
dnl AM_PROG_AR requires automake 1.11 (and uses AC_COMPILE_IFELSE which wants AC_USE_SYSTEM_EXTENSIONS)
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

dnl check environment
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl AC_CANONICAL_HOST
case $host_os in
	*darwin*|*cygwin*|*aix*|*mingw*|*midipix* ) NO_RDYNAMIC=yes;;
	* ) NO_RDYNAMIC=no;;
esac
AM_CONDITIONAL(NO_RDYNAMIC, test x$NO_RDYNAMIC = xyes)

if test -z "$LIGHTTPD_STATIC"; then
  LIGHTTPD_STATIC=no
fi
AM_CONDITIONAL(LIGHTTPD_STATIC, test x$LIGHTTPD_STATIC = xyes)

AC_EXEEXT

dnl libtool
AC_DISABLE_STATIC
AC_ENABLE_SHARED

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

dnl for solaris
CPPFLAGS="${CPPFLAGS} -D_REENTRANT -D__EXTENSIONS__"

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h \
sys/socket.h sys/time.h unistd.h sys/sendfile.h sys/uio.h \
getopt.h sys/epoll.h sys/select.h poll.h sys/poll.h sys/devpoll.h sys/filio.h \
sys/mman.h sys/event.h port.h pwd.h \
sys/resource.h sys/un.h syslog.h sys/prctl.h uuid/uuid.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_CHAR_UNSIGNED
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_CHECK_MEMBER(struct tm.tm_gmtoff,[AC_DEFINE([HAVE_STRUCT_TM_GMTOFF],[1],[gmtoff in struct tm])],,[#include <time.h>])
AC_CHECK_TYPES(struct sockaddr_storage,,,[#include <sys/socket.h>])
AC_CHECK_TYPES(socklen_t,,,[#include <sys/types.h>
#include <sys/socket.h>])

dnl Checks for library functions.
AC_FUNC_FORK
dnl AC_FUNC_MALLOC
dnl AC_FUNC_MMAP
dnl AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME

if test -z "$PKG_CONFIG"; then
  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
fi

dnl checking for libev
AC_MSG_CHECKING(for libev support)
AC_ARG_WITH(libev,
    AC_HELP_STRING([--with-libev@<:@=PATH@:>@],[Include libev support for fdevent handlers in PATH/include and PATH/lib]),
    [WITH_LIBEV=$withval],[WITH_LIBEV=no])

LIBEV_CFLAGS=""
LIBEV_LIBS=""

if test "$WITH_LIBEV" != "no"; then
	PKG_CHECK_MODULES(LIBEV, libev, [], [
		dnl no pkg-config for libev, searching manually:

		if test "$WITH_LIBEV" != "yes"; then
			LIBEV_CFLAGS="-I$WITH_LIBEV/include"
			LIBEV_LIBS="-L$WITH_LIBEV/lib -lev"
		else
			AC_CHECK_HEADERS([ev.h],[
				AC_CHECK_LIB([ev], [ev_time], [
						LIBEV_CFLAGS=""
						LIBEV_LIBS="-lev"
					],[
					AC_MSG_ERROR([libev not found])
					]
				)],[
					AC_MSG_ERROR([libev not found])
				]
			)
		fi
	])

	AC_DEFINE([HAVE_LIBEV], [1], [libev support])
fi

AC_SUBST(LIBEV_CFLAGS)
AC_SUBST(LIBEV_LIBS)
AC_MSG_RESULT([$WITH_LIBEV])

dnl Checks for database.
MYSQL_INCLUDE=""
MYSQL_LIBS=""

AC_MSG_CHECKING(for MySQL support)
AC_ARG_WITH(mysql,
    AC_HELP_STRING([--with-mysql@<:@=PATH@:>@],[Include MySQL support. PATH is the path to 'mysql_config']),
    [WITH_MYSQL=$withval],[WITH_MYSQL=no])

if test "$WITH_MYSQL" != "no"; then
  AC_MSG_RESULT(yes)
  if test "$WITH_MYSQL" = "yes"; then
    AC_PATH_PROG(MYSQL_CONFIG, mysql_config)
  else
    MYSQL_CONFIG=$WITH_MYSQL
  fi

  if test "$MYSQL_CONFIG" = ""; then
    AC_MSG_ERROR(mysql_config is not found)
  fi
  if test \! -x $MYSQL_CONFIG; then
    AC_MSG_ERROR(mysql_config not exists or not executable, use --with-mysql=path-to-mysql_config)
  fi

  if $MYSQL_CONFIG | grep -- '--include' > /dev/null ; then
    MYSQL_INCLUDE="`$MYSQL_CONFIG --include | sed s/\'//g`"
  else
    MYSQL_INCLUDE="`$MYSQL_CONFIG --cflags | sed s/\'//g`"
  fi
  MYSQL_LIBS="`$MYSQL_CONFIG --libs | sed s/\'//g`"

  AC_MSG_CHECKING(for MySQL includes at)
  AC_MSG_RESULT($MYSQL_INCLUDE)

  AC_MSG_CHECKING(for MySQL libraries at)
  AC_MSG_RESULT($MYSQL_LIBS)
dnl check for errmsg.h, which isn't installed by some versions of 3.21
  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $MYSQL_INCLUDE"
  AC_CHECK_HEADERS(errmsg.h mysql.h)
  CPPFLAGS="$old_CPPFLAGS"

  AC_DEFINE([HAVE_MYSQL], [1], [mysql support])
else
  AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(BUILD_WITH_MYSQL, test ! $WITH_MYSQL = no)

AC_SUBST(MYSQL_LIBS)
AC_SUBST(MYSQL_INCLUDE)

dnl Check for LDAP
AC_MSG_CHECKING(for LDAP support)
AC_ARG_WITH(ldap, AC_HELP_STRING([--with-ldap],[enable LDAP support]),
[WITH_LDAP=$withval], [WITH_LDAP=no])
AC_MSG_RESULT([$withval])
if test "$WITH_LDAP" != "no"; then
 AC_CHECK_LIB(ldap, ldap_bind, [
  AC_CHECK_HEADERS([ldap.h],[
    LDAP_LIB=-lldap
    AC_DEFINE([HAVE_LIBLDAP], [1], [libldap])
    AC_DEFINE([HAVE_LDAP_H], [1])
    AC_DEFINE([LDAP_DEPRECATED], [1], [Using deprecated ldap api])
  ])
 ])
 AC_SUBST(LDAP_LIB)
 AC_CHECK_LIB(lber, ber_printf, [
  AC_CHECK_HEADERS([lber.h],[
    LBER_LIB=-llber
    AC_DEFINE([HAVE_LIBLBER], [1], [liblber])
    AC_DEFINE([HAVE_LBER_H], [1])
  ])
 ])
 AC_SUBST(LBER_LIB)
fi
AM_CONDITIONAL(BUILD_WITH_LDAP, test ! $WITH_LDAP = no)

dnl Check for xattr
AC_MSG_CHECKING(for extended attributes support)
AC_ARG_WITH(attr, AC_HELP_STRING([--with-attr],[enable extended attribute support]),
[WITH_ATTR=$withval],[WITH_ATTR=no])
AC_MSG_RESULT($withval)
if test "$WITH_ATTR" != "no"; then
  # libattr (linux only?)
  AC_CHECK_LIB(attr, attr_get, [
    AC_CHECK_HEADERS([attr/attributes.h],[
      ATTR_LIB=-lattr
      AC_DEFINE([HAVE_XATTR], [1], [libattr])
      AC_DEFINE([HAVE_ATTR_ATTRIBUTES_H], [1])
    ])
  ])
  AC_SUBST(ATTR_LIB)

  # (Free)BSD extattr
  AC_CHECK_FUNC([extattr_get_file], [
    AC_CHECK_HEADERS([sys/extattr.h],[
      AC_DEFINE([HAVE_EXTATTR], [1], [BSD extended attributes])
      AC_DEFINE([HAVE_SYS_EXTATTR_H], [1])
    ])
  ])
fi

dnl openssl on solaris needs -lsocket -lnsl
AC_SEARCH_LIBS(socket,socket)
AC_SEARCH_LIBS(gethostbyname,nsl socket)

dnl On Haiku accept() and friends are in libnetwork
AC_SEARCH_LIBS(accept,network)

dnl clock_gettime() needs -lrt with glibc < 2.17, and possibly other platforms
AC_SEARCH_LIBS([clock_gettime],[rt])

save_LIBS=$LIBS
AC_SEARCH_LIBS(dlopen,dl,[
  AC_CHECK_HEADERS([dlfcn.h],[
    if test "$ac_cv_search_dlopen" != no; then
      test "$ac_cv_search_dlopen" = "none required" || DL_LIB="$ac_cv_search_dlopen"
    fi

    AC_DEFINE([HAVE_LIBDL], [1], [libdl])
    AC_DEFINE([HAVE_DLFCN_H], [1])
  ])
])
LIBS=$save_LIBS
AC_SUBST(DL_LIB)

dnl Check for valgrind
AC_MSG_CHECKING(for valgrind)
AC_ARG_WITH(valgrind, AC_HELP_STRING([--with-valgrind],[enable internal support for valgrind]),
[WITH_VALGRIND=$withval],[WITH_VALGRIND=no])
AC_MSG_RESULT([$WITH_VALGRIND])
if test "$WITH_VALGRIND" != "no"; then
 AC_CHECK_HEADERS([valgrind/valgrind.h])
fi

dnl Checking for libunwind
AC_MSG_CHECKING(for libunwind)
AC_ARG_WITH(libunwind,
    AC_HELP_STRING([--with-libunwind],[Include libunwind support for backtraces on assert failures]),
    [WITH_LIBUNWIND=$withval],[WITH_LIBUNWIND=no])

if test "$WITH_LIBUNWIND" != "no"; then
  PKG_CHECK_MODULES(LIBUNWIND, libunwind)
  AC_DEFINE(HAVE_LIBUNWIND, 1, [Have libunwind support])
fi
AC_MSG_RESULT([$WITH_LIBUNWIND])

dnl Checking for kerberos5
AC_MSG_CHECKING(for kerberos5)
AC_ARG_WITH(krb5,
	AC_HELP_STRING([--with-krb5@<:@=DIR@:>@],[Use Kerberos 5]),
	[WITH_KRB5=$withval],[WITH_KRB5=no])
if test "$WITH_KRB5" != "no"; then
	use_krb5=yes
	if test "x$WITH_KRB5" != "xyes"; then
		CPPFLAGS="$CPPFLAGS -I$WITH_KRB5/include"
		LDFLAGS="$LDFLAGS -L$WITH_KRB5/lib"
	fi
else
	use_krb5=no
fi
AC_MSG_RESULT([$use_krb5])
AM_CONDITIONAL(BUILD_WITH_KRB5, test ! $WITH_KRB5 = no)

AC_ARG_WITH(krb5-includes,
	AC_HELP_STRING([--with-krb5-includes=DIR],[Kerberos includes]),
	[ use_krb5=yes CPPFLAGS="$CPPFLAGS -I$withval" ]
)

AC_ARG_WITH(krb5-libs,
	AC_HELP_STRING([--with-krb5-libs=DIR],[Kerberos libraries]),
	[ use_krb5=yes LDFLAGS="$LDFLAGS -L$withval" ]
)

if test "x$use_krb5" = "xyes"; then
	AC_CHECK_LIB(gssapi_krb5, gss_acquire_cred, [
		AC_CHECK_HEADERS([gssapi/gssapi_krb5.h],[
			KRB5_LIB="-lkrb5 -lgssapi_krb5"
			AC_DEFINE(HAVE_KRB5, [1], [libgssapi_krb5])
		])
	])
	if test "x$KRB5_LIB" = x; then
		AC_MSG_ERROR([gssapi_krb5 headers and/or libs where not found, install them or build with --without-krb5])
	fi
fi
AC_SUBST(KRB5_LIB)

dnl Check for openssl
AC_MSG_CHECKING(for OpenSSL)
AC_ARG_WITH(openssl,
    AC_HELP_STRING([--with-openssl@<:@=DIR@:>@],[Include openssl support (default no)]),
    [WITH_OPENSSL=$withval],[WITH_OPENSSL=no])

if test "$WITH_OPENSSL" != "no"; then
    use_openssl=yes
    if test "$WITH_OPENSSL" != "yes"; then
	CPPFLAGS="$CPPFLAGS -I$WITH_OPENSSL/include"
	LDFLAGS="$LDFLAGS -L$WITH_OPENSSL/lib"
    fi
else
    use_openssl=no
fi
AC_MSG_RESULT([$use_openssl])

AC_ARG_WITH(openssl-includes,
    AC_HELP_STRING([--with-openssl-includes=DIR],[OpenSSL includes]),
    [ use_openssl=yes CPPFLAGS="$CPPFLAGS -I$withval" ]
)

AC_ARG_WITH(openssl-libs,
    AC_HELP_STRING([--with-openssl-libs=DIR],[OpenSSL libraries]),
    [ use_openssl=yes LDFLAGS="$LDFLAGS -L$withval" ]
)

if test "x$use_openssl" = "xyes"; then
    if test "x$use_krb5" = "xyes"; then
        AC_DEFINE([USE_OPENSSL_KERBEROS], [1], [with kerberos])
    fi

    AC_CHECK_HEADERS([openssl/ssl.h])
    OLDLIBS="$LIBS"
    AC_CHECK_LIB(crypto, BIO_f_base64, [
      AC_CHECK_LIB(ssl, SSL_new, [ SSL_LIB="-lssl -lcrypto"
				 AC_DEFINE(HAVE_LIBSSL, [], [Have libssl]) ], [], [ -lcrypto "$DL_LIB" ])
    ], [], [])
    LIBS="$OLDLIBS"
    AC_SUBST(SSL_LIB)
fi

AC_MSG_CHECKING(for perl regular expressions support)
AC_ARG_WITH(pcre, AC_HELP_STRING([--with-pcre],[Enable pcre support (default yes)]),
    [WITH_PCRE=$withval],[WITH_PCRE=yes])
AC_MSG_RESULT([$WITH_PCRE])

if test "$WITH_PCRE" != "no"; then
		if test "$WITH_PCRE" != "yes"; then
			PCRE_LIB="-L$WITH_PCRE/lib -lpcre"
			CPPFLAGS="$CPPFLAGS -I$WITH_PCRE/include"
		else
			AC_PATH_PROG(PCRECONFIG, pcre-config)
			if test x"$PCRECONFIG" != x; then
				PCRE_LIB=`$PCRECONFIG --libs`
				CPPFLAGS="$CPPFLAGS `$PCRECONFIG --cflags`"
			fi
		fi

  if test x"$PCRE_LIB" != x; then
    AC_DEFINE([HAVE_LIBPCRE], [1], [libpcre])
    AC_DEFINE([HAVE_PCRE_H], [1], [pcre.h])
    AC_SUBST(PCRE_LIB)
  else
    AC_MSG_ERROR([pcre-config not found, install the pcre-devel package or build with --without-pcre])
  fi
fi

AC_MSG_CHECKING(for zlib support)
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib],[Enable zlib support for mod_compress]),
    [WITH_ZLIB=$withval],[WITH_ZLIB=yes])
AC_MSG_RESULT([$WITH_ZLIB])

if test "$WITH_ZLIB" != "no"; then
		if test "$WITH_ZLIB" != "yes"; then
			Z_LIB="-L$WITH_ZLIB -lz"
			CPPFLAGS="$CPPFLAGS -I$WITH_ZLIB"
		else
			AC_CHECK_LIB(z, deflate, [
			  AC_CHECK_HEADERS([zlib.h],[Z_LIB=-lz])
			])
		fi

  if test x"$Z_LIB" != x; then
    AC_DEFINE([HAVE_LIBZ], [1], [libz])
    AC_DEFINE([HAVE_ZLIB_H], [1])
    AC_SUBST(Z_LIB)
  else
    AC_MSG_ERROR([zlib-headers and/or libs were not found, install them or build with --without-zlib])
  fi
fi

AC_MSG_CHECKING(for bzip2 support)
AC_ARG_WITH(bzip2, AC_HELP_STRING([--with-bzip2],[Enable bzip2 support for mod_compress]),
    [WITH_BZIP2=$withval],[WITH_BZIP2=yes])
AC_MSG_RESULT([$WITH_BZIP2])

if test "$WITH_BZIP2" != "no"; then
		if test "$WITH_BZIP2" != "yes"; then
			BZ_LIB="-L$WITH_BZIP2 -lbz2"
			CPPFLAGS="$CPPFLAGS -I$WITH_BZIP2"
		else
			AC_CHECK_LIB(bz2, BZ2_bzCompress, [
			  AC_CHECK_HEADERS([bzlib.h],[BZ_LIB=-lbz2])
			])
		fi

  if test x"$BZ_LIB" != x; then
    AC_DEFINE([HAVE_LIBBZ2], [1], [libbz2])
    AC_DEFINE([HAVE_BZLIB_H], [1])
    AC_SUBST(BZ_LIB)
  else
    AC_MSG_ERROR([bzip2-headers and/or libs were not found, install them or build with --without-bzip2])
  fi
fi

dnl Check for gamin
AC_MSG_CHECKING(for FAM)
AC_ARG_WITH(fam, AC_HELP_STRING([--with-fam],[fam/gamin for reducing number of stat() calls]),
[WITH_FAM=$withval],[WITH_FAM=no])
AC_MSG_RESULT([$WITH_FAM])

if test "$WITH_FAM" != "no"; then
		if test "$WITH_FAM" != "yes"; then
			FAM_LIBS="-L$WITH_FAM -lfam"
			CPPFLAGS="$CPPFLAGS -I$WITH_FAM"
		else
			AC_CHECK_LIB(fam, FAMOpen2, [
			  AC_CHECK_HEADERS([fam.h],[FAM_LIBS=-lfam])
			])
			if test "x$FAM_LIBS" = x; then
			  PKG_CHECK_MODULES(FAM, gamin >= 0.1.0)
			fi
		fi

  if test x"$FAM_LIBS" != x; then
    OLD_LIBS=$LIBS
    LIBS=$FAM_LIBS
    AC_CHECK_FUNCS([FAMNoExists])
    LIBS=$OLD_LIBS

    AC_DEFINE([HAVE_LIBFAM], [1], [libfam])
    AC_DEFINE([HAVE_FAM_H], [1], [fam.h])
    AC_SUBST(FAM_LIBS)
  else
     AC_MSG_ERROR([fam/gamin-headers and/or libs were not found, install them or build with --without-fam])
  fi
fi

AC_MSG_CHECKING(for properties in mod_webdav)
AC_ARG_WITH(webdav-props, AC_HELP_STRING([--with-webdav-props],[properties in mod_webdav]),
[WITH_WEBDAV_PROPS=$withval],[WITH_WEBDAV_PROPS=no])
AC_MSG_RESULT([$WITH_WEBDAV_PROPS])

if test "$WITH_WEBDAV_PROPS" != "no"; then

  AC_MSG_CHECKING(for libxml2)
  AC_ARG_WITH(libxml, AC_HELP_STRING([--with-libxml],[libxml2 for properties in mod_webdav]),
  [WITH_LIBXML=$withval],[WITH_LIBXML=yes])
  AC_MSG_RESULT([$WITH_LIBXML])

  if test "$WITH_LIBXML" != "no"; then
		if test "$WITH_LIBXML" != "yes"; then
			XML_LIBS="-L$WITH_LIBXML/.libs -lxml2"
			XML_CFLAGS="-I$WITH_LIBXML/include"
		else
			PKG_CHECK_MODULES(XML, libxml-2.0)
		fi

    if test x"$XML_LIBS" != x; then
      AC_DEFINE([HAVE_LIBXML2], [1], [libxml2])
      AC_DEFINE([HAVE_LIBXML_H], [1], [libxml.h])
      AC_SUBST(XML_LIBS)
      AC_SUBST(XML_CFLAGS)
    else
      AC_MSG_ERROR([libxml2-headers and/or libs were not found, install them or build with --without-webdav-props])
    fi
  fi

  AC_MSG_CHECKING(for sqlite)
  AC_ARG_WITH(sqlite, AC_HELP_STRING([--with-sqlite],[sqlite for properties in mod_webdav]),
  [WITH_SQLITE=$withval],[WITH_SQLITE=yes])
  AC_MSG_RESULT([$WITH_SQLITE])

  if test "$WITH_SQLITE" != "no"; then
		if test "$WITH_SQLITE" != "yes"; then
			SQLITE_LIBS="-L$WITH_SQLITE/.libs -lsqlite3"
			SQLITE_CFLAGS="-I$WITH_SQLITE"
		else
			PKG_CHECK_MODULES(SQLITE, sqlite3)
		fi

    if test x"$SQLITE_LIBS" != x; then
      AC_DEFINE([HAVE_SQLITE3], [1], [libsqlite3])
      AC_DEFINE([HAVE_SQLITE3_H], [1], [sqlite3.h])
      AC_SUBST(SQLITE_LIBS)
      AC_SUBST(SQLITE_CFLAGS)
    else
      AC_MSG_ERROR([sqlite-headers and/or libs were not found, install them or build with --without-webdav-props])
    fi
  fi

  AC_MSG_CHECKING(for locks in mod_webdav)
  AC_ARG_WITH(webdav-locks, AC_HELP_STRING([--with-webdav-locks],[locks in mod_webdav]),
  [WITH_WEBDAV_LOCKS=$withval],[WITH_WEBDAV_LOCKS=no])
  AC_MSG_RESULT([$WITH_WEBDAV_LOCKS])

  if test "$WITH_WEBDAV_LOCKS" != "no"; then

    AC_MSG_CHECKING(for libuuid)
    AC_ARG_WITH(uuid, AC_HELP_STRING([--with-uuid],[uuid for locks in mod_webdav]),
    [WITH_UUID=$withval],[WITH_UUID=yes])
    AC_MSG_RESULT([$WITH_UUID])

    if test "$WITH_UUID" != "no"; then
		if test "$WITH_UUID" != "yes"; then
			UUID_LIBS="-L$WITH_UUID -luuid"
			CPPFLAGS="$CPPFLAGS -I$WITH_UUID"
		else
			AC_CHECK_LIB(uuid, uuid_unparse, [
			  AC_CHECK_HEADERS([uuid/uuid.h],[UUID_LIBS=-luuid])
			])
		fi

      if test x"$UUID_LIBS" != x; then
        AC_DEFINE([HAVE_UUID], [1], [libuuid])
        AC_DEFINE([HAVE_UUID_H], [1], [uuid/uuid.h is available])
        AC_SUBST(UUID_LIBS)
      else
        AC_MSG_ERROR([uuid-headers and/or libs were not found, install them or build with --without-webdav-locks])
      fi
    fi

  fi

fi

dnl Check for gdbm
AC_MSG_CHECKING(for gdbm)
AC_ARG_WITH(gdbm, AC_HELP_STRING([--with-gdbm],[gdbm storage for mod_trigger_b4_dl]),
[WITH_GDBM=$withval],[WITH_GDBM=no])
AC_MSG_RESULT([$WITH_GDBM])

if test "$WITH_GDBM" != "no"; then
		if test "$WITH_GDBM" != "yes"; then
			GDBM_LIB="-L$WITH_GDBM -lgdbm"
			CPPFLAGS="$CPPFLAGS -I$WITH_GDBM"
		else
			AC_CHECK_LIB(gdbm, gdbm_open, [
			  AC_CHECK_HEADERS([gdbm.h],[GDBM_LIB=-lgdbm])
			])
		fi

  if test x"$GDBM_LIB" != x; then
    AC_DEFINE([HAVE_GDBM], [1], [libgdbm])
    AC_DEFINE([HAVE_GDBM_H], [1])
    AC_SUBST(GDBM_LIB)
  else
    AC_MSG_ERROR([gdbm headers and/or libs were not found, install them or build with --without-gdbm])
  fi
fi
AM_CONDITIONAL(BUILD_WITH_GDBM, test ! $WITH_GDBM = no)

dnl Check for GeoIP
AC_MSG_CHECKING(for GeoIP)
AC_ARG_WITH(geoip, AC_HELP_STRING([--with-geoip],[IP-based geolocation lookup]),
[WITH_GEOIP=$withval],[WITH_GEOIP=no])
AC_MSG_RESULT([$WITH_GEOIP])

if test "$WITH_GEOIP" != "no"; then
		if test "$WITH_GEOIP" != "yes"; then
			GEOIP_LIB="-L$WITH_GEOIP -lGeoIP"
			CPPFLAGS="$CPPFLAGS -I$WITH_GEOIP"
		else
			AC_CHECK_LIB(GeoIP, GeoIP_country_name_by_addr, [
			  AC_CHECK_HEADERS([GeoIP.h],[GEOIP_LIB=-lGeoIP])
			])
		fi

  if test x"$GEOIP_LIB" != x; then
    AC_DEFINE([HAVE_GEOIP], [1], [libGeoIP])
    AC_DEFINE([HAVE_GEOIP_H], [1])
    AC_SUBST(GEOIP_LIB)
  else
    AC_MSG_ERROR([GeoIP headers and/or libs were not found, install them or build with --without-geoip])
  fi
fi
AM_CONDITIONAL(BUILD_WITH_GEOIP, test ! $WITH_GEOIP = no)

dnl Check for memcached
AC_MSG_CHECKING(for memcached)
AC_ARG_WITH(memcached, AC_HELP_STRING([--with-memcached],[memcached storage for mod_trigger_b4_dl]),
[WITH_MEMCACHED=$withval],[WITH_MEMCACHED=no])
AC_MSG_RESULT([$WITH_MEMCACHED])

if test "$WITH_MEMCACHED" != "no"; then
		if test "$WITH_MEMCACHED" != "yes"; then
			MEMCACHED_LIB="-L$WITH_MEMCACHED -lMEMCACHED"
			CPPFLAGS="$CPPFLAGS -I$WITH_MEMCACHED"
		else
			AC_CHECK_LIB(memcached, memcached, [
			  AC_CHECK_HEADERS([libmemcached/memcached.h],[
			    MEMCACHED_LIB=-lmemcached
			  ])
			])
		fi

  if test x"$MEMCACHED_LIB" != x; then
    AC_DEFINE([USE_MEMCACHED], [1], [libmemcached])
    AC_SUBST(MEMCACHED_LIB)
  else
    AC_MSG_ERROR([memcached headers and/or libs were not found, install them or build with --without-memcached])
  fi
fi
AM_CONDITIONAL(BUILD_WITH_MEMCACHED, test ! $WITH_MEMCACHED = no)

AM_CONDITIONAL(BUILD_MOD_TRIGGER_B4_DL, test ! $WITH_MEMCACHED = no || test ! $WITH_GDBM = no)

dnl Check for lua
AC_MSG_CHECKING(if lua-support is requested)
AC_ARG_WITH(lua, AC_HELP_STRING([--with-lua],[lua engine for mod_cml]),
[WITH_LUA=$withval],[WITH_LUA=no])

AC_MSG_RESULT($WITH_LUA)
if test "$WITH_LUA" != "no"; then
  found_lua=0
  if test "$WITH_LUA" != "yes"; then
    PKG_CHECK_MODULES(LUA, $WITH_LUA >= 5.1, [
      found_lua=1
    ],[
      AC_MSG_NOTICE([Couldn't find requested lua pkg-config module $WITH_LUA])
    ])
    if test "$found_lua" = "0"; then
      LUA_LIBS="-L$WITH_LUA -llua"
      LUA_CFLAGS="-I$WITH_LUA"
    fi
  else
    for luaname in lua5.3 lua-5.3 lua5.2 lua-5.2 lua5.1 lua-5.1 lua; do
      if test "$found_lua" = "0"; then
        PKG_CHECK_MODULES(LUA, $luaname >= 5.1, [
          found_lua=1
        ], [
          AC_MSG_NOTICE([Couldn't find $luaname])
        ])
      fi
    done
    if test "$found_lua" = "0"; then
      AC_MSG_ERROR([Couldn't find any lua pkg-config module])
    fi
  fi

  if test x"$LUA_LIBS" != x; then
    AC_DEFINE([HAVE_LUA], [1], [liblua])
    AC_DEFINE([HAVE_LUA_H], [1], [lua.h])
    AC_SUBST(LUA_LIBS)
    AC_SUBST(LUA_CFLAGS)
  else
    AC_MSG_ERROR([lua headers and/or libs were not found, install them or build with --without-lua])
  fi
fi
AM_CONDITIONAL(BUILD_WITH_LUA, test ! $WITH_LUA = no)

dnl search for crypt_r and (fallback) for crypt
save_LIBS=$LIBS
LIBS=
AC_SEARCH_LIBS([crypt_r],[crypt],[
  AC_DEFINE([HAVE_CRYPT_R], [1], [crypt_r])
  AC_CHECK_HEADERS([crypt.h],[
    AC_DEFINE([HAVE_CRYPT_H], [1], [crypt.h])
  ])

  CRYPT_LIB=$LIBS
],[
  AC_SEARCH_LIBS([crypt],[crypt],[
    AC_DEFINE([HAVE_CRYPT], [1], [crypt])
    AC_CHECK_HEADERS([crypt.h],[
      AC_DEFINE([HAVE_CRYPT_H], [1], [crypt.h])
    ])

    CRYPT_LIB=$LIBS
  ])
])
LIBS=$save_LIBS
AC_SUBST([CRYPT_LIB])

save_LIBS=$LIBS
AC_SEARCH_LIBS(sendfilev,sendfile,[
  if test "$ac_cv_search_sendfilev" != no; then
    test "$ac_cv_search_sendfilev" = "none required" || SENDFILE_LIB="$ac_cv_search_sendfilev"
    AC_DEFINE([HAVE_SENDFILEV], [1], [solaris sendfilev])
  fi
])
LIBS=$save_LIBS
AC_SUBST(SENDFILE_LIB)

case $host_os in
	*mingw* ) LIBS="$LIBS -lwsock32";;
        * ) ;;
esac

AC_CHECK_FUNCS([dup2 getcwd inet_ntoa inet_ntop inet_pton issetugid memset mmap munmap strchr \
		  strdup strerror strstr strtol sendfile  getopt socket lstat \
		  gethostbyname poll epoll_ctl getrlimit chroot \
		  getuid select signal pathconf madvise posix_fadvise posix_madvise \
		  writev sigaction sendfile64 send_file kqueue port_create localtime_r gmtime_r \
		  memset_s explicit_bzero clock_gettime pipe2 \
		  getentropy arc4random_buf jrand48 srandom getloadavg])
AC_CHECK_HEADERS([linux/random.h],[
  AC_CHECK_FUNC([getrandom], AC_DEFINE([HAVE_GETRANDOM], [1], [getrandom]))
])

AC_MSG_CHECKING(if weak symbols are supported)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
		__attribute__((weak)) void __dummy(void *x) { }
		void f(void *x) { __dummy(x); }
	]])],
	[
		AC_MSG_RESULT(yes)
		AC_DEFINE([HAVE_WEAK_SYMBOLS], [1], [weak symbols are supported])
	],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for Large File System support)
AC_ARG_ENABLE(lfs,
 AC_HELP_STRING([--enable-lfs],[Turn on Large File System (default)]),
 [case "${enableval}" in
   yes) CPPFLAGS="${CPPFLAGS} -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGE_FILES" ;;
    no) ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-lfs) ;;
  esac],[CPPFLAGS="${CPPFLAGS} -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGE_FILES"
	 enable_lfs=yes])
AC_MSG_RESULT($enableval)

AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(off_t)

if test "x$ac_cv_func_sendfile" = xyes; then
	dnl check if sendfile works
        AC_MSG_CHECKING(if sendfile works)
	if test "x$cross_compiling" = xno; then
	AC_TRY_RUN([
		    #ifdef HAVE_SYS_SENDFILE_H
		    #include <sys/sendfile.h>
		    #endif /* HAVE_SYS_SENDFILE_H */
		    #include <errno.h>
		    int main() {
		      int o = 0;
		      if (-1 == sendfile(-1, 0, &o, 0) && errno == ENOSYS) return -1;
		      return 0;
		    } ],
		    AC_MSG_RESULT(yes),
		    [ AC_MSG_RESULT(no)
		    AC_DEFINE([HAVE_SENDFILE_BROKEN], [1], [broken sendfile]) ] )
	else
	      AC_MSG_RESULT(no, cross-compiling)
	      AC_DEFINE([HAVE_SENDFILE_BROKEN], [1], [broken sendfile])
	fi
fi

dnl Check for IPv6 support

AC_ARG_ENABLE(ipv6,
 AC_HELP_STRING([--disable-ipv6],[disable IPv6 support]),
 [case "${enableval}" in
   yes) ipv6=true ;;
    no) ipv6=false ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-ipv6) ;;
  esac],[ipv6=true])

if test x$ipv6 = xtrue; then
  AC_CACHE_CHECK([for IPv6 support], ac_cv_ipv6_support,
  [AC_TRY_LINK([ #include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>], [struct sockaddr_in6 s; struct in6_addr t=in6addr_any; int i=AF_INET6; s; t.s6_addr[0] = 0; ],
  [ac_cv_ipv6_support=yes], [ac_cv_ipv6_support=no])])

  if test "$ac_cv_ipv6_support" = yes; then
    AC_DEFINE(HAVE_IPV6,1,[Whether to enable IPv6 support])
  fi
fi

# disable mmap by default; if a mmapped file gets truncated, the process gets a SIGBUS signal
# on reading the truncated area which we can't handle (yet).
# lighttpd may always use mmap with files it owns (created tmp files)
AC_ARG_ENABLE(mmap,
  AC_HELP_STRING([--enable-mmap],[use mmap if available (DANGEROUS, allows local users to trigger SIGBUS crashes)]),
  [case "${enableval}" in
   yes) mmap=true ;;
    no) mmap=false ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-mmap) ;;
  esac],[mmap=false])

if test x$mmap = xtrue; then
  AC_DEFINE(ENABLE_MMAP, [1], [Use mmap if available])
fi

dnl check for fastcgi lib, for the tests only
fastcgi_found=no
AC_CHECK_LIB(fcgi, FCGI_Accept, [
 AC_CHECK_HEADERS([fastcgi.h fastcgi/fastcgi.h],[
   fastcgi_found=yes
 ])
])

AM_CONDITIONAL(CHECK_WITH_FASTCGI, test "x$fastcgi_found" = xyes)


dnl check for extra compiler options (warning options)
if test "${GCC}" = "yes"; then
  TRY_CFLAGS([-Wall -W -Wshadow -pedantic])
  TRY_CFLAGS([-std=gnu99])
fi

AC_ARG_ENABLE(extra-warnings,
 AC_HELP_STRING([--enable-extra-warnings],[enable extra warnings (gcc specific)]),
 [case "${enableval}" in
   yes) extrawarnings=true ;;
    no) extrawarnings=false ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-extra-warnings) ;;
  esac],[extrawarnings=false])

if test x$extrawarnings = xtrue; then
  TRY_CFLAGS([-g -O2 -g2 -Wall -Wmissing-declarations -Wdeclaration-after-statement -Wcast-align -Wsign-compare -Wnested-externs -Wpointer-arith -D_FORTIFY_SOURCE=2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security])
  TRY_LDFLAGS([-Wl,--as-needed])
fi

dnl build version-id
LIGHTTPD_VERSION_ID=`echo $PACKAGE_VERSION | $AWK -F '.' '{print "(" $1 " << 16 | " $2 " << 8 | " $3 ")"}'`
AC_DEFINE_UNQUOTED([LIGHTTPD_VERSION_ID], [$LIGHTTPD_VERSION_ID], [lighttpd-version-id])

AC_CONFIG_FILES([Makefile src/Makefile
		 doc/config/conf.d/Makefile \
		 doc/config/vhosts.d/Makefile \
		 doc/config/Makefile \
		 doc/scripts/Makefile \
		 doc/initscripts/Makefile \
		 doc/systemd/Makefile \
		 doc/outdated/Makefile \
		 doc/Makefile \
		 tests/Makefile \
		 tests/docroot/Makefile \
		 tests/docroot/123/Makefile \
		 tests/docroot/www/Makefile \
		 tests/docroot/www/go/Makefile \
		 tests/docroot/www/indexfile/Makefile \
		 tests/docroot/www/expire/Makefile \
		 distribute.sh])
AC_OUTPUT


do_build="mod_cgi mod_fastcgi mod_extforward mod_proxy mod_evhost mod_simple_vhost mod_access mod_alias mod_setenv mod_usertrack mod_auth mod_authn_file mod_status mod_accesslog"
do_build="$do_build mod_rrdtool mod_secdownload mod_expire mod_compress mod_dirlisting mod_indexfile mod_userdir mod_webdav mod_staticfile mod_scgi mod_flv_streaming mod_ssi mod_deflate"

plugins="mod_rewrite mod_redirect"
features="regex-conditionals"
if test ! "x$PCRE_LIB" = x; then
	do_build="$do_build $plugins"
	enable_feature="$features"
else
	no_build="$no_build $plugins"
	disable_feature="$features"
fi

plugins="mod_trigger_b4_dl"
if test ! "x$PCRE_LIB" = x; then
if test ! "x$WITH_MEMCACHED" = xno || test ! "x$WITH_GDBM" = xno; then
	do_build="$do_build $plugins"
else
	no_build="$no_build $plugins"
fi
fi

plugins="mod_authn_mysql mod_mysql_vhost"
if test ! "x$MYSQL_LIBS" = x; then
	do_build="$do_build $plugins"
else
	no_build="$no_build $plugins"
fi

plugins="mod_cml mod_magnet"
if test ! "x$LUA_LIBS" = x; then
	do_build="$do_build $plugins"
else
	no_build="$no_build $plugins"
fi

plugins="mod_geoip"
if test ! "x$GEOIP_LIB" = x; then
	do_build="$do_build $plugins"
else
	no_build="$no_build $plugins"
fi

features="storage-gdbm"
if test ! "x$GDBM_LIB" = x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

features="storage-memcached"
if test ! "x$MEMCACHED_LIB" = x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

features="compress-gzip compress-deflate"
if test ! "x$Z_LIB" = x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

features="compress-bzip2"
if test ! "x$BZ_LIB" = x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

plugins="mod_authn_gssapi"
if test ! "x$KRB5_LIB" = x; then
	do_build="$do_build $plugins"
else
	no_build="$no_build $plugins"
fi

plugins="mod_authn_ldap"
if test ! "x$LDAP_LIB" = x; then
	do_build="$do_build $plugins"
else
	no_build="$no_build $plugins"
fi

features="network-openssl"
if test ! "x$SSL_LIB" = x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

dnl no crypt call
features="auth-crypt"
if test "$ac_cv_search_crypt" = no; then
	disable_feature="$disable_feature $features"
else
	enable_feature="$enable_feature $features"
fi

features="network-ipv6"
if test "$ac_cv_ipv6_support" = yes; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

features="large-files"
if test "$enable_lfs" = yes; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

features="stat-cache-fam"
if test ! "x$FAM_LIBS" = x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

features="webdav-properties"
if test "x$XML_LIBS" \!= x -a "x$SQLITE_LIBS" \!= x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi

features="webdav-locks"
if test "x$UUID_LIBS" \!= x; then
	enable_feature="$enable_feature $features"
else
	disable_feature="$disable_feature $features"
fi


dnl output

$ECHO
$ECHO "Plugins:"
$ECHO

$ECHO "enabled: "
for p in $do_build; do
	$ECHO "  $p"
done | sort

$ECHO "disabled: "
for p in $no_build; do
	$ECHO "  $p"
done | sort

$ECHO
$ECHO "Features:"
$ECHO

$ECHO "enabled: "
for p in $enable_feature; do
	$ECHO "  $p"
done | sort

$ECHO "disabled: "
for p in $disable_feature; do
	$ECHO "  $p"
done | sort

$ECHO
