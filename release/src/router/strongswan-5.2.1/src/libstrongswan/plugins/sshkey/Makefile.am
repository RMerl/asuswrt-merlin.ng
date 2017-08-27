AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-sshkey.la
else
plugin_LTLIBRARIES = libstrongswan-sshkey.la
endif

libstrongswan_sshkey_la_SOURCES = \
	sshkey_plugin.h sshkey_plugin.c \
	sshkey_builder.h sshkey_builder.c \
	sshkey_encoder.h sshkey_encoder.c

libstrongswan_sshkey_la_LDFLAGS = -module -avoid-version
