AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ldap.la
else
plugin_LTLIBRARIES = libstrongswan-ldap.la
endif

libstrongswan_ldap_la_SOURCES = \
	ldap_plugin.h ldap_plugin.c ldap_fetcher.h ldap_fetcher.c

libstrongswan_ldap_la_LDFLAGS = -module -avoid-version
libstrongswan_ldap_la_LIBADD  = -lldap -llber
