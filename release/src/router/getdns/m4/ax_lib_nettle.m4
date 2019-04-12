# ===========================================================================
#      https://www.gnu.org/software/autoconf-archive/ax_lib_nettle.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_NETTLE([yes|no|auto])
#
# DESCRIPTION
#
#   Searches for the 'nettle' library with the --with... option.
#
#   If found, define HAVE_NETTLE and macro NETTLE_LIBS. Also defines
#   NETTLE_WITH_<algo> for the algorithms found available. Possible
#   algorithms: AES ARCTWO BLOWFISH CAST128 DES DES3 SERPENT TWOFISH MD2 MD4
#   MD5 SHA1 SHA256.
#
#   The argument is used if no --with...-nettle option is set. Value "yes"
#   requires the configuration by default. Value "no" does not require it by
#   default. Value "auto" configures the library only if available.
#
#   See also AX_LIB_BEECRYPT, AX_LIB_CRYPTO, and AX_LIB_GCRYPT.
#
# LICENSE
#
#   Copyright (c) 2009 Fabien Coelho <autoconf.archive@coelho.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 10

# AX_CHECK_NETTLE_ALGO([name],[function])
AC_DEFUN([AX_CHECK_NETTLE_ALGO],[
  AC_CHECK_LIB([nettle], [nettle_$2],
    AC_DEFINE([NETTLE_WITH_$1],[1],[Algorithm $1 in nettle library]))
])

# AX_LIB_NETTLE([yes|no|auto])
AC_DEFUN([AX_LIB_NETTLE],[
  AC_MSG_CHECKING([whether nettle is enabled])
  AC_ARG_WITH([nettle],
        AC_HELP_STRING([--with-nettle], [Require nettle library (required with GnuTLS)]),[
    AC_MSG_RESULT([$withval])
    ax_with_nettle=$withval
  ],[
    AC_MSG_RESULT([$1])
    ax_with_nettle=$1
  ])
  if test "$ax_with_nettle" = "yes" -o "$ax_with_nettle" = "auto" ; then
    AC_CHECK_HEADERS([nettle/nettle-meta.h],[
      AC_CHECK_LIB([nettle],[nettle_base64_encode_final],[
        AC_DEFINE([HAVE_NETTLE],[1],[Nettle library is available])
	HAVE_NETTLE=1
        AC_SUBST([NETTLE_LIBS],[-lnettle])
	# ciphers
        AX_CHECK_NETTLE_ALGO([AES],[aes_encrypt])
        AX_CHECK_NETTLE_ALGO([ARCTWO],[arctwo_encrypt])
        AX_CHECK_NETTLE_ALGO([BLOWFISH],[blowfish_encrypt])
        AX_CHECK_NETTLE_ALGO([CAST128],[cast128_encrypt])
        AX_CHECK_NETTLE_ALGO([DES],[des_encrypt])
        AX_CHECK_NETTLE_ALGO([DES3],[des3_encrypt])
        AX_CHECK_NETTLE_ALGO([SERPENT],[serpent_encrypt])
        AX_CHECK_NETTLE_ALGO([TWOFISH],[twofish_encrypt])
	# digests
        AX_CHECK_NETTLE_ALGO([MD2],[md2_digest])
        AX_CHECK_NETTLE_ALGO([MD4],[md4_digest])
        AX_CHECK_NETTLE_ALGO([MD5],[md5_digest])
        AX_CHECK_NETTLE_ALGO([SHA1],[sha1_digest])
        AX_CHECK_NETTLE_ALGO([SHA256],[sha256_digest])
      ])
    ])
    # complain only if explicitly required
    if test "$ax_with_nettle" = "yes" -a "x$HAVE_NETTLE" = "x" ; then
        AC_MSG_ERROR([cannot configure required nettle library])
    fi
  fi
])
