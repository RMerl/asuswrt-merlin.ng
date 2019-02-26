AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-files.la
else
plugin_LTLIBRARIES = libstrongswan-files.la
endif

libstrongswan_files_la_SOURCES = \
	files_plugin.h files_plugin.c files_fetcher.c files_fetcher.h

libstrongswan_files_la_LDFLAGS = -module -avoid-version
