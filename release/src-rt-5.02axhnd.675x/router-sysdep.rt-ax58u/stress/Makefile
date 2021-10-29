
EXE = stress

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	rm -f $(INSTALL_DIR)/opt/scripts/stress-xdslupdown.sh
	rm -f $(INSTALL_DIR)/opt/scripts/stress-proc.sh
	rm -f $(INSTALL_DIR)/opt/scripts/stress-fs.sh
	rm -f $(INSTALL_DIR)/opt/scripts/stress-fs-flist.txt
	-rm -rf $(INSTALL_DIR)/opt
	-mkdir -p $(INSTALL_DIR)/opt/scripts
	install -m 755 scripts/stress-xdslupdown.sh $(INSTALL_DIR)/opt/scripts
	install -m 755 scripts/stress-proc.sh $(INSTALL_DIR)/opt/scripts
	install -m 755 scripts/stress-fs.sh $(INSTALL_DIR)/opt/scripts
	install -m 755 scripts/stress-fs-flist.txt $(INSTALL_DIR)/opt/scripts

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
