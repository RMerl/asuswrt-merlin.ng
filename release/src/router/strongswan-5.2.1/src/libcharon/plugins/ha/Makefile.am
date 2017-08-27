AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-DIPSEC_PIDDIR=\"${piddir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ha.la
else
plugin_LTLIBRARIES = libstrongswan-ha.la
endif

libstrongswan_ha_la_SOURCES = \
  ha_plugin.h ha_plugin.c \
  ha_message.h ha_message.c \
  ha_socket.h ha_socket.c \
  ha_tunnel.h ha_tunnel.c \
  ha_dispatcher.h ha_dispatcher.c \
  ha_segments.h ha_segments.c \
  ha_cache.h ha_cache.c \
  ha_kernel.h ha_kernel.c \
  ha_ctl.h ha_ctl.c \
  ha_ike.h ha_ike.c \
  ha_child.h ha_child.c \
  ha_attribute.h ha_attribute.c
libstrongswan_ha_la_LDFLAGS = -module -avoid-version
