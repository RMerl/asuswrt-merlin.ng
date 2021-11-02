
EXE = stress

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	-rm -rf $(INSTALL_DIR)/rom/rom/scripts
	-mkdir -p $(INSTALL_DIR)/rom/rom/scripts
	install -m 755 scripts/stress-xdslupdown.sh $(INSTALL_DIR)/rom/rom/scripts
	install -m 755 scripts/stress-proc.sh $(INSTALL_DIR)/rom/rom/scripts
	install -m 755 scripts/stress-fs.sh $(INSTALL_DIR)/rom/rom/scripts
	install -m 755 scripts/stress-fs-flist.txt $(INSTALL_DIR)/rom/rom/scripts

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
