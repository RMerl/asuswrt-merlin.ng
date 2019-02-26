AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-led.la
else
plugin_LTLIBRARIES = libstrongswan-led.la
endif

libstrongswan_led_la_SOURCES = led_plugin.h led_plugin.c \
	led_listener.h led_listener.c

libstrongswan_led_la_LDFLAGS = -module -avoid-version
