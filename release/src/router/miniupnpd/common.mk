# common targets for linux Makefile's

miniupnpd:	$(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)

testupnpdescgen:	testupnpdescgen.o upnpdescgen.o

testgetifstats:	testgetifstats.o getifstats.o

testupnppermissions:	testupnppermissions.o upnppermissions.o

testgetifaddr:	testgetifaddr.o getifaddr.o

testgetroute:	testgetroute.o getroute.o upnputils.o

testssdppktgen:	testssdppktgen.o

testasyncsendto:	testasyncsendto.o asyncsendto.o upnputils.o \
	getroute.o

testminissdp:	testminissdp.o minissdp.o upnputils.o upnpglobalvars.o \
	asyncsendto.o getroute.o

miniupnpdctl:	miniupnpdctl.o

dox:	$(SRCDIR)/miniupnpd.doxyconf
	(cat $< ; echo "INPUT=$(SRCDIR)" ) | $(DOXYGEN) -

# useful for debug :  make print-CPPFLAGS
print-%:
	@echo "$* = $($*)"


