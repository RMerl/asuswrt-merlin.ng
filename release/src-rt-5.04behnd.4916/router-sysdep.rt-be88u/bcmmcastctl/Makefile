
EXE = bcmmcastctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	mkdir -p $(INSTALL_DIR)/bin
	(cd $(INSTALL_DIR)/bin; rm -f bmc; ln -s $(EXE) bmc)

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
