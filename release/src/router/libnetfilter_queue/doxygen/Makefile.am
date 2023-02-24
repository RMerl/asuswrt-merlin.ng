if HAVE_DOXYGEN
doc_srcs = $(top_srcdir)/src/libnetfilter_queue.c  \
           $(top_srcdir)/src/nlmsg.c               \
           $(top_srcdir)/src/extra/checksum.c      \
           $(top_srcdir)/src/extra/ipv6.c          \
           $(top_srcdir)/src/extra/ipv4.c          \
           $(top_srcdir)/src/extra/tcp.c           \
           $(top_srcdir)/src/extra/udp.c           \
           $(top_srcdir)/src/extra/pktbuff.c

doxyfile.stamp: $(doc_srcs) $(top_srcdir)/fixmanpages.sh
	rm -rf html man && cd .. && doxygen doxygen.cfg && ./fixmanpages.sh
	touch doxyfile.stamp

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf $(top_srcdir)/doxygen/man $(top_srcdir)/doxygen/html
install-data-local:
	mkdir -p $(DESTDIR)$(mandir)/man3
	cp --no-dereference --preserve=links,mode,timestamps man/man3/*.3 $(DESTDIR)$(mandir)/man3/
endif
