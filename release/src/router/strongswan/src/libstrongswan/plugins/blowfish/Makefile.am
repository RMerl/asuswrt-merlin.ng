AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-blowfish.la
else
plugin_LTLIBRARIES = libstrongswan-blowfish.la
endif

libstrongswan_blowfish_la_SOURCES = \
	blowfish_plugin.h blowfish_plugin.c blowfish_crypter.c blowfish_crypter.h \
	bf_skey.c blowfish.h bf_pi.h bf_locl.h bf_enc.c

libstrongswan_blowfish_la_LDFLAGS = -module -avoid-version
