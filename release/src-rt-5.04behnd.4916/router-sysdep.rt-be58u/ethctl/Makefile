
EXE = ethctl

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	mkdir -p $(INSTALL_DIR)/sbin
	(cd $(INSTALL_DIR)/sbin; rm -f ethctl; ln -s ../bin/$(EXE) $(EXE))

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
