include $(top_srcdir)/Make_global.am

pkglib_LTLIBRARIES = ct_helper_amanda.la \
		     ct_helper_dhcpv6.la \
		     ct_helper_ftp.la	\
		     ct_helper_mdns.la	\
		     ct_helper_rpc.la	\
		     ct_helper_tftp.la	\
		     ct_helper_tns.la	\
		     ct_helper_sane.la	\
		     ct_helper_ssdp.la

HELPER_LDFLAGS = -avoid-version -module $(LIBNETFILTER_CONNTRACK_LIBS) @LAZY_LDFLAGS@
HELPER_CFLAGS = $(AM_CFLAGS) $(LIBNETFILTER_CONNTRACK_CFLAGS)

ct_helper_amanda_la_SOURCES = amanda.c
ct_helper_amanda_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_amanda_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_dhcpv6_la_SOURCES = dhcpv6.c
ct_helper_dhcpv6_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_dhcpv6_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_ftp_la_SOURCES = ftp.c
ct_helper_ftp_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_ftp_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_mdns_la_SOURCES = mdns.c
ct_helper_mdns_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_mdns_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_rpc_la_SOURCES = rpc.c
ct_helper_rpc_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_rpc_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_tftp_la_SOURCES = tftp.c
ct_helper_tftp_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_tftp_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_tns_la_SOURCES = tns.c
ct_helper_tns_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_tns_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_sane_la_SOURCES = sane.c
ct_helper_sane_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_sane_la_CFLAGS = $(HELPER_CFLAGS)

ct_helper_ssdp_la_SOURCES = ssdp.c
ct_helper_ssdp_la_LDFLAGS = $(HELPER_LDFLAGS)
ct_helper_ssdp_la_CFLAGS = $(HELPER_CFLAGS)
