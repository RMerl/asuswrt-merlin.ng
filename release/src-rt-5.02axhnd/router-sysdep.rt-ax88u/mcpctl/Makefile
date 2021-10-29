

#GLOBAL_RELEASE_SCRIPT_NOTOUCH

EXE = mcpctl
clean:
	rm -f $(EXE)
mcpctl:
ifneq ($(PREBUILT_BCMBIN),1)
	cp $(EXE)_$(PROFILE).save $(EXE)
else
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif

install: $(EXE)
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(EXE)
	mkdir -p $(INSTALL_DIR)/sbin
	ln -sf ../bin/$(EXE) $(INSTALL_DIR)/sbin/$(EXE)
