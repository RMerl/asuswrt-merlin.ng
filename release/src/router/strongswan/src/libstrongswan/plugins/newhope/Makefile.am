AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/math/libnttfft

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

# these files are also used by the tests, we can't directly refer to them
# because of the subdirectory, which would cause distclean to fail
noinst_LTLIBRARIES = libnewhope.la
libnewhope_la_SOURCES = \
	newhope_ke.h newhope_ke.c \
	newhope_noise.h newhope_noise.c \
	newhope_reconciliation.h newhope_reconciliation.c

libnewhope_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/math/libnttfft/libnttfft.la

if MONOLITHIC
noinst_LTLIBRARIES += libstrongswan-newhope.la
else
plugin_LTLIBRARIES = libstrongswan-newhope.la
endif

libstrongswan_newhope_la_SOURCES = \
	newhope_plugin.h newhope_plugin.c

libstrongswan_newhope_la_LDFLAGS = -module -avoid-version

libstrongswan_newhope_la_LIBADD = libnewhope.la


