AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-padlock.la
else
plugin_LTLIBRARIES = libstrongswan-padlock.la
endif

libstrongswan_padlock_la_SOURCES = \
	padlock_plugin.h padlock_plugin.c \
	padlock_aes_crypter.c padlock_aes_crypter.h \
	padlock_sha1_hasher.c padlock_sha1_hasher.h \
	padlock_rng.c padlock_rng.h

libstrongswan_padlock_la_LDFLAGS = -module -avoid-version
