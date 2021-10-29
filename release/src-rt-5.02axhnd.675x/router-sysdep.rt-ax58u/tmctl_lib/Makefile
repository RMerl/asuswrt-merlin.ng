
LIB = libtmctl.so
ifneq ($(strip $(BUILD_TMCTL)),)
all:
	@cp -f prebuilt/$(LIB) .

install: generic_private_lib_install
else
all:
install:
endif
clean:

dynamic: all install

include $(BUILD_DIR)/make.common
