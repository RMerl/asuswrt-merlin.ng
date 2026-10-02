LIB = libiqctl.so

all:
	@cp -f prebuilt/$(LIB) .

install: generic_private_lib_install

clean:

dynamic: all install

BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common
