install: install.bindir $(R)$(bindir)/radlast

$(R)$(bindir)/radlast: src/main/radlast
	@echo INSTALL $(notdir $<)
	@$(INSTALL) -m 755 $< $(R)$(bindir)
