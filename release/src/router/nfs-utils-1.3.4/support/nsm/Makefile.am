## Process this file with automake to produce Makefile.in

GENFILES_CLNT	= sm_inter_clnt.c
GENFILES_SVC	= sm_inter_svc.c
GENFILES_XDR	= sm_inter_xdr.c
GENFILES_H	= sm_inter.h

GENFILES	= $(GENFILES_CLNT) $(GENFILES_SVC) $(GENFILES_XDR) $(GENFILES_H)

EXTRA_DIST	= sm_inter.x

noinst_LIBRARIES = libnsm.a
libnsm_a_SOURCES = $(GENFILES) file.c rpc.c

BUILT_SOURCES = $(GENFILES)

if CONFIG_RPCGEN
RPCGEN	= $(top_builddir)/tools/rpcgen/rpcgen
$(RPCGEN):
	make -C ../../tools/rpcgen all
else
RPCGEN = @RPCGEN_PATH@
endif

$(GENFILES_CLNT): %_clnt.c: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -l -o $@ $<

$(GENFILES_SVC): %_svc.c: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -m -o $@ $<

$(GENFILES_XDR): %_xdr.c: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -c -o $@ $<

$(GENFILES_H): %.h: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -h -o $@ $<
	rm -f $(top_builddir)/support/include/sm_inter.h
	$(LN_S) ../nsm/sm_inter.h $(top_builddir)/support/include/sm_inter.h

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = $(GENFILES) $(top_builddir)/support/include/sm_inter.h
