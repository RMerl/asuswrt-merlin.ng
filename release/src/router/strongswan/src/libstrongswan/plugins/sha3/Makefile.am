AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-sha3.la
else
plugin_LTLIBRARIES = libstrongswan-sha3.la
endif

libstrongswan_sha3_la_SOURCES = \
	sha3_plugin.h sha3_plugin.c \
	sha3_hasher.c sha3_hasher.h \
	sha3_shake.h sha3_shake.c \
	sha3_keccak.h sha3_keccak.c

libstrongswan_sha3_la_LDFLAGS = -module -avoid-version
