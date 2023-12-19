################################################################################
#
# readymedia
#
################################################################################

READYMEDIA_VERSION = v1_3_3
READYMEDIA_SITE = https://git.code.sf.net/p/minidlna/git
#READYMEDIA_SITE = ssh://localhost/home/jmaggard/source/minidlna
READYMEDIA_SITE_METHOD = git
READYMEDIA_LICENSE = GPL-2.0, BSD-3-Clause
READYMEDIA_LICENSE_FILES = COPYING LICENCE.miniupnpd
READYMEDIA_CPE_ID_VENDOR = readymedia_project
READYMEDIA_CPE_ID_PRODUCT = readymedia

READYMEDIA_DEPENDENCIES = \
	$(TARGET_NLS_DEPENDENCIES) \
	ffmpeg flac libvorbis libogg libid3tag libexif jpeg sqlite \
	host-xutil_makedepend

READYMEDIA_CONF_OPTS = \
	--enable-static \
	--enable-tivo \
	--enable-lto

define READYMEDIA_RUN_AUTOGEN
    cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
READYMEDIA_PRE_CONFIGURE_HOOKS = READYMEDIA_RUN_AUTOGEN

$(eval $(autotools-package))
