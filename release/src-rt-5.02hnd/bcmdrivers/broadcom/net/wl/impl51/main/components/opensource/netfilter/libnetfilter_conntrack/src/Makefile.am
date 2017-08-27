include $(top_srcdir)/Make_global.am

#AUTOMAKE_OPTIONS = no-dependencies foreign

#EXTRA_DIST = $(man_MANS) acinclude.m4

SUBDIRS=conntrack expect

lib_LTLIBRARIES = libnetfilter_conntrack.la 

libnetfilter_conntrack_la_LIBADD = conntrack/libnfconntrack.la \
				   expect/libnfexpect.la \
				   ${LIBNFNETLINK_LIBS} ${LIBMNL_LIBS}
libnetfilter_conntrack_la_LDFLAGS = -Wc,-nostartfiles -lnfnetlink -ldl 	\
				    -version-info $(LIBVERSION)
libnetfilter_conntrack_la_SOURCES = main.c callback.c
