# -*- Makefile -*-

EXTRA_DIST = \
	util.h

if ENABLE_UNIT_TESTS

AM_CPPFLAGS  = -Wall -I${top_srcdir}/include -I${top_builddir}/include -D_GNU_SOURCE -DSYSCONFDIR=\"$(sysconfdir)/libnl\"

LDADD = \
	${top_builddir}/lib/libnl-3.la \
	${top_builddir}/lib/libnl-nf-3.la \
	${top_builddir}/lib/libnl-genl-3.la \
	${top_builddir}/lib/libnl-route-3.la \
	@CHECK_LIBS@

AM_CFLAGS = @CHECK_CFLAGS@

UNIT_TESTS = check-all

check_PROGRAMS = \
	test-create-bond \
	test-create-vlan \
	test-create-vxlan \
	test-create-veth \
	test-create-bridge \
	test-create-ip6tnl \
	test-create-ipgre \
	test-create-ipip \
	test-create-ipvti \
	test-create-sit \
	test-delete-link \
	test-socket-creation \
	test-complex-HTB-with-hash-filters \
	test-u32-filter-with-actions \
	${UNIT_TESTS}

TESTS = \
	${UNIT_TESTS}

if ENABLE_CLI
LDADD += ${top_builddir}/src/lib/libnl-cli-3.la
check_PROGRAMS += \
	test-cache-mngr \
	test-genl \
	test-nf-cache-mngr
endif

test_cache_mngr_SOURCES = test-cache-mngr.c
test_create_bond_SOURCES = test-create-bond.c
test_create_vlan_SOURCES = test-create-vlan.c
test_create_vxlan_SOURCES = test-create-vxlan.c
test_create_veth_SOURCES = test-create-veth.c
test_create_bridge_SOURCES = test-create-bridge.c
test_delete_link_SOURCES = test-delete-link.c
test_genl_SOURCES = test-genl.c
test_nf_cache_mngr_SOURCES = test-nf-cache-mngr.c
test_socket_creation_SOURCES = test-socket-creation.c
test_complex_HTB_with_hash_filters_SOURCES = test-complex-HTB-with-hash-filters.c
test_u32_filter_with_actions_SOURCES = test-u32-filter-with-actions.c

# Unit tests
check_all_SOURCES = \
	check-all.c \
	check-addr.c \
	check-attr.c
endif
