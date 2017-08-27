# -*- Makefile -*-

AM_CPPFLAGS  = -Wall -I${top_srcdir}/include -I${top_builddir}/include -D_GNU_SOURCE -DPKGLIBDIR=\"$(pkglibdir)\" -DSYSCONFDIR=\"$(sysconfdir)\" -rdynamic
AM_LDFLAGS = \
	-L${top_builddir}/lib \
	-ldl \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

#nobase_pkglib_LTLIBRARIES = cls/basic.la cls/ematch/cmp.la
#cls_basic_la_LDFLAGS = -module -version-info 2:0:0
#cls_ematch_cmp_la_LDFLAGS = -module -version-info 2:0:0

#cls/ematch_grammar.c: cls/ematch_grammar.l
#	$(LEX) --header-file=cls/ematch_grammar.h $(LFLAGS) -o $@ $^

#cls/ematch_syntax.c: cls/ematch_syntax.y
#	$(YACC) -d $(YFLAGS) -o $@ $^

#cls/pktloc_grammar.c: cls/pktloc_grammar.l
#	$(LEX) --header-file=cls/pktloc_grammar.h $(LFLAGS) -o $@ $^

#cls/pktloc_syntax.c: cls/pktloc_syntax.y
#	$(YACC) -d $(YFLAGS) -o $@ $^

#CLEANFILES = \
#	cls/ematch_grammar.c cls/ematch_grammar.h \
#	cls/ematch_syntax.c cls/ematch_syntax.h \
#	cls/pktloc_grammar.c cls/pktloc_grammar.h \
#	cls/pktloc_syntax.c cls/pktloc_syntax.h

lib_LTLIBRARIES = \
	libnl-cli-3.la

libnl_cli_3_la_LIBADD  = ${top_builddir}/lib/libnl-3.la \
		       ${top_builddir}/lib/libnl-route-3.la \
		       ${top_builddir}/lib/libnl-nf-3.la \
		       ${top_builddir}/lib/libnl-genl-3.la

libnl_cli_3_la_SOURCES = \
	utils.c addr.c ct.c link.c neigh.c rule.c route.c \
	tc.c qdisc.c class.c cls.c
#	cls/ematch_syntax.c cls/ematch_grammar.c cls/ematch.c
#	cls/pktloc_syntax.c cls/pktloc_grammar.c cls/utils.c
