AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) $(libiptc_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-forecast.la
else
plugin_LTLIBRARIES = libstrongswan-forecast.la
endif

libstrongswan_forecast_la_SOURCES = \
	forecast_listener.h forecast_listener.c \
	forecast_forwarder.h forecast_forwarder.c \
	forecast_plugin.h forecast_plugin.c

libstrongswan_forecast_la_LDFLAGS = -module -avoid-version
libstrongswan_forecast_la_LIBADD = $(libiptc_LIBS)
