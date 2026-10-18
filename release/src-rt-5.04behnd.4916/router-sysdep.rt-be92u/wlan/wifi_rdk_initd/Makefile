EXE := wifi_rdk_initd

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


install: install_exe


clean:
	rm -f *.o *.d $(EXE)
	rm -f $(EXE_INSTALL_DIR)/$(EXE)
