## Process this file with automake to produce Makefile.in

# Files to be included in distribution
EXTRA_DIST    = ent/entitle.gif nist/template9 \
                contrib/diags/data_prep.c contrib/diags/bins.p contrib/diags/inc.p \
                contrib/diags/lognorm.r contrib/diags/lognorm.sh \
                haveged.spec contrib/build/build.sh contrib/build/lib.spec contrib/build/nolib.spec \
                contrib/build/havege_sample.c contrib/build/fedora.spec

# programs used in check targets
EXTRA_PROGRAMS = ent/entest nist/nist

# Build install script and tests as necessary
SUBDIRS       = src man ent nist

# check entire package
DISTCHECK_CONFIGURE_FLAGS = "--enable-nistest"
