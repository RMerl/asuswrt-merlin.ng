
EXE = ethswctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	ln -sf $(EXE) $(INSTALL_DIR)/bin/ethswctl

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
