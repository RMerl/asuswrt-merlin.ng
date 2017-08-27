info_TEXINFOS = stress.texi
man_MANS = stress.1
EXTRA_DIST = $(man_MANS) stress.html

MAINTAINERCLEANFILES = Makefile.in mdate-sh texinfo.tex stress.html $(man_MANS)

stress.1:
	help2man -n 'tool to impose load on and stress test systems' ../src/stress > $@

stress.html: stress.texi
	$(MAKEINFO) --no-split --html $<
