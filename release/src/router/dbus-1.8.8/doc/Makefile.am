apidir = @htmldir@/api

man_pages = \
	dbus-cleanup-sockets.1 \
	dbus-daemon.1 \
	dbus-launch.1 \
	dbus-monitor.1 \
	dbus-run-session.1 \
	dbus-send.1 \
	dbus-uuidgen.1 \
	$(NULL)

MAN_XML_FILES = $(patsubst %.1,%.1.xml,$(man_pages))

if DBUS_XML_DOCS_ENABLED
man1_MANS = $(man_pages)
endif

MAN_HTML_FILES = $(patsubst %.1,%.1.html,$(man_pages))

DTDS = \
	busconfig.dtd \
	introspect.dtd

dist_doc_DATA = system-activation.txt

# uploaded and distributed, but not installed
STATIC_DOCS = \
	dbus-faq.xml \
	dbus-specification.xml \
	dbus-test-plan.xml \
	dbus-tutorial.xml \
	dcop-howto.txt \
	introspect.xsl \
	$(DTDS)

EXTRA_DIST = \
	file-boilerplate.c \
	doxygen_to_devhelp.xsl \
	$(STATIC_DOCS)

html_DATA =

dist_html_DATA =

# diagram.png/diagram.svg aren't really HTML, but must go in the same
# directory as the HTML to avoid broken links
STATIC_HTML = \
	diagram.png \
	diagram.svg \
	$(NULL)

dist_html_DATA += $(STATIC_HTML)

XMLTO_HTML =					\
	dbus-faq.html				\
	dbus-specification.html			\
	dbus-test-plan.html			\
	dbus-tutorial.html			\
	$(MAN_HTML_FILES)			\
	$(NULL)

if DBUS_XML_DOCS_ENABLED
html_DATA += $(XMLTO_HTML)

%.html: %.xml
	$(XMLTO) html-nochunks $<

%.1: %.1.xml
	$(XMLTO) man $<
endif

if DBUS_DOXYGEN_DOCS_ENABLED
all-local:: doxygen.stamp

doxygen.stamp: $(wildcard $(top_srcdir)/dbus/*.[ch])
	$(AM_V_GEN)cd $(top_builddir) && doxygen Doxyfile
	@touch $@

if DBUS_HAVE_XSLTPROC
html_DATA += dbus.devhelp

dbus.devhelp: $(srcdir)/doxygen_to_devhelp.xsl doxygen.stamp
	$(XSLTPROC) -o $@ $< api/xml/index.xml
endif

# this assumes CREATE_SUBDIRS isn't set to YES in Doxyfile
# (which it isn't currently)
install-data-local:: doxygen.stamp
	$(MKDIR_P) $(DESTDIR)$(apidir)
	$(INSTALL_DATA) api/html/* $(DESTDIR)$(apidir)

uninstall-local::
	rm -f $(DESTDIR)$(apidir)/*.html
	rm -f $(DESTDIR)$(apidir)/*.png
	rm -f $(DESTDIR)$(apidir)/*.css
	rm -f $(DESTDIR)$(apidir)/*.js
	rm -f $(DESTDIR)$(htmldir)/*.html
	rm -f $(DESTDIR)$(docdir)/*.txt
	rm -f $(DESTDIR)$(htmldir)/*.png
	rm -f $(DESTDIR)$(htmldir)/*.svg
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(apidir) || \
		rmdir $(DESTDIR)$(apidir)
endif

if DBUS_CAN_UPLOAD_DOCS
BONUS_FILES = \
	$(top_srcdir)/README \
	$(top_srcdir)/HACKING \
	$(top_srcdir)/AUTHORS \
	$(top_srcdir)/NEWS \
	$(top_srcdir)/COPYING \
	$(top_srcdir)/ChangeLog

dbus-docs: $(STATIC_DOCS) $(MAN_XML_FILES) $(dist_doc_DATA) $(dist_html_DATA) $(MAN_HTML_FILES) $(BONUS_FILES) doxygen.stamp $(XMLTO_HTML)
	$(AM_V_at)rm -rf $@ $@.tmp
	$(AM_V_GEN)$(MKDIR_P) $@.tmp/api
	$(AM_V_at)cd $(srcdir) && cp $(STATIC_DOCS) @abs_builddir@/$@.tmp
	$(AM_V_at)cd $(srcdir) && cp $(dist_doc_DATA) @abs_builddir@/$@.tmp
	$(AM_V_at)cd $(srcdir) && cp $(STATIC_HTML) @abs_builddir@/$@.tmp
	$(AM_V_at)cp $(XMLTO_HTML) @abs_builddir@/$@.tmp
	$(AM_V_at)cp $(MAN_HTML_FILES) @abs_builddir@/$@.tmp
	$(AM_V_at)cp $(MAN_XML_FILES) @abs_builddir@/$@.tmp
	$(AM_V_at)cp $(BONUS_FILES) @abs_builddir@/$@.tmp
	$(AM_V_at)cp -r api/html @abs_builddir@/$@.tmp/api
	$(AM_V_at)mv $@.tmp $@

dbus-docs.tar.gz: dbus-docs
	$(AM_V_GEN)tar czf $@ $<

DOC_SERVER = dbus.freedesktop.org
DOC_WWW_DIR = /srv/dbus.freedesktop.org/www

SPECIFICATION_SERVER = specifications.freedesktop.org
SPECIFICATION_PATH = /srv/specifications.freedesktop.org/www/dbus/1.0

maintainer-upload-docs: dbus-docs.tar.gz dbus-docs
	scp dbus-docs.tar.gz $(DOC_SERVER):$(DOC_WWW_DIR)/
	rsync -rpvzP --chmod=Dg+s,ug+rwX,o=rX \
		dbus-docs/ $(DOC_SERVER):$(DOC_WWW_DIR)/doc/
	cd $(srcdir) && scp -p $(DTDS) $(SPECIFICATION_SERVER):$(SPECIFICATION_PATH)/
else
maintainer-upload-docs:
	@echo "Can't upload documentation! Re-run configure with"
	@echo "  --enable-doxygen-docs --enable-xml-docs"
	@echo "and ensure that man2html is installed."
	@false
endif

CLEANFILES = \
	$(man1_MANS) \
	$(MAN_XML_FILES) \
	$(XMLTO_HTML) \
	$(NULL)

clean-local:
	rm -f $(html_DATA)
	rm -rf api
	rm -rf dbus-docs dbus-docs.tmp
	rm -f *.1.html
	rm -f doxygen.stamp
