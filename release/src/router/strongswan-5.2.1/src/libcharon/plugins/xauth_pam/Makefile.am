AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-xauth-pam.la
else
plugin_LTLIBRARIES = libstrongswan-xauth-pam.la
endif

libstrongswan_xauth_pam_la_SOURCES = \
	xauth_pam_plugin.h xauth_pam_plugin.c \
	xauth_pam_listener.h xauth_pam_listener.c \
	xauth_pam.h xauth_pam.c

libstrongswan_xauth_pam_la_LDFLAGS = -module -avoid-version -lpam
