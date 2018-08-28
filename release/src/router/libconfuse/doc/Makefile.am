EXTRA_DIST        = doxygen-footer.html doxygen-header.html doxygen.css tutorial.xml \
		    $(TUTORIAL_CSOURCES) css.xsl tutorial.css Doxyfile.in Doxyfile

# How can we conditionalize building these programs so they are only
# built when calling 'make documentation'?
# noinst_PROGRAMS = listing1 listing2 listing3 listing4 listing5 listing8
LDADD             = ../src/libconfuse.la
AM_CPPFLAGS       = -I$(top_srcdir)/src
AM_LDFLAGS        = -L../src/
LIBS              = $(LTLIBINTL)
TUTORIAL_CSOURCES = listing1.c listing2.c listing3.c listing4.c \
		    listing5.c listing6.c listing7.c listing8.c
TUTORIAL_LISTINGS = $(TUTORIAL_CSOURCES:.c=.xml)
MOSTLYCLEANFILES  = tutorial.pdf
DISTCLEANFILES    =
CLEANFILES        =*~ '\#*\#' $(TUTORIAL_LISTINGS)

.c.xml:
	@echo "<programlisting id=\"$*\"><![CDATA[" > $@
	@sed = $< | sed 'N;s/\n/\t/' >> $@
	@echo ']]></programlisting>' >> $@

tutorial-html/index.html: tutorial.xml $(TUTORIAL_LISTINGS) css.xsl tutorial.css
	@xmllint --valid --noout --path . $< && xmlto -v --searchpath . -o tutorial-html \
		--extensions -m $(top_srcdir)/doc/css.xsl xhtml $<
	@cp -f $(top_srcdir)/doc/tutorial.css tutorial-html/

tutorial.pdf: tutorial.xml $(TUTORIAL_LISTINGS)
	@xmllint --valid --noout --path . $< && xmlto pdf $<

tutorial: tutorial-html
tutorial-html: tutorial-html/index.html
tutorial-pdf: tutorial.pdf

html/index.html: Doxyfile
	@doxygen

# Remove "Directory Reference", not needed
doxygen-manual: html/index.html ../src/confuse.h
	@find man -name '_home_*' -exec rm {} \;

documentation: tutorial doxygen-manual

dist-hook: tutorial doxygen-manual
	@cp -pr html $(distdir)
	@cp -pr man $(distdir)
	@cp -pr tutorial-html $(distdir)

# remove directories created by DoxyGen and xmlto to pass distcheck
distclean-local:
	@rm -rfv html man tutorial-html
