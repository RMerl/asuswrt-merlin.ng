#
# Include makefile for libtommath
#

#version of library
VERSION=1.0.1
VERSION_PC=1.0.1
VERSION_SO=1:1

PLATFORM := $(shell uname | sed -e 's/_.*//')

# default make target
default: ${LIBNAME}

# Compiler and Linker Names
ifndef CROSS_COMPILE
  CROSS_COMPILE=
endif

# Dropbear passes these down
#ifeq ($(CC),cc)
#  CC = $(CROSS_COMPILE)gcc
#endif
#LD=$(CROSS_COMPILE)ld
#AR=$(CROSS_COMPILE)ar
#RANLIB=$(CROSS_COMPILE)ranlib

ifndef MAKE
   MAKE=make
endif

CFLAGS += -I./ -Wall -Wsign-compare -Wextra -Wshadow

ifndef NO_ADDTL_WARNINGS
# additional warnings
CFLAGS += -Wsystem-headers -Wdeclaration-after-statement -Wbad-function-cast -Wcast-align
CFLAGS += -Wstrict-prototypes -Wpointer-arith
endif

ifdef COMPILE_DEBUG
#debug
CFLAGS += -g3
else

ifdef COMPILE_SIZE
#for size
CFLAGS += -Os
else

ifndef IGNORE_SPEED
#for speed
CFLAGS += -O3 -funroll-loops

#x86 optimizations [should be valid for any GCC install though]
CFLAGS  += -fomit-frame-pointer
endif

endif # COMPILE_SIZE
endif # COMPILE_DEBUG

ifneq ($(findstring clang,$(CC)),)
CFLAGS += -Wno-typedef-redefinition -Wno-tautological-compare -Wno-builtin-requires-header
endif
ifeq ($(PLATFORM), Darwin)
CFLAGS += -Wno-nullability-completeness
endif

# adjust coverage set
ifneq ($(filter $(shell arch), i386 i686 x86_64 amd64 ia64),)
   COVERAGE = test_standalone timing
   COVERAGE_APP = ./test && ./ltmtest
else
   COVERAGE = test_standalone
   COVERAGE_APP = ./test
endif

HEADERS_PUB=tommath.h tommath_class.h tommath_superclass.h
HEADERS=tommath_private.h $(HEADERS_PUB)

test_standalone: CFLAGS+=-DLTM_DEMO_TEST_VS_MTEST=0

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
coverage: CFLAGS += -fprofile-arcs -ftest-coverage -DTIMING_NO_LOGS
coverage: LFLAGS += -lgcov
coverage: LDFLAGS += -lgcov

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
	rm -f *.gcda *.gcno *.gcov *.bat *.o *.a *.obj *.lib *.exe *.dll etclib/*.o demo/demo.o test ltmtest mpitest mtest/mtest mtest/mtest.exe \
        *.idx *.toc *.log *.aux *.dvi *.lof *.ind *.ilg *.ps *.log *.s mpi.c *.da *.dyn *.dpi tommath.tex `find . -type f | grep [~] | xargs` *.lo *.la
	rm -rf .libs/
	#${MAKE} -C etc/ clean MAKE=${MAKE}
	#${MAKE} -C doc/ clean MAKE=${MAKE}
