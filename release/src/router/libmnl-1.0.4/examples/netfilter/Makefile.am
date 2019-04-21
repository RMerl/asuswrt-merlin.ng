include $(top_srcdir)/Make_global.am

check_PROGRAMS = nf-queue \
		 nf-log \
		 nfct-dump \
		 nfct-event \
		 nfct-create-batch \
		 nfct-daemon

nf_queue_SOURCES = nf-queue.c
nf_queue_LDADD = ../../src/libmnl.la

nf_log_SOURCES = nf-log.c
nf_log_LDADD = ../../src/libmnl.la

nfct_dump_SOURCES = nfct-dump.c
nfct_dump_LDADD = ../../src/libmnl.la

nfct_daemon_SOURCES = nfct-daemon.c
nfct_daemon_LDADD = ../../src/libmnl.la

nfct_event_SOURCES = nfct-event.c
nfct_event_LDADD = ../../src/libmnl.la

nfct_create_batch_SOURCES = nfct-create-batch.c
nfct_create_batch_LDADD = ../../src/libmnl.la
