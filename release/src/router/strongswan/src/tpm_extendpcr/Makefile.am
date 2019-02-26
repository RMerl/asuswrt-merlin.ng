bin_PROGRAMS = tpm_extendpcr

tpm_extendpcr_SOURCES = tpm_extendpcr.c

tpm_extendpcr_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtpmtss/libtpmtss.la

tpm_extendpcr.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtpmtss \
	-DIPSEC_CONFDIR=\"${sysconfdir}\"
