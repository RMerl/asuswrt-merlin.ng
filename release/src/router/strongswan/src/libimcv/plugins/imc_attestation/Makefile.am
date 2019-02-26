AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv \
	-I$(top_srcdir)/src/libtpmtss 

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

imcv_LTLIBRARIES = imc-attestation.la

imc_attestation_la_LIBADD = $(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

imc_attestation_la_SOURCES = imc_attestation.c \
	imc_attestation_state.h imc_attestation_state.c \
	imc_attestation_process.h imc_attestation_process.c

imc_attestation_la_LDFLAGS = -module -avoid-version -no-undefined
