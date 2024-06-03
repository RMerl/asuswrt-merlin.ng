
EXE := blogctl

FINAL_EXE_INSTALL_DIR := $(INSTALL_DIR)/bin

all:
	@cp -f prebuilt/$(EXE) .

install: all
#	mkdir -p $(EXE_INSTALL_DIR)
#	install -p -m 755 -t $(EXE_INSTALL_DIR) $(EXE)
	mkdir -p $(INSTALLDIR)/bin
	install -p -m 755 -t $(FINAL_EXE_INSTALL_DIR) $(EXE)
	(cd $(FINAL_EXE_INSTALL_DIR); ln -sf $(EXE) blog)

clean:
	rm -f *.o *.d $(EXE)
	rm -f $(EXE_INSTALL_DIR)/$(EXE)
