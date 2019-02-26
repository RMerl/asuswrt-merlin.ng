SRC = $(abs_top_srcdir)/src
OBJ = $(abs_top_builddir)/src

AM_CPPFLAGS = \
	-include $(abs_top_builddir)/config.h \
	-I$(SRC)/libstrongswan \
	-I$(SRC)/libcharon

LIBLD = \
	-L$(OBJ)/libstrongswan/.libs \
	-L$(OBJ)/libcharon/.libs
LIBPT = $(OBJ)/libstrongswan/.libs:$(OBJ)/libcharon/.libs
LIBFL = -lstrongswan -lcharon

DEFS += -DPLUGINS=\""$(PLUGINS)\"" -DIPSEC_PIDDIR=\"${piddir}\"

BUILD_OPTS = \
	-XOBJ_DIR=$(abs_builddir)/obj \
	-cargs $(AM_CPPFLAGS) $(DEFS) \
	-largs $(LIBLD) $(LIBFL)

TEST_OPTS = \
	-cargs -DBUILDDIR=\"${abs_top_builddir}\" \
	-largs -L$(OBJ)/libstrongswan/tests/.libs -ltest

# plugins to enable
PLUGINS = \
	kernel-netlink \
	pem \
	socket-default \
	openssl \
	stroke

all: build_charon

build_charon: build_charon.gpr src/charon-tkm.c
	@$(GPRBUILD) -p $< $(BUILD_OPTS)

build_tests: build_tests.gpr
	@$(GPRBUILD) -p $< $(BUILD_OPTS) $(TEST_OPTS)

check-tkm: build_tests
	@LD_LIBRARY_PATH=$(LIBPT) obj/tests

check:
	@echo "TKM tests are not run automatically because they have to be run as root" >&2
	@echo "and require a properly configured TKM daemon to be running." >&2
	@echo "They can be run from '$(abs_builddir)' with 'make check-tkm'" >&2

install: build_charon
	$(INSTALL) -m 755 obj/charon-tkm $(DESTDIR)$(ipsecdir)

clean:
	rm -rf obj

EXTRA_DIST = build_charon.gpr build_common.gpr build_tests.gpr src tests
