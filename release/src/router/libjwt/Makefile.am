ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = README.md README.cmake.md CMakeLists.txt cmake images

SUBDIRS = include libjwt tests dist
if BUILD_EXAMPLES
SUBDIRS += examples
endif

include $(top_srcdir)/doxygen.mk

check-valgrind: all
	$(MAKE) $(AM_MAKEFLAGS) -C tests check-valgrind

check-code-coverage: all
	$(MAKE) $(AM_MAKEFLAGS) -C tests check-code-coverage
