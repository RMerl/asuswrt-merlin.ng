AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(tss2_CFLAGS)

AM_LDFLAGS = \
	-no-undefined

ipseclib_LTLIBRARIES = libtpmtss.la

libtpmtss_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

if USE_TSS2
libtpmtss_la_LIBADD += $(tss2_LIBS)
endif

if USE_TROUSERS
libtpmtss_la_LIBADD += -ltspi
endif

libtpmtss_la_SOURCES = \
	tpm_tss.h tpm_tss.c \
	tpm_tss_quote_info.h tpm_tss_quote_info.c \
	tpm_tss_trousers.h tpm_tss_trousers.c \
	tpm_tss_tss2.h tpm_tss_tss2_v1.c tpm_tss_tss2_v2.c \
	tpm_tss_tss2_names.h tpm_tss_tss2_names_v1.c tpm_tss_tss2_names_v2.c

if MONOLITHIC
SUBDIRS =
else
SUBDIRS = .
endif

if STATIC_PLUGIN_CONSTRUCTORS
BUILT_SOURCES = $(srcdir)/plugin_constructors.c
CLEANFILES = $(srcdir)/plugin_constructors.c

$(srcdir)/plugin_constructors.c: $(top_srcdir)/src/libstrongswan/plugins/plugin_constructors.py
		$(AM_V_GEN) \
		$(PYTHON) $(top_srcdir)/src/libstrongswan/plugins/plugin_constructors.py ${p_plugins} > $@
endif

if USE_TPM
  SUBDIRS += plugins/tpm
if MONOLITHIC
  libtpmtss_la_LIBADD += plugins/tpm/libstrongswan-tpm.la
endif
endif
