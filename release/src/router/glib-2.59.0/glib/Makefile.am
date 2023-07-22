## Process this file with automake to produce Makefile.in
include $(top_srcdir)/glib.mk

bin_PROGRAMS =

#
# Generate glibconfig.h
#
# The timestamp of the stamp file is used to indicate if glibconfig.h is
# up to date with respect to config.status.  In the usual case, changes
# to config.status will not result in changes to glibconfig.h so we
# avoid touching its timestamp (in order not to rebuild the whole tree).
#
DISTCLEANFILES += glibconfig-stamp glibconfig.h
BUILT_SOURCES += glibconfig-stamp
configexecincludedir = $(libdir)/glib-2.0/include
nodist_configexecinclude_HEADERS = glibconfig.h
glibconfig-stamp: ../config.status
	$(AM_V_GEN) cd $(top_builddir) && \
	  $(SHELL) ./config.status glib/glibconfig.h
	@touch glibconfig-stamp



if HAVE_GOOD_PRINTF
else
PRINTF_SUBDIR = gnulib
printf_la = gnulib/libgnulib.la
endif 

if USE_SYSTEM_PCRE
else
MAYBE_PCRE = pcre 
endif

SUBDIRS = libcharset $(PRINTF_SUBDIR) $(MAYBE_PCRE) update-pcre . tests
DIST_SUBDIRS = libcharset gnulib pcre update-pcre tests

AM_CPPFLAGS = 				\
	$(glib_INCLUDES) 		\
	$(pcre_inc) 			\
	-DG_LOG_DOMAIN=\"GLib\" 	\
	$(GLIB_DEBUG_FLAGS) 		\
	-DGLIB_COMPILATION 		\
	-DPCRE_STATIC

AM_CFLAGS = $(GLIB_WARN_CFLAGS)

MIRRORING_TAB_SOURCE = 				\
	glib-mirroring-tab/Makefile 		\
	glib-mirroring-tab/gen-mirroring-tab.c	\
	glib-mirroring-tab/packtab.h		\
	glib-mirroring-tab/packtab.c

# The compilation of GRegex can be disabled, but the source files must
# be distributed.
EXTRA_DIST += 			\
	glib.rc.in		\
	gen-unicode-tables.pl	\
	gregex.c		\
	gregex.h		\
	win_iconv.c		\
	libglib-gdb.py.in	\
	docs.c			\
	gconstructor.h		\
	$(MIRRORING_TAB_SOURCE)

CLEANFILES += libglib-gdb.py

# These may be in the builddir too
BUILT_EXTRA_DIST += 		\
	glib.rc

lib_LTLIBRARIES = libglib-2.0.la

if OS_WIN32_AND_DLL_COMPILATION
if MS_LIB_AVAILABLE
noinst_DATA += glib-2.0.lib

install_ms_lib_cmd = $(INSTALL) glib-2.0.lib $(DESTDIR)$(libdir)
uninstall_ms_lib_cmd = -rm $(DESTDIR)$(libdir)/glib-2.0.lib
endif
endif

install-ms-lib:
	$(install_ms_lib_cmd)

uninstall-ms-lib:
	$(uninstall_ms_lib_cmd)

deprecated_sources = \
	deprecated/gallocator.c		\
	deprecated/gcache.c		\
	deprecated/gcompletion.c	\
	deprecated/grel.c		\
	deprecated/gthread-deprecated.c

libglib_2_0_la_SOURCES = 	\
	$(deprecated_sources)	\
	glib_probes.d		\
	garcbox.c		\
	garray.c		\
	gasyncqueue.c		\
	gasyncqueueprivate.h	\
	gatomic.c		\
	gbacktrace.c		\
	gbase64.c		\
	gbitlock.c		\
	gbookmarkfile.c 	\
	gbsearcharray.h		\
	gbytes.c		\
	gbytes.h		\
	gcharset.c		\
	gcharsetprivate.h	\
	gchecksum.c		\
	gconvert.c		\
	gdataset.c		\
	gdatasetprivate.h	\
	gdate.c	 		\
	gdatetime.c	 	\
	gdir.c			\
	genviron.c		\
	gerror.c		\
	gfileutils.c		\
	ggettext.c		\
	ghash.c			\
	ghmac.c			\
	ghook.c			\
	ghostutils.c		\
	giochannel.c    	\
	gkeyfile.c		\
	glibintl.h		\
	glib_trace.h		\
	glib-init.h		\
	glib-init.c		\
	glib-private.h		\
	glib-private.c		\
	glist.c			\
	gmain-internal.h	\
	gmain.c	 		\
	gmappedfile.c		\
	gmarkup.c		\
	gmem.c			\
	gmessages.c		\
	gmirroringtable.h	\
	gnode.c			\
	goption.c		\
	gpattern.c		\
	gpoll.c			\
	gprimes.c		\
	gqsort.c		\
	gquark.c		\
	gqueue.c		\
	grand.c			\
	grcbox.c		\
	grcboxprivate.h		\
	grefcount.c		\
	grefstring.c		\
	gregex.c		\
	gscanner.c		\
	gscripttable.h		\
	gsequence.c		\
	gshell.c		\
	gslice.c		\
	gslist.c		\
	gspawn-private.h	\
	gstdio.c		\
	gstdioprivate.h		\
	gstrfuncs.c		\
	gstring.c		\
	gstringchunk.c		\
	gtestutils.c		\
	gthread.c		\
	gthreadprivate.h	\
	gthreadpool.c		\
	gtimer.c		\
	gtimezone.c	 	\
	gtranslit.c		\
	gtranslit-data.h	\
	gtrashstack.c		\
	gtree.c			\
	guniprop.c		\
	gutf8.c			\
	gunibreak.h		\
	gunibreak.c		\
	gunichartables.h	\
	gunicollate.c		\
	gunicomp.h		\
	gunidecomp.h		\
	gunidecomp.c		\
	gunicodeprivate.h	\
	gurifuncs.c 		\
	gutils.c		\
	gutilsprivate.h		\
	guuid.c			\
	gvalgrind.h		\
	gvariant.h		\
	gvariant.c		\
	gvariant-core.h		\
	gvariant-core.c		\
	gvariant-internal.h	\
	gvariant-parser.c	\
	gvariant-serialiser.h	\
	gvariant-serialiser.c	\
	gvarianttypeinfo.h	\
	gvarianttypeinfo.c	\
	gvarianttype.c		\
	gversion.c		\
	gwakeup.h		\
	gwakeup.c		\
	gprintf.c		\
	gprintfint.h		\
	valgrind.h

if OS_UNIX
libglib_2_0_la_SOURCES += glib-unix.c
endif

if THREADS_WIN32
libglib_2_0_la_SOURCES += gthread-win32.c
else
if THREADS_POSIX
libglib_2_0_la_SOURCES += gthread-posix.c
endif
endif

if PLATFORM_WIN32
libglib_2_0_la_SOURCES += gwin32.c
endif

if G_SPAWN_WIN32
libglib_2_0_la_SOURCES += gspawn-win32.c
else
libglib_2_0_la_SOURCES += gspawn.c
endif

if G_IO_CHANNEL_WIN32
libglib_2_0_la_SOURCES += giowin32.c
else
libglib_2_0_la_SOURCES += giounix.c
endif

glibincludedir=$(includedir)/glib-2.0
glibinclude_HEADERS =   \
	glib-unix.h     \
	glib-object.h	\
	glib.h

deprecatedincludedir=$(includedir)/glib-2.0/glib/deprecated
deprecatedinclude_HEADERS = \
	deprecated/gallocator.h		\
	deprecated/gcache.h		\
	deprecated/gcompletion.h	\
	deprecated/gmain.h		\
	deprecated/grel.h		\
	deprecated/gthread.h

glibsubincludedir=$(includedir)/glib-2.0/glib
glibsubinclude_HEADERS = \
	glib-autocleanups.h	\
	galloca.h	\
	garray.h	\
	gasyncqueue.h	\
	gatomic.h	\
	gbacktrace.h	\
	gbase64.h	\
	gbitlock.h	\
	gbookmarkfile.h	\
	gbytes.h	\
	gcharset.h	\
	gchecksum.h	\
	gconvert.h	\
	gdataset.h	\
	gdate.h		\
	gdatetime.h	\
	gdir.h		\
	genviron.h	\
	gerror.h	\
	gfileutils.h	\
	ggettext.h	\
	ghash.h		\
	ghmac.h		\
	ghook.h		\
	ghostutils.h	\
	gi18n.h		\
	gi18n-lib.h	\
	giochannel.h	\
	gkeyfile.h 	\
	glist.h		\
	gmacros.h	\
	gmain.h		\
	gmappedfile.h	\
	gmarkup.h	\
	gmem.h		\
	gmessages.h	\
	gnode.h		\
	goption.h	\
	gpattern.h	\
	gpoll.h		\
	gprimes.h	\
	gqsort.h	\
	gquark.h	\
	gqueue.h	\
	grand.h		\
	grcbox.h	\
	grefcount.h	\
	grefstring.h	\
	gregex.h	\
	gscanner.h	\
	gsequence.h	\
	gshell.h	\
	gslice.h	\
	gslist.h	\
	gspawn.h	\
	gstdio.h	\
	gstrfuncs.h	\
	gtestutils.h	\
	gstring.h	\
	gstringchunk.h	\
	gthread.h	\
	gthreadpool.h	\
	gtimer.h	\
	gtimezone.h	\
	gtrashstack.h	\
	gtree.h		\
	gtypes.h	\
	gunicode.h	\
	gurifuncs.h 	\
	gutils.h	\
	guuid.h		\
	gvarianttype.h	\
	gvariant.h	\
	gversion.h	\
	gversionmacros.h \
	gwin32.h	\
	gprintf.h

# This is read by gobject-introspection/misc/ and gtk-doc
glib-public-headers.txt: Makefile
	$(AM_V_GEN) echo $(glibinclude_HEADERS) $(glibsubinclude_HEADERS) > $@.tmp && mv $@.tmp $@

CLEANFILES += glib-public-headers.txt

all-local: glib-public-headers.txt

install-data-local: install-ms-lib
	@if test -f $(glibincludedir)/glist.h ; then					\
	  echo "*** Old headers found in $(glibincludedir). You should remove the" ;	\
	  echo "*** contents of this directory and type 'make install' again." ;	\
	  false ;									\
	fi

uninstall-local: uninstall-ms-lib uninstall-gdb

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32_AND_DLL_COMPILATION
glib_win32_res = glib-win32-res.o
glib_win32_res_ldflag = -Wl,$(glib_win32_res)
endif

if USE_SYSTEM_PCRE
pcre_lib = $(PCRE_LIBS)
pcre_inc = $(PCRE_CFLAGS)
else
pcre_lib = pcre/libpcre.la
pcre_inc =
endif

libglib_2_0_la_CFLAGS = $(AM_CFLAGS) $(GLIB_HIDDEN_VISIBILITY_CFLAGS) $(LIBSYSTEMD_CFLAGS)
libglib_2_0_la_LIBADD = libcharset/libcharset.la $(printf_la) @ICONV_LIBS@ @G_LIBS_EXTRA@ $(pcre_lib) $(G_THREAD_LIBS_EXTRA) $(G_THREAD_LIBS_FOR_GTHREAD) $(LIBSYSTEMD_LIBS)
libglib_2_0_la_DEPENDENCIES = libcharset/libcharset.la $(printf_la) $(glib_win32_res) $(glib_def)

libglib_2_0_la_LDFLAGS = $(GLIB_LINK_FLAGS) \
	 $(glib_win32_res_ldflag) \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic $(no_undefined)

if ENABLE_DTRACE
glib_probes.h: glib_probes.d
	$(AM_V_GEN) $(DTRACE) -C -h -s $< -o $@.tmp
	@$(SED) \
		-e "s,define STAP_HAS_SEMAPHORES 1,undef STAP_HAS_SEMAPHORES," \
		-e "s,define _SDT_HAS_SEMAPHORES 1,undef _SDT_HAS_SEMAPHORES," \
		< $@.tmp > $@ && rm -f $@.tmp

glib_probes.lo: glib_probes.d
	$(AM_V_GEN) $(LIBTOOL) --mode=compile $(AM_V_lt) --tag=CC env CFLAGS="$(CFLAGS)" $(DTRACE) -G -s $< -o $@

BUILT_SOURCES += glib_probes.h glib_probes.lo
CLEANFILES += glib_probes.h glib_probes.h.tmp
libglib_2_0_la_LIBADD += glib_probes.lo
endif

tapsetdir   = @ABS_TAPSET_DIR@
EXTRA_DIST += glib.stp.in

if ENABLE_SYSTEMTAP
tapset_DATA = libglib-2.0.so.0.@LT_CURRENT@.@LT_REVISION@.stp
CLEANFILES += $(tapset_DATA)

$(tapset_DATA): glib.stp.in Makefile
	$(AM_V_GEN)$(SED) \
		-e 's|[@]ABS_GLIB_RUNTIME_LIBDIR[@]|$(ABS_GLIB_RUNTIME_LIBDIR)|g' \
		-e 's|[@]LT_CURRENT[@]|$(LT_CURRENT)|g' \
		-e 's|[@]LT_REVISION[@]|$(LT_REVISION)|g' \
		$< > $@
endif

gspawn-win32-helper-console.c:
	echo '#define HELPER_CONSOLE' >$@
	echo '#include "gspawn-win32-helper.c"' >>$@

gspawn-win64-helper.c:
	echo '#include "gspawn-win32-helper.c"' >$@

gspawn-win64-helper-console.c:
	echo '#define HELPER_CONSOLE' >$@
	echo '#include "gspawn-win32-helper.c"' >>$@

nodist_win_helper_sources = \
	gspawn-win32-helper-console.c \
	gspawn-win64-helper.c \
	gspawn-win64-helper-console.c \
	$(NULL)

BUILT_SOURCES += $(nodist_win_helper_sources)
CLEANFILES += $(nodist_win_helper_sources)

if OS_WIN32
if OS_WIN32_X64
bin_PROGRAMS += gspawn-win64-helper gspawn-win64-helper-console
nodist_gspawn_win64_helper_SOURCES = gspawn-win64-helper.c
nodist_gspawn_win64_helper_console_SOURCES = gspawn-win64-helper-console.c
gspawn_win64_helper_LDADD = libglib-2.0.la
gspawn_win64_helper_LDFLAGS = -mwindows
gspawn_win64_helper_console_LDADD = libglib-2.0.la
else
bin_PROGRAMS += gspawn-win32-helper gspawn-win32-helper-console
nodist_gspawn_win32_helper_console_SOURCES = gspawn-win32-helper-console.c
gspawn_win32_helper_LDADD = libglib-2.0.la
gspawn_win32_helper_LDFLAGS = -mwindows
gspawn_win32_helper_console_LDADD = libglib-2.0.la
endif
endif

glib-win32-res.o: glib.rc
	$(WINDRES) glib.rc $@

if OS_UNIX

bin_PROGRAMS += gtester
gtester_SOURCES	 = gtester.c
gtester_LDADD	 = libglib-2.0.la 

bin_SCRIPTS = gtester-report
EXTRA_DIST += gtester-report.in
CLEANFILES += gtester-report

gtester-report: gtester-report.in Makefile
	$(AM_V_GEN) set -e ; \
	sed < $< > $@.tmp \
	    -e '1,1s|#!.*|#!${PYTHON}|' \
	    -e 's|[@]GLIB_VERSION[@]|${GLIB_VERSION}|' \
	  || exit $$? ; \
	  chmod +x $@.tmp ; \
	  mv $@.tmp $@

endif

glib.def: libglib-2.0.la
	$(AM_V_GEN) dumpbin.exe -exports .libs/libglib-2.0-0.dll | awk 'BEGIN { print "EXPORTS" } / +[[:digit:]]+ +[[:xdigit:]]+ +[[:xdigit:]]+/{ print $$4 }' > glib.def.tmp && mv glib.def.tmp glib.def

glib-2.0.lib: libglib-2.0.la glib.def
	$(AM_V_GEN) lib.exe -machine:@LIB_EXE_MACHINE_FLAG@ -name:libglib-2.0-$(LT_CURRENT_MINUS_AGE).dll -def:$(builddir)/glib.def -out:$@

dist-hook: $(BUILT_EXTRA_DIST)
	files='$(BUILT_EXTRA_DIST)'; \
	for f in $$files; do \
	  if test -f $$f; then d=.; else d=$(srcdir); fi; \
	  cp $$d/$$f $(distdir) || exit 1; done

# install gdb scripts
gdbdir = $(datadir)/glib-2.0/gdb
dist_gdb_DATA = glib_gdb.py

all-local: libglib-gdb.py
libglib-gdb.py: libglib-gdb.py.in
	$(AM_V_GEN) $(SED) -e "s|\@datadir\@|$(datadir)|" $(srcdir)/libglib-gdb.py.in > $(builddir)/libglib-gdb.py

if HAVE_GLIB_RUNTIME_LIBDIR
ABS_GLIB_RUNTIME_LIBDIR = $(realpath $(libdir)/$(GLIB_RUNTIME_LIBDIR))
else
ABS_GLIB_RUNTIME_LIBDIR = $(libdir)
endif

install-data-hook: libglib-gdb.py
	mkdir -p $(DESTDIR)$(datadir)/gdb/auto-load$(ABS_GLIB_RUNTIME_LIBDIR)
	$(INSTALL) $(builddir)/libglib-gdb.py $(DESTDIR)$(datadir)/gdb/auto-load$(ABS_GLIB_RUNTIME_LIBDIR)/libglib-2.0.so.0.$(LT_CURRENT).$(LT_REVISION)-gdb.py
if HAVE_GLIB_RUNTIME_LIBDIR
	mkdir -p $(DESTDIR)$(libdir)/$(GLIB_RUNTIME_LIBDIR)
	mv $(DESTDIR)$(libdir)/libglib-2.0.so.0 $(DESTDIR)$(libdir)/$(GLIB_RUNTIME_LIBDIR)
	mv $(DESTDIR)$(libdir)/libglib-2.0.so.0.$(LT_CURRENT).$(LT_REVISION) $(DESTDIR)$(libdir)/$(GLIB_RUNTIME_LIBDIR)
	rm -f $(DESTDIR)$(libdir)/libglib-2.0.so
	ln -s $(GLIB_RUNTIME_LIBDIR)/libglib-2.0.so.0.$(LT_CURRENT).$(LT_REVISION) $(DESTDIR)$(libdir)/libglib-2.0.so
endif

uninstall-gdb:
	-rm -r $(DESTDIR)$(datadir)/gdb
