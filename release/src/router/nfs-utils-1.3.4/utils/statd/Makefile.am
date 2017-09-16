## Process this file with automake to produce Makefile.in

man8_MANS = statd.man sm-notify.man

RPCPREFIX	= rpc.
KPREFIX		= @kprefix@
sbin_PROGRAMS	= statd sm-notify
dist_sbin_SCRIPTS	= start-statd
statd_SOURCES = callback.c notlist.c misc.c monitor.c hostname.c \
	        simu.c stat.c statd.c svc_run.c rmtcall.c \
	        notlist.h statd.h system.h
sm_notify_SOURCES = sm-notify.c

BUILT_SOURCES = $(GENFILES)
statd_LDADD = ../../support/nsm/libnsm.a \
	      ../../support/nfs/libnfs.a \
	      ../../support/misc/libmisc.a \
	      $(LIBWRAP) $(LIBNSL) $(LIBCAP) $(LIBTIRPC)
sm_notify_LDADD = ../../support/nsm/libnsm.a \
		  ../../support/nfs/libnfs.a \
		  $(LIBNSL) $(LIBCAP) $(LIBTIRPC)

EXTRA_DIST = sim_sm_inter.x $(man8_MANS) simulate.c

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

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = $(GENFILES)

#######################################################################
# The following allows the current practice of having
# daemons renamed during the install to include RPCPREFIX
# and the KPREFIX
# This could all be done much easier with program_transform_name
# ( program_transform_name = s/^/$(RPCPREFIX)$(KPREFIX)/ )
# but that also renames the man pages, which the current
# practice does not do.
install-exec-hook:
	(cd $(DESTDIR)$(sbindir) && \
	  for p in $(sbin_PROGRAMS); do \
	    [ $$p = sm-notify ] || mv -f $$p$(EXEEXT) $(RPCPREFIX)$(KPREFIX)$$p$(EXEEXT) ;\
	  done)
uninstall-hook:
	(cd $(DESTDIR)$(sbindir) && \
	  for p in $(sbin_PROGRAMS); do \
	    [ $$p = sm-notify ] || rm -f $(RPCPREFIX)$(KPREFIX)$$p$(EXEEXT) ;\
	  done)


# XXX This makes some assumptions about what automake does.
# XXX But there is no install-man-hook or install-man-local.
install-man: install-man8 install-man-links
uninstall-man: uninstall-man8 uninstall-man-links

install-man-links:
	(cd $(DESTDIR)$(man8dir) && \
	  for m in $(man8_MANS) $(dist_man8_MANS) $(nodist_man8_MANS); do \
	    inst=`echo $$m | sed -e 's/man$$/8/'`; \
	    rm -f $(RPCPREFIX)$$inst ; \
	    $(LN_S) $$inst $(RPCPREFIX)$$inst ; \
	  done)

uninstall-man-links:
	(cd $(DESTDIR)$(man8dir) && \
	  for m in $(man8_MANS) $(dist_man8_MANS) $(nodist_man8_MANS); do \
	    inst=`echo $$m | sed -e 's/man$$/8/'`; \
	    rm -f $(RPCPREFIX)$$inst ; \
	  done)

