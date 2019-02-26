AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-curve25519.la
else
plugin_LTLIBRARIES = libstrongswan-curve25519.la
endif

libstrongswan_curve25519_la_SOURCES = \
	curve25519_dh.h curve25519_dh.c \
	curve25519_drv.h curve25519_drv.c \
	curve25519_drv_portable.h curve25519_drv_portable.c \
	curve25519_identity_hasher.h curve25519_identity_hasher.c \
	curve25519_plugin.h curve25519_plugin.c \
	curve25519_private_key.h curve25519_private_key.c \
	curve25519_public_key.h curve25519_public_key.c \
	ref10/ref10.h ref10/ref10.c ref10/base.h ref10/base2.h

libstrongswan_curve25519_la_LDFLAGS = -module -avoid-version
