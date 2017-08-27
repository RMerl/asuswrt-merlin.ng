## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.6

# The name of the module.
DOC_MODULE=glib

# The top-level SGML file.
DOC_MAIN_SGML_FILE=glib-docs.xml

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR=$(top_srcdir)/glib $(top_srcdir)/gmodule

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS=--deprecated-guards="G_DISABLE_DEPRECATED" --ignore-decorators="GLIB_VAR|G_GNUC_WARN_UNUSED_RESULT"

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS=--sgml-mode --output-format=xml --name-space=g

# Used for dependencies
HFILE_GLOB=$(top_srcdir)/glib/*.h $(top_srcdir)/gmodule/*.h
CFILE_GLOB=$(top_srcdir)/glib/*.c $(top_srcdir)/gmodule/*.c

# Ignore some private headers
IGNORE_HFILES = \
	gallocator.h		\
	gdatasetprivate.h	\
	glibintl.h		\
	gbsearcharray.h		\
	glib-private.h		\
	gmoduleconf.h		\
	gthreadprivate.h	\
	gunibreak.h		\
	gunicomp.h		\
	gunidecomp.h		\
	gunichartables.h	\
	glib_probes.h		\
	glib_trace.h		\
	libcharset.h		\
	gdebug.h		\
	gprintfint.h		\
	gmirroringtable.h	\
	gscripttable.h		\
	glib-mirroring-tab	\
	gnulib			\
	pcre			\
	update-pcre		\
	gbytesprivate.h		\
	gvariant-internal.h	\
	gvariant-serialiser.h	\
	gvariant-core.h		\
	gvarianttypeinfo.h	\
	gwakeup.h

# Images to copy into HTML directory
HTML_IMAGES =  				\
	file-name-encodings.png		\
	mainloop-states.gif

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE)
content_files =				\
	cross.xml			\
	running.xml			\
	building.xml			\
	changes.xml			\
	compiling.xml			\
	programming.xml			\
	resources.xml			\
	regex-syntax.xml		\
	version.xml			\
	glib-gettextize.xml		\
	gtester.xml			\
	gtester-report.xml		\
	gvariant-varargs.xml		\
	gvariant-text.xml

expand_content_files = 			\
	compiling.xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS=--extra-dir=$(srcdir)/../gobject/html --extra-dir=$(srcdir)/../gio/html

# include common portion ...
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
EXTRA_DIST +=				\
	file-name-encodings.png		\
	file-name-encodings.sxd		\
	mainloop-states.fig		\
	mainloop-states.png		\
	mainloop-states.eps		\
	version.xml.in

########################################################################

man_MANS =

if ENABLE_MAN

man_MANS += \
	glib-gettextize.1 	\
	gtester.1		\
	gtester-report.1

XSLTPROC_FLAGS = \
        --nonet \
        --stringparam man.output.quietly 1 \
        --stringparam funcsynopsis.style ansi \
        --stringparam man.th.extra1.suppress 1 \
        --stringparam man.authors.section.enabled 0 \
        --stringparam man.copyright.section.enabled 0

.xml.1:
	$(AM_V_GEN) $(XSLTPROC) $(XSLTPROC_FLAGS) http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<

endif

CLEANFILES ?=
CLEANFILES += $(man_MANS)

EXTRA_DIST += $(man_MANS)

dist-hook-local: all-local

glib-docs-clean: clean
	cd $(srcdir) && rm -rf xml html
