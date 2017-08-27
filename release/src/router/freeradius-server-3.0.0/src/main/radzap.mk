install: install.bindir $(R)$(bindir)/radzap

$(R)$(bindir)/radzap: src/main/radzap
	@echo INSTALL $(notdir $<)
	@$(INSTALL) -m 755 $< $(R)$(bindir)
