include ${top_srcdir}/Make_global.am

check_PROGRAMS = nf-queue

nf_queue_SOURCES = nf-queue.c
nf_queue_LDADD = ../src/libnetfilter_queue.la
nf_queue_LDFLAGS = -dynamic -lmnl
