install: install.sbindir $(R)$(sbindir)/checkrad

$(R)$(sbindir)/checkrad: src/main/checkrad
	@echo INSTALL $(notdir $<)
	@$(INSTALL) -m 755 $< $(R)$(sbindir)
