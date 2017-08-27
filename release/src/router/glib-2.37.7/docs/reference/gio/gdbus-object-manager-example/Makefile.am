NULL =

# The name of the module.
DOC_MODULE=gdbus-object-manager-example

# The top-level SGML file.
DOC_MAIN_SGML_FILE=gdbus-object-manager-example-docs.xml

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS=--deprecated-guards="G_DISABLE_DEPRECATED"

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR=$(top_builddir)/gio/tests/gdbus-object-manager-example

HFILE_GLOB=$(top_builddir)/gio/tests/gdbus-object-manager-example/*.h
CFILE_GLOB=$(top_builddir)/gio/tests/gdbus-object-manager-example/*.c

# Headers to ignore
IGNORE_HFILES =				\
	$(NULL)

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
AM_CPPFLAGS = 				\
	$(gio_INCLUDES)			\
	$(GLIB_DEBUG_FLAGS)

GTKDOC_LIBS = 						\
	$(top_builddir)/glib/libglib-2.0.la		\
	$(top_builddir)/gobject/libgobject-2.0.la	\
	$(top_builddir)/gmodule/libgmodule-2.0.la	\
	$(top_builddir)/gio/libgio-2.0.la		\
	$(top_builddir)/gio/tests/gdbus-object-manager-example/libgdbus-example-objectmanager.la \
	$(NULL)

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --output-format=xml --sgml-mode --name-space=g \
	$(NULL)

# Images to copy into HTML directory
HTML_IMAGES =			\
	$(NULL)

content_files =			\
	$(NULL)

expand_content_files =		\
	$(NULL)

extra_files =			\
	$(NULL)

include $(top_srcdir)/gtk-doc.make

EXTRA_DIST +=			\
	$(NULL)

MAINTAINERCLEANFILES = $(BUILT_SOURCES)

dist-hook-local: all-local

gdbus-object-manager-example-docs-clean: clean
	cd $(srcdir) && rm -rf xml html

# Nuke installed docs (don't know how to avoid installing them)
install-data-hook :
	rm -rf $(DESTDIR)$(datadir)/gtk-doc/html/gdbus-object-manager-example
