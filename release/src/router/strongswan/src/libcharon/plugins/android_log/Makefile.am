AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-android-log.la
else
plugin_LTLIBRARIES = libstrongswan-android-log.la
endif

libstrongswan_android_log_la_SOURCES = \
	android_log_plugin.c android_log_plugin.h \
	android_log_logger.c android_log_logger.h

libstrongswan_android_log_la_LDFLAGS = -module -avoid-version
libstrongswan_android_log_la_LIBADD  = -llog
