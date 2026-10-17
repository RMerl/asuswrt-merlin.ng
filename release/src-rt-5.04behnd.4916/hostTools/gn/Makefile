all: gn

#
# GN is a meta-build system that generates build files for Ninja.
#

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

HOSTTOOLS_DIR = $(BUILD_DIR)/hostTools
NINJA=$(HOSTTOOLS_DIR)/ninja/ninja

# GN versioning is based on the number of commits since the tag `initial-commit`,
# and is exposed to build scripts via the `gn_version` variable. When updating
# PKG_SOURCE_VERSION to a new commit hash, GN_VERSION needs to be obtained via
#   git describe HEAD --match initial-commit | cut -f 3 -d -
# and updated below.
PKG_SOURCE_VERSION:=95b0f8fe31a992a33c040bbe3867901335c12762
GN_VERSION:=2200

GN_TAR_XZ := $(PKG_SOURCE_VERSION).tar.xz
APP       := gn-$(PKG_SOURCE_VERSION)

prepare_tarball:
	$(RM) $(GN_TAR_XZ) -r $(APP)
	git clone https://gn.googlesource.com/gn $(APP) && \
	(cd $(APP) && git checkout $(PKG_SOURCE_VERSION)) && \
	TIMESTAMP=`cd $(APP) && git log -1 --format='@%ct'` && \
	$(RM) -r $(APP)/.git && \
	tar --numeric-owner --owner=0 --group=0 --mode=a-s --sort=name \
		$${TIMESTAMP:+--mtime="$$TIMESTAMP"} -c $(APP) | xz -zc -7e > $(GN_TAR_XZ) && \
	$(RM) -r $(APP)

define GN_LAST_COMMIT_HEADER
#ifndef OUT_LAST_COMMIT_POSITION_H_
#define OUT_LAST_COMMIT_POSITION_H_
#define LAST_COMMIT_POSITION_NUM $$VER_NUM
#define LAST_COMMIT_POSITION "$$VER_STR"
#endif // OUT_LAST_COMMIT_POSITION_H_
endef
export GN_LAST_COMMIT_HEADER

gn: $(NINJA)
	tar -xvf $(GN_TAR_XZ)
	cd $(APP) ; CFLAGS+=" -w" python3 build/gen.py --no-last-commit-position ; \
	echo "$$GN_LAST_COMMIT_HEADER" | \
		VER_NUM=$(GN_VERSION) VER_STR="$(GN_VERSION) (`echo "$(PKG_SOURCE_VERSION)" | cut -c 1-12`)" \
		perl -lpe 's/\$$(\w+)/$$ENV{$$1}/ge' >out/last_commit_position.h ; \
	$(NINJA) -C out gn
	ln -s $(APP)/out/gn gn

clean:
	$(RM) gn -r $(APP)

