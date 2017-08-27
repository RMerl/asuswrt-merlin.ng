/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Apache httpd */
#define APACHE_HTTPD "/usr/sbin/httpd"

/* always defined to indicate that i18n is enabled */
/* #undef ENABLE_NLS */

/* The gettext domain name */
#define GETTEXT_PACKAGE "libsoup"

/* Whether or not apache can be used for tests */
/* #undef HAVE_APACHE */

/* Apache is 2.2.x */
/* #undef HAVE_APACHE_2_2 */

/* Apache is 2.4.x */
/* #undef HAVE_APACHE_2_4 */

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
/* #undef HAVE_BIND_TEXTDOMAIN_CODESET */

/* Whether or not curl can be used for tests */
#define HAVE_CURL 1

/* Define to 1 if you have the `dcgettext' function. */
/* #undef HAVE_DCGETTEXT */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT */

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Defined if GNOME support is enabled */
/* #undef HAVE_GNOME */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if your <locale.h> file defines LC_MESSAGES. */
#define HAVE_LC_MESSAGES 1

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
#define HAVE_MMAP 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Samba's 'winbind' daemon helper 'ntlm_auth' which can be used for NTLM
   single-sign-on */
#define NTLM_AUTH "/usr/bin/ntlm_auth"

/* Name of package */
#define PACKAGE "libsoup"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://bugzilla.gnome.org/enter_bug.cgi?product=libsoup"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libsoup"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libsoup 2.41.2"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libsoup"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.41.2"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Whether or not use Samba's 'winbind' daemon helper 'ntlm_auth' for NTLM
   single-sign-on */
#define USE_NTLM_AUTH 1

/* Version number of package */
#define VERSION "2.41.2"
