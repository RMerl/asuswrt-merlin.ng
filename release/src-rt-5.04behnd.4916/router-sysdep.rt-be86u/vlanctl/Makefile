
EXE = vlanctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	mkdir -p $(INSTALL_DIR)/bin
	install prebuilt/rawSocketTest $(INSTALL_DIR)/bin

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
