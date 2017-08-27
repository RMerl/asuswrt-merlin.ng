AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_LDFLAGS = \
	-no-undefined

ipseclib_LTLIBRARIES = libradius.la

libradius_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

libradius_la_SOURCES = \
	radius_message.h radius_message.c \
	radius_socket.h radius_socket.c \
	radius_client.h radius_client.c \
	radius_config.h radius_config.c \
	radius_mppe.h
