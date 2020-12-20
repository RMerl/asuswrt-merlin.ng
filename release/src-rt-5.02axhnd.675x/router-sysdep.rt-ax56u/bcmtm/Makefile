
EXE = bcmtmctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	mkdir -p $(INSTALL_DIR)/bin
	(cd $(INSTALL_DIR)/bin; rm -f bcmtm; ln -s $(EXE) bcmtm)

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
