nm_plugin_LTLIBRARIES = libnm-vpn-plugin-strongswan.la

if WITH_LIBNM_GLIB
nm_plugin_LTLIBRARIES += libnm-strongswan-properties.la
endif

libnm_vpn_plugin_strongswan_la_SOURCES = \
	nm-strongswan.c \
	nm-strongswan.h

libnm_strongswan_properties_la_SOURCES = \
	$(libnm_vpn_plugin_strongswan_la_SOURCES)

uidir = $(datadir)/gnome-vpn-properties/strongswan
ui_DATA = nm-strongswan-dialog.ui

common_CFLAGS = \
	$(GTK_CFLAGS) \
	-DUIDIR=\""$(uidir)"\" \
	-DG_DISABLE_DEPRECATED \
	-DGDK_DISABLE_DEPRECATED

libnm_vpn_plugin_strongswan_la_CFLAGS = \
	$(LIBNM_CFLAGS) \
	$(LIBNMA_CFLAGS) \
	$(common_CFLAGS)

libnm_strongswan_properties_la_CFLAGS = \
	-DNM_STRONGSWAN_OLD \
	$(LIBNM_GTK_CFLAGS) \
	$(LIBNM_GLIB_CFLAGS) \
	$(common_CFLAGS)

libnm_vpn_plugin_strongswan_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBNMA_LIBS) \
	$(LIBNM_LIBS)

libnm_strongswan_properties_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBNM_GTK_LIBS) \
	$(LIBNM_GLIB_LIBS)

libnm_vpn_plugin_strongswan_la_LDFLAGS = \
	-avoid-version

libnm_strongswan_properties_la_LDFLAGS = \
	$(libnm_vpn_plugin_strongswan_la_LDFLAGS)

EXTRA_DIST = $(ui_DATA)
