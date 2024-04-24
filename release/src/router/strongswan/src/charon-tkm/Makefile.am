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
	pkcs1 \
	random \
	sha1 \
	socket-default \
	vici \
	x509

all: build_charon

build_charon: build_charon.gpr src/charon-tkm.c
	@$(GPRBUILD) -p $< $(BUILD_OPTS)

build_tests: build_tests.gpr
	@$(GPRBUILD) -p $< $(BUILD_OPTS) $(TEST_OPTS)

check: build_tests
	if test -z "${TESTS_TKM}"; then \
		echo -e >&2 \
			"\nSome TKM tests are not enabled, by default, because they have" \
			"\nto be run as root and require a running TKM daemon. They may" \
			"\nbe enabled by defining the TESTS_TKM environment variable.\n"; \
	fi
	@LD_LIBRARY_PATH=$(LIBPT) obj/tests

install: build_charon
	$(INSTALL) -m 755 obj/charon-tkm $(DESTDIR)$(ipsecdir)

clean:
	rm -rf obj

EXTRA_DIST = build_charon.gpr build_common.gpr build_tests.gpr src tests
