AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(MYSQLCFLAG) \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-mysql.la
else
plugin_LTLIBRARIES = libstrongswan-mysql.la
endif

libstrongswan_mysql_la_SOURCES = \
	mysql_plugin.h mysql_plugin.c  \
	mysql_database.h mysql_database.c

libstrongswan_mysql_la_LDFLAGS = -module -avoid-version
libstrongswan_mysql_la_LIBADD  = $(MYSQLLIB)
