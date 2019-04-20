include $(top_srcdir)/Make_global.am

check_PROGRAMS = expect_dump expect_create expect_get expect_delete \
	       expect_flush expect_events expect_create_userspace \
	       expect_create_nat \
	       conntrack_create conntrack_dump conntrack_update \
	       conntrack_delete conntrack_flush conntrack_create_nat \
	       conntrack_get conntrack_events \
	       conntrack_master conntrack_filter \
	       conntrack_grp_create \
	       conntrack_dump_filter \
	       ctexp_events

conntrack_grp_create_SOURCES = conntrack_grp_create.c
conntrack_grp_create_LDADD = ../src/libnetfilter_conntrack.la

conntrack_create_SOURCES = conntrack_create.c
conntrack_create_LDADD = ../src/libnetfilter_conntrack.la

conntrack_get_SOURCES = conntrack_get.c
conntrack_get_LDADD = ../src/libnetfilter_conntrack.la

conntrack_create_nat_SOURCES = conntrack_create_nat.c
conntrack_create_nat_LDADD = ../src/libnetfilter_conntrack.la

conntrack_update_SOURCES = conntrack_update.c
conntrack_update_LDADD = ../src/libnetfilter_conntrack.la

conntrack_delete_SOURCES = conntrack_delete.c
conntrack_delete_LDADD = ../src/libnetfilter_conntrack.la

conntrack_dump_SOURCES = conntrack_dump.c
conntrack_dump_LDADD = ../src/libnetfilter_conntrack.la

conntrack_dump_filter_SOURCES = conntrack_dump_filter.c
conntrack_dump_filter_LDADD = ../src/libnetfilter_conntrack.la

conntrack_flush_SOURCES = conntrack_flush.c
conntrack_flush_LDADD = ../src/libnetfilter_conntrack.la

conntrack_events_SOURCES = conntrack_events.c
conntrack_events_LDADD = ../src/libnetfilter_conntrack.la

conntrack_filter_SOURCES = conntrack_filter.c
conntrack_filter_LDADD = ../src/libnetfilter_conntrack.la

conntrack_master_SOURCES = conntrack_master.c
conntrack_master_LDADD = ../src/libnetfilter_conntrack.la

expect_dump_SOURCES = expect_dump.c
expect_dump_LDADD = ../src/libnetfilter_conntrack.la

expect_create_nat_SOURCES = expect_create_nat.c
expect_create_nat_LDADD = ../src/libnetfilter_conntrack.la

expect_create_SOURCES = expect_create.c
expect_create_LDADD = ../src/libnetfilter_conntrack.la

expect_create_userspace_SOURCES = expect_create_userspace.c
expect_create_userspace_LDADD = ../src/libnetfilter_conntrack.la

expect_get_SOURCES = expect_get.c
expect_get_LDADD = ../src/libnetfilter_conntrack.la

expect_delete_SOURCES = expect_delete.c
expect_delete_LDADD = ../src/libnetfilter_conntrack.la

expect_flush_SOURCES = expect_flush.c
expect_flush_LDADD = ../src/libnetfilter_conntrack.la

expect_events_SOURCES = expect_events.c
expect_events_LDADD = ../src/libnetfilter_conntrack.la

ctexp_events_SOURCES = ctexp_events.c
ctexp_events_LDADD = ../src/libnetfilter_conntrack.la
