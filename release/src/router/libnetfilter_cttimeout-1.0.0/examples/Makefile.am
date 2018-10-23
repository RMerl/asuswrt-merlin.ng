include $(top_srcdir)/Make_global.am

check_PROGRAMS = nfct-timeout-add	\
		 nfct-timeout-get

nfct_timeout_add_SOURCES = nfct-timeout-add.c
nfct_timeout_add_LDADD = ../src/libnetfilter_cttimeout.la ${LIBMNL_LIBS}

nfct_timeout_get_SOURCES = nfct-timeout-get.c
nfct_timeout_get_LDADD = ../src/libnetfilter_cttimeout.la ${LIBMNL_LIBS}
