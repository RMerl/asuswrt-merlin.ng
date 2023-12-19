MAKEFLAGS=-s

.PHONY: manpages

docs: clean-docs manpages

web/tcpreplay.html:
	mkdir -p web; @GROFF@ -Thtml -mman ../src/tcpreplay.1 > web/tcpreplay.html

web/tcpprep.html:
	@GROFF@ -Thtml -mman ../src/tcpprep.1 > web/tcpprep.html

web/tcprewrite.html:
	@GROFF@ -Thtml -mman ../src/tcprewrite.1 > web/tcprewrite.html

web/tcpbridge.html:
	@GROFF@ -Thtml -mman ../src/tcpbridge.1 > web/tcpbridge.html

web/tcpreplay-edit.html:
	@GROFF@ -Thtml -mman ../src/tcpreplay-edit.1 > web/tcpreplay-edit.html

web/tcpliveplay.html:
	@GROFF@ -Thtml -mman ../src/tcpliveplay.1 > web/tcpliveplay.html

web/tcpcapinfo.html:
	@GROFF@ -Thtml -mman ../src/tcpcapinfo.1 > web/tcpcapinfo.html

manpages: web/tcpreplay.html web/tcpprep.html web/tcprewrite.html \
    web/tcpbridge.html web/tcpreplay-edit.html web/tcpliveplay.html \
    web/tcpcapinfo.html

postweb: manpages
	rsync -qe ssh --exclude '.git/' --exclude '/**/.git/' --exclude '/**~' \
	--exclude '*~' -avz web/ \
	aturner@malbec.synfin.net:/var/vhosts/tcpreplay/
	scp CHANGELOG TODO aturner@malbec.synfin.net:/var/vhosts/tcpreplay/


EXTRA_DIST = CHANGELOG CREDIT HACKING INSTALL LICENSE Win32Readme.txt

clean-docs: clean
	-rm -f web/*.html

maintainer-clean-local: clean-docs
	-rm -f web/*.html

MAINTAINERCLEANFILES = Makefile.in 

