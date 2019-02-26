AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-pkcs11.la
else
plugin_LTLIBRARIES = libstrongswan-pkcs11.la
endif

libstrongswan_pkcs11_la_SOURCES = \
	pkcs11_plugin.h pkcs11_plugin.c pkcs11.h \
	pkcs11_library.h pkcs11_library.c \
	pkcs11_creds.h pkcs11_creds.c \
	pkcs11_private_key.h pkcs11_private_key.c \
	pkcs11_public_key.h pkcs11_public_key.c \
	pkcs11_hasher.h pkcs11_hasher.c \
	pkcs11_rng.h pkcs11_rng.c \
	pkcs11_dh.h pkcs11_dh.c \
	pkcs11_manager.h pkcs11_manager.c

libstrongswan_pkcs11_la_LDFLAGS = -module -avoid-version
