include $(top_srcdir)/Make_global.am

check_PROGRAMS = test_api test_filter test_connlabel ct_stress \
	ct_events_reliable

test_api_SOURCES = test_api.c
test_api_LDADD = ../src/libnetfilter_conntrack.la

test_connlabel_SOURCES = test_connlabel.c
test_connlabel_LDADD = ../src/libnetfilter_conntrack.la

test_filter_SOURCES = test_filter.c
test_filter_LDADD = ../src/libnetfilter_conntrack.la

ct_stress_SOURCES = ct_stress.c
ct_stress_LDADD = ../src/libnetfilter_conntrack.la

ct_events_reliable_SOURCES = ct_events_reliable.c
ct_events_reliable_LDADD = ../src/libnetfilter_conntrack.la
