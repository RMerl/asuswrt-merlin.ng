AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

noinst_LTLIBRARIES =
if MONOLITHIC
noinst_LTLIBRARIES += libstrongswan-chapoly.la
else
plugin_LTLIBRARIES = libstrongswan-chapoly.la
endif

libstrongswan_chapoly_la_SOURCES = \
	chapoly_plugin.h chapoly_plugin.c \
	chapoly_drv.h chapoly_drv.c \
	chapoly_drv_portable.h chapoly_drv_portable.c \
	chapoly_aead.h chapoly_aead.c \
	chapoly_xof.h chapoly_xof.c

noinst_LTLIBRARIES += libchapoly-drv-ssse3.la
libchapoly_drv_ssse3_la_SOURCES = chapoly_drv_ssse3.h chapoly_drv_ssse3.c
if USE_X86X64
  libchapoly_drv_ssse3_la_CFLAGS = $(PLUGIN_CFLAGS) -mssse3
endif

libstrongswan_chapoly_la_LIBADD = \
	libchapoly-drv-ssse3.la

libstrongswan_chapoly_la_LDFLAGS = -module -avoid-version
