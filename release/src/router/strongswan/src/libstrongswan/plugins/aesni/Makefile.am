AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	-maes \
	-mpclmul \
	-mssse3 \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-aesni.la
else
plugin_LTLIBRARIES = libstrongswan-aesni.la
endif

libstrongswan_aesni_la_SOURCES = \
	aesni_key.h aesni_key.c \
	aesni_cbc.h aesni_cbc.c \
	aesni_ctr.h aesni_ctr.c \
	aesni_ccm.h aesni_ccm.c \
	aesni_gcm.h aesni_gcm.c \
	aesni_xcbc.h aesni_xcbc.c \
	aesni_cmac.h aesni_cmac.c \
	aesni_plugin.h aesni_plugin.c

libstrongswan_aesni_la_LDFLAGS = -module -avoid-version
