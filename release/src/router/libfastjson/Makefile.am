
EXTRA_DIST = README.html

SUBDIRS = . tests

lib_LTLIBRARIES = libfastjson.la
noinst_LTLIBRARIES = libfastjson-internal.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfastjson.pc

libfastjsonincludedir = $(includedir)/libfastjson
libfastjsoninclude_HEADERS = \
	atomic.h \
	json.h \
	json_object.h \
	json_object_iterator.h \
	json_object_private.h \
	json_tokener.h \
	json_util.h

libfastjson_la_CFLAGS = $(WARN_CFLAGS)
# info on version-info:
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
libfastjson_la_LDFLAGS = \
	-version-info 6:0:2 \
	-export-symbols-regex '^fjson_.*' \
	-no-undefined \
	@JSON_BSYMBOLIC_LDFLAGS@
libfastjson_la_LIBADD = libfastjson-internal.la

libfastjson_la_SOURCES = \
	json_version.c \
	json_object.c \
	json_print.c \
	json_object_iterator.c \
	json_tokener.c \
	json_util.c

libfastjson_internal_la_CFLAGS = $(WARN_CFLAGS)
libfastjson_internal_la_SOURCES = \
	arraylist.h \
	arraylist.c \
	debug.h \
	debug.c \
	printbuf.h \
	printbuf.c

ACLOCAL_AMFLAGS = -I m4
