EXTRA_DIST = \
	Vici-Session/Changes \
	Vici-Session/Makefile.PL \
	Vici-Session/MANIFEST \
	Vici-Session/README.pod \
	Vici-Session/t/Vici-Session.t \
	Vici-Session/lib/Vici/Message.pm \
	Vici-Session/lib/Vici/Packet.pm \
	Vici-Session/lib/Vici/Session.pm \
	Vici-Session/lib/Vici/Transport.pm

all-local: Vici-Session/pm_to_blib

Vici-Session/Makefile: $(srcdir)/Vici-Session/Makefile.PL
	(cd $(srcdir)/Vici-Session; $(PERL) Makefile.PL)

Vici-Session/pm_to_blib: $(EXTRA_DIST) $(srcdir)/Vici-Session/Makefile
	(cd $(srcdir)/Vici-Session; make)

clean-local:
	(cd $(srcdir)/Vici-Session; [ ! -f Makefile ] || make clean)

if PERL_CPAN_INSTALL
install-exec-local: Vici-Session/pm_to_blib
	(cd $(srcdir)/Vici-Session; make install)
endif

