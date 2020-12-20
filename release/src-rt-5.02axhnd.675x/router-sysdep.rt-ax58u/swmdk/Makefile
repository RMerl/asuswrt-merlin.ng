
EXE = swmdk

ifneq ($(wildcard prebuilt/$(EXE)),)
all:
	@cp -f prebuilt/$(EXE) .
else
all:
	echo "no prebuilt $(EXE), touch $(EXE) for non-swmdk models"
	touch ./$(EXE)
endif

install: generic_exe_install
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	install scripts/swmdk.sh $(INSTALL_DIR)/etc/init.d
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f S45swmdk; ln -s ../init.d/swmdk.sh S45swmdk)

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
