include ../common.mak

PBBINS=$(wildcard prebuild/*)
QCA_GPL_LIBS=$(filter %.so,$(PBBINS))
QCA_GPL_SH=$(filtert %.sh,$(PBBINS))
QCA_GPL_EXEC=$(filter-out %.so %.sh,$(PBBINS))

all:
	@echo $(PBBINS)

install: all all
	@[ -d $(INSTALLDIR)/usr/sbin ] || install -d $(INSTALLDIR)/usr/sbin
	@[ -d $(INSTALLDIR)/usr/lib ] || install -d $(INSTALLDIR)/usr/lib
	@[ -d $(INSTALLDIR)/lib/functions ] || install -d $(INSTALLDIR)/lib/functions
	$(if $(QCA_GPL_LIBS),install -m 0755 $(foreach lib,$(QCA_GPL_LIBS),$(lib)) $(INSTALLDIR)/usr/lib)
	@for i in $(QCA_GPL_EXEC); do install -m 755 -D $$i $(INSTALLDIR)/usr/sbin ; done
	@for i in $(QCA_GPL_SH); do install -m 755 -D $$i $(INSTALLDIR)/lib/functions ; done
	@$(STRIP) $(INSTALLDIR)/usr/sbin/*
	@$(STRIP) $(INSTALLDIR)/usr/lib/*

