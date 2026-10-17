# -*- Makefile -*-

AM_CPPFLAGS  =			 	\
	-Wall \
	-I${top_srcdir}/include \
	-I${top_builddir}/include \
	-I${builddir}/route \
	-I${builddir}/route/cls \
	-D_GNU_SOURCE \
	-DSYSCONFDIR=\"$(sysconfdir)/libnl\"

AM_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-Wl,--version-script=$(top_builddir)/libnl.sym

lib_LTLIBRARIES = \
	libnl-3.la libnl-genl-3.la libnl-route-3.la libnl-nf-3.la libnl-idiag-3.la

libnl_3_la_SOURCES = \
	addr.c attr.c cache.c cache_mngr.c cache_mngt.c data.c \
	error.c handlers.c msg.c nl.c object.c socket.c utils.c \
	version.c hash.c hashtable.c

libnl_idiag_3_la_LIBADD = libnl-3.la
libnl_idiag_3_la_SOURCES = \
	idiag/idiag_meminfo_obj.c idiag/idiag_vegasinfo_obj.c \
	idiag/idiag_msg_obj.c idiag/idiag_req_obj.c idiag/idiag.c

libnl_genl_3_la_LIBADD  = libnl-3.la
libnl_genl_3_la_SOURCES = \
	genl/ctrl.c genl/family.c genl/genl.c genl/mngt.c

libnl_nf_3_la_LIBADD  = libnl-route-3.la
libnl_nf_3_la_SOURCES = \
	netfilter/ct.c netfilter/ct_obj.c netfilter/log.c \
	netfilter/log_msg.c netfilter/log_msg_obj.c netfilter/log_obj.c \
	netfilter/netfilter.c netfilter/nfnl.c netfilter/queue.c \
	netfilter/queue_msg.c netfilter/queue_msg_obj.c netfilter/queue_obj.c \
	netfilter/exp.c netfilter/exp_obj.c

CLEANFILES = \
	route/pktloc_grammar.c route/pktloc_grammar.h \
	route/pktloc_syntax.c route/pktloc_syntax.h \
	route/cls/ematch_grammar.c route/cls/ematch_grammar.h \
	route/cls/ematch_syntax.c route/cls/ematch_syntax.h

# Hack to avoid using ylwrap. It does not function correctly in combination
# with --header-file=
route/pktloc_grammar.c: route/pktloc_grammar.l
	$(AM_V_GEN) $(MKDIR_P) route; $(FLEX) --header-file=route/pktloc_grammar.h $(LFLAGS) -o $@ $^

route/pktloc_syntax.c: route/pktloc_syntax.y
	$(AM_V_GEN) $(MKDIR_P) route; $(YACC) -d $(YFLAGS) -o $@ $^

route/cls/ematch_grammar.c: route/cls/ematch_grammar.l
	$(AM_V_GEN) $(MKDIR_P) route/cls; $(FLEX) --header-file=route/cls/ematch_grammar.h $(LFLAGS) -o $@ $^

route/cls/ematch_syntax.c: route/cls/ematch_syntax.y
	$(AM_V_GEN) $(MKDIR_P) route/cls; $(YACC) -d $(YFLAGS) -o $@ $^

libnl_route_3_la_LIBADD  = libnl-3.la
libnl_route_3_la_SOURCES = \
	route/addr.c route/class.c route/cls.c route/act.c route/link.c \
	route/neigh.c route/neightbl.c route/nexthop.c route/qdisc.c \
	route/route.c route/route_obj.c route/route_utils.c route/rtnl.c \
	route/rule.c route/tc.c route/classid.c \
	\
	route/cls/fw.c route/cls/police.c route/cls/u32.c route/cls/basic.c \
	route/cls/cgroup.c \
	\
	route/act/mirred.c \
	\
	route/cls/ematch.c \
	route/cls/ematch/container.c route/cls/ematch/cmp.c \
	route/cls/ematch/nbyte.c route/cls/ematch/text.c \
	route/cls/ematch/meta.c \
	\
	route/link/api.c route/link/vlan.c route/link/dummy.c \
	route/link/bridge.c route/link/inet6.c route/link/inet.c \
	route/link/bonding.c route/link/can.c route/link/macvlan.c \
	route/link/vxlan.c route/link/veth.c route/link/ipip.c \
	route/link/ipgre.c route/link/sit.c route/link/ipvti.c \
	route/link/ip6tnl.c \
	\
	route/qdisc/blackhole.c route/qdisc/cbq.c route/qdisc/dsmark.c \
	route/qdisc/fifo.c route/qdisc/htb.c route/qdisc/netem.c \
	route/qdisc/prio.c route/qdisc/red.c route/qdisc/sfq.c \
	route/qdisc/tbf.c route/qdisc/plug.c route/qdisc/ingress.c \
	route/qdisc/fq_codel.c \
	\
	fib_lookup/lookup.c fib_lookup/request.c \
	\
	route/pktloc.c

nodist_libnl_route_3_la_SOURCES = \
	route/pktloc_syntax.c route/pktloc_syntax.h \
	route/pktloc_grammar.c route/pktloc_grammar.h \
	route/cls/ematch_syntax.c route/cls/ematch_syntax.h \
	route/cls/ematch_grammar.c route/cls/ematch_grammar.h

BUILT_SOURCES = \
	route/cls/ematch_grammar.c \
	route/cls/ematch_syntax.c \
	route/pktloc_grammar.c \
	route/pktloc_syntax.c

EXTRA_DIST = \
	route/pktloc_grammar.l \
	route/pktloc_syntax.y \
	route/cls/ematch_grammar.l \
	route/cls/ematch_syntax.y

if ENABLE_CLI
nobase_pkglib_LTLIBRARIES = \
	cli/qdisc/htb.la \
	cli/qdisc/blackhole.la \
	cli/qdisc/pfifo.la \
	cli/qdisc/plug.la \
	cli/qdisc/bfifo.la \
	cli/qdisc/ingress.la \
	cli/qdisc/fq_codel.la \
	cli/cls/basic.la \
	cli/cls/cgroup.la

cli_qdisc_htb_la_LDFLAGS = -module -avoid-version
cli_qdisc_blackhole_la_LDFLAGS = -module -avoid-version
cli_qdisc_pfifo_la_LDFLAGS = -module -avoid-version
cli_qdisc_plug_la_LDFLAGS = -module -avoid-version
cli_qdisc_bfifo_la_LDFLAGS = -module -avoid-version
cli_qdisc_ingress_la_LDFLAGS = -module -avoid-version
cli_qdisc_fq_codel_la_LDFLAGS = -module -avoid-version
cli_cls_basic_la_LDFLAGS = -module -avoid-version
cli_cls_cgroup_la_LDFLAGS = -module -avoid-version
endif
