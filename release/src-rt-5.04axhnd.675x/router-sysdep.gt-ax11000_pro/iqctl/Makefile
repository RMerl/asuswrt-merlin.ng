
EXE = iqctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	mkdir -p $(INSTALL_DIR)/bin
	(cd $(INSTALL_DIR)/bin; rm -f iq; ln -s $(EXE) iq)

clean:

dynamic: all install

BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common
