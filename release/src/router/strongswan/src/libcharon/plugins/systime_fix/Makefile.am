AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-systime-fix.la
else
plugin_LTLIBRARIES = libstrongswan-systime-fix.la
endif

libstrongswan_systime_fix_la_SOURCES = \
	systime_fix_validator.h systime_fix_validator.c \
	systime_fix_plugin.h systime_fix_plugin.c

libstrongswan_systime_fix_la_LDFLAGS = -module -avoid-version
