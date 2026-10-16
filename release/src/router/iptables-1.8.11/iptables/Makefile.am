# -*- Makefile -*-

AM_CFLAGS        = ${regular_CFLAGS}
AM_CPPFLAGS      = ${regular_CPPFLAGS} \
                   -I${top_builddir}/include \
                   -I${top_srcdir}/include \
                   -I${top_srcdir} \
                   ${kinclude_CPPFLAGS} \
                   ${libmnl_CFLAGS} \
                   ${libnftnl_CFLAGS} \
                   ${libnetfilter_conntrack_CFLAGS}
AM_LDFLAGS       = ${regular_LDFLAGS}

BUILT_SOURCES =

common_sources = iptables-xml.c xtables-multi.h xshared.c xshared.h
common_ldadd   = ../extensions/libext.a ../libxtables/libxtables.la -lm -ldl
common_cflags  = ${AM_CFLAGS}
if ENABLE_STATIC
common_cflags += -DALL_INCLUSIVE
endif

xtables_legacy_multi_SOURCES  = ${common_sources} xtables-legacy-multi.c \
				iptables-restore.c iptables-save.c
xtables_legacy_multi_CFLAGS   = ${common_cflags}
xtables_legacy_multi_LDADD    = ${common_ldadd}
if ENABLE_IPV4
xtables_legacy_multi_SOURCES += iptables-standalone.c iptables.c iptables-multi.h
xtables_legacy_multi_CFLAGS  += -DENABLE_IPV4
xtables_legacy_multi_LDADD   += ../libiptc/libip4tc.la ../extensions/libext4.a
endif
if ENABLE_IPV6
xtables_legacy_multi_SOURCES += ip6tables-standalone.c ip6tables.c ip6tables-multi.h
xtables_legacy_multi_CFLAGS  += -DENABLE_IPV6
xtables_legacy_multi_LDADD   += ../libiptc/libip6tc.la ../extensions/libext6.a
endif

# iptables using nf_tables api
if ENABLE_NFTABLES
xtables_nft_multi_SOURCES  = ${common_sources} xtables-nft-multi.c
xtables_nft_multi_CFLAGS   = ${common_cflags}
xtables_nft_multi_LDADD    = ${common_ldadd} \
			     ../extensions/libext_arpt.a \
			     ../extensions/libext_ebt.a \
			     ../extensions/libext4.a \
			     ../extensions/libext6.a \
			     ${libmnl_LIBS} ${libnftnl_LIBS} \
			     ${libnetfilter_conntrack_LIBS}
xtables_nft_multi_CFLAGS  += -DENABLE_NFTABLES -DENABLE_IPV4 -DENABLE_IPV6
xtables_nft_multi_SOURCES += nft.c nft.h \
			     nft-arp.c nft-ipv4.c nft-ipv6.c \
			     nft-bridge.c nft-bridge.h \
			     nft-cache.c nft-cache.h \
			     nft-chain.c nft-chain.h \
			     nft-cmd.c nft-cmd.h \
			     nft-ruleparse.c nft-ruleparse.h \
			     nft-ruleparse-arp.c nft-ruleparse-bridge.c \
			     nft-ruleparse-ipv4.c nft-ruleparse-ipv6.c \
			     nft-shared.c nft-shared.h \
			     xtables-monitor.c \
			     xtables.c xtables-arp.c xtables-eb.c \
			     xtables-standalone.c xtables-eb-standalone.c \
			     xtables-translate.c xtables-eb-translate.c \
			     xtables-save.c xtables-restore.c
endif

sbin_PROGRAMS    = xtables-legacy-multi
if ENABLE_NFTABLES
sbin_PROGRAMS	+= xtables-nft-multi
endif
man_MANS         = iptables.8 iptables-restore.8 iptables-save.8 \
                   iptables-xml.1 ip6tables.8 ip6tables-restore.8 \
                   ip6tables-save.8 iptables-extensions.8 \
                   iptables-apply.8 ip6tables-apply.8

dist_sbin_SCRIPTS = iptables-apply
dist_pkgdata_DATA = iptables.xslt

xlate_man_links = iptables-translate.8 ip6tables-translate.8 \
		  iptables-restore-translate.8 ip6tables-restore-translate.8 \
		  ebtables-translate.8 arptables-translate.8

if ENABLE_NFTABLES
man_MANS	+= ${xlate_man_links} xtables-monitor.8

dist_man_MANS	 = xtables-nft.8 xtables-translate.8 xtables-legacy.8 \
		   arptables-nft.8 arptables-nft-restore.8 arptables-nft-save.8 \
		   ebtables-nft.8
endif
CLEANFILES       = ${man_MANS} iptables-extensions.8.tmpl

vx_bin_links   = iptables-xml
if ENABLE_IPV4
v4_sbin_links  = iptables-legacy iptables-legacy-restore iptables-legacy-save \
		 iptables iptables-restore iptables-save
endif
if ENABLE_IPV6
v6_sbin_links  = ip6tables-legacy ip6tables-legacy-restore ip6tables-legacy-save \
		 ip6tables ip6tables-restore ip6tables-save
endif
if ENABLE_NFTABLES
x_sbin_links  = iptables-nft iptables-nft-restore iptables-nft-save \
		ip6tables-nft ip6tables-nft-restore ip6tables-nft-save \
		iptables-translate ip6tables-translate ebtables-translate \
		iptables-restore-translate ip6tables-restore-translate \
		arptables-nft arptables \
		arptables-nft-restore arptables-restore \
		arptables-nft-save arptables-save \
		arptables-translate \
		ebtables-nft ebtables \
		ebtables-nft-restore ebtables-restore \
		ebtables-nft-save ebtables-save \
		xtables-monitor
endif

iptables-extensions.8: iptables-extensions.8.tmpl ../extensions/matches.man ../extensions/targets.man
	${AM_V_GEN} sed \
		-e '/@MATCH@/ r ../extensions/matches.man' \
		-e '/@TARGET@/ r ../extensions/targets.man' $< >$@;

${xlate_man_links}:
	${AM_V_GEN} echo '.so man8/xtables-translate.8' >$@

ip6tables.8 ip6tables-apply.8 ip6tables-restore.8 ip6tables-save.8:
	${AM_V_GEN} echo "$@" | sed 's|^ip6|.so man8/ip|' >$@

pkgconfig_DATA = xtables.pc

# Using if..fi avoids an ugly "error (ignored)" message :)
install-exec-hook:
	${INSTALL} -dm0755 "${DESTDIR}${bindir}";
	for i in ${vx_bin_links}; do ${LN_S} -f "${sbindir}/xtables-legacy-multi" "${DESTDIR}${bindir}/$$i"; done;
	for i in ${v4_sbin_links}; do ${LN_S} -f xtables-legacy-multi "${DESTDIR}${sbindir}/$$i"; done;
	for i in ${v6_sbin_links}; do ${LN_S} -f xtables-legacy-multi "${DESTDIR}${sbindir}/$$i"; done;
	for i in ${x_sbin_links}; do ${LN_S} -f xtables-nft-multi "${DESTDIR}${sbindir}/$$i"; done;
	${LN_S} -f iptables-apply "${DESTDIR}${sbindir}/ip6tables-apply"

uninstall-hook:
	dir=${DESTDIR}${bindir}; { \
		test ! -d "$$dir" && test ! -f "$$dir" && test ! -r "$$dir"; \
	} || { \
		test -z "${vx_bin_links}" || ( \
			cd "$$dir" && rm -f ${vx_bin_links} \
		) \
	}
	dir=${DESTDIR}${sbindir}; { \
		test ! -d "$$dir" && test ! -f "$$dir" && test ! -r "$$dir"; \
	} || { \
		test -z "${v4_sbin_links}" || ( \
			cd "$$dir" && rm -f ${v4_sbin_links} \
		); \
		test -z "${v6_sbin_links}" || ( \
			cd "$$dir" && rm -f ${v6_sbin_links} \
		); \
		test -z "${x_sbin_links}" || ( \
			cd "$$dir" && rm -f ${x_sbin_links} \
		); \
		( cd "$$dir" && rm -f ip6tables-apply ); \
	}

EXTRA_DIST = tests
