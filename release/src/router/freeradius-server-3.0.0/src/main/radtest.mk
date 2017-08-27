install: install.bindir $(R)$(bindir)/radtest

$(R)$(bindir)/radtest: src/main/radtest
	@echo INSTALL $(notdir $<)
	@$(INSTALL) -m 755 $< $(R)$(bindir)
