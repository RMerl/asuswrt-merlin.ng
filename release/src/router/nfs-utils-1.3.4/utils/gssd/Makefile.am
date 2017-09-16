## Process this file with automake to produce Makefile.in

man8_MANS	= gssd.man
if CONFIG_SVCGSS
man8_MANS	+= svcgssd.man
endif

RPCPREFIX	= rpc.
KPREFIX		= @kprefix@
sbin_PREFIXED	= gssd
if CONFIG_SVCGSS
sbin_PREFIXED	+= svcgssd
endif

sbin_PROGRAMS	= $(sbin_PREFIXED)

EXTRA_DIST = \
	$(man8_MANS)

COMMON_SRCS = \
	context.c \
	context_mit.c \
	context_heimdal.c \
	context_lucid.c \
	gss_util.c \
	gss_oids.c \
	gss_names.c \
	err_util.c \
	\
	context.h \
	err_util.h \
	gss_oids.h \
	gss_names.h \
	gss_util.h

gssd_SOURCES = \
	$(COMMON_SRCS) \
	gssd.c \
	gssd_proc.c \
	krb5_util.c \
	\
	gssd.h \
	krb5_util.h \
	write_bytes.h

gssd_LDADD = \
	../../support/nfs/libnfs.a \
	$(LIBEVENT) \
	$(RPCSECGSS_LIBS) \
	$(KRBLIBS) \
	$(GSSAPI_LIBS) \
	$(LIBTIRPC) \
	$(LIBPTHREAD)

gssd_LDFLAGS = \
	$(KRBLDFLAGS)

gssd_CFLAGS = \
	$(AM_CFLAGS) \
	$(CFLAGS) \
	$(RPCSECGSS_CFLAGS) \
	$(KRBCFLAGS) \
	$(GSSAPI_CFLAGS)

svcgssd_SOURCES = \
	$(COMMON_SRCS) \
	svcgssd.c \
	svcgssd_main_loop.c \
	svcgssd_mech2file.c \
	svcgssd_proc.c \
	svcgssd_krb5.c \
	\
	svcgssd_krb5.h \
	svcgssd.h

svcgssd_LDADD = \
	../../support/nfs/libnfs.a \
	$(RPCSECGSS_LIBS) $(LIBNFSIDMAP) \
	$(KRBLIBS) $(GSSAPI_LIBS) $(LIBTIRPC)

svcgssd_LDFLAGS = $(KRBLDFLAGS)

svcgssd_CFLAGS = $(AM_CFLAGS) $(CFLAGS) \
		 $(RPCSECGSS_CFLAGS) $(KRBCFLAGS) $(GSSAPI_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in

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
	  for p in $(sbin_PREFIXED); do \
	    mv -f $$p$(EXEEXT) $(RPCPREFIX)$(KPREFIX)$$p$(EXEEXT) ;\
	  done)
uninstall-hook:
	(cd $(DESTDIR)$(sbindir) && \
	  for p in $(sbin_PREFIXED); do \
	    rm -f $(RPCPREFIX)$(KPREFIX)$$p$(EXEEXT) ;\
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

