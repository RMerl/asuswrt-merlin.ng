## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.6

# The name of the module.
DOC_MODULE=gobject

# The top-level SGML file.
DOC_MAIN_SGML_FILE=gobject-docs.xml

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR=$(top_srcdir)/gobject

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS=--deprecated-guards="G_DISABLE_DEPRECATED" \
             --ignore-decorators="G_GNUC_INTERNAL|G_GNUC_WARN_UNUSED_RESULT"

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS=--sgml-mode --output-format=xml --name-space=g

# Used for dependencies
HFILE_GLOB=$(top_srcdir)/gobject/*.h
CFILE_GLOB=$(top_srcdir)/gobject/*.c

# Headers to ignore
IGNORE_HFILES = \
	tests			\
	gobject_trace.h		\
	gtype-private.h		\
	gatomicarray.h


# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
AM_CPPFLAGS = 				\
	-I$(srcdir) 			\
	$(gobject_INCLUDES) 		\
	$(GLIB_DEBUG_FLAGS)

GTKDOC_LIBS = 					\
	$(top_builddir)/glib/libglib-2.0.la	\
	$(top_builddir)/gobject/libgobject-2.0.la

# Images to copy into HTML directory
HTML_IMAGES =  \
	images/glue.png

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE)
content_files = version.xml 		\
		glib-mkenums.xml 	\
		glib-genmarshal.xml	\
		gobject-query.xml \
		tut_gobject.xml \
		tut_gsignal.xml \
		tut_gtype.xml \
		tut_howto.xml \
		tut_intro.xml \
		tut_tools.xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS=--extra-dir=$(srcdir)/../glib/html

include $(top_srcdir)/gtk-doc.make

# Other files to distribute
EXTRA_DIST += 		\
	gobject.cI	\
	version.xml.in

########################################################################

man_MANS =

if ENABLE_MAN

man_MANS += 			\
	glib-mkenums.1 		\
	glib-genmarshal.1 	\
	gobject-query.1


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

gobject-docs-clean: clean
	cd $(srcdir) && rm -rf xml html
