include $(top_srcdir)/Make_global.am

check_PROGRAMS = nfct-mnl-create	\
		 nfct-mnl-del		\
		 nfct-mnl-dump		\
		 nfct-mnl-dump-labels	\
		 nfct-mnl-set-label	\
		 nfct-mnl-event		\
		 nfct-mnl-flush		\
		 nfct-mnl-get		\
		 nfexp-mnl-dump		\
		 nfexp-mnl-event

nfct_mnl_create_SOURCES = nfct-mnl-create.c
nfct_mnl_create_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_create_LDFLAGS = -dynamic -ldl -lmnl

nfct_mnl_del_SOURCES = nfct-mnl-del.c
nfct_mnl_del_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_del_LDFLAGS = -dynamic -ldl -lmnl

nfct_mnl_dump_SOURCES = nfct-mnl-dump.c
nfct_mnl_dump_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_dump_LDFLAGS = -dynamic -ldl -lmnl

nfct_mnl_dump_labels_SOURCES = nfct-mnl-dump-labels.c
nfct_mnl_dump_labels_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_dump_labels_LDFLAGS = -dynamic -ldl -lmnl

nfct_mnl_set_label_SOURCES = nfct-mnl-set-label.c
nfct_mnl_set_label_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_set_label_LDFLAGS = -dynamic -ldl -lmnl

nfct_mnl_event_SOURCES = nfct-mnl-event.c
nfct_mnl_event_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_event_LDFLAGS = -dynamic -ldl -lmnl

nfct_mnl_flush_SOURCES = nfct-mnl-flush.c
nfct_mnl_flush_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_flush_LDFLAGS = -dynamic -ldl -lmnl

nfct_mnl_get_SOURCES = nfct-mnl-get.c
nfct_mnl_get_LDADD = ../src/libnetfilter_conntrack.la
nfct_mnl_get_LDFLAGS = -dynamic -ldl -lmnl

nfexp_mnl_dump_SOURCES = nfexp-mnl-dump.c
nfexp_mnl_dump_LDADD = ../src/libnetfilter_conntrack.la
nfexp_mnl_dump_LDFLAGS = -dynamic -ldl -lmnl

nfexp_mnl_event_SOURCES = nfexp-mnl-event.c
nfexp_mnl_event_LDADD = ../src/libnetfilter_conntrack.la
nfexp_mnl_event_LDFLAGS = -dynamic -ldl -lmnl
