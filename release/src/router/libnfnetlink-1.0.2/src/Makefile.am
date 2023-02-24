include $(top_srcdir)/Make_global.am

lib_LTLIBRARIES = libnfnetlink.la

libnfnetlink_la_LDFLAGS = -Wc,-nostartfiles	\
			  -version-info $(LIBVERSION) \
			  -Wl,--version-script=$(srcdir)/nfnl.version
libnfnetlink_la_SOURCES = libnfnetlink.c iftable.c rtnl.c nfnl.version
EXTRA_libnfnetlink_la_DEPENDENCIES = $(srcdir)/nfnl.version

noinst_HEADERS = iftable.h rtnl.h
