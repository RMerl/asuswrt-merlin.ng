AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DIPSEC_PIDDIR=\"${piddir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-error-notify.la
else
plugin_LTLIBRARIES = libstrongswan-error-notify.la
endif

libstrongswan_error_notify_la_SOURCES = \
	error_notify_plugin.h error_notify_plugin.c \
	error_notify_socket.h error_notify_socket.c \
	error_notify_listener.h error_notify_listener.c \
	error_notify_msg.h

libstrongswan_error_notify_la_LDFLAGS = -module -avoid-version

ipsec_PROGRAMS = error-notify
error_notify_SOURCES = error_notify.c
