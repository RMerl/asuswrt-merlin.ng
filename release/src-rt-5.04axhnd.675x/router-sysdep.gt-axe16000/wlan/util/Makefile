
EXE = nvramUpdate

all:
	@cp -f prebuilt/$(EXE) .

install: generic_exe_install
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/wlan
	install -m 755 nvram_params $(INSTALL_DIR)/rom/rom/etc/wlan

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
