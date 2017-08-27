TARGETS = ip-address.8 ip-link.8 ip-route.8

MAN8PAGES = $(TARGETS) ip.8 arpd.8 lnstat.8 routel.8 rtacct.8 rtmon.8 rtpr.8 ss.8 \
	tc.8 tc-bfifo.8 tc-bpf.8 tc-cbq.8 tc-cbq-details.8 tc-choke.8 tc-codel.8 \
	tc-fq.8 \
	tc-drr.8 tc-ematch.8 tc-fq_codel.8 tc-hfsc.8 tc-htb.8 tc-pie.8 \
	tc-mqprio.8 tc-netem.8 tc-pfifo.8 tc-pfifo_fast.8 tc-prio.8 tc-red.8 \
	tc-sfb.8 tc-sfq.8 tc-stab.8 tc-tbf.8 \
	bridge.8 rtstat.8 ctstat.8 nstat.8 routef.8 \
	ip-addrlabel.8 ip-fou.8 ip-gue.8 ip-l2tp.8 \
	ip-maddress.8 ip-monitor.8 ip-mroute.8 ip-neighbour.8 \
	ip-netns.8 ip-ntable.8 ip-rule.8 ip-tunnel.8 ip-xfrm.8 \
	ip-tcp_metrics.8 ip-netconf.8 ip-token.8 \
	tipc.8 tipc-bearer.8 tipc-link.8 tipc-media.8 tipc-nametable.8 \
	tipc-node.8 tipc-socket.8 \
	tc-basic.8 tc-cgroup.8 tc-flow.8 tc-flower.8 tc-fw.8 tc-route.8 \
	tc-tcindex.8 tc-u32.8

all: $(TARGETS)

ip-address.8: ip-address.8.in
	sed "s|@SYSCONFDIR@|$(CONFDIR)|g" $< > $@

ip-link.8: ip-link.8.in
	sed "s|@SYSCONFDIR@|$(CONFDIR)|g" $< > $@

ip-route.8: ip-route.8.in
	sed "s|@SYSCONFDIR@|$(CONFDIR)|g" $< > $@

distclean: clean

clean:
	@rm -f $(TARGETS)

install:
	$(INSTALLDIR) $(DESTDIR)$(MANDIR)/man8
	$(INSTALLMAN) $(MAN8PAGES) $(DESTDIR)$(MANDIR)/man8

.PHONY: install clean distclean
