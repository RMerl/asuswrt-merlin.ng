include Make_global.am

ACLOCAL_AMFLAGS  = -I m4

man_MANS = conntrack.8 conntrackd.8 nfct.8 conntrackd.conf.5
EXTRA_DIST = $(man_MANS) Make_global.am doc m4 tests

SUBDIRS   = extensions src
DIST_SUBDIRS = include src extensions
LIBS = @LIBNETFILTER_CONNTRACK_LIBS@

dist-hook:
	rm -rf `find $(distdir)/doc -name *.orig`
	rm -rf `find $(distdir)/doc -name *.rej`
