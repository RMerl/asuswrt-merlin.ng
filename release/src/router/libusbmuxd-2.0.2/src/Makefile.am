AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/common

AM_CFLAGS = $(GLOBAL_CFLAGS) $(libplist_CFLAGS)
AM_LDFLAGS = $(GLOBAL_LIBS) $(libplist_LIBS)

lib_LTLIBRARIES = libusbmuxd-2.0.la
libusbmuxd_2_0_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBUSBMUXD_SO_VERSION) -no-undefined
libusbmuxd_2_0_la_LIBADD = $(top_builddir)/common/libinternalcommon.la
libusbmuxd_2_0_la_SOURCES = libusbmuxd.c

if WIN32
libusbmuxd_2_0_la_LDFLAGS += -avoid-version -static-libgcc
libusbmuxd_2_0_la_LIBADD += -lws2_32
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libusbmuxd-2.0.pc
