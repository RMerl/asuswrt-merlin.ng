BUILT_SOURCES = grammar.h
CLEANFILES = grammar.h grammar.c lexer.c
AM_YFLAGS = -d
noinst_LTLIBRARIES = libgrammar.la
#bin_PROGRAMS = testdriver # TODO: make this conditional

libgrammar_la_SOURCES = \
	grammar.y \
	lexer.l \
	rainerscript.c \
	rainerscript.h \
	parserif.h \
	grammar.h
libgrammar_la_CPPFLAGS =  $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
libgrammar_la_LIBADD =

#testdriver_SOURCES = testdriver.c libgrammar.la
#testdriver_CPPFLAGS =  $(RSRT_CFLAGS)
#testdriver_LDADD = libgrammar.la
#testdriver_LDFLAGS = -lestr
