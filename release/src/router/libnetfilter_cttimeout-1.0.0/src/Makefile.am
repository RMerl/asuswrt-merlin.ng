include $(top_srcdir)/Make_global.am
lib_LTLIBRARIES = libnetfilter_cttimeout.la

libnetfilter_cttimeout_la_LIBADD = ${LIBMNL_LIBS}
libnetfilter_cttimeout_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libnetfilter_cttimeout.map -version-info $(LIBVERSION)
libnetfilter_cttimeout_la_SOURCES = libnetfilter_cttimeout.c	\
				    libnetfilter_cttimeout.map	\
				    internal.h
