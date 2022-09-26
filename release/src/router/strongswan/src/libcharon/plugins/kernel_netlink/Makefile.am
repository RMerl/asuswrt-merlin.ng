AM_CPPFLAGS = \
	-I${linux_headers} \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DROUTING_TABLE=${routing_table} \
	-DROUTING_TABLE_PRIO=${routing_table_prio}

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-kernel-netlink.la
else
plugin_LTLIBRARIES = libstrongswan-kernel-netlink.la
endif

libstrongswan_kernel_netlink_la_SOURCES = \
	kernel_netlink_plugin.h kernel_netlink_plugin.c \
	kernel_netlink_ipsec.h kernel_netlink_ipsec.c \
	kernel_netlink_net.h kernel_netlink_net.c \
	kernel_netlink_shared.h kernel_netlink_shared.c

libstrongswan_kernel_netlink_la_LIBADD = $(DLLIB)

libstrongswan_kernel_netlink_la_LDFLAGS = -module -avoid-version

TESTS = kernel_netlink_tests

check_PROGRAMS = $(TESTS)

kernel_netlink_tests_SOURCES = \
	tests.h tests.c \
	suites/test_socket.c \
	kernel_netlink_shared.c

kernel_netlink_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-DNETLINK_MSG_LOSS_HOOK=netlink_msg_loss \
	@COVERAGE_CFLAGS@

kernel_netlink_tests_LDFLAGS = @COVERAGE_LDFLAGS@
kernel_netlink_tests_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la
