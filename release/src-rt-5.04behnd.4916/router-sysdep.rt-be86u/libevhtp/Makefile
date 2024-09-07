APP = libevhtp
VERSION = 1.2.18
APP_NAME = $(APP)-$(VERSION)

ifeq ($(strip $(DESKTOP_LINUX)), y)
BCM_BLT32_FLAGS = CFLAGS='$(BCM_LD_FLAGS)'
TOOLCHAIN_PREFIX = $(PROFILE_ARCH)-none-linux
endif

ifneq ($(strip $(RDK_BUILD)),)
FIND_ROOT_PATH = $(ROOT_PATH);$(RDKB_BCALINUX_SYSROOT)
else
FIND_ROOT_PATH = $(ROOT_PATH)
endif

check_cmake: $(APP_NAME).tar.gz
	if [ ! -e $(APP)/Makefile ]; then \
	    echo "Untarring original $(APP_NAME) source and configure"; \
		mkdir -p $(APP); \
		( cd $(APP); tar --strip-components=1 -xzf ../$(APP_NAME).tar.gz; \
		cmake \
		-DCMAKE_BUILD_TYPE=release \
		-DBUILD_SHARED_LIBS=ON \
		-DCMAKE_RANLIB=$(RANLIB) \
		-DCMAKE_STRIP=$(STRIP) \
		-DCMAKE_NM:FILEPATH=$(NM) \
		-DCMAKE_OBJCOPY:FILEPATH=$(OBJCOPY) \
		-DCMAKE_OBJDUMP=$(OBJDUMP) \
		-DCMAKE_LINKER:FILEPATH="$(LD)" \
		-DCMAKE_AR:FILEPATH=$(AR) \
		-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
		-DCMAKE_FIND_ROOT_PATH:PATH="$(FIND_ROOT_PATH)" \
		-DCMAKE_PREFIX_PATH:PATH="$(PREFIX)" \
		-DCMAKE_INSTALL_PREFIX:PATH=$(PREFIX) ); \
	fi

install: check_cmake
	make -C $(APP)
	make -C $(APP) install

clean:
	-[ ! -e $(APP)/Makefile ] || make -C $(APP) uninstall
	rm -rf $(APP)

