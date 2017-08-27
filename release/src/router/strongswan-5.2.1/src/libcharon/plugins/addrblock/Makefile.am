AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-addrblock.la
else
plugin_LTLIBRARIES = libstrongswan-addrblock.la
endif

libstrongswan_addrblock_la_SOURCES = \
	addrblock_plugin.h addrblock_plugin.c \
	addrblock_narrow.h addrblock_narrow.c \
	addrblock_validator.h addrblock_validator.c

libstrongswan_addrblock_la_LDFLAGS = -module -avoid-version
