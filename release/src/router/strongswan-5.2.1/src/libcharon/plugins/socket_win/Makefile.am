AM_CPPFLAGS = \
	-I${linux_headers} \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-socket-win.la
else
plugin_LTLIBRARIES = libstrongswan-socket-win.la
endif

libstrongswan_socket_win_la_SOURCES = \
	socket_win_socket.h socket_win_socket.c \
	socket_win_plugin.h socket_win_plugin.c

libstrongswan_socket_win_la_LDFLAGS = -module -avoid-version
libstrongswan_socket_win_la_LIBADD  = -lws2_32
