#
# Include makefile for libtommath
#

#version of library
VERSION=1.2.0
VERSION_PC=1.2.0
VERSION_SO=3:0:2

PLATFORM := $(shell uname | sed -e 's/_.*//')

# default make target
default: ${LIBNAME}

# Compiler and Linker Names
ifndef CROSS_COMPILE
  CROSS_COMPILE=
endif

# We only need to go through this dance of determining the right compiler if we're using
# cross compilation, otherwise $(CC) is fine as-is.
ifneq (,$(CROSS_COMPILE))
ifeq ($(origin CC),default)
CSTR := "\#ifdef __clang__\nCLANG\n\#endif\n"
ifeq ($(PLATFORM),FreeBSD)
  # XXX: FreeBSD needs extra escaping for some reason
  CSTR := $$$(CSTR)
endif
ifneq (,$(shell echo $(CSTR) | $(CC) -E - | grep CLANG))
  CC := $(CROSS_COMPILE)clang
else
  CC := $(CROSS_COMPILE)gcc
endif # Clang
endif # cc is Make's default
endif # CROSS_COMPILE non-empty

# Dropbear passes these down
#LD=$(CROSS_COMPILE)ld
#AR=$(CROSS_COMPILE)ar
#RANLIB=$(CROSS_COMPILE)ranlib

ifndef MAKE
# BSDs refer to GNU Make as gmake
ifneq (,$(findstring $(PLATFORM),FreeBSD OpenBSD DragonFly NetBSD))
  MAKE=gmake
else
  MAKE=make
endif
endif

LTM_CFLAGS += -I./ -Wall -Wsign-compare -Wextra -Wshadow

# renamed for Dropbear to avoid clash with oss-fuzz $SANITIZER var
ifdef LTM_SANITIZER
LTM_CFLAGS += -fsanitize=undefined -fno-sanitize-recover=all -fno-sanitize=float-divide-by-zero
endif

ifndef NO_ADDTL_WARNINGS
# additional warnings
LTM_CFLAGS += -Wdeclaration-after-statement -Wbad-function-cast -Wcast-align
LTM_CFLAGS += -Wstrict-prototypes -Wpointer-arith
endif

ifdef CONV_WARNINGS
LTM_CFLAGS += -std=c89 -Wconversion -Wsign-conversion
ifeq ($(CONV_WARNINGS), strict)
LTM_CFLAGS += -DMP_USE_ENUMS -Wc++-compat
endif
else
LTM_CFLAGS += -Wsystem-headers
endif

ifdef COMPILE_DEBUG
#debug
LTM_CFLAGS += -g3
endif

ifdef COMPILE_SIZE
#for size
LTM_CFLAGS += -Os
else

ifndef IGNORE_SPEED
#for speed
LTM_CFLAGS += -O3 -funroll-loops

#x86 optimizations [should be valid for any GCC install though]
LTM_CFLAGS  += -fomit-frame-pointer
endif

endif # COMPILE_SIZE

ifneq ($(findstring clang,$(CC)),)
LTM_CFLAGS += -Wno-typedef-redefinition -Wno-tautological-compare -Wno-builtin-requires-header
endif
ifneq ($(findstring mingw,$(CC)),)
LTM_CFLAGS += -Wno-shadow
endif
ifeq ($(PLATFORM), Darwin)
LTM_CFLAGS += -Wno-nullability-completeness
endif
ifeq ($(PLATFORM), CYGWIN)
LIBTOOLFLAGS += -no-undefined
endif

# add in the standard FLAGS
LTM_CFLAGS := $(CFLAGS) $(LTM_CFLAGS)
LTM_LFLAGS += $(LFLAGS)
LTM_LDFLAGS += $(LDFLAGS)
LTM_LIBTOOLFLAGS += $(LIBTOOLFLAGS)


ifeq ($(PLATFORM),FreeBSD)
  _ARCH := $(shell sysctl -b hw.machine_arch)
else
  _ARCH := $(shell uname -m)
endif

# adjust coverage set
ifneq ($(filter $(_ARCH), i386 i686 x86_64 amd64 ia64),)
   COVERAGE = test_standalone timing
   COVERAGE_APP = ./test && ./timing
else
   COVERAGE = test_standalone
   COVERAGE_APP = ./test
endif

HEADERS_PUB=tommath.h
HEADERS=tommath_private.h tommath_class.h tommath_superclass.h tommath_cutoffs.h $(HEADERS_PUB)

#LIBPATH  The directory for libtommath to be installed to.
#INCPATH  The directory to install the header files for libtommath.
#DATAPATH The directory to install the pdf docs.
DESTDIR  ?=
PREFIX   ?= /usr/local
LIBPATH  ?= $(PREFIX)/lib
INCPATH  ?= $(PREFIX)/include
DATAPATH ?= $(PREFIX)/share/doc/libtommath/pdf

#make the code coverage of the library
#
coverage: LTM_CFLAGS += -fprofile-arcs -ftest-coverage -DTIMING_NO_LOGS
coverage: LTM_LFLAGS += -lgcov
coverage: LTM_LDFLAGS += -lgcov

coverage: $(COVERAGE)
	$(COVERAGE_APP)

lcov: coverage
	rm -f coverage.info
	lcov --capture --no-external --no-recursion $(LCOV_ARGS) --output-file coverage.info -q
	genhtml coverage.info --output-directory coverage -q

# target that removes all coverage output
cleancov-clean:
	rm -f `find . -type f -name "*.info" | xargs`
	rm -rf coverage/

# cleans everything - coverage output and standard 'clean'
cleancov: cleancov-clean clean

clean:
	rm -f *.gcda *.gcno *.gcov *.bat *.o *.a *.obj *.lib *.exe *.dll etclib/*.o \
				demo/*.o test timing mtest_opponent mtest/mtest mtest/mtest.exe tuning_list \
				*.s mpi.c *.da *.dyn *.dpi tommath.tex `find . -type f | grep [~] | xargs` *.lo *.la
	rm -rf .libs/ demo/.libs
