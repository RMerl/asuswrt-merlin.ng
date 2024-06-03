# -*- Makefile -*-

# For debugging, use ./configure CPPFLAGS=-DEBT_DEBUG CFLAGS="-O0 -ggdb3"

PROGNAME = ${PACKAGE_NAME}
PROGVERSION = ${PACKAGE_VERSION}
PROGDATE = December\ 2011
INITDIR = /etc/rc.d/init.d
initddir = ${INITDIR}
sysconfigdir = ${sysconfdir}/sysconfig
EBTD_CMDLINE_MAXLN = 2048
EBTD_ARGC_MAX = 50
PIPE_DIR = /tmp/${PACKAGE_NAME}-v${PROGVERSION}
PIPE = ${PIPE_DIR}/ebtablesd_pipe


ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = ${regular_CPPFLAGS} -I${top_srcdir}/include \
	-DPROGVERSION=\"${PACKAGE_VERSION}\" -DPROGNAME=\"${PACKAGE_NAME}\" \
	-DPROGDATE=\"${PROGDATE}\" \
	-D_PATH_ETHERTYPES=\"${sysconfdir}/ethertypes\" \
	-DLOCKFILE=\"${LOCKFILE}\" \
	-DEBTD_ARGC_MAX=${EBTD_ARGC_MAX} -DEBTD_CMDLINE_MAXLN=${EBTD_CMDLINE_MAXLN} \
	-DEBTD_PIPE=\"${PIPE}\" -DEBTD_PIPE_DIR=\"${PIPE_DIR}\"
AM_CFLAGS = ${regular_CFLAGS}

sbin_PROGRAMS = ebtables-legacy ebtablesd ebtablesu ebtables-legacy-restore
EXTRA_PROGRAMS = static examples/ulog/test_ulog
sysconf_DATA = ethertypes
sbin_SCRIPTS = ebtables-legacy-save
man8_MANS = ebtables-legacy.8
lib_LTLIBRARIES = libebtc.la

libebtc_la_SOURCES = \
	communication.c ebtables.c getethertype.c \
	libebtc.c useful_functions.c \
	extensions/ebt_802_3.c extensions/ebt_among.c extensions/ebt_arp.c \
	extensions/ebt_arpreply.c extensions/ebt_ip.c extensions/ebt_ip6.c \
	extensions/ebt_limit.c extensions/ebt_log.c extensions/ebt_mark.c \
	extensions/ebt_mark_m.c extensions/ebt_nat.c extensions/ebt_nflog.c \
	extensions/ebt_pkttype.c extensions/ebt_redirect.c \
	extensions/ebt_standard.c extensions/ebt_stp.c extensions/ebt_string.c \
	extensions/ebt_ulog.c extensions/ebt_vlan.c extensions/ebt_AUDIT.c \
	extensions/ebtable_broute.c extensions/ebtable_filter.c \
	extensions/ebtable_nat.c \
	extensions/ebt_qos_map.c extensions/ebt_time.c extensions/ebt_ftos.c \
	extensions/ebt_skiplog.c extensions/ebt_skbvlan.c extensions/ebt_blog.c \
	extensions/ebt_reject.c extensions/ebt_ip_extend.c extensions/ebt_ip6_extend.c \
	extensions/ebt_u32.c extensions/ebt_vtag.c extensions/ebt_wmm_mark.c

# Make sure ebtables.c can be built twice
libebtc_la_CPPFLAGS = ${AM_CPPFLAGS}
ebtables_legacy_SOURCES = ebtables-standalone.c
ebtables_legacy_LDADD = libebtc.la
ebtablesd_LDADD = libebtc.la
ebtables_legacy_restore_SOURCES = ebtables-restore.c
ebtables_legacy_restore_LDADD = libebtc.la
static_SOURCES = ebtables.c
static_LDFLAGS = -static
static_LDADD = libebtc.la
examples_ulog_test_ulog_SOURCES = examples/ulog/test_ulog.c getethertype.c

daemon: ebtablesd ebtablesu
exec: ebtables-legacy ebtables-legacy-restore

CLEANFILES = ebtables-legacy-save ebtables-legacy.8

ebtables-legacy-save: ebtables-save.in ${top_builddir}/config.status
	${AM_V_GEN}sed -e 's![@]sbindir@!${sbindir}!g' <$< >$@

ebtables-legacy.8: ebtables-legacy.8.in ${top_builddir}/config.status
	${AM_V_GEN}sed -e 's![@]PACKAGE_VERSION!${PACKAGE_VERSION}!g' \
		-e 's![@]PACKAGE_DATE@!${PROGDATE}!g' \
		-e 's![@]LOCKFILE@!${LOCKFILE}!g' <$< >$@
