EXE   := sotpctl

SAVED := $(EXE)_$(ARCH).shipped

all: $(EXE)


# Set VPATH so we can compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)
EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/bin

ifneq ($(wildcard $(mdir)/Makefile.fullsrc),)

include $(mdir)/Makefile.fullsrc

else

#$(EXE): $(SAVED)
$(EXE):
	cat $< > $@

endif


install: all
	mkdir -p $(EXE_INSTALL_DIR)
	install -p -m 755 -t $(EXE_INSTALL_DIR) $(EXE)


clean:
	rm -f *.o *.d $(EXE)
	rm -f $(EXE_INSTALL_DIR)/$(EXE)

