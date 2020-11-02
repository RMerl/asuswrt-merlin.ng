
EXE = archerctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	ln -sf $(EXE) $(INSTALL_DIR)/bin/archer

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
