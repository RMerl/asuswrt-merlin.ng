
EXE = ethctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	ln -sf ../bin/$(EXE) $(INSTALL_DIR)/sbin/ethctl

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
