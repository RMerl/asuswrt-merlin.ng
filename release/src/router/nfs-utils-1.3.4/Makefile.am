## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = tools support utils linux-nfs tests systemd

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	autogen.sh \
	\
	aclocal/bsdsignals.m4 \
	aclocal/nfs-utils.m4 \
	aclocal/kerberos5.m4 \
	aclocal/tcp-wrappers.m4 \
	aclocal/libtirpc.m4 \
	aclocal/libevent.m4 \
	aclocal/libnfsidmap.m4 \
	aclocal/rpcsec_vers.m4 \
	aclocal/ipv6.m4

ACLOCAL_AMFLAGS = -I aclocal

install-data-hook:
	if [ ! -d $(DESTDIR)$(statedir) ]; then mkdir -p $(DESTDIR)$(statedir); fi
	touch $(DESTDIR)$(statedir)/xtab; chmod 644 $(DESTDIR)$(statedir)/xtab
	touch $(DESTDIR)$(statedir)/etab; chmod 644 $(DESTDIR)$(statedir)/etab
	touch $(DESTDIR)$(statedir)/rmtab; chmod 644 $(DESTDIR)$(statedir)/rmtab
	mkdir -p $(DESTDIR)$(statdpath)/sm $(DESTDIR)$(statdpath)/sm.bak
	touch $(DESTDIR)$(statdpath)/state
	chmod go-rwx $(DESTDIR)$(statdpath)/sm $(DESTDIR)$(statdpath)/sm.bak $(DESTDIR)$(statdpath)/state
	-chown $(statduser) $(DESTDIR)$(statdpath)/sm $(DESTDIR)$(statdpath)/sm.bak $(DESTDIR)$(statdpath)/state

uninstall-hook:
	rm $(DESTDIR)$(statedir)/xtab
	rm $(DESTDIR)$(statedir)/etab
	rm $(DESTDIR)$(statedir)/rmtab
	rm $(DESTDIR)$(statdpath)/state
