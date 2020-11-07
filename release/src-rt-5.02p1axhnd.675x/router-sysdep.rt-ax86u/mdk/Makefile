
LIB = libbmdapi.so \
libbmdpkgsrc.so \
libbmdshared.so \
libbmdshell.so \
libcdkdsym.so \
libcdklibc.so \
libcdkmain.so \
libcdkpkgsrc.so \
libcdkshared.so \
libcdkshell.so \
libcdksym.so \
libphygeneric.so \
libphypkgsrc.so \
libphysym.so \
libphyutil.so

all:
	@cp -f prebuilt/*.so .

install: generic_private_lib_install

clean:

dynamic: all install

include $(BUILD_DIR)/make.common
