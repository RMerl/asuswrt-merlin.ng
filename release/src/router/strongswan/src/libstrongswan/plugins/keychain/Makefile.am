
AM_CPPFLAGS = -I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = $(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-keychain.la
else
plugin_LTLIBRARIES = libstrongswan-keychain.la
endif

libstrongswan_keychain_la_SOURCES = \
	keychain_plugin.h keychain_plugin.c \
	keychain_creds.h keychain_creds.c

libstrongswan_keychain_la_LDFLAGS = -module -avoid-version \
	-framework Security -framework CoreFoundation
