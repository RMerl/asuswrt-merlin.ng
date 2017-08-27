INSTALLKMODDIR:=$(INSTALLDIR)/lib/modules/$(LINUX_KERNEL)/kernel/net
PBBINS=$(wildcard prebuild/*)
QUICKSEC_KMOD=$(filter %.ko,$(PBBINS))
QUICKSEC_EXEC=$(filter-out %.ko,$(PBBINS))

all:

install:
	@[ -d $(INSTALLKMODDIR) ] || install -d $(INSTALLKMODDIR)
	@for i in $(QUICKSEC_KMOD); do install $$i $(INSTALLKMODDIR) ; done
	@[ -d $(INSTALLDIR)/usr/sbin ] || install -d $(INSTALLDIR)/usr/sbin
	@for i in $(QUICKSEC_EXEC); do install $$i $(INSTALLDIR)/usr/sbin ; done
	@$(STRIPX) $(INSTALLKMODDIR)/*.ko
	@$(STRIP) $(INSTALLDIR)/usr/sbin/*

clean:

stage:
