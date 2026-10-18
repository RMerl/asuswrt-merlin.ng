EXE := wlssk
 
SAVED := $(EXE)_$(ARCH).shipped
 
all: $(EXE)
 
# Set VPATH so we can compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)
 
ifneq ($(wildcard $(mdir)/Makefile.fullsrc),)
 
include $(mdir)/Makefile.fullsrc
 
else
 
$(EXE): $(SAVED)
	cat $< > $@
 
save_binaries:
 
delete_source:
 
endif
 
install_exe: all
	mkdir -p $(EXE_INSTALL_DIR)
	install -p -m 755 -t $(EXE_INSTALL_DIR) $(EXE)
 
 
install_scripts:
	@echo "wlssk scripts-install: wlssk.sh"
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	install $(mdir)/scripts/wlssk.sh $(INSTALL_DIR)/etc/init.d
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f S81wlssk.sh ; ln -s ../init.d/wlssk.sh S81wlssk.sh)
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f K50wlssk.sh ; ln -s ../init.d/wlssk.sh K50wlssk.sh)
 
 

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
install: install_exe
else # BASE_SHELL 
install: install_exe install_scripts
endif

 
 
clean: 
	rm -f *.o *.d $(EXE)
	rm -f $(EXE_INSTALL_DIR)/$(EXE) 
	rm -f $(INSTALL_DIR)/etc/init.d/wlssk.sh
	rm -f $(INSTALL_DIR)/etc/rc3.d/S81wlssk.sh
	rm -f $(INSTALL_DIR)/etc/rc3.d/K50wlssk.sh

