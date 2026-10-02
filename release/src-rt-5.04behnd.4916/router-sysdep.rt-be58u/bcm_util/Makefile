LIB = libbcm_util.so

# These are public API headers needed by other apps which want to call this lib.
# Private/internal headers should not be listed here.
HEADERS = bcm_ulog.h bcm_timestamp.h bcm_net.h bcm_fsutils.h \
          bcm_boardutils.h bcm_ethswutils.h bcm_strutils.h

HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include


all:
	@cp -f prebuilt/$(LIB) .

install:
	mkdir -p $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	install -m 755 $(LIB) $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	mkdir -p $(BCM_FSBUILD_DIR)/private/lib$(BCM_INSTALL_SUFFIX_DIR)
	install -m 755 $(LIB) $(BCM_FSBUILD_DIR)/private/lib$(BCM_INSTALL_SUFFIX_DIR)
	mkdir -p $(HEADER_INSTALL_DIR)
	install -p -m 444 -t $(HEADER_INSTALL_DIR) $(addprefix $(mdir)/,$(HEADERS))

clean:

dynamic: all install


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)


include $(BUILD_DIR)/make.common
