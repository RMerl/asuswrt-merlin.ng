AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtpmtss

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-tpm.la
else
plugin_LTLIBRARIES = libstrongswan-tpm.la
libstrongswan_tpm_la_LIBADD = \
	$(top_builddir)/src/libtpmtss/libtpmtss.la
endif

libstrongswan_tpm_la_SOURCES = \
	tpm_plugin.h tpm_plugin.c \
	tpm_cert.h tpm_cert.c \
	tpm_private_key.h tpm_private_key.c \
	tpm_rng.h tpm_rng.c

libstrongswan_tpm_la_LDFLAGS = -module -avoid-version
