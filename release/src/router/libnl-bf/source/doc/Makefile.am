# -*- Makefile -*-

.PHONY: gendoc api_refs asciidoc

ASCIIDOCOPTS=-a pygments -a language=c -a icons \
	     -a toc2 \
	     -a numbered \
	     -a imagesdir="./images/" \
	     -a iconsdir="./images/icons" \
	     -a stylesdir="${abs_srcdir}/stylesheets/"

EXTRA_DIST = \
	Doxyfile \
	DoxygenLayout.xml \
	core.txt \
	route.txt \
	index.txt \
	libnl.css \
	README \
	doxygen-link.py \
	gen-tags.sh \
	resolve-asciidoc-refs.py \
	tags2dict.sh \
	stylesheets/asciidoc.css \
	stylesheets/asciidoc-manpage.css \
	stylesheets/docbook-xsl.css \
	stylesheets/flask.css \
	stylesheets/flask-manpage.css \
	stylesheets/pygments.css \
	stylesheets/slidy.css \
	stylesheets/toc2.css \
	stylesheets/volnitsky.css \
	stylesheets/volnitsky-manpage.css \
	stylesheets/xhtml11-quirks.css \
	images/asciidoc__1.png \
	images/asciidoc__2.png \
	images/asciidoc__3.png \
	images/asciidoc__4.png \
	images/asciidoc__5.png \
	images/asciidoc__6.png \
	images/asciidoc__7.png \
	images/attribute_hdr.png \
	images/classful_qdisc.png \
	images/classless_qdisc_nbands.png \
	images/classless_qdisc.png \
	images/ifinfomsg.png \
	images/library_overview.png \
	images/nlmsgerr.png \
	images/nlmsghdr.png \
	images/qdisc_default.png \
	images/qdisc_mq.png \
	images/tc_obj.png \
	images/tc_overview.png \
	images/icons/caution.png \
	images/icons/example.png \
	images/icons/home.png \
	images/icons/important.png \
	images/icons/next.png \
	images/icons/note.png \
	images/icons/prev.png \
	images/icons/README \
	images/icons/tip.png \
	images/icons/up.png \
	images/icons/warning.png \
	images/icons/callouts/1.png \
	images/icons/callouts/2.png \
	images/icons/callouts/3.png \
	images/icons/callouts/4.png \
	images/icons/callouts/5.png \
	images/icons/callouts/6.png \
	images/icons/callouts/7.png \
	images/icons/callouts/8.png \
	images/icons/callouts/9.png \
	images/icons/callouts/10.png \
	images/icons/callouts/11.png \
	images/icons/callouts/12.png \
	images/icons/callouts/13.png \
	images/icons/callouts/14.png \
	images/icons/callouts/15.png

%.html: %.txt
	./resolve-asciidoc-refs.py $< > asciidoc.tmp
	asciidoc $(ASCIIDOCOPTS) -o $@ asciidoc.tmp
	./doxygen-link.py libnl.dict $@ > asciidoc.tmp
	mv asciidoc.tmp $@

asciidoc: core.html route.html index.html

api_ref:
	doxygen Doxyfile;
	./gen-tags.sh | ./tags2dict.sh > libnl.dict

gendoc:
	$(MAKE) api_ref
	$(MAKE) asciidoc

clean-local:
	rm -f api/* libnl.dict *.html;
