
# These headers have no home in any lib, so install them from here.
# Most other headers should be moved into and installed from their lib dirs.
#EXTERNAL_HEADERS := bcm_common_llist.h bcmctl_syslog.h bcm_ipc_api.h \
#                    bcm_MACsec_crypto.h bcm_mmap.h bcm_OS_Deps_Usr.h \
#                    blogctl_api.h maclimit_api.h \
#                    macsec_api.h oam_util.h

# All of the headers will be installed here for now until they are moved
# to their respective libs.  This allows the dependent applications to switch to
# BCM_FSBUILD_DIR instead of BUILD_DIR.
EXTERNAL_HEADERS := $(wildcard *.h)

default all: install

install:
	mkdir -p $(HEADER_INSTALL_DIR)
	$(INSTALL_HEADERS_WITH_CP) $(EXTERNAL_HEADERS) $(HEADER_INSTALL_DIR)


clean:
	rm -f $(addprefix $(HEADER_INSTALL_DIR)/,$(notdir $(EXTERNAL_HEADERS)))

