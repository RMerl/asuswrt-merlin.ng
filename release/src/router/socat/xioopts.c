/* source: xioopts.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for address options handling */

#include "xiosysincludes.h"
#include "xioopen.h"
#include "xio-unix.h"

#include "xiomodes.h"
#include "xiolockfile.h"
#include "nestlex.h"

bool xioopts_ignoregroups;

#define IF_ANY(a,b) {a,b},

#if WITH_NAMED
#  define IF_NAMED(a,b) {a,b},
#else
#  define IF_NAMED(a,b) 
#endif

#if WITH_PIPE || WITH_GOPEN
#  define IF_OPEN(a,b) {a,b},
#else
#  define IF_OPEN(a,b) 
#endif

#if WITH_TERMIOS
#  define IF_TERMIOS(a,b) {a,b},
#else
#  define IF_TERMIOS(a,b) 
#endif

#if WITH_EXEC
#  define IF_EXEC(a,b) {a,b},
#else
#  define IF_EXEC(a,b) 
#endif

#if _WITH_SOCKET
#  define IF_SOCKET(a,b) {a,b},
#else
#  define IF_SOCKET(a,b) 
#endif

#if WITH_LISTEN
#  define IF_LISTEN(a,b) {a,b},
#else
#  define IF_LISTEN(a,b) 
#endif

#if (WITH_UDP || WITH_TCP) && WITH_LISTEN
#  define IF_RANGE(a,b) {a,b},
#else
#  define IF_RANGE(a,b)
#endif

#if WITH_IP4 || WITH_IP6
#  define IF_IP(a,b) {a,b},
#else
#  define IF_IP(a,b) 
#endif

#if WITH_IP6
#  define IF_IP6(a,b) {a,b},
#else
#  define IF_IP6(a,b) 
#endif

#if WITH_TCP|WITH_UDP
#  define IF_IPAPP(a,b) {a,b},
#else
#  define IF_IPAPP(a,b) 
#endif

#if WITH_TCP
#  define IF_TCP(a,b) {a,b},
#else
#  define IF_TCP(a,b) 
#endif

#if WITH_SCTP
#  define IF_SCTP(a,b) {a,b},
#else
#  define IF_SCTP(a,b) 
#endif

#if WITH_SOCKS4
#  define IF_SOCKS4(a,b) {a,b},
#else
#  define IF_SOCKS4(a,b) 
#endif

#if WITH_PROXY
#  define IF_PROXY(a,b) {a,b},
#else
#  define IF_PROXY(a,b) 
#endif

#if WITH_READLINE
#  define IF_READLINE(a,b) {a,b},
#else
#  define IF_READLINE(a,b)
#endif

#if WITH_PTY
#  define IF_PTY(a,b) {a,b},
#else
#  define IF_PTY(a,b)
#endif

#if WITH_OPENSSL
#  define IF_OPENSSL(a,b) {a,b},
#else
#  define IF_OPENSSL(a,b)
#endif

#if WITH_TUN
#  define IF_TUN(a,b) {a,b},
#else
#  define IF_TUN(a,b)
#endif

#if WITH_UNIX
#  define IF_UNIX(a,b) {a,b},
#else
#  define IF_UNIX(a,b)
#endif

#if WITH_RETRY
#  define IF_RETRY(a,b) {a,b},
#else
#  define IF_RETRY(a,b) 
#endif


static int applyopt_offset(struct single *xfd, struct opt *opt);


/* address options - keep this array strictly alphabetically sorted for
   binary search! */
/* NULL terminated */
const struct optname optionnames[] = {
#if HAVE_RESOLV_H
	IF_IP     ("aaonly",	&opt_res_aaonly)
#endif /* HAVE_RESOLV_H */
#ifdef TCP_ABORT_THRESHOLD  /* HP_UX */
	IF_TCP    ("abort-threshold",	&opt_tcp_abort_threshold)
#endif
#ifdef SO_ACCEPTCONN /* AIX433 */
	IF_SOCKET ("acceptconn",	&opt_so_acceptconn)
#endif /* SO_ACCEPTCONN */
#ifdef IP_ADD_MEMBERSHIP
	IF_IP     ("add-membership",	&opt_ip_add_membership)
#endif
	IF_TUN    ("allmulti",	&opt_iff_allmulti)
#if WITH_LIBWRAP && defined(HAVE_HOSTS_ALLOW_TABLE)
	IF_IPAPP  ("allow-table",	&opt_tcpwrap_hosts_allow_table)
#endif
	IF_ANY    ("append",	&opt_append)
#ifdef O_ASYNC
	IF_ANY    ("async",	&opt_async)
#endif
#ifdef SO_ATTACH_FILTER
	IF_SOCKET ("attach-filter",	&opt_so_attach_filter)
	IF_SOCKET ("attachfilter",	&opt_so_attach_filter)
#endif
#ifdef SO_AUDIT	/* AIX 4.3.3 */
	IF_SOCKET ("audit",	&opt_so_audit)
#endif /* SO_AUDIT */
#ifdef IPV6_AUTHHDR
	IF_IP6    ("authhdr",	&opt_ipv6_authhdr)
#endif
	IF_TUN    ("automedia",	&opt_iff_automedia)
#ifdef CBAUD
	IF_TERMIOS("b0",	&opt_b0)
#ifdef B1000000
	IF_TERMIOS("b1000000",	&opt_b1000000)
#endif
	IF_TERMIOS("b110",	&opt_b110)
#ifdef B115200
	IF_TERMIOS("b115200",	&opt_b115200)
#endif
#ifdef B1152000
	IF_TERMIOS("b1152000",	&opt_b1152000)
#endif
	IF_TERMIOS("b1200",	&opt_b1200)
	IF_TERMIOS("b134",	&opt_b134)
	IF_TERMIOS("b150",	&opt_b150)
#ifdef B1500000
	IF_TERMIOS("b1500000",	&opt_b1500000)
#endif
	IF_TERMIOS("b1800",	&opt_b1800)
	IF_TERMIOS("b19200",	&opt_b19200)
	IF_TERMIOS("b200",	&opt_b200)
#ifdef B2000000
	IF_TERMIOS("b2000000",	&opt_b2000000)
#endif
#ifdef B230400
	IF_TERMIOS("b230400",	&opt_b230400)
#endif
	IF_TERMIOS("b2400",	&opt_b2400)
#ifdef B2500000
	IF_TERMIOS("b2500000",	&opt_b2500000)
#endif
	IF_TERMIOS("b300",	&opt_b300)
#ifdef B3000000
	IF_TERMIOS("b3000000",	&opt_b3000000)
#endif
#ifdef B3500000
	IF_TERMIOS("b3500000",	&opt_b3500000)
#endif
#ifdef B3600	/* HP-UX */
	IF_TERMIOS("b3600",	&opt_b3600)
#endif
	IF_TERMIOS("b38400",	&opt_b38400)
#ifdef B4000000
	IF_TERMIOS("b4000000",	&opt_b4000000)
#endif
#ifdef B460800
	IF_TERMIOS("b460800",	&opt_b460800)
#endif
	IF_TERMIOS("b4800",	&opt_b4800)
	IF_TERMIOS("b50",	&opt_b50)
#ifdef B500000
	IF_TERMIOS("b500000",	&opt_b500000)
#endif
#ifdef B57600
	IF_TERMIOS("b57600",	&opt_b57600)
#endif
#ifdef B576000
	IF_TERMIOS("b576000",	&opt_b576000)
#endif
	IF_TERMIOS("b600",	&opt_b600)
#ifdef B7200	/* HP-UX */
	IF_TERMIOS("b7200",	&opt_b7200)
#endif
	IF_TERMIOS("b75",	&opt_b75)
#ifdef B900	/* HP-UX */
	IF_TERMIOS("b900",	&opt_b900)
#endif
#ifdef B921600
	IF_TERMIOS("b921600",	&opt_b921600)
#endif
	IF_TERMIOS("b9600",	&opt_b9600)
#endif /* defined(CBAUD) */
	IF_LISTEN ("backlog",	&opt_backlog)
#ifdef O_BINARY
	IF_OPEN   ("bin",		&opt_o_binary)
	IF_OPEN   ("binary",		&opt_o_binary)
#endif
	IF_SOCKET ("bind",	&opt_bind)
#ifdef SO_BINDTODEVICE
	IF_SOCKET ("bindtodevice",	&opt_so_bindtodevice)
#endif
	IF_TERMIOS("brkint",	&opt_brkint)
	IF_SOCKET ("broadcast",	&opt_so_broadcast)
#ifdef BSDLY
#  ifdef BS0
	IF_TERMIOS("bs0",	&opt_bs0)
#  endif
#  ifdef BS1
	IF_TERMIOS("bs1",	&opt_bs1)
#  endif
#endif
#ifdef SO_BSDCOMPAT
	IF_SOCKET ("bsdcompat",	&opt_so_bsdcompat)
#endif
#ifdef BSDLY
	IF_TERMIOS("bsdly",	&opt_bsdly)
#endif
	IF_ANY    ("bytes",     &opt_readbytes)
	IF_OPENSSL("cafile",	&opt_openssl_cafile)
	IF_OPENSSL("capath",	&opt_openssl_capath)
	IF_OPENSSL("cert",	&opt_openssl_certificate)
	IF_OPENSSL("certificate",	&opt_openssl_certificate)
	IF_TERMIOS("cfmakeraw",		&opt_termios_cfmakeraw)
	IF_ANY    ("chroot",	&opt_chroot)
	IF_ANY    ("chroot-early",	&opt_chroot_early)
	/*IF_TERMIOS("cibaud",	&opt_cibaud)*/
	IF_OPENSSL("cipher",	&opt_openssl_cipherlist)
	IF_OPENSSL("cipherlist",	&opt_openssl_cipherlist)
	IF_OPENSSL("ciphers",	&opt_openssl_cipherlist)
#ifdef SO_CKSUMRECV
	IF_SOCKET ("cksumrecv",	&opt_so_cksumrecv)
#endif /* SO_CKSUMRECV */
	/*IF_NAMED  ("cleanup",	&opt_cleanup)*/
	IF_TERMIOS("clocal",	&opt_clocal)
	IF_ANY    ("cloexec",	&opt_cloexec)
	IF_ANY    ("close",	&opt_end_close)
	IF_OPENSSL("cn",		&opt_openssl_commonname)
	IF_OPENSSL("commonname",	&opt_openssl_commonname)
#if WITH_EXT2 && defined(EXT2_COMPR_FL)
	IF_ANY    ("compr",	&opt_ext2_compr)
#endif
#if OPENSSL_VERSION_NUMBER >= 0x00908000L && !defined(OPENSSL_NO_COMP)
	IF_OPENSSL("compress",	&opt_openssl_compress)
#endif
#ifdef TCP_CONN_ABORT_THRESHOLD  /* HP_UX */
	IF_TCP    ("conn-abort-threshold",	&opt_tcp_conn_abort_threshold)
#endif
	IF_SOCKET ("connect-timeout",	&opt_connect_timeout)
	IF_LISTEN ("cool-write",	&opt_cool_write)
	IF_LISTEN ("coolwrite",	&opt_cool_write)
#ifdef TCP_CORK
	IF_TCP    ("cork",	&opt_tcp_cork)
#endif
	IF_ANY    ("cr",        &opt_cr)
#ifdef CRDLY
#  ifdef CR0
	IF_TERMIOS("cr0",	&opt_cr0)
#  endif
#  ifdef CR1
	IF_TERMIOS("cr1",	&opt_cr1)
#  endif
#  ifdef CR2
	IF_TERMIOS("cr2",	&opt_cr2)
#  endif
#  ifdef CR3
	IF_TERMIOS("cr3",	&opt_cr3)
#  endif
#  if CRDLY_SHIFT >= 0
	IF_TERMIOS("crdly",	&opt_crdly)
#  endif
#endif /* defined(CRDLY) */
	IF_TERMIOS("cread",	&opt_cread)
	IF_OPEN   ("creat",	&opt_o_create)
	IF_OPEN   ("create",	&opt_o_create)
	IF_ANY    ("crlf",      &opt_crnl)
	IF_ANY    ("crnl",      &opt_crnl)
	IF_TERMIOS("crterase",	&opt_echoe)
	IF_TERMIOS("crtkill",	&opt_echoke)
#ifdef CRTSCTS
	IF_TERMIOS("crtscts",	&opt_crtscts)
#endif
	IF_TERMIOS("cs5",	&opt_cs5)
	IF_TERMIOS("cs6",	&opt_cs6)
	IF_TERMIOS("cs7",	&opt_cs7)
	IF_TERMIOS("cs8",	&opt_cs8)
#if CSIZE_SHIFT >= 0
	IF_TERMIOS("csize",	&opt_csize)
#endif
	IF_TERMIOS("cstopb",	&opt_cstopb)
	IF_TERMIOS("ctlecho",	&opt_echoctl)
	IF_TERMIOS("ctty",	&opt_tiocsctty)
	IF_EXEC   ("dash",	&opt_dash)
	IF_SOCKET ("debug",	&opt_so_debug)
	/*IF_IP     ("debug",	&opt_res_debug)*/
#ifdef O_DEFER
	IF_OPEN   ("defer",	&opt_o_defer)
#endif
#ifdef TCP_DEFER_ACCEPT	/* Linux 2.4.0 */
	IF_TCP    ("defer-accept",	&opt_tcp_defer_accept)
#endif
#if HAVE_RESOLV_H
	IF_IP     ("defnames",	&opt_res_defnames)
#endif /* HAVE_RESOLV_H */
#ifdef O_DELAY
	IF_OPEN   ("delay",	&opt_o_delay)
#endif
	IF_NAMED  ("delete",	&opt_unlink)
#if WITH_LIBWRAP && defined(HAVE_HOSTS_DENY_TABLE)
	IF_IPAPP  ("deny-table",	&opt_tcpwrap_hosts_deny_table)
#endif
#ifdef SO_DETACH_FILTER
	IF_SOCKET ("detach-filter", &opt_so_detach_filter)
	IF_SOCKET ("detachfilter",  &opt_so_detach_filter)
#endif
#ifdef SO_DGRAM_ERRIND
	IF_SOCKET ("dgram-errind",	&opt_so_dgram_errind)
	IF_SOCKET ("dgramerrind",	&opt_so_dgram_errind)
#endif
	IF_OPENSSL("dh",	&opt_openssl_dhparam)
	IF_OPENSSL("dhparam",	&opt_openssl_dhparam)
	IF_OPENSSL("dhparams",	&opt_openssl_dhparam)
#ifdef O_DIRECT
	IF_OPEN   ("direct",	&opt_o_direct)
#endif
#ifdef O_DIRECTORY
	IF_OPEN   ("directory",	&opt_o_directory)
#endif
#if WITH_EXT2 && defined(EXT2_DIRSYNC_FL)
	IF_ANY    ("dirsync",	&opt_ext2_dirsync)
#endif
#ifdef VDISCARD
	IF_TERMIOS("discard",	&opt_vdiscard)
#endif
#if HAVE_RESOLV_H
	IF_IP     ("dnsrch",	&opt_res_dnsrch)
#endif /* HAVE_RESOLV_H */
#ifdef SO_DONTLINGER
	IF_SOCKET ("dontlinger",	&opt_so_dontlinger)
#endif
	IF_SOCKET ("dontroute",	&opt_so_dontroute)
#ifdef IPV6_DSTOPTS
	IF_IP6    ("dstopts",	&opt_ipv6_dstopts)
#endif
#ifdef VDSUSP	/* HP-UX */
	IF_TERMIOS("dsusp",	&opt_vdsusp)
#endif
#ifdef O_DSYNC
	IF_OPEN   ("dsync",	&opt_o_dsync)
#endif
	IF_TERMIOS("echo",	&opt_echo)
	IF_TERMIOS("echoctl",	&opt_echoctl)
	IF_TERMIOS("echoe",	&opt_echoe)
	IF_TERMIOS("echok",	&opt_echok)
	IF_TERMIOS("echoke",	&opt_echoke)
	IF_TERMIOS("echonl",	&opt_echonl)
#ifdef ECHOPRT
	IF_TERMIOS("echoprt",	&opt_echoprt)
#endif
	IF_OPENSSL("egd",	&opt_openssl_egd)
	IF_ANY    ("end-close",	&opt_end_close)
	IF_TERMIOS("eof",	&opt_veof)
	IF_TERMIOS("eol",	&opt_veol)
	IF_TERMIOS("eol2",	&opt_veol2)
	IF_TERMIOS("erase",	&opt_verase)
	IF_SOCKET ("error",	&opt_so_error)
	IF_ANY    ("escape",	&opt_escape)
	IF_OPEN   ("excl",	&opt_o_excl)
#if WITH_EXT2 && defined(EXT2_APPEND_FL)
	IF_ANY    ("ext2-append",	&opt_ext2_append)
#endif
#if WITH_EXT2 && defined(EXT2_COMPR_FL)
	IF_ANY    ("ext2-compr",	&opt_ext2_compr)
#endif
#if WITH_EXT2 && defined(EXT2_DIRSYNC_FL)
	IF_ANY    ("ext2-dirsync",	&opt_ext2_dirsync)
#endif
#if WITH_EXT2 && defined(EXT2_IMMUTABLE_FL)
	IF_ANY    ("ext2-immutable",	&opt_ext2_immutable)
#endif
#if WITH_EXT2 && defined(EXT2_JOURNAL_DATA_FL)
	IF_ANY    ("ext2-journal-data",	&opt_ext2_journal_data)
#endif
#if WITH_EXT2 && defined(EXT2_NOATIME_FL)
	IF_ANY    ("ext2-noatime",	&opt_ext2_noatime)
#endif
#if WITH_EXT2 && defined(EXT2_NODUMP_FL)
	IF_ANY    ("ext2-nodump",	&opt_ext2_nodump)
#endif
#if WITH_EXT2 && defined(EXT2_NOTAIL_FL)
	IF_ANY    ("ext2-notail",	&opt_ext2_notail)
#endif
#if WITH_EXT2 && defined(EXT2_SECRM_FL)
	IF_ANY    ("ext2-secrm",	&opt_ext2_secrm)
#endif
#if WITH_EXT2 && defined(EXT2_SYNC_FL)
	IF_ANY    ("ext2-sync",		&opt_ext2_sync)
#endif
#if WITH_EXT2 && defined(EXT2_TOPDIR_FL)
	IF_ANY    ("ext2-topdir",	&opt_ext2_topdir)
#endif
#if WITH_EXT2 && defined(EXT2_UNRM_FL)
	IF_ANY    ("ext2-unrm",		&opt_ext2_unrm)
#endif
#if WITH_EXT2 && defined(EXT2_APPEND_FL)
	IF_ANY    ("ext3-append",	&opt_ext2_append)
#endif
#if WITH_EXT2 && defined(EXT2_COMPR_FL)
	IF_ANY    ("ext3-compr",	&opt_ext2_compr)
#endif
#if WITH_EXT2 && defined(EXT2_DIRSYNC_FL)
	IF_ANY    ("ext3-dirsync",	&opt_ext2_dirsync)
#endif
#if WITH_EXT2 && defined(EXT2_IMMUTABLE_FL)
	IF_ANY    ("ext3-immutable",	&opt_ext2_immutable)
#endif
#if WITH_EXT2 && defined(EXT2_JOURNAL_DATA_FL)
	IF_ANY    ("ext3-journal-data",	&opt_ext2_journal_data)
#endif
#if WITH_EXT2 && defined(EXT2_NOATIME_FL)
	IF_ANY    ("ext3-noatime",	&opt_ext2_noatime)
#endif
#if WITH_EXT2 && defined(EXT2_NODUMP_FL)
	IF_ANY    ("ext3-nodump",	&opt_ext2_nodump)
#endif
#if WITH_EXT2 && defined(EXT2_NOTAIL_FL)
	IF_ANY    ("ext3-notail",	&opt_ext2_notail)
#endif
#if WITH_EXT2 && defined(EXT2_SECRM_FL)
	IF_ANY    ("ext3-secrm",	&opt_ext2_secrm)
#endif
#if WITH_EXT2 && defined(EXT2_SYNC_FL)
	IF_ANY    ("ext3-sync",		&opt_ext2_sync)
#endif
#if WITH_EXT2 && defined(EXT2_TOPDIR_FL)
	IF_ANY    ("ext3-topdir",	&opt_ext2_topdir)
#endif
#if WITH_EXT2 && defined(EXT2_UNRM_FL)
	IF_ANY    ("ext3-unrm",		&opt_ext2_unrm)
#endif
	IF_ANY 	  ("f-setlk",	&opt_f_setlk_wr)
	IF_ANY 	  ("f-setlk-rd",	&opt_f_setlk_rd)
	IF_ANY 	  ("f-setlk-wr",	&opt_f_setlk_wr)
	IF_ANY 	  ("f-setlkw",	&opt_f_setlkw_wr)
	IF_ANY 	  ("f-setlkw-rd",	&opt_f_setlkw_rd)
	IF_ANY 	  ("f-setlkw-wr",	&opt_f_setlkw_wr)
	IF_EXEC   ("fdin",	&opt_fdin)
	IF_EXEC   ("fdout",	&opt_fdout)
#ifdef FFDLY
#  ifdef FF0
	IF_TERMIOS("ff0",	&opt_ff0)
#  endif
#  ifdef FF1
	IF_TERMIOS("ff1",	&opt_ff1)
#  endif
	IF_TERMIOS("ffdly",	&opt_ffdly)
#endif
#ifdef FIOSETOWN
	IF_SOCKET ("fiosetown",	&opt_fiosetown)
#endif
#if WITH_FIPS
	IF_OPENSSL("fips",	&opt_openssl_fips)
#endif
#if HAVE_FLOCK
	IF_ANY    ("flock",	&opt_flock_ex)
	IF_ANY    ("flock-ex",	&opt_flock_ex)
	IF_ANY    ("flock-ex-nb",	&opt_flock_ex_nb)
	IF_ANY    ("flock-nb",	&opt_flock_ex_nb)
	IF_ANY    ("flock-sh",	&opt_flock_sh)
	IF_ANY    ("flock-sh-nb",	&opt_flock_sh_nb)
#endif
#ifdef IPV4_FLOWINFO
	IF_IP6    ("flowinfo",	&opt_ipv6_flowinfo)
#endif
	IF_TERMIOS("flusho",	&opt_flusho)
	IF_RETRY  ("forever",	&opt_forever)
	IF_LISTEN ("fork",	&opt_fork)
#ifdef IP_FREEBIND
	IF_IP     ("freebind",	&opt_ip_freebind)
#endif
#if HAVE_FTRUNCATE64
	IF_ANY    ("ftruncate",	&opt_ftruncate64)
#else
	IF_ANY    ("ftruncate",	&opt_ftruncate32)
#endif
	IF_ANY    ("ftruncate32",	&opt_ftruncate32)
#if HAVE_FTRUNCATE64
	IF_ANY    ("ftruncate64",	&opt_ftruncate64)
#endif
	IF_ANY    ("gid",	&opt_group)
	IF_NAMED  ("gid-e",	&opt_group_early)
	IF_ANY    ("gid-l",	&opt_group_late)
	IF_ANY    ("group",	&opt_group)
	IF_NAMED  ("group-early",	&opt_group_early)
	IF_ANY    ("group-late",	&opt_group_late)
#ifdef IP_HDRINCL
	IF_IP     ("hdrincl",	&opt_ip_hdrincl)
#endif
	IF_READLINE("history",	&opt_history_file)
	IF_READLINE("history-file",	&opt_history_file)
#ifdef IPV6_HOPLIMIT
	IF_IP6    ("hoplimit",	&opt_ipv6_hoplimit)
#endif
#ifdef	IPV6_HOPOPTS
	IF_IP6    ("hopopts",	&opt_ipv6_hopopts)
#endif
#if WITH_LIBWRAP && defined(HAVE_HOSTS_ALLOW_TABLE)
	IF_IPAPP  ("hosts-allow",	&opt_tcpwrap_hosts_allow_table)
#endif
#if WITH_LIBWRAP && defined(HAVE_HOSTS_DENY_TABLE)
	IF_IPAPP  ("hosts-deny",	&opt_tcpwrap_hosts_deny_table)
#endif
	IF_TERMIOS("hup",	&opt_hupcl)
	IF_TERMIOS("hupcl",	&opt_hupcl)
#ifdef I_POP
	IF_ANY    ("i-pop-all",	&opt_streams_i_pop_all)
#endif
#ifdef I_PUSH
	IF_ANY    ("i-push",	&opt_streams_i_push)
#endif
	IF_TERMIOS("icanon",	&opt_icanon)
	IF_TERMIOS("icrnl",	&opt_icrnl)
	IF_TERMIOS("iexten",	&opt_iexten)
#ifdef SO_BINDTODEVICE
	IF_SOCKET ("if",	&opt_so_bindtodevice)
#endif
	IF_TUN    ("iff-allmulti",	&opt_iff_allmulti)
	IF_TUN    ("iff-automedia",	&opt_iff_automedia)
	IF_TUN    ("iff-broadcast",	&opt_iff_broadcast)
	IF_TUN    ("iff-debug",	&opt_iff_debug)
	/*IF_TUN    ("iff-dynamic",	&opt_iff_dynamic)*/
	IF_TUN    ("iff-loopback",	&opt_iff_loopback)
	IF_TUN    ("iff-master",	&opt_iff_master)
	IF_TUN    ("iff-multicast",	&opt_iff_multicast)
	IF_TUN    ("iff-no-pi",	&opt_iff_no_pi)
	IF_TUN    ("iff-noarp",	&opt_iff_noarp)
	IF_TUN    ("iff-notrailers",	&opt_iff_notrailers)
	IF_TUN    ("iff-pointopoint",	&opt_iff_pointopoint)
	IF_TUN    ("iff-portsel",	&opt_iff_portsel)
	IF_TUN    ("iff-promisc",	&opt_iff_promisc)
	IF_TUN    ("iff-running",	&opt_iff_running)
	IF_TUN    ("iff-slave",	&opt_iff_slave)
	IF_TUN    ("iff-up",	&opt_iff_up)
	IF_TERMIOS("ignbrk",	&opt_ignbrk)
	IF_TERMIOS("igncr",	&opt_igncr)
  /* you might need to terminate socat manually if you use this option: */
	IF_PROXY  ("ignorecr",	&opt_ignorecr)
	IF_ANY    ("ignoreeof",	&opt_ignoreeof)
	IF_ANY    ("ignoreof",	&opt_ignoreeof)
	IF_TERMIOS("ignpar",	&opt_ignpar)
#if HAVE_RESOLV_H
	IF_IP     ("igntc",	&opt_res_igntc)
#endif /* HAVE_RESOLV_H */
	IF_TERMIOS("imaxbel",	&opt_imaxbel)
#if WITH_EXT2 && defined(EXT2_IMMUTABLE_FL)
	IF_ANY    ("immutable",	&opt_ext2_immutable)
#endif
#ifdef TCP_INFO	/* Linux 2.4.0 */
	IF_TCP    ("info",	&opt_tcp_info)
#endif
	IF_TERMIOS("inlcr",	&opt_inlcr)
	IF_TERMIOS("inpck",	&opt_inpck)
#ifdef SO_BINDTODEVICE
	IF_SOCKET ("interface",	&opt_so_bindtodevice)
#endif
	IF_RETRY  ("interval",	&opt_intervall)
	IF_RETRY  ("intervall",	&opt_intervall)
	IF_TERMIOS("intr",	&opt_vintr)
	IF_ANY    ("ioctl",	&opt_ioctl_void)
	IF_ANY    ("ioctl-bin",	&opt_ioctl_bin)
	IF_ANY    ("ioctl-int",	&opt_ioctl_int)
	IF_ANY    ("ioctl-intp",	&opt_ioctl_intp)
	IF_ANY    ("ioctl-string",	&opt_ioctl_string)
	IF_ANY    ("ioctl-void",	&opt_ioctl_void)
#ifdef IP_ADD_MEMBERSHIP
	IF_IP     ("ip-add-membership",	&opt_ip_add_membership)
#endif
#ifdef IP_FREEBIND
	IF_IP     ("ip-freebind",	&opt_ip_freebind)
#endif
#ifdef IP_HDRINCL
	IF_IP     ("ip-hdrincl",	&opt_ip_hdrincl)
#endif
#ifdef IP_ADD_MEMBERSHIP
	IF_IP     ("ip-membership",	&opt_ip_add_membership)
#endif
#ifdef IP_MTU
	IF_IP     ("ip-mtu",	&opt_ip_mtu)
#endif
#ifdef IP_MTU_DISCOVER
	IF_IP     ("ip-mtu-discover",	&opt_ip_mtu_discover)
#endif
	IF_IP     ("ip-multicast-if",	&opt_ip_multicast_if)
	IF_IP     ("ip-multicast-loop",	&opt_ip_multicast_loop)
	IF_IP     ("ip-multicast-ttl",	&opt_ip_multicast_ttl)
#ifdef IP_OPTIONS
	IF_IP     ("ip-options",	&opt_ip_options)
#endif
#ifdef IP_PKTINFO
	IF_IP     ("ip-pktinfo",	&opt_ip_pktinfo)
#endif
#ifdef IP_PKTOPTIONS
	IF_IP     ("ip-pktoptions",	&opt_ip_pktoptions)
#endif
#ifdef IP_RECVDSTADDR
	IF_IP     ("ip-recvdstaddr",	&opt_ip_recvdstaddr)
#endif
#ifdef IP_RECVERR
	IF_IP     ("ip-recverr",	&opt_ip_recverr)
#endif
#ifdef IP_RECVIF
	IF_IP     ("ip-recvif",		&opt_ip_recvif)
#endif
#ifdef IP_RECVOPTS
	IF_IP     ("ip-recvopts",	&opt_ip_recvopts)
#endif
#ifdef IP_RECVTOS
	IF_IP     ("ip-recvtos",	&opt_ip_recvtos)
#endif
#ifdef IP_RECVTTL
	IF_IP     ("ip-recvttl",	&opt_ip_recvttl)
#endif
#ifdef IP_RETOPTS
	IF_IP     ("ip-retopts",	&opt_ip_retopts)
#endif
#ifdef IP_ROUTER_ALERT
	IF_IP     ("ip-router-alert",	&opt_ip_router_alert)
#endif
	IF_IP     ("ip-tos",	&opt_ip_tos)
	IF_IP     ("ip-ttl",	&opt_ip_ttl)
#ifdef IP_FREEBIND
	IF_IP     ("ipfreebind",	&opt_ip_freebind)
#endif
#ifdef IP_HDRINCL
	IF_IP     ("iphdrincl",	&opt_ip_hdrincl)
#endif
#ifdef IP_MTU
	IF_IP     ("ipmtu",	&opt_ip_mtu)
#endif
#ifdef IP_MTU_DISCOVER
	IF_IP     ("ipmtudiscover",	&opt_ip_mtu_discover)
#endif
	IF_IP     ("ipmulticastloop",	&opt_ip_multicast_loop)
	IF_IP     ("ipmulticastttl",	&opt_ip_multicast_ttl)
#ifdef IP_OPTIONS
	IF_IP     ("ipoptions",	&opt_ip_options)
#endif
#ifdef IP_PKTINFO
	IF_IP     ("ippktinfo",	&opt_ip_pktinfo)
#endif
#ifdef IP_PKTOPTIONS
	IF_IP     ("ippktoptions",	&opt_ip_pktoptions)
#endif
#ifdef IP_RECVDSTADDR
	IF_IP     ("iprecvdstaddr",	&opt_ip_recvdstaddr)
#endif
#ifdef IP_RECVERR
	IF_IP     ("iprecverr",	&opt_ip_recverr)
#endif
#ifdef IP_RECVOPTS
	IF_IP     ("iprecvopts",	&opt_ip_recvopts)
#endif
#ifdef IP_RECVTOS
	IF_IP     ("iprecvtos",	&opt_ip_recvtos)
#endif
#ifdef IP_RECVTTL
	IF_IP     ("iprecvttl",	&opt_ip_recvttl)
#endif
#ifdef IP_RETOPTS
	IF_IP     ("ipretopts",	&opt_ip_retopts)
#endif
#ifdef IP_ROUTER_ALERT
	IF_IP     ("iprouteralert",	&opt_ip_router_alert)
#endif
	IF_IP     ("iptos",	&opt_ip_tos)
	IF_IP     ("ipttl",	&opt_ip_ttl)
#ifdef IPV6_JOIN_GROUP
	IF_IP6    ("ipv6-add-membership",	&opt_ipv6_join_group)
#endif
#ifdef IPV6_AUTHHDR
	IF_IP6    ("ipv6-authhdr",	&opt_ipv6_authhdr)
#endif
#ifdef IPV6_DSTOPTS
	IF_IP6    ("ipv6-dstopts",	&opt_ipv6_dstopts)
#endif
#ifdef IPV4_FLOWINFO
	IF_IP6    ("ipv6-flowinfo",	&opt_ipv6_flowinfo)
#endif
#ifdef IPV6_HOPLIMIT
	IF_IP6    ("ipv6-hoplimit",	&opt_ipv6_hoplimit)
#endif
#ifdef IPV6_HOPOPTS
	IF_IP6    ("ipv6-hopopts",	&opt_ipv6_hopopts)
#endif
#ifdef IPV6_JOIN_GROUP
	IF_IP6    ("ipv6-join-group",	&opt_ipv6_join_group)
#endif
#ifdef IPV6_PKTINFO
	IF_IP6    ("ipv6-pktinfo",	&opt_ipv6_pktinfo)
#endif
#ifdef IPV6_RECVDSTOPTS
	IF_IP6    ("ipv6-recvdstopts",	&opt_ipv6_recvdstopts)
#endif
#ifdef IPV6_RECVERR
	IF_IP6    ("ipv6-recverr",	&opt_ipv6_recverr)
#endif
#ifdef IPV6_RECVHOPLIMIT
	IF_IP6    ("ipv6-recvhoplimit",	&opt_ipv6_recvhoplimit)
#endif
#ifdef IPV6_RECVHOPOPTS
	IF_IP6    ("ipv6-recvhopopts",	&opt_ipv6_recvhopopts)
#endif
#ifdef IPV6_PATHMTU
	IF_IP6    ("ipv6-recvpathmtu",	&opt_ipv6_recvpathmtu)
#endif
#ifdef IPV6_RECVPKTINFO
	IF_IP6    ("ipv6-recvpktinfo",	&opt_ipv6_recvpktinfo)
#endif
#ifdef IPV6_RECVRTHDR
	IF_IP6    ("ipv6-recvrthdr",	&opt_ipv6_recvrthdr)
#endif
#ifdef IPV6_RECVTCLASS
	IF_IP6    ("ipv6-recvtclass",	&opt_ipv6_recvtclass)
#endif
#ifdef IPV6_RTHDR
	IF_IP6    ("ipv6-rthdr",	&opt_ipv6_rthdr)
#endif
#ifdef IPV6_TCLASS
	IF_IP6    ("ipv6-tclass",	&opt_ipv6_tclass)
#endif
	IF_IP6    ("ipv6-unicast-hops",	&opt_ipv6_unicast_hops)
#ifdef IPV6_V6ONLY
	IF_IP6    ("ipv6-v6only",	&opt_ipv6_v6only)
	IF_IP6    ("ipv6only",	&opt_ipv6_v6only)
#endif
	IF_TERMIOS("isig",	&opt_isig)
#if defined(HAVE_TERMIOS_ISPEED) && defined(ISPEED_OFFSET) && (ISPEED_OFFSET != -1)
	IF_TERMIOS("ispeed",	&opt_ispeed)
#endif
	IF_TERMIOS("istrip",	&opt_istrip)
#ifdef IUCLC
	IF_TERMIOS("iuclc",	&opt_iuclc)
#endif
	IF_TERMIOS("ixany",	&opt_ixany)
	IF_TERMIOS("ixoff",	&opt_ixoff)
	IF_TERMIOS("ixon",	&opt_ixon)
#ifdef IPV6_JOIN_GROUP
	IF_IP6    ("join-group",	&opt_ipv6_join_group)
#endif
#if WITH_EXT2 && defined(EXT2_JOURNAL_DATA_FL)
	IF_ANY    ("journal",		&opt_ext2_journal_data)
	IF_ANY    ("journal-data",	&opt_ext2_journal_data)
#endif
	IF_SOCKET ("keepalive",	&opt_so_keepalive)
#ifdef TCP_KEEPCNT	/* Linux 2.4.0 */
	IF_TCP    ("keepcnt",	&opt_tcp_keepcnt)
#endif
#ifdef TCP_KEEPIDLE	/* Linux 2.4.0 */
	IF_TCP    ("keepidle",	&opt_tcp_keepidle)
#endif
#ifdef TCP_KEEPINIT	/* OSF1 */
	IF_TCP    ("keepinit",	&opt_tcp_keepinit)
#endif
#ifdef TCP_KEEPINTVL	/* Linux 2.4.0 */
	IF_TCP    ("keepintvl",	&opt_tcp_keepintvl)
#endif
#ifdef SO_KERNACCEPT	/* AIX 4.3.3 */
	IF_SOCKET ("kernaccept",	&opt_so_kernaccept)
#endif /* SO_KERNACCEPT */
	IF_OPENSSL("key",	&opt_openssl_key)
	IF_TERMIOS("kill",	&opt_vkill)
#ifdef O_LARGEFILE
	IF_OPEN   ("largefile",	&opt_o_largefile)
#endif
#if WITH_LIBWRAP
	IF_IPAPP  ("libwrap",		&opt_tcpwrappers)
#endif
	IF_SOCKET ("linger",	&opt_so_linger)
#ifdef TCP_LINGER2	/* Linux 2.4.0 */
	IF_TCP    ("linger2",	&opt_tcp_linger2)
#endif
	IF_PTY    ("link",	&opt_symbolic_link)
	IF_TERMIOS("lnext",	&opt_vlnext)
#if defined(F_SETLKW)
	IF_ANY    ("lock",	&opt_f_setlkw_wr)	/* POSIX, first choice */
#elif defined(HAVE_FLOCK)
	IF_ANY    ("lock",	&opt_flock_ex)	/* BSD, fallback */
#endif
	IF_ANY    ("lockfile",	&opt_lockfile)
#if defined(F_SETLKW)
	IF_ANY    ("lockw",	&opt_f_setlkw_wr)	/* POSIX, first choice */
#elif defined(HAVE_FLOCK)
	IF_ANY    ("lockw",	&opt_flock_ex_nb)	/* BSD, fallback */
#endif
	IF_EXEC   ("login",	&opt_dash)
	IF_TUN    ("loopback",	&opt_iff_loopback)
	IF_IPAPP  ("lowport",	&opt_lowport)
#if HAVE_LSEEK64
	IF_ANY    ("lseek",	&opt_lseek64_set)
#else
	IF_ANY    ("lseek",	&opt_lseek32_set)
#endif
	IF_ANY    ("lseek32",		&opt_lseek32_set)
	IF_ANY    ("lseek32-cur",	&opt_lseek32_cur)
	IF_ANY    ("lseek32-end",	&opt_lseek32_end)
	IF_ANY    ("lseek32-set",	&opt_lseek32_set)
#if HAVE_LSEEK64
	IF_ANY    ("lseek64",		&opt_lseek64_set)
	IF_ANY    ("lseek64-cur",	&opt_lseek64_cur)
	IF_ANY    ("lseek64-end",	&opt_lseek64_end)
	IF_ANY    ("lseek64-set",	&opt_lseek64_set)
#endif
	IF_TUN    ("master",	&opt_iff_master)
	IF_LISTEN ("max-children",	&opt_max_children)
	IF_LISTEN ("maxchildren",	&opt_max_children)
#ifdef TCP_MAXSEG
	IF_TCP    ("maxseg",	&opt_tcp_maxseg)
	IF_TCP    ("maxseg-late",	&opt_tcp_maxseg_late)
#endif
#ifdef TCP_MD5SUM
	IF_TCP    ("md5sig",	&opt_tcp_md5sig)
#endif
#ifdef IP_ADD_MEMBERSHIP
	IF_IP     ("membership",	&opt_ip_add_membership)
#endif
	IF_OPENSSL("method",	&opt_openssl_method)
	IF_TERMIOS("min",	&opt_vmin)
	IF_ANY    ("mode",	&opt_perm)
#ifdef TCP_MAXSEG
	IF_TCP    ("mss",	&opt_tcp_maxseg)
	IF_TCP    ("mss-late",	&opt_tcp_maxseg_late)
#endif
#ifdef IP_MTU
	IF_IP     ("mtu",	&opt_ip_mtu)
#endif
#ifdef IP_MTU_DISCOVER
	IF_IP     ("mtudiscover",	&opt_ip_mtu_discover)
#endif
	IF_TUN    ("multicast",	&opt_iff_multicast)
	IF_IP     ("multicast-if",	&opt_ip_multicast_if)
	IF_IP     ("multicast-loop",	&opt_ip_multicast_loop)
	IF_IP     ("multicast-ttl",	&opt_ip_multicast_ttl)
	IF_IP     ("multicastloop",	&opt_ip_multicast_loop)
	IF_IP     ("multicastttl",	&opt_ip_multicast_ttl)
#if defined(O_NDELAY) && (!defined(O_NONBLOCK) || O_NDELAY != O_NONBLOCK)
	IF_ANY    ("ndelay",	&opt_o_ndelay)
#else
	IF_ANY    ("ndelay",	&opt_nonblock)
#endif
	IF_NAMED  ("new",	&opt_unlink_early)
#ifdef NLDLY
#  ifdef NL0
	IF_TERMIOS("nl0",	&opt_nl0)
#  endif
#  ifdef NL1
	IF_TERMIOS("nl1",	&opt_nl1)
#  endif
	IF_TERMIOS("nldly",	&opt_nldly)
#endif /* defined(NLDLY) */
#ifdef SO_NO_CHECK
	IF_SOCKET ("no-check",	&opt_so_no_check)
#endif
	IF_TUN    ("no-pi",	&opt_iff_no_pi)
	IF_TUN    ("noarp",	&opt_iff_noarp)
#ifdef O_NOATIME
	IF_OPEN   ("noatime",	&opt_o_noatime)
#endif
#ifdef SO_NO_CHECK
	IF_SOCKET ("nocheck",	&opt_so_no_check)
#endif
	IF_OPEN   ("noctty",	&opt_o_noctty)
#ifdef TCP_NODELAY
	IF_TCP    ("nodelay",	&opt_tcp_nodelay)
#endif
#if WITH_EXT2 && defined(EXT2_NODUMP_FL)
	IF_ANY    ("nodump",	&opt_ext2_nodump)
#endif
#if HAVE_REGEX_H
	IF_READLINE("noecho",	&opt_noecho)
#endif /* HAVE_REGEX_H */
	IF_TERMIOS("noflsh",	&opt_noflsh)
#ifdef O_NOFOLLOW
	IF_OPEN   ("nofollow",	&opt_o_nofollow)
#endif
	IF_EXEC   ("nofork",	&opt_nofork) 
#ifdef O_NOINHERIT
	IF_ANY    ("noinherit",		&opt_o_noinherit)
#endif
	IF_ANY    ("nonblock",	&opt_nonblock)
#ifdef TCP_NOOPT
	IF_TCP    ("noopt",		&opt_tcp_noopt)
#endif
	IF_READLINE("noprompt",	&opt_noprompt)
#ifdef TCP_NOPUSH
	IF_TCP    ("nopush",	&opt_tcp_nopush)
#endif
#ifdef SO_NOREUSEADDR	/* AIX 4.3.3 */
	IF_SOCKET ("noreuseaddr",	&opt_so_noreuseaddr)
#endif /* SO_NOREUSEADDR */
	IF_TUN    ("notrailers",	&opt_iff_notrailers)
#ifdef O_NSHARE
	IF_OPEN   ("nshare",	&opt_o_nshare)
#endif
	IF_SOCKET ("null-eof",		&opt_null_eof)
	IF_ANY    ("o-append",		&opt_append)
#ifdef O_ASYNC
	IF_ANY    ("o-async",		&opt_async)
#endif
#ifdef O_BINARY
	IF_OPEN   ("o-binary",		&opt_o_binary)
#endif
	IF_OPEN   ("o-creat",	&opt_o_create)
	IF_OPEN   ("o-create",	&opt_o_create)
#ifdef O_DEFER
	IF_OPEN   ("o-defer",	&opt_o_defer)
#endif
#ifdef O_DELAY
	IF_OPEN   ("o-delay",	&opt_o_delay)
#endif
#ifdef O_DIRECT
	IF_OPEN   ("o-direct",	&opt_o_direct)
#endif
#ifdef O_DIRECTORY
	IF_OPEN   ("o-directory",	&opt_o_directory)
#endif
#ifdef O_DSYNC
	IF_OPEN   ("o-dsync",	&opt_o_dsync)
#endif
	IF_OPEN   ("o-excl",	&opt_o_excl)
#ifdef O_LARGEFILE
	IF_OPEN   ("o-largefile",	&opt_o_largefile)
#endif
#if defined(O_NDELAY) && (!defined(O_NONBLOCK) || O_NDELAY != O_NONBLOCK)
	IF_ANY    ("o-ndelay",	&opt_o_ndelay)
#else
	IF_ANY    ("o-ndelay",	&opt_nonblock)
#endif
#ifdef O_NOATIME
	IF_OPEN   ("o-noatime",	&opt_o_noatime)
#endif
	IF_OPEN   ("o-noctty",	&opt_o_noctty)
#ifdef O_NOFOLLOW
	IF_OPEN   ("o-nofollow",	&opt_o_nofollow)
#endif
#ifdef O_NOINHERIT
	IF_ANY    ("o-noinherit",	&opt_o_noinherit)
#endif
	IF_ANY    ("o-nonblock",	&opt_nonblock)
#ifdef O_NSHARE
	IF_OPEN   ("o-nshare",	&opt_o_nshare)
#endif
#ifdef O_PRIV
	IF_OPEN   ("o-priv",	&opt_o_priv)
#endif
	IF_OPEN   ("o-rdonly",	&opt_o_rdonly)
	IF_OPEN   ("o-rdwr",	&opt_o_rdwr)
#ifdef O_RSHARE
	IF_OPEN   ("o-rshare",	&opt_o_rshare)
#endif
#ifdef O_RSYNC
	IF_OPEN   ("o-rsync",	&opt_o_rsync)
#endif
#ifdef O_SYNC
	IF_OPEN   ("o-sync",	&opt_o_sync)
#endif
#ifdef O_TEXT
	IF_ANY    ("o-text",		&opt_o_text)
#endif
	IF_OPEN   ("o-trunc",	&opt_o_trunc)
	IF_OPEN   ("o-wronly",	&opt_o_wronly)
	IF_OPEN   ("o_create",	&opt_o_create)
#ifdef O_DEFER
	IF_OPEN   ("o_defer",	&opt_o_defer)
#endif
#ifdef O_DELAY
	IF_OPEN   ("o_delay",	&opt_o_delay)
#endif
#ifdef O_DIRECT
	IF_OPEN   ("o_direct",	&opt_o_direct)
#endif
#ifdef O_DIRECTORY
	IF_OPEN   ("o_directory",	&opt_o_directory)
#endif
#ifdef O_DSYNC
	IF_OPEN   ("o_dsync",	&opt_o_dsync)
#endif
	IF_OPEN   ("o_excl",	&opt_o_excl)
#ifdef O_LARGEFILE
	IF_OPEN   ("o_largefile",	&opt_o_largefile)
#endif
#if defined(O_NDELAY) && (!defined(O_NONBLOCK) || O_NDELAY != O_NONBLOCK)
	IF_ANY    ("o_ndelay",	&opt_o_ndelay)
#else
	IF_ANY    ("o_ndelay",	&opt_nonblock)
#endif
	IF_OPEN   ("o_noctty",	&opt_o_noctty)
#ifdef O_NOFOLLOW
	IF_OPEN   ("o_nofollow",	&opt_o_nofollow)
#endif
#ifdef O_NSHARE
	IF_OPEN   ("o_nshare",	&opt_o_nshare)
#endif
#ifdef O_PRIV
	IF_OPEN   ("o_priv",	&opt_o_priv)
#endif
	IF_OPEN   ("o_rdonly",	&opt_o_rdonly)
	IF_OPEN   ("o_rdwr",	&opt_o_rdwr)
#ifdef O_RSHARE
	IF_OPEN   ("o_rshare",	&opt_o_rshare)
#endif
#ifdef O_RSYNC
	IF_OPEN   ("o_rsync",	&opt_o_rsync)
#endif
#ifdef O_SYNC
	IF_OPEN   ("o_sync",	&opt_o_sync)
#endif
	IF_OPEN   ("o_wronly",	&opt_o_wronly)
#ifdef OCRNL
	IF_TERMIOS("ocrnl",	&opt_ocrnl)
#endif
#ifdef OFDEL
	IF_TERMIOS("ofdel",	&opt_ofdel)
#endif
#ifdef OFILL
	IF_TERMIOS("ofill",	&opt_ofill)
#endif
#ifdef OLCUC
	IF_TERMIOS("olcuc",	&opt_olcuc)
#endif
	IF_TERMIOS("onlcr",	&opt_onlcr)
#ifdef ONLRET
	IF_TERMIOS("onlret",	&opt_onlret)
#endif
#ifdef ONOCR
	IF_TERMIOS("onocr",	&opt_onocr)
#endif
	IF_SOCKET ("oobinline",	&opt_so_oobinline)
#if HAVE_OPENPTY
	IF_EXEC   ("openpty",	&opt_openpty)
#endif /* HAVE_OPENPTY */
	IF_OPENSSL("openssl-cafile",	&opt_openssl_cafile)
	IF_OPENSSL("openssl-capath",	&opt_openssl_capath)
	IF_OPENSSL("openssl-certificate",	&opt_openssl_certificate)
	IF_OPENSSL("openssl-cipherlist",	&opt_openssl_cipherlist)
	IF_OPENSSL("openssl-commonname",	&opt_openssl_commonname)
#if OPENSSL_VERSION_NUMBER >= 0x00908000L && !defined(OPENSSL_NO_COMP)
	IF_OPENSSL("openssl-compress",	&opt_openssl_compress)
#endif
	IF_OPENSSL("openssl-dhparam",	&opt_openssl_dhparam)
	IF_OPENSSL("openssl-dhparams",	&opt_openssl_dhparam)
	IF_OPENSSL("openssl-egd",	&opt_openssl_egd)
#if WITH_FIPS
	IF_OPENSSL("openssl-fips",	&opt_openssl_fips)
#endif
	IF_OPENSSL("openssl-key",	&opt_openssl_key)
	IF_OPENSSL("openssl-method",	&opt_openssl_method)
	IF_OPENSSL("openssl-pseudo",	&opt_openssl_pseudo)
	IF_OPENSSL("openssl-verify",	&opt_openssl_verify)
	IF_TERMIOS("opost",	&opt_opost)
#if defined(HAVE_TERMIOS_ISPEED) && defined(OSPEED_OFFSET) && (OSPEED_OFFSET != -1)
	IF_TERMIOS("ospeed",	&opt_ospeed)
#endif
	IF_ANY    ("owner",	&opt_user)
	IF_TERMIOS("parenb",	&opt_parenb)
	IF_TERMIOS("parmrk",	&opt_parmrk)
	IF_TERMIOS("parodd",	&opt_parodd)
#ifdef SO_PASSCRED
	IF_SOCKET ("passcred",	&opt_so_passcred)
#endif
	IF_EXEC   ("path",	&opt_path)
#ifdef TCP_PAWS	/* OSF1 */
	IF_TCP    ("paws",	&opt_tcp_paws)
#endif
#ifdef SO_PEERCRED
	IF_SOCKET ("peercred",	&opt_so_peercred)
#endif
#ifdef PENDIN
	IF_TERMIOS("pendin",	&opt_pendin)
#endif
	IF_ANY    ("perm",	&opt_perm)
	IF_NAMED  ("perm-early",	&opt_perm_early)
	IF_ANY    ("perm-late",	&opt_perm_late)
	IF_SOCKET ("pf",	&opt_protocol_family)
	IF_EXEC   ("pgid",	&opt_setpgid)
	IF_EXEC   ("pipes",	&opt_pipes)
#ifdef IP_PKTINFO
	IF_IP     ("pktinfo",	&opt_ip_pktinfo)
#endif
#ifdef IP_PKTOPTIONS
	IF_IP     ("pktoptions",	&opt_ip_pktoptions)
	IF_IP     ("pktopts",	&opt_ip_pktoptions)
#endif
	IF_TUN    ("pointopoint",	&opt_iff_pointopoint)
#ifdef I_POP
	IF_ANY    ("pop-all",	&opt_streams_i_pop_all)
#endif
	/*IF_IPAPP("port",	&opt_port)*/
	IF_TUN    ("portsel",	&opt_iff_portsel)
#if HAVE_RESOLV_H
	IF_IP     ("primary",	&opt_res_primary)
#endif /* HAVE_RESOLV_H */
#ifdef SO_PRIORITY
	IF_SOCKET ("priority",	&opt_so_priority)
#endif
#ifdef O_PRIV
	IF_OPEN   ("priv",	&opt_o_priv)
#endif
	IF_TUN    ("promisc",	&opt_iff_promisc)
	IF_READLINE("prompt",	&opt_prompt)
#ifdef SO_PROTOTYPE
	IF_SOCKET ("protocol",	&opt_so_prototype)
#endif
	IF_SOCKET ("protocol-family",	&opt_protocol_family)
#ifdef SO_PROTOTYPE
	IF_SOCKET ("prototype",	&opt_so_prototype)
#endif
	IF_PROXY  ("proxy-auth",	&opt_proxy_authorization)
	IF_PROXY  ("proxy-authorization",	&opt_proxy_authorization)
	IF_PROXY  ("proxy-resolve",	&opt_proxy_resolve)
	IF_PROXY  ("proxyauth",	&opt_proxy_authorization)
	IF_PROXY  ("proxyport",	&opt_proxyport)
#ifdef ECHOPRT
	IF_TERMIOS("prterase",	&opt_echoprt)
#endif
	IF_OPENSSL("pseudo",	&opt_openssl_pseudo)
#if HAVE_DEV_PTMX || HAVE_DEV_PTC
	IF_EXEC   ("ptmx",	&opt_ptmx)
#endif
#if HAVE_PTY
	IF_EXEC   ("pty",	&opt_pty)
#endif
#if HAVE_PTY && HAVE_POLL
	IF_PTY    ("pty-interval",	&opt_pty_intervall)
	IF_PTY    ("pty-intervall",	&opt_pty_intervall)
	IF_PTY    ("pty-wait-slave",	&opt_pty_wait_slave)
#endif /* HAVE_PTY && HAVE_POLL */
#ifdef I_PUSH
	IF_ANY    ("push",	&opt_streams_i_push)
#endif
#ifdef TCP_QUICKACK
	IF_TCP    ("quickack",	&opt_tcp_quickack)
#endif
	IF_TERMIOS("quit",	&opt_vquit)
	IF_RANGE  ("range",	&opt_range)
	IF_TERMIOS("raw",	&opt_raw)
	IF_TERMIOS("rawer",	&opt_termios_rawer)
	IF_SOCKET ("rcvbuf",	&opt_so_rcvbuf)
	IF_SOCKET ("rcvbuf-late",	&opt_so_rcvbuf_late)
#ifdef SO_RCVLOWAT
	IF_SOCKET ("rcvlowat",	&opt_so_rcvlowat)
#endif
	IF_OPEN   ("rdonly",	&opt_o_rdonly)
	IF_OPEN   ("rdwr",	&opt_o_rdwr)
	IF_ANY    ("readbytes", &opt_readbytes)
#if HAVE_RESOLV_H
	IF_IP     ("recurse",	&opt_res_recurse)
#endif /* HAVE_RESOLV_H */
#ifdef IP_RECVDSTADDR
	IF_IP     ("recvdstaddr",	&opt_ip_recvdstaddr)
#endif
#ifdef IPV6_RECVDSTOPTS
	IF_IP6    ("recvdstopts",	&opt_ipv6_recvdstopts)
#endif
#ifdef IP_RECVERR
	IF_IP     ("recverr",	&opt_ip_recverr)
#endif
#ifdef IPV6_RECVHOPLIMIT
	IF_IP6    ("recvhoplimit",	&opt_ipv6_recvhoplimit)
#endif
#ifdef IPV6_RECVHOPOPTS
	IF_IP6    ("recvhopopts",	&opt_ipv6_recvhopopts)
#endif
#ifdef IP_RECVIF
	IF_IP     ("recvif",		&opt_ip_recvif)
#endif
#ifdef IP_RECVOPTS
	IF_IP     ("recvopts",	&opt_ip_recvopts)
#endif
#ifdef IPV6_RECVPKTINFO
	IF_IP6    ("recvpktinfo",	&opt_ipv6_recvpktinfo)
#endif
#ifdef IPV6_RECVRTHDR
	IF_IP6    ("recvrthdr",	&opt_ipv6_recvrthdr)
#endif
#ifdef IP_RECVTOS
	IF_IP     ("recvtos",	&opt_ip_recvtos)
#endif
#ifdef IP_RECVTTL
	IF_IP     ("recvttl",	&opt_ip_recvttl)
#endif
	IF_NAMED  ("remove",	&opt_unlink)
#ifdef VREPRINT
	IF_TERMIOS("reprint",	&opt_vreprint)
#endif
#if HAVE_RESOLV_H
	IF_IP     ("res-aaonly",	&opt_res_aaonly)
	IF_IP     ("res-debug",	&opt_res_debug)
	IF_IP     ("res-defnames",	&opt_res_defnames)
	IF_IP     ("res-dnsrch",	&opt_res_dnsrch)
	IF_IP     ("res-igntc",	&opt_res_igntc)
	IF_IP     ("res-primary",	&opt_res_primary)
	IF_IP     ("res-recurse",	&opt_res_recurse)
	IF_IP     ("res-stayopen",	&opt_res_stayopen)
	IF_IP     ("res-usevc",	&opt_res_usevc)
#endif /* HAVE_RESOLV_H */
	IF_PROXY  ("resolv",	&opt_proxy_resolve)
	IF_PROXY  ("resolve",	&opt_proxy_resolve)
#ifdef IP_RETOPTS
	IF_IP     ("retopts",	&opt_ip_retopts)
#endif
	IF_RETRY  ("retry",	&opt_retry)
	IF_SOCKET ("reuseaddr",	&opt_so_reuseaddr)
#ifdef SO_REUSEPORT	/* AIX 4.3.3 */
	IF_SOCKET ("reuseport",	&opt_so_reuseport)
#endif /* defined(SO_REUSEPORT) */
#ifdef TCP_RFC1323
	IF_TCP    ("rfc1323",	&opt_tcp_rfc1323)
#endif
#ifdef IP_ROUTER_ALERT
	IF_IP     ("routeralert",	&opt_ip_router_alert)
#endif
#ifdef VREPRINT
	IF_TERMIOS("rprnt",	&opt_vreprint)
#endif
#ifdef O_RSHARE
	IF_OPEN   ("rshare",	&opt_o_rshare)
#endif
#ifdef O_RSYNC
	IF_OPEN   ("rsync",	&opt_o_rsync)
#endif
#ifdef IPV6_RTHDR
	IF_IP6    ("rthdr",	&opt_ipv6_rthdr)
#endif
	IF_TUN    ("running",	&opt_iff_running)
#ifdef TCP_SACK_DISABLE
	IF_TCP    ("sack-disable",	&opt_tcp_sack_disable)
#endif
#ifdef TCP_SACKENA	/* OSF1 */
	IF_TCP    ("sackena",	&opt_tcp_sackena)
#endif
	IF_TERMIOS("sane",	&opt_sane)
#ifdef SCTP_MAXSEG
	IF_SCTP   ("sctp-maxseg",	&opt_sctp_maxseg)
	IF_SCTP   ("sctp-maxseg-late",	&opt_sctp_maxseg_late)
#endif
#ifdef SCTP_NODELAY
	IF_SCTP   ("sctp-nodelay",	&opt_sctp_nodelay)
#endif
#if WITH_EXT2 && defined(EXT2_SECRM_FL)
	IF_ANY    ("secrm",	&opt_ext2_secrm)
#endif
#ifdef SO_SECURITY_AUTHENTICATION
	IF_SOCKET ("security-authentication",	&opt_so_security_authentication)
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
	IF_SOCKET ("security-encryption-network",	&opt_so_security_encryption_network)
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
	IF_SOCKET ("security-encryption-transport",	&opt_so_security_encryption_transport)
#endif
#ifdef SO_SECURITY_AUTHENTICATION
	IF_SOCKET ("securityauthentication",	&opt_so_security_authentication)
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
	IF_SOCKET ("securityencryptionnetwork",	&opt_so_security_encryption_network)
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
	IF_SOCKET ("securityencryptiontransport",	&opt_so_security_encryption_transport)
#endif
#if HAVE_LSEEK64
	IF_ANY    ("seek",		&opt_lseek64_set)
	IF_ANY    ("seek-cur",		&opt_lseek64_cur)
	IF_ANY    ("seek-end",		&opt_lseek64_end)
	IF_ANY    ("seek-set",		&opt_lseek64_set)
#else
	IF_ANY    ("seek",		&opt_lseek32_set)
	IF_ANY    ("seek-cur",		&opt_lseek32_cur)
	IF_ANY    ("seek-end",		&opt_lseek32_end)
	IF_ANY    ("seek-set",		&opt_lseek32_set)
#endif
	IF_ANY    ("setgid",	&opt_setgid)
	IF_ANY    ("setgid-early",	&opt_setgid_early)
	IF_ANY 	  ("setlk",	&opt_f_setlk_wr)
	IF_ANY 	  ("setlk-rd",	&opt_f_setlk_rd)
	IF_ANY 	  ("setlk-wr",	&opt_f_setlk_wr)
	IF_ANY 	  ("setlkw",	&opt_f_setlkw_wr)
	IF_ANY 	  ("setlkw-rd",	&opt_f_setlkw_rd)
	IF_ANY 	  ("setlkw-wr",	&opt_f_setlkw_wr)
	IF_EXEC   ("setpgid",	&opt_setpgid)
#if WITH_EXEC || WITH_SYSTEM
	IF_EXEC   ("setsid",	&opt_setsid)
#endif
	IF_SOCKET ("setsockopt-bin",	&opt_setsockopt_bin)
	IF_SOCKET ("setsockopt-int",	&opt_setsockopt_int)
	IF_SOCKET ("setsockopt-string",	&opt_setsockopt_string)
	IF_ANY    ("setuid",	&opt_setuid)
	IF_ANY    ("setuid-early",	&opt_setuid_early)
	IF_ANY    ("shut-close",	&opt_shut_close)
	IF_ANY    ("shut-down",	&opt_shut_down)
	IF_ANY    ("shut-none",	&opt_shut_none)
	IF_ANY    ("shut-null",		&opt_shut_null)
#if WITH_EXEC || WITH_SYSTEM
	IF_ANY    ("sid",	&opt_setsid)
#endif
	IF_EXEC   ("sighup",	&opt_sighup)
	IF_EXEC   ("sigint",	&opt_sigint)
#ifdef TCP_SIGNATURE_ENABLE
	IF_TCP    ("signature-enable",	&opt_tcp_signature_enable)
#endif
	IF_EXEC   ("sigquit",	&opt_sigquit)
#ifdef SIOCSPGRP
	IF_SOCKET ("siocspgrp",	&opt_siocspgrp)
#endif
	IF_TUN    ("slave",	&opt_iff_slave)
	IF_SOCKET ("sndbuf",	&opt_so_sndbuf)
	IF_SOCKET ("sndbuf-late",	&opt_so_sndbuf_late)
#ifdef SO_SNDLOWAT
	IF_SOCKET ("sndlowat",	&opt_so_sndlowat)
#endif
#ifdef SO_ACCEPTCONN /* AIX433 */
	IF_SOCKET ("so-acceptconn",	&opt_so_acceptconn)
#endif /* SO_ACCEPTCONN */
#ifdef SO_ATTACH_FILTER
	IF_SOCKET ("so-attach-filter",	&opt_so_attach_filter)
#endif
#ifdef SO_AUDIT	/* AIX 4.3.3 */
	IF_SOCKET ("so-audit",	&opt_so_audit)
#endif /* SO_AUDIT */
#ifdef SO_BINDTODEVICE
	IF_SOCKET ("so-bindtodevice",	&opt_so_bindtodevice)
#endif
	IF_SOCKET ("so-broadcast",	&opt_so_broadcast)
#ifdef SO_BSDCOMPAT
	IF_SOCKET ("so-bsdcompat",	&opt_so_bsdcompat)
#endif
#ifdef SO_CKSUMRECV
	IF_SOCKET ("so-cksumrecv",	&opt_so_cksumrecv)
#endif /* SO_CKSUMRECV */
	IF_SOCKET ("so-debug",	&opt_so_debug)
#ifdef SO_DETACH_FILTER
	IF_SOCKET ("so-detach-filter", &opt_so_detach_filter)
#endif
#ifdef SO_DGRAM_ERRIND
	IF_SOCKET ("so-dgram-errind",	&opt_so_dgram_errind)
#endif
#ifdef SO_DONTLINGER
	IF_SOCKET ("so-dontlinger",	&opt_so_dontlinger)
#endif
	IF_SOCKET ("so-dontroute",	&opt_so_dontroute)
	IF_SOCKET ("so-error",	&opt_so_error)
	IF_SOCKET ("so-keepalive",	&opt_so_keepalive)
#ifdef SO_KERNACCEPT	/* AIX 4.3.3 */
	IF_SOCKET ("so-kernaccept",	&opt_so_kernaccept)
#endif /* SO_KERNACCEPT */
	IF_SOCKET ("so-linger",	&opt_so_linger)
#ifdef SO_NO_CHECK
	IF_SOCKET ("so-no-check",	&opt_so_no_check)
#endif
#ifdef SO_NOREUSEADDR	/* AIX 4.3.3 */
	IF_SOCKET ("so-noreuseaddr",	&opt_so_noreuseaddr)
#endif /* SO_NOREUSEADDR */
	IF_SOCKET ("so-oobinline",	&opt_so_oobinline)
#ifdef SO_PASSCRED
	IF_SOCKET ("so-passcred",	&opt_so_passcred)
#endif
#ifdef SO_PEERCRED
	IF_SOCKET ("so-peercred",	&opt_so_peercred)
#endif
#ifdef SO_PRIORITY
	IF_SOCKET ("so-priority",	&opt_so_priority)
#endif
#ifdef SO_PROTOTYPE
	IF_SOCKET ("so-prototype",	&opt_so_prototype)
#endif
	IF_SOCKET ("so-rcvbuf",	&opt_so_rcvbuf)
	IF_SOCKET ("so-rcvbuf-late",	&opt_so_rcvbuf_late)
#ifdef SO_RCVLOWAT
	IF_SOCKET ("so-rcvlowat",	&opt_so_rcvlowat)
#endif
	IF_SOCKET ("so-reuseaddr",	&opt_so_reuseaddr)
#ifdef SO_REUSEPORT	/* AIX 4.3.3 */
	IF_SOCKET ("so-reuseport",	&opt_so_reuseport)
#endif /* defined(SO_REUSEPORT) */
#ifdef SO_SECURITY_AUTHENTICATION
	IF_SOCKET ("so-security-authentication",	&opt_so_security_authentication)
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
	IF_SOCKET ("so-security-encryption-network",	&opt_so_security_encryption_network)
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
	IF_SOCKET ("so-security-encryption-transport",	&opt_so_security_encryption_transport)
#endif
	IF_SOCKET ("so-sndbuf",	&opt_so_sndbuf)
	IF_SOCKET ("so-sndbuf-late",	&opt_so_sndbuf_late)
#ifdef SO_SNDLOWAT
	IF_SOCKET ("so-sndlowat",	&opt_so_sndlowat)
#endif
#ifdef SO_TIMESTAMP
	IF_SOCKET ("so-timestamp",	&opt_so_timestamp)
#endif
	IF_SOCKET ("so-type",	&opt_so_type)
#ifdef SO_USE_IFBUFS
	IF_SOCKET ("so-use-ifbufs",	&opt_so_use_ifbufs)
#endif /* SO_USE_IFBUFS */
#ifdef SO_USELOOPBACK /* AIX433, Solaris */
	IF_SOCKET ("so-useloopback",	&opt_so_useloopback)
#endif /* SO_USELOOPBACK */
	IF_SOCKET ("sockopt-bin",	&opt_setsockopt_bin)
	IF_SOCKET ("sockopt-int",	&opt_setsockopt_int)
	IF_SOCKET ("sockopt-string",	&opt_setsockopt_string)
	IF_SOCKS4 ("socksport",	&opt_socksport)
	IF_SOCKS4 ("socksuser",	&opt_socksuser)
	IF_SOCKET ("socktype",	&opt_so_type)
	IF_IPAPP  ("sourceport",	&opt_sourceport)
	IF_IPAPP  ("sp",	&opt_sourceport)
	IF_TERMIOS("start",	&opt_vstart)
#if HAVE_RESOLV_H
	IF_IP     ("stayopen",	&opt_res_stayopen)
#endif /* HAVE_RESOLV_H */
	IF_EXEC   ("stderr",    &opt_stderr)
#ifdef TCP_STDURG
	IF_TCP    ("stdurg",	&opt_tcp_stdurg)
#endif
	IF_TERMIOS("stop",	&opt_vstop)
#ifdef I_POP
	IF_ANY    ("streams-i-pop-all",	&opt_streams_i_pop_all)
#endif
#ifdef I_PUSH
	IF_ANY    ("streams-i-push",	&opt_streams_i_push)
#endif
	IF_ANY    ("su",	&opt_substuser)
#if defined(HAVE_SETGRENT) && defined(HAVE_GETGRENT) && defined(HAVE_ENDGRENT)
	IF_ANY    ("su-d",	&opt_substuser_delayed)
#endif
	IF_ANY    ("su-e",	&opt_substuser_early)
	IF_ANY    ("substuser",	&opt_substuser)
#if defined(HAVE_SETGRENT) && defined(HAVE_GETGRENT) && defined(HAVE_ENDGRENT)
	IF_ANY    ("substuser-delayed",	&opt_substuser_delayed)
#endif
	IF_ANY    ("substuser-early",	&opt_substuser_early)
	IF_TERMIOS("susp",	&opt_vsusp)
#ifdef VSWTC
	IF_TERMIOS("swtc",	&opt_vswtc)
	IF_TERMIOS("swtch",	&opt_vswtc)
#endif
	IF_PTY    ("symbolic-link",	&opt_symbolic_link)
#ifdef O_SYNC
	IF_OPEN   ("sync",	&opt_o_sync)
#elif EXT2_SYNC_FL
	IF_ANY    ("sync",	&opt_ext2_sync)
#endif
#ifdef TCP_SYNCNT
	IF_TCP    ("syncnt",	&opt_tcp_syncnt)
#endif
#ifdef TABDLY
#  ifdef TAB0
	IF_TERMIOS("tab0",	&opt_tab0)
#  endif
#  ifdef TAB1
	IF_TERMIOS("tab1",	&opt_tab1)
#  endif
#  ifdef TAB2
	IF_TERMIOS("tab2",	&opt_tab2)
#  endif
#  ifdef TAB3
	IF_TERMIOS("tab3",	&opt_tab3)
#  endif
#  if TABDLY_SHIFT >= 0
	IF_TERMIOS("tabdly",	&opt_tabdly)
#  endif
#endif
	IF_TERMIOS("tandem",	&opt_ixoff)
#ifdef TCP_ABORT_THRESHOLD  /* HP_UX */
	IF_TCP    ("tcp-abort-threshold",	&opt_tcp_abort_threshold)
#endif
#ifdef TCP_CONN_ABORT_THRESHOLD  /* HP_UX */
	IF_TCP    ("tcp-conn-abort-threshold",	&opt_tcp_conn_abort_threshold)
#endif
#ifdef TCP_CORK
	IF_TCP    ("tcp-cork",	&opt_tcp_cork)
#endif
#ifdef TCP_DEFER_ACCEPT	/* Linux 2.4.0 */
	IF_TCP    ("tcp-defer-accept",	&opt_tcp_defer_accept)
#endif
#ifdef TCP_INFO	/* Linux 2.4.0 */
	IF_TCP    ("tcp-info",	&opt_tcp_info)
#endif
#ifdef TCP_KEEPCNT	/* Linux 2.4.0 */
	IF_TCP    ("tcp-keepcnt",	&opt_tcp_keepcnt)
#endif
#ifdef TCP_KEEPIDLE	/* Linux 2.4.0 */
	IF_TCP    ("tcp-keepidle",	&opt_tcp_keepidle)
#endif
#ifdef TCP_KEEPINIT	/* OSF1 */
	IF_TCP    ("tcp-keepinit",	&opt_tcp_keepinit)
#endif
#ifdef TCP_KEEPINTVL	/* Linux 2.4.0 */
	IF_TCP    ("tcp-keepintvl",	&opt_tcp_keepintvl)
#endif
#ifdef TCP_LINGER2	/* Linux 2.4.0 */
	IF_TCP    ("tcp-linger2",	&opt_tcp_linger2)
#endif
#ifdef TCP_MAXSEG
	IF_TCP    ("tcp-maxseg",	&opt_tcp_maxseg)
	IF_TCP    ("tcp-maxseg-late",	&opt_tcp_maxseg_late)
#endif
#ifdef TCP_MD5SIG
	IF_TCP    ("tcp-md5sig",	&opt_tcp_md5sig)
#endif
#ifdef TCP_NODELAY
	IF_TCP    ("tcp-nodelay",	&opt_tcp_nodelay)
#endif
#ifdef TCP_NOOPT
	IF_TCP    ("tcp-noopt",		&opt_tcp_noopt)
#endif
#ifdef TCP_NOPUSH
	IF_TCP    ("tcp-nopush",	&opt_tcp_nopush)
#endif
#ifdef TCP_PAWS	/* OSF1 */
	IF_TCP    ("tcp-paws",		&opt_tcp_paws)
#endif
#ifdef TCP_QUICKACK
	IF_TCP    ("tcp-quickack",	&opt_tcp_quickack)
#endif
#ifdef TCP_RFC1323
	IF_TCP    ("tcp-rfc1323",	&opt_tcp_rfc1323)
#endif
#ifdef TCP_SACK_DISABLE
	IF_TCP    ("tcp-sack-disable",	&opt_tcp_sack_disable)
#endif
#ifdef TCP_SACKENA	/* OSF1 */
	IF_TCP    ("tcp-sackena",	&opt_tcp_sackena)
#endif
#ifdef TCP_SIGNATURE_ENABLE
	IF_TCP    ("tcp-signature-enable",	&opt_tcp_signature_enable)
#endif
#ifdef TCP_STDURG
	IF_TCP    ("tcp-stdurg",	&opt_tcp_stdurg)
#endif
#ifdef TCP_SYNCNT	/* Linux 2.4.0 */
	IF_TCP    ("tcp-syncnt",	&opt_tcp_syncnt)
#endif
#ifdef TCP_TSOPTENA	/* OSF1 */
	IF_TCP    ("tcp-tsoptena",	&opt_tcp_tsoptena)
#endif
#ifdef TCP_WINDOW_CLAMP	/* Linux 2.4.0 */
	IF_TCP    ("tcp-window-clamp",	&opt_tcp_window_clamp)
#endif
#if WITH_LIBWRAP
	IF_IPAPP  ("tcpwrap",		&opt_tcpwrappers)
	IF_IPAPP  ("tcpwrap-dir",	&opt_tcpwrap_etc)
	IF_IPAPP  ("tcpwrap-etc",	&opt_tcpwrap_etc)
#if WITH_LIBWRAP && defined(HAVE_HOSTS_ALLOW_TABLE)
	IF_IPAPP  ("tcpwrap-hosts-allow-table",	&opt_tcpwrap_hosts_allow_table)
#endif
#if WITH_LIBWRAP && defined(HAVE_HOSTS_DENY_TABLE)
	IF_IPAPP  ("tcpwrap-hosts-deny-table",	&opt_tcpwrap_hosts_deny_table)
#endif
	IF_IPAPP  ("tcpwrapper",	&opt_tcpwrappers)
	IF_IPAPP  ("tcpwrappers",	&opt_tcpwrappers)
#endif
	IF_TERMIOS("termios-cfmakeraw",	&opt_termios_cfmakeraw)
	IF_TERMIOS("termios-rawer",	&opt_termios_rawer)
#ifdef O_TEXT
	IF_ANY    ("text",	&opt_o_text)
#endif
	IF_UNIX   ("tightsocklen",	&xioopt_unix_tightsocklen)
	IF_TERMIOS("time",	&opt_vtime)
#ifdef SO_TIMESTAMP
	IF_SOCKET ("timestamp",	&opt_so_timestamp)
#endif
	IF_TERMIOS("tiocsctty",	&opt_tiocsctty)
#if WITH_EXT2 && defined(EXT2_TOPDIR_FL)
	IF_ANY    ("topdir",	&opt_ext2_topdir)
#endif
	IF_IP     ("tos",	&opt_ip_tos)
	IF_TERMIOS("tostop",	&opt_tostop)
	IF_OPEN   ("trunc",	&opt_o_trunc)
#if HAVE_FTRUNCATE64
	IF_ANY    ("truncate",	&opt_ftruncate64)
#else
	IF_ANY    ("truncate",	&opt_ftruncate32)
#endif
#ifdef TCP_TSOPTENA	/* OSF1 */
	IF_TCP    ("tsoptena",	&opt_tcp_tsoptena)
#endif
	IF_IP     ("ttl",	&opt_ip_ttl)
	IF_TUN    ("tun-device",	&opt_tun_device)
	IF_TUN    ("tun-name",	&opt_tun_name)
	IF_TUN    ("tun-no-pi",	&opt_iff_no_pi)
	IF_TUN    ("tun-type",	&opt_tun_type)
	IF_SOCKET ("type",	&opt_so_type)
	IF_ANY    ("uid",	&opt_user)
	IF_NAMED  ("uid-e",	&opt_user_early)
	IF_ANY    ("uid-l",	&opt_user_late)
	IF_NAMED  ("umask",	&opt_umask)
	IF_IP6    ("unicast-hops",	&opt_ipv6_unicast_hops)
	IF_UNIX   ("unix-tightsocklen",	&xioopt_unix_tightsocklen)
	IF_NAMED  ("unlink",	&opt_unlink)
	IF_NAMED  ("unlink-close",	&opt_unlink_close)
	IF_NAMED  ("unlink-early",	&opt_unlink_early)
	IF_NAMED  ("unlink-late",	&opt_unlink_late)
#if WITH_EXT2 && defined(EXT2_UNRM_FL)
	IF_ANY    ("unrm",		&opt_ext2_unrm)
#endif
	IF_TUN    ("up",	&opt_iff_up)
#ifdef SO_USE_IFBUFS
	IF_SOCKET ("use-ifbufs",	&opt_so_use_ifbufs)
	IF_SOCKET ("useifbufs",		&opt_so_use_ifbufs)
#endif /* SO_USE_IFBUFS */
#ifdef SO_USELOOPBACK /* AIX433, Solaris */
	IF_SOCKET ("useloopback",	&opt_so_useloopback)
#endif /* SO_USELOOPBACK */
	IF_ANY    ("user",	&opt_user)
	IF_NAMED  ("user-early",	&opt_user_early)
	IF_ANY    ("user-late",	&opt_user_late)
#if HAVE_RESOLV_H
	IF_IP     ("usevc",	&opt_res_usevc)
#endif /* HAVE_RESOLV_H */
#ifdef IPV6_V6ONLY
	IF_IP6    ("v6only",	&opt_ipv6_v6only)
#endif
#ifdef VDISCARD
	IF_TERMIOS("vdiscard",	&opt_vdiscard)
#endif
#ifdef VDSUSP	/* HP-UX */
	IF_TERMIOS("vdsusp",	&opt_vdsusp)
#endif
	IF_TERMIOS("veof",	&opt_veof)
	IF_TERMIOS("veol",	&opt_veol)
	IF_TERMIOS("veol2",	&opt_veol2)
	IF_TERMIOS("verase",	&opt_verase)
	IF_OPENSSL("verify",	&opt_openssl_verify)
	IF_TERMIOS("vintr",	&opt_vintr)
	IF_TERMIOS("vkill",	&opt_vkill)
	IF_TERMIOS("vlnext",	&opt_vlnext)
	IF_TERMIOS("vmin",	&opt_vmin)
	IF_TERMIOS("vquit",	&opt_vquit)
#ifdef VREPRINT
	IF_TERMIOS("vreprint",	&opt_vreprint)
#endif
	IF_TERMIOS("vstart",	&opt_vstart)
	IF_TERMIOS("vstop",	&opt_vstop)
	IF_TERMIOS("vsusp",	&opt_vsusp)
#ifdef VSWTC
	IF_TERMIOS("vswtc",	&opt_vswtc)
#endif
#ifdef VTDLY
#  ifdef VT0
	IF_TERMIOS("vt0",	&opt_vt0)
#  endif
#  ifdef VT1
	IF_TERMIOS("vt1",	&opt_vt1)
#  endif
	IF_TERMIOS("vtdly",	&opt_vtdly)
#endif
	IF_TERMIOS("vtime",	&opt_vtime)
#ifdef VWERASE
	IF_TERMIOS("vwerase",	&opt_vwerase)
#endif
#if HAVE_PTY && HAVE_POLL
	IF_PTY    ("wait-slave",	&opt_pty_wait_slave)
#endif /* HAVE_PTY && HAVE_POLL */
	IF_ANY    ("waitlock",	&opt_waitlock)
#if HAVE_PTY && HAVE_POLL
	IF_PTY    ("waitslave",	&opt_pty_wait_slave)
#endif /* HAVE_PTY && HAVE_POLL */
#ifdef VWERASE
	IF_TERMIOS("werase",	&opt_vwerase)
#endif
#ifdef TCP_WINDOW_CLAMP	/* Linux 2.4.0 */
	IF_TCP    ("window-clamp",	&opt_tcp_window_clamp)
#endif
#if WITH_LIBWRAP
	IF_IPAPP  ("wrap",		&opt_tcpwrappers)
#endif
	IF_OPEN   ("wronly",	&opt_o_wronly)
#ifdef XCASE
	IF_TERMIOS("xcase",	&opt_xcase)
#endif
#if defined(TABDLY) && defined(XTABS)
	IF_TERMIOS("xtabs",	&opt_xtabs)
#endif
	{ NULL }
} ;


/* walks the text argument a and writes its options that conform to groups 
   to the array opts. Uses the option table 'optionnames'.
   returns 0 on success, -1 on error, 1 on unknown/wrong option  
*/
int parseopts(const char **a, unsigned int groups, struct opt **opts) {

   return parseopts_table(a, groups, opts, optionnames, 
			  sizeof(optionnames)/sizeof(struct optname)-1);
}


/* walks the text argument a and writes its options that conform to groups 
   to the array opts. Uses the specified option table.
   returns 0 on success, -1 on error, 1 on unknown/wrong option  
*/
int parseopts_table(const char **a, unsigned int groups, struct opt **opts,
	      const struct optname optionnames[], size_t optionnum) {
   int i=0;
   struct opt *opt;
   bool assign;
   const char *a0 = *a;
   unsigned long ulongval;
   long slongval;
   long long slonglongval;
   char token[512], *tokp;  size_t len;
   int parsres;
   int result;
   char optbuf[256];  size_t optlen;
   const char *endkey[6+1];
   const char *endval[5+1];
   const char *assign_str = "=";
   const char *hquotes[] = {
      "'",
      NULL
   } ;
   const char *squotes[] = {
      "\"",
      NULL
   } ;
   const char *nests[] = {
      "(", ")",
      "[", "]",
      "{", "}",
      NULL
   } ;

   i = 0;
   /*endkey[i++] = xioopts.chainsep;*/	/* default: "|" */
   endkey[i++] = xioopts.pipesep;		/* default: "!!" */
   endkey[i++] = ","/*xioopts.comma*/;		/* default: "," */
   endkey[i++] = "=";
   endkey[i++] = NULL;

   i = 0;
   /*endval[i++] = xioopts.chainsep;*/	/* default: "|" */
   endval[i++] = xioopts.pipesep;		/* default: "!!" */
   endval[i++] = ","/*xioopts.comma*/;		/* default: "," */
   endval[i++] = NULL;

   i = 0;
   *opts = Malloc((i+8)*sizeof(struct opt));
   if (*opts == NULL) {
      return -1;
   }
   if (*a == NULL) {
      (*opts)[i].desc = ODESC_END;
      return 0;
   }

   while (true) {
      const struct optname *ent;

      if (a == NULL || *a == NULL || **a == '\0')
	 break;

      while (!strncmp(*a, ",", strlen(",")))  { (*a) += strlen(","); }
      a0 = *a;

      len = sizeof(token);  tokp = token;
      parsres =
	 nestlex(a, &tokp, &len, endkey, hquotes, squotes, nests,
		 true, true, false);
      if (parsres < 0) {
	 Error1("option too long:  \"%s\"", *a);
	 return -1;
      } else if (parsres > 0) {
	 Error1("syntax error in \"%s\"", *a);
	 return -1;
      }
      if (tokp == token) {
	 /* no option found */
	 break;
      }
      *tokp = '\0';

      ent = (struct optname *)
	 keyw((struct wordent *)optionnames, token, optionnum);
      if (ent == NULL) {
	 Error1("parseopts(): unknown option \"%s\"", token);
	 continue;
      }

      if (!(ent->desc->group & groups) && !(ent->desc->group & GROUP_ANY) &&
	  !xioopts_ignoregroups) {
	 Error1("parseopts(): option \"%s\" not supported with this address type",
		token /*a0*/);
	 Info2("parseopts()  groups=%08x, ent->group=%08x",
	       groups, ent->desc->group);
#if 0
	 continue;
#endif
      }
      (*opts)[i].desc = ent->desc;
      
      if (!strncmp(*a, assign_str, strlen(assign_str))) {
	 /* there is an assignment (mostly "=") */
	 (*a) += strlen(assign_str);
	 len = sizeof(token);  tokp = token;
	 parsres =
	    nestlex(a, &tokp, &len, endval, hquotes, squotes, nests,
		    true, true, false);
	 if (parsres < 0) {
	    Error1("option too long:  \"%s\"", *a);
	    return -1;
	 } else if (parsres > 0) {
	    Error1("syntax error in \"%s\"", *a);
	    return -1;
	 }
	 *tokp = '\0';
	 assign = true;

      } else {
	 assign = false;
      }
      opt = &(*opts)[i];

      switch (ent->desc->type) {
      case TYPE_CONST:
	 if (assign) {
	    Error1("no value permitted for option \"%s\"",
		   ent->desc->defname);
	    continue;
	 }
	 Info1("setting option \"%s\"", ent->desc->defname);
	 break;
      case TYPE_BIN:
	 if (!assign) { Error1("option \"%s\": value required", a0);
	    continue; }
	 optlen = 0;
	 if ((result = dalan(token, optbuf, &optlen, sizeof(optbuf))) != 0) {
	    Error1("parseopts(): problem with \"%s\" data", token);
	    continue;
	 }
	 if (((*opts)[i].value.u_bin.b_data = memdup(optbuf, optlen)) == NULL) {
	    Error1("memdup(, "F_Zu"): out of memory", optlen);
	    return -1;
	 }
	 (*opts)[i].value.u_bin.b_len = optlen;
	 break;
      case TYPE_BYTE:
	 if (assign) {
	  unsigned long ul;
	  char *rest;
	  ul = strtoul(token, &rest/*!*/, 0);
	  if (ul > UCHAR_MAX) {
	    Error3("parseopts(%s): byte value exceeds limit (%lu vs. %u), using max",
		   a0, ul, UCHAR_MAX);
	    (*opts)[i].value.u_byte = UCHAR_MAX;
	  } else {
	    (*opts)[i].value.u_byte = ul;
	  }
	 } else {
	    (*opts)[i].value.u_byte = 1;
	 }
	 Info2("setting option \"%s\" to %d", ent->desc->defname,
	       (*opts)[i].value.u_byte);
	 break;
#if HAVE_BASIC_OFF_T==3
      case TYPE_OFF32:
#endif
      case TYPE_INT:
	 if (assign) {
	    char *rest;
	    (*opts)[i].value.u_int = strtoul(token, &rest/*!*/, 0);
	 } else {
	    (*opts)[i].value.u_int = 1;
	 }
	 Info2("setting option \"%s\" to %d", ent->desc->defname,
	       (*opts)[i].value.u_int);
	 break;
      case TYPE_BOOL:
	 if (!assign) {
	    (*opts)[i].value.u_bool = 1;
	 } else {
	    char *rest;
	    (*opts)[i].value.u_bool = strtoul(token, &rest, 0);
	    if (rest && *rest) {
	       Error1("error in option \"%s\": \"0\" or \"1\" required", a0);
	    }
	 }
	 Info2("setting option \"%s\" to %d", ent->desc->defname,
	       (*opts)[i].value.u_bool);
	 break;

#if HAVE_BASIC_SIZE_T==4
      case TYPE_SIZE_T:
#endif
      case TYPE_UINT:
	 if (!assign) {
	    (*opts)[i].value.u_uint = 1;
	 } else {
	    char *rest;
	    ulongval = strtoul(token, &rest/*!*/, 0);
	    if (ulongval > UINT_MAX) {
	       Error3("parseopts(%s): unsigned int value exceeds limit (%lu vs. %u), using max",
		      a0, ulongval, UINT_MAX);
	    }
	    (*opts)[i].value.u_uint = ulongval;
	 }
	 Info2("setting option \"%s\" to %u", ent->desc->defname,
	       (*opts)[i].value.u_uint);
	 break;

#if HAVE_BASIC_SIZE_T==2
      case TYPE_SIZE_T:
#endif
      case TYPE_USHORT:
	 if (!assign) {
	    (*opts)[i].value.u_ushort = 1;
	 } else {
	    char *rest;
	    ulongval = strtoul(token, &rest/*!*/, 0);
	    if (ulongval > USHRT_MAX) {
	       Error3("parseopts(%s): unsigned short value exceeds limit (%lu vs. %u), using max",
		      a0, ulongval, USHRT_MAX);
	    }
	    (*opts)[i].value.u_ushort = ulongval;
	 }
	 Info2("setting option \"%s\" to %u", ent->desc->defname,
	       (*opts)[i].value.u_ushort);
	 break;

#if HAVE_BASIC_OFF_T==5
      case TYPE_OFF32:
#endif
#if HAVE_STAT64 && defined(HAVE_BASIC_OFF64_T) && HAVE_BASIC_OFF64_T==5
      case TYPE_OFF64:
#endif
      case TYPE_LONG:
	 if (!assign) {
	    (*opts)[i].value.u_long = 1;
	 } else {
	    char *rest;
	    slongval = strtol(token, &rest, 0);
	    (*opts)[i].value.u_long = slongval;
	 }
	 Info2("setting option \"%s\" to %lu", ent->desc->defname,
	       (*opts)[i].value.u_long);
	 break;

#if HAVE_BASIC_SIZE_T==6
      case TYPE_SIZE_T:
#endif
      case TYPE_ULONG:
	 if (!assign) {
	    (*opts)[i].value.u_ulong = 1;
	 } else {
	    char *rest;
	    ulongval = strtoul(token, &rest, 0);
	    (*opts)[i].value.u_ulong = ulongval;
	 }
	 Info2("setting option \"%s\" to %lu", ent->desc->defname,
	       (*opts)[i].value.u_ulong);
	 break;

#if HAVE_BASIC_OFF_T==7
      case TYPE_OFF32:
#endif
#if HAVE_TYPE_LONGLONG
      case TYPE_LONGLONG:
#  if HAVE_STAT64 && defined(HAVE_BASIC_OFF64_T) && HAVE_BASIC_OFF64_T==7
      case TYPE_OFF64:
#  endif
	 if (!assign) {
	    (*opts)[i].value.u_longlong = 1;
	 } else {
	    char *rest;
#  if HAVE_STRTOLL
	    slonglongval = strtoll(token, &rest, 0);
#  else
	    /* in this case, input value range is limited */
	    slonglongval = strtol(token, &rest, 0);
#  endif /* HAVE_STRTOLL */
	    (*opts)[i].value.u_longlong = slonglongval;
	 }
	 Info2("setting option \"%s\" to %Lu", ent->desc->defname,
	       (*opts)[i].value.u_longlong);
	 break;
#endif /* HAVE_TYPE_LONGLONG */

      case TYPE_UIDT:
	 if (!assign) {
	    Error1("option \"%s\": value required", a0);
	    continue;
	 }
	 if (isdigit((*token)&0xff)) {
	    char *rest;
	    (*opts)[i].value.u_uidt = strtoul(token, &rest/*!*/, 0);
	 } else {
	    struct passwd *pwd;
	    if ((pwd = getpwnam(token)) == NULL) {
	       Error1("getpwnam(\"%s\"): no such user", token);
	       continue;
	    }
	    (*opts)[i].value.u_uidt = getpwnam(token)->pw_uid;
	 }
	 Info2("setting option \"%s\" to %u", ent->desc->defname,
	       (*opts)[i].value.u_uidt);
	 break;

      case TYPE_GIDT:
	 if (!assign) { Error1("option \"%s\": value required", a0);
	    continue; }
	 if (isdigit((token[0])&0xff)) {
	    char *rest;
	    (*opts)[i].value.u_gidt = strtoul(token, &rest/*!*/, 0);
	 } else {
	    struct group *grp;
	    grp = getgrnam(token);
	    if (grp == NULL) {
	       Error1("getgrnam(\"%s\"): no such group", token);
	       continue;
	    }
	    (*opts)[i].value.u_gidt = grp->gr_gid;
	 }
	 Info2("setting option \"%s\" to %u", ent->desc->defname,
	       (*opts)[i].value.u_gidt);
	 break;

      case TYPE_MODET:
	 if (!assign) { Error1("option \"%s\": value required", a0);
	    continue;
	 }
	 {
	    char *rest;
	    (*opts)[i].value.u_modet = strtoul(token, &rest/*!*/, 8);
	 }
	 Info2("setting option \"%s\" to %u", ent->desc->defname,
	       (*opts)[i].value.u_modet);
	 break;

      case TYPE_STRING:
	 if (!assign) {
	    Error1("option \"%s\": value required", a0);
	    continue;
	 }
	 if (((*opts)[i].value.u_string = strdup(token)) == NULL) {
	    Error("out of memory"); return -1;
	 }
	 Info2("setting option \"%s\" to \"%s\"", ent->desc->defname,
	       (*opts)[i].value.u_string);
	 break;

      case TYPE_STRING_NULL:
	 if (!assign) {
	    (*opts)[i].value.u_string = NULL;
	    Info1("setting option \"%s\" to NULL", ent->desc->defname);
	 } else {
	    (*opts)[i].value.u_string = strdup(token);
	    Info2("setting option \"%s\" to \"%s\"", ent->desc->defname,
		  (*opts)[i].value.u_string);
	 }
	 break;

#if LATER
      case TYPE_INT3:
	 
	 break;
#endif

      case TYPE_TIMEVAL:
	 if (!assign) {
	    Error1("option \"%s\": value required", a0);
	    continue;
	 } else {
	    double val;
	    val = strtod(token, NULL);
	    if (val == HUGE_VAL || val == -HUGE_VAL ||
		val == 0.0 && errno == ERANGE) {
	       Error2("strtod(\"%s\", NULL): %s", token, strerror(errno));
	       val = 0.0;
	    }
	    (*opts)[i].value.u_timeval.tv_sec  = val;
	    (*opts)[i].value.u_timeval.tv_usec =
	       (val-(*opts)[i].value.u_timeval.tv_sec) * 1000000;
	 }
	 break;

#if HAVE_STRUCT_TIMESPEC
      case TYPE_TIMESPEC:
	 if (!assign) {
	    Error1("option \"%s\": value required", a0);
	    continue;
	 } else {
	    double val;
	    val = strtod(token, NULL);
	    if (val == HUGE_VAL || val == -HUGE_VAL ||
		val == 0.0 && errno == ERANGE) {
	       Error2("strtod(\"%s\", NULL): %s", token, strerror(errno));
	       val = 0.0;
	    }
	    (*opts)[i].value.u_timespec.tv_sec  = val;
	    (*opts)[i].value.u_timespec.tv_nsec =
	       (val-(*opts)[i].value.u_timespec.tv_sec) * 1000000000.;
	 }
	 break;
#endif /* HAVE_STRUCT_TIMESPEC */

#if HAVE_STRUCT_LINGER
      case TYPE_LINGER:
	 if (!assign) {
	    Error1("option \"%s\": value required", a0);
	    continue;
	 }
	 (*opts)[i].value.u_linger.l_onoff = 1;
	 {
	    char *rest;
	    (*opts)[i].value.u_linger.l_linger = strtoul(token, &rest/*!*/, 0);
	 }
	 Info3("setting option \"%s\" to {%d,%d}", ent->desc->defname,
	       (*opts)[i].value.u_linger.l_onoff,
	       (*opts)[i].value.u_linger.l_linger);
	 break;
#endif /* HAVE_STRUCT_LINGER */

      case TYPE_INT_INT:
      case TYPE_INT_INTP:
	 if (!assign) {
	    Error1("option \"%s\": values required", a0);
	    continue;
	 }
	 {
	    char *rest;
	    (*opts)[i].value.u_int = strtoul(token, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 2 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    (*opts)[i].value2.u_int = strtoul(rest, &rest, 0);
	 }
	 Info3("setting option \"%s\" to %d:%d", ent->desc->defname,
	       (*opts)[i].value.u_int, (*opts)[i].value2.u_int);
	 break;

      case TYPE_INT_BIN:
	 if (!assign) {
	    Error1("option \"%s\": values required", a0);
	    continue;
	 }
	 {
	    char *rest;
	    (*opts)[i].value.u_int = strtoul(token, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 2 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    optlen = 0;
	    if ((result = dalan(rest, optbuf, &optlen, sizeof(optbuf))) != 0) {
	       Error1("parseopts(): problem with \"%s\" data", rest);
	       continue;
	    }
	    if (((*opts)[i].value2.u_bin.b_data = memdup(optbuf, optlen)) == NULL) {
	       Error1("memdup(, "F_Zu"): out of memory", optlen);
	       return -1;
	    }
	    (*opts)[i].value2.u_bin.b_len = optlen;
	 }
	 Info2("setting option \"%s\" to %d:..."/*!!!*/, ent->desc->defname,
	       (*opts)[i].value.u_int);
	 break;

      case TYPE_INT_STRING:
	 if (!assign) {
	    Error1("option \"%s\": values required", a0);
	    continue;
	 }
	 {
	    char *rest;
	    (*opts)[i].value.u_int = strtoul(token, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 2 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    if (((*opts)[i].value2.u_string = strdup(rest)) == NULL) {
	       Error("out of memory"); return -1;
	    }
	 }
	 Info3("setting option \"%s\" to %d:\"%s\"", ent->desc->defname,
	       (*opts)[i].value.u_int, (*opts)[i].value2.u_string);
	 break;

      case TYPE_INT_INT_INT:
	 if (!assign) {
	    Error1("option \"%s\": values required", a0);
	    continue;
	 }
	 {
	    char *rest;
	    (*opts)[i].value.u_int = strtoul(token, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 3 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    (*opts)[i].value2.u_int = strtoul(rest, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 3 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    (*opts)[i].value3.u_int = strtoul(rest, &rest, 0);
	 }
	 Info4("setting option \"%s\" to %d:%d:%d", ent->desc->defname,
	       (*opts)[i].value.u_int, (*opts)[i].value2.u_int, (*opts)[i].value3.u_int);
	 break;

      case TYPE_INT_INT_BIN:
	 if (!assign) {
	    Error1("option \"%s\": values required", a0);
	    continue;
	 }
	 {
	    char *rest;
	    (*opts)[i].value.u_int = strtoul(token, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 3 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    (*opts)[i].value2.u_int = strtoul(rest, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 3 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    optlen = 0;
	    if ((result = dalan(rest, optbuf, &optlen, sizeof(optbuf))) != 0) {
	       Error1("parseopts(): problem with \"%s\" data", rest);
	       continue;
	    }
	    if (((*opts)[i].value3.u_bin.b_data = memdup(optbuf, optlen)) == NULL) {
	       Error1("memdup(, "F_Zu"): out of memory", optlen);
	       return -1;
	    }
	    (*opts)[i].value3.u_bin.b_len = optlen;
	 }
	 Info3("setting option \"%s\" to %d:%d:..."/*!!!*/, ent->desc->defname,
	       (*opts)[i].value.u_int, (*opts)[i].value2.u_int);
	 break;

      case TYPE_INT_INT_STRING:
	 if (!assign) {
	    Error1("option \"%s\": values required", a0);
	    continue;
	 }
	 {
	    char *rest;
	    (*opts)[i].value.u_int = strtoul(token, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 3 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    (*opts)[i].value2.u_int = strtoul(rest, &rest, 0);
	    if (*rest != ':') {
	       Error1("option \"%s\": 3 arguments required",
		      ent->desc->defname);
	    }
	    ++rest;
	    if (((*opts)[i].value3.u_string = strdup(rest)) == NULL) {
	       Error("out of memory"); return -1;
	    }
	 }
	 Info4("setting option \"%s\" to %d:%d:\"%s\"", ent->desc->defname,
	       (*opts)[i].value.u_int, (*opts)[i].value2.u_int,
	       (*opts)[i].value3.u_string);
	 break;
#if defined(HAVE_STRUCT_IP_MREQ) || defined (HAVE_STRUCT_IP_MREQN)

      case TYPE_IP_MREQN:
	 {
	    /* we do not resolve the addresses here because we do not yet know
	       if we are coping with a IPv4 or IPv6 socat address */
	    const char *ends[] = { ":", NULL };
	    const char *nests[] = { "[","]", NULL };
	    char buff[512], *buffp=buff; size_t bufspc = sizeof(buff)-1;

	    /* parse first IP address, expect ':' */
	    tokp = token;
	    /*! result= */
	    parsres =
	       nestlex((const char **)&tokp, &buffp, &bufspc,
		       ends, NULL, NULL, nests,
		       true, false, false);
	    if (parsres < 0) {
	       Error1("option too long:  \"%s\"", *a);
	       return -1;
	    } else if (parsres > 0) {
	       Error1("syntax error in \"%s\"", *a);
	       return -1;
	    }
	    if (*tokp != ':') {
	       Error1("syntax in option %s: missing ':'", token);
	    }
	    *buffp++ = '\0';
	    (*opts)[i].value.u_ip_mreq.multiaddr = strdup(buff); /*!!! NULL */

	    ++tokp;
	    /* parse second IP address, expect ':' or '\0'' */
	    buffp = buff;
	    /*! result= */
	    parsres =
	       nestlex((const char **)&tokp, &buffp, &bufspc,
		       ends, NULL, NULL, nests,
		       true, false, false);
	    if (parsres < 0) {
	       Error1("option too long:  \"%s\"", *a);
	       return -1;
	    } else if (parsres > 0) {
	       Error1("syntax error in \"%s\"", *a);
	       return -1;
	    }
	    *buffp++ = '\0';
	    (*opts)[i].value.u_ip_mreq.param2 = strdup(buff); /*!!! NULL */

#if HAVE_STRUCT_IP_MREQN	    
	    if (*tokp++ == ':') {
	       strncpy((*opts)[i].value.u_ip_mreq.ifindex, tokp, IF_NAMESIZE);	/* ok */
	       Info4("setting option \"%s\" to {\"%s\",\"%s\",\"%s\"}",
		     ent->desc->defname,
		     (*opts)[i].value.u_ip_mreq.multiaddr,
		     (*opts)[i].value.u_ip_mreq.param2,
		     (*opts)[i].value.u_ip_mreq.ifindex);
	    } else {
	       (*opts)[i].value.u_ip_mreq.ifindex[0] = '\0';
	       Info3("setting option \"%s\" to {\"%s\",\"%s\"}",
		     ent->desc->defname,
		     (*opts)[i].value.u_ip_mreq.multiaddr,
		     (*opts)[i].value.u_ip_mreq.param2);
	    }
#else /* !HAVE_STRUCT_IP_MREQN */
	    Info3("setting option \"%s\" to {0x%08x,0x%08x}",
		  ent->desc->defname,
		  (*opts)[i].value.u_ip_mreq.multiaddr,
		  (*opts)[i].value.u_ip_mreq.param2);
#endif /* !HAVE_STRUCT_IP_MREQN */
	 }
	 break;
#endif /* defined(HAVE_STRUCT_IP_MREQ) || defined (HAVE_STRUCT_IP_MREQN) */

#if WITH_IP4
      case TYPE_IP4NAME:
	 {
	    struct sockaddr_in sa;  socklen_t salen = sizeof(sa);
	    const char *ends[] = { NULL };
	    const char *nests[] = { "[","]", NULL };
	    char buff[512], *buffp=buff; size_t bufspc = sizeof(buff)-1;

	    tokp = token;
	    parsres =
	    nestlex((const char **)&tokp, &buffp, &bufspc,
		    ends, NULL, NULL, nests,
		    true, false, false);
	    if (parsres < 0) {
	       Error1("option too long:  \"%s\"", *a);
	       return -1;
	    } else if (parsres > 0) {
	       Error1("syntax error in \"%s\"", *a);
	       return -1;
	    }
	    if (*tokp != '\0') {
	       Error1("trailing data in option \"%s\"", token);
	    }
	    *buffp = '\0';
	    if (xiogetaddrinfo(buff, NULL, AF_INET, SOCK_DGRAM, IPPROTO_IP,
			       (union sockaddr_union *)&sa, &salen,
			       0, 0/*!!!*/) != STAT_OK) {
	       opt->desc = ODESC_ERROR; continue;
	    }
	    opt->value.u_ip4addr = sa.sin_addr;
	 }
	 break;
#endif /* defined(WITH_IP4) */

      default:
	 Error2("parseopts(): internal error on option \"%s\": unimplemented type %d",
		ent->desc->defname, ent->desc->type);
	 continue;
      }

      ++i;
      if ((i % 8) == 0) {
	 *opts = Realloc(*opts, (i+8) * sizeof(struct opt));
	 if (*opts == NULL) {
	    return -1;
	 }
      }
   }

   /*(*opts)[i+1].desc = ODESC_END;*/
   (*opts)[i].desc = ODESC_END;
   return 0;
}

/* copy the already parsed options for repeated application, but only those
   matching groups ANY and <groups> */
struct opt *copyopts(const struct opt *opts, unsigned int groups) {
   struct opt *new;
   int i, j, n;

   if (!opts)  return NULL;

   /* just count the options in the array */
   i = 0; while (opts[i].desc != ODESC_END) {
      ++i;
   }
   n = i+1;

   new = Malloc(n * sizeof(struct opt));
   if (new == NULL) {
      return NULL;
   }

   i = 0, j = 0;
   while (i < n-1) {
      if (opts[i].desc == ODESC_DONE) {
	 new[j].desc = ODESC_DONE;
      } else if ((opts[i].desc->group & (GROUP_ANY&~GROUP_PROCESS)) ||
		 (opts[i].desc->group & groups)) {
	 new[j++] = opts[i];
      }
      ++i;
   }
   new[j].desc = ODESC_END;
   return new;
}

/* move options to a new options list
   move only those matching <groups> */
struct opt *moveopts(struct opt *opts, unsigned int groups) {
   struct opt *new;
   int i, j, n;

   if (!opts)  return NULL;

   /* just count the options in the array */
   i = 0; j = 0; while (opts[i].desc != ODESC_END) {
      if (opts[i].desc != ODESC_DONE &&
	  opts[i].desc != ODESC_ERROR)
	 ++j;
      ++i;
   }
   n = i;

   new = Malloc((j+1) * sizeof(struct opt));
   if (new == NULL) {
      return NULL;
   }

   i = 0, j = 0;
   while (i < n) {
      if (opts[i].desc == ODESC_DONE ||
	  opts[i].desc == ODESC_ERROR) {
	 ++i; continue;
      } else if (opts[i].desc->group & groups) {
	 new[j++] = opts[i];
	 opts[i].desc = ODESC_DONE;
      }
      ++i;
   }
   new[j].desc = ODESC_END;
   return new;
}

/* return the number of yet unconsumed options; -1 on error */
int leftopts(const struct opt *opts) {
   const struct opt *opt = opts;
   int num = 0;

   if (!opts)  return 0;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE) {
	 ++num;
      }
      ++opt;
   }
   return num;
}

/* show as warning which options are still unused */
int showleft(const struct opt *opts) {
   const struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE) {
	 Warn1("showleft(): option \"%s\" not inquired", opt->desc->defname);
      }
      ++opt;
   }
   return 0;
}


/* determines the address group from mode_t */
/* does not set GROUP_FD; cannot determine GROUP_TERMIOS ! */
int _groupbits(mode_t mode) {
   unsigned int result = 0;
  
   switch ((mode&S_IFMT)>>12) {
   case (S_IFIFO>>12):	/* 1, FIFO */
      result = GROUP_FIFO;   break;
   case (S_IFCHR>>12):	/* 2, character device */
      result = GROUP_CHR|GROUP_TERMIOS;    break;
   case (S_IFDIR>>12):	/* 4, directory !!! not supported */
      result = GROUP_NONE;   break;
   case (S_IFBLK>>12):	/* 6, block device */
      result = GROUP_BLK;    break;
   case (S_IFREG>>12):	/* 8, regular file */
      result = GROUP_REG;    break;
   case (S_IFLNK>>12):	/* 10, symbolic link !!! not supported */
      result = GROUP_NONE;   break;
#ifdef S_IFSOCK
   case (S_IFSOCK>>12): /* 12, socket */
      result = GROUP_SOCKET|GROUP_SOCK_UNIX; break;
#else
   default: /* some systems (pure POSIX.1) do not know S_IFSOCK */
      result = GROUP_SOCKET|GROUP_SOCK_UNIX; break;
#endif
   }
   Debug2("_groupbits(%d) -> %d", mode, result);
   return result;
}

/* does not set GROUP_FD */
int groupbits(int fd) {
#if HAVE_STAT64
   struct stat64 buf;
#else
   struct stat buf;
#endif /* !HAVE_STAT64 */
   int result;

   if (
#if HAVE_STAT64
       Fstat64(fd, &buf) < 0
#else
       Fstat(fd, &buf) < 0
#endif /* !HAVE_STAT64 */
      ) {
      Error4("groupbits(%d): fstat(%d, %p): %s",
	     fd, fd, &buf, strerror(errno));
      return -1;
   }
   result = _groupbits(buf.st_mode&S_IFMT);
   if (result == GROUP_CHR) {
      if (Isatty(fd) > 0) {
	 result |= GROUP_TERMIOS;
      }
   }
   return result;
}

#if 0	/* currently not used */
int retropt(struct opt *opts, int optcode, union integral *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 *result = opt->value;
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}
#endif

static struct opt *xio_findopt(struct opt *opts, int optcode) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 return opt;
      }
      ++opt;
   }
   return NULL;
}

int retropt_timespec(struct opt *opts, int optcode, struct timespec *result) {
   struct opt *opt;

   if (!(opt = xio_findopt(opts, optcode))) {
      return -1;
   }
   *result = opt->value.u_timespec;
   opt->desc = ODESC_DONE;
   return 0;
}
      
   
/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its bool value in *result, "consumes" the
   option, and returns 0.
   If the option is not found, *result is not modified, and -1 is returned. */
int retropt_bool(struct opt *opts, int optcode, bool *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 *result = opt->value.u_bool;
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}

#if 0	/* currently not used */
/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its short value in *result, "consumes" the
   option, and returns 0.
   If the option is not found, *result is not modified, and -1 is returned. */
int retropt_short(struct opt *opts, int optcode, short *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 *result = opt->value.u_short;
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}
#endif

/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its unsigned short value in *result, "consumes" the
   option, and returns 0.
   If the option is not found, *result is not modified, and -1 is returned. */
int retropt_ushort(struct opt *opts, int optcode, unsigned short *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 *result = opt->value.u_ushort;
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}

/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its int value in *result, "consumes" the
   option, and returns 0.
   If the option is not found, *result is not modified, and -1 is returned. */
int retropt_int(struct opt *opts, int optcode, int *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 switch (opt->desc->type) {
	 case TYPE_INT: *result = opt->value.u_int; break;
	 case TYPE_STRING: *result = strtol(opt->value.u_string, NULL, 0);
	    break;
	 default: Error2("cannot convert type %d of option %s to int",
			 opt->desc->type, opt->desc->defname);
	    opt->desc = ODESC_ERROR;
	    return -1;
	 }
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}

/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its unsigned int value in *result, "consumes" the
   option, and returns 0.
   If the option is not found, *result is not modified, and -1 is returned. */
int retropt_uint(struct opt *opts, int optcode, unsigned int *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 *result = opt->value.u_uint;
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}

/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its long value in *result, "consumes" the option,
   and returns 0.
   If the option is not found, *result is not modified, and -1 is returned. */
int retropt_long(struct opt *opts, int optcode, long *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 *result = opt->value.u_long;
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}

/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its unsigned long value in *result, "consumes" the
   option, and returns 0.
   If the option is not found, *result is not modified, and -1 is returned. */
int retropt_ulong(struct opt *opts, int optcode, unsigned long *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 *result = opt->value.u_ulong;
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}

#if 0	/* currently not used */
/* get the value of a FLAG typed option, and apply it to the appropriate
   bit position. Mark the option as consumed (done). return 0 if options was found and successfully applied,
   or -1 if option was not in opts */
int retropt_flag(struct opt *opts, int optcode, flags_t *result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 if (opt->value.u_bool) {
	    *result |= opt->desc->major;
	 } else {
	    *result &= ~opt->desc->major;
	 }
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}
#endif

/* Looks for the first option of type <optcode>. If the option is found,
   this function stores its character pointer value in *result, "consumes" the
   option, and returns 0. Note that, for options of type STRING_NULL, the
   character pointer might degenerate to NULL.
   The resulting string is malloc'ed and should be freed after use.
   If the option is not found, *result is not modified, and -1 is returned.
 */
int retropt_string(struct opt *opts, int optcode, char **result) {
   struct opt *opt = opts;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->optcode == optcode) {
	 if (opt->value.u_string == NULL) {
	    *result = NULL;
	 } else if ((*result = strdup(opt->value.u_string)) == NULL) {
	    Error1("strdup("F_Zu"): out of memory",
		   strlen(opt->value.u_string));
	    return -1;
	 }
	 opt->desc = ODESC_DONE;
	 return 0;
      }
      ++opt;
   }
   return -1;
}


#if _WITH_SOCKET
/* looks for a bind option and, if found, overwrites the complete contents of
   sa with the appropriate value(s).
   returns STAT_OK if option exists and could be resolved,
   STAT_NORETRY if option exists but had error,
   or STAT_NOACTION if it does not exist */
/* currently only for IP (v4, v6) and raw (PF_UNSPEC) */
int retropt_bind(struct opt *opts,
		 int af,
		 int socktype,
		 int ipproto,
		 struct sockaddr *sa,
		 socklen_t *salen,
		 int feats,	/* TCP etc: 1..address allowed,
					    3..address and port allowed
				   UNIX (or'd): 1..tight
						2..abstract
				*/
		 unsigned long res_opts0, unsigned long res_opts1) {
   const char portsep[] = ":";
   const char *ends[] = { portsep, NULL };
   const char *nests[] = { "[", "]", NULL };
   bool portallowed;
   char *bindname, *bindp;
   char hostname[512], *hostp = hostname, *portp = NULL;
   size_t hostlen = sizeof(hostname)-1;
   int parsres;
   int result;

   if (retropt_string(opts, OPT_BIND, &bindname) < 0) {
      return STAT_NOACTION;
   }
   bindp = bindname;

   switch (af) {

   case AF_UNSPEC:
      {
	 size_t p = 0;
	 dalan(bindname, (char *)sa->sa_data, &p, *salen-sizeof(sa->sa_family));
	 *salen = p + sizeof(sa->sa_family);
	 *salen = p +
#if HAVE_STRUCT_SOCKADDR_SALEN
	    sizeof(sa->sa_len) +
#endif
	    sizeof(sa->sa_family);
#if HAVE_STRUCT_SOCKADDR_SALEN
	 sa->sa_len = *salen;
#endif
      }
      break;

#if WITH_IP4 || WITH_IP6
#if WITH_IP4
   case AF_INET:
#endif
#if WITH_IP6
   case AF_INET6:
#endif /*WITH_IP6 */
      portallowed = (feats>=2);
      parsres =
	 nestlex((const char **)&bindp, &hostp, &hostlen, ends, NULL, NULL, nests,
		 true, false, false);
      if (parsres < 0) {
	 Error1("option too long:  \"%s\"", bindp);
	 return STAT_NORETRY;
      } else if (parsres > 0) {
	 Error1("syntax error in \"%s\"", bindp);
	 return STAT_NORETRY;
      }
      *hostp++ = '\0';
      if (!strncmp(bindp, portsep, strlen(portsep))) {
	 if (!portallowed) {
	    Error("port specification not allowed in this bind option");
	    return STAT_NORETRY;
	 } else {
	    portp = bindp + strlen(portsep);
	 }
      }
      if ((result =
	   xiogetaddrinfo(hostname[0]!='\0'?hostname:NULL, portp,
			  af, socktype, ipproto,
			  (union sockaddr_union *)sa, salen,
			  res_opts0, res_opts1))
	  != STAT_OK) {
	 Error("error resolving bind option");
	 return STAT_NORETRY;
      }	   
      break;
#endif /* WITH_IP4 || WITH_IP6 */

#if WITH_UNIX
   case AF_UNIX:
      {
	 bool abstract = (feats&2);
	 bool tight = (feats&1);
	 struct sockaddr_un *s_un = (struct sockaddr_un *)sa;
	 *salen = xiosetunix(af, s_un, bindname, abstract, tight);
      }
      break;
#endif /* WITH_UNIX */

   default:
      Error1("bind: unknown address family %d", af);
      return STAT_NORETRY;
   }
   return STAT_OK;
}
#endif /* _WITH_SOCKET */


/* applies to fd all options belonging to phase */
/* note: not all options can be applied this way (e.g. OFUNC_SPEC with PH_OPEN)
   implemented are: OFUNC_FCNTL, OFUNC_SOCKOPT (probably not all types),
   OFUNC_TERMIOS_FLAG, OFUNC_TERMIOS_PATTERN, and some OFUNC_SPEC */
int applyopts(int fd, struct opt *opts, enum e_phase phase) {
   struct opt *opt;

   opt = opts; while (opt && opt->desc != ODESC_END) {
      if (opt->desc == ODESC_DONE ||
	  (phase != PH_ALL && opt->desc->phase != phase)) {
	 ++opt; continue; }

      if (opt->desc->func == OFUNC_SEEK32) {
	 if (Lseek(fd, opt->value.u_off, opt->desc->major) < 0) {
	    Error4("lseek(%d, "F_off", %d): %s",
		   fd, opt->value.u_off, opt->desc->major, strerror(errno));
	 }
#if HAVE_LSEEK64
      } else if (opt->desc->func == OFUNC_SEEK64) {

	 /*! this depends on off64_t atomic type */
	 if (Lseek64(fd, opt->value.u_off64, opt->desc->major) < 0) {
	    Error4("lseek64(%d, "F_off64", %d): %s",
		   fd, opt->value.u_off64, opt->desc->major, strerror(errno));
	 }
#endif /* HAVE_LSEEK64 */

      } else if (opt->desc->func == OFUNC_FCNTL) {
	 int flag;

	 /* retrieve existing flag setttings */
	 if ((flag = Fcntl(fd, opt->desc->major-1)) < 0) {
	    Error3("fcntl(%d, %d): %s",
		   fd, opt->desc->major, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 } else {
	    if (opt->value.u_bool) {
	       flag |= opt->desc->minor;
	    } else {
	       flag &= ~opt->desc->minor;
	    }
	    if (Fcntl_l(fd, opt->desc->major, flag) < 0) {
	       Error4("fcntl(%d, %d, %d): %s",
		      fd, opt->desc->major, flag, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	 }

      } else if (opt->desc->func == OFUNC_IOCTL) {
	 if (Ioctl(fd, opt->desc->major, (void *)&opt->value) < 0) {
	    Error4("ioctl(%d, 0x%x, %p): %s",
		   fd, opt->desc->major, (void *)&opt->value, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }

      } else if (opt->desc->func == OFUNC_IOCTL_MASK_LONG) {
	 long val;
	 int getreq = opt->desc->major;
	 int setreq = opt->desc->minor;
	 long mask  = opt->desc->arg3;

	 if (Ioctl(fd, getreq, (void *)&val) < 0) {
	    Error4("ioctl(%d, 0x%x, %p): %s",
		   fd, opt->desc->major, (void *)&val, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
	 val &= ~mask;
	 if (opt->value.u_bool)  val |= mask;
	 if (Ioctl(fd, setreq, (void *)&val) < 0) {
	    Error4("ioctl(%d, 0x%x, %p): %s",
		   fd, opt->desc->major, (void *)&val, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }

      } else if (opt->desc->func == OFUNC_IOCTL_GENERIC) {
	 switch (opt->desc->type) {
	 case TYPE_INT:
	    if (Ioctl(fd, opt->value.u_int, NULL) < 0) {
	       Error3("ioctl(%d, 0x%x, NULL): %s",
		      fd, opt->value.u_int, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case TYPE_INT_INT:
	    if (Ioctl_int(fd, opt->value.u_int, opt->value2.u_int) < 0) {
	       Error4("ioctl(%d, 0x%x, 0x%x): %s",
		      fd, opt->value.u_int, opt->value2.u_int, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case TYPE_INT_INTP:
	    if (Ioctl(fd, opt->value.u_int, (void *)&opt->value2.u_int) < 0) {
	       Error4("ioctl(%d, 0x%x, %p): %s",
		      fd, opt->value.u_int, (void *)&opt->value2.u_int, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case TYPE_INT_BIN:
	    if (Ioctl(fd, opt->value.u_int, (void *)opt->value2.u_bin.b_data) < 0) {
	       Error4("ioctl(%d, 0x%x, %p): %s",
		      fd, opt->value.u_int, (void *)opt->value2.u_bin.b_data, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case TYPE_INT_STRING:
	    if (Ioctl(fd, opt->value.u_int, (void *)opt->value2.u_string) < 0) {
	       Error4("ioctl(%d, 0x%x, %p): %s",
		      fd, opt->value.u_int, (void *)opt->value2.u_string, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 default:
	    Error1("ioctl() data type %d not implemented",
		   opt->desc->type);
	 }

#if _WITH_SOCKET
      } else if (opt->desc->func == OFUNC_SOCKOPT) {
	 if (0) {
	    ;
#if 0 && HAVE_STRUCT_LINGER
	 } else if (opt->desc->optcode == OPT_SO_LINGER) {
	    struct linger lingstru;
	    lingstru.l_onoff = (opt->value.u_int>=0 ? 1 : 0);
	    lingstru.l_linger = opt->value.u_int;
	    if (Setsockopt(fd, opt->desc->major, opt->desc->minor, &lingstru,
			   sizeof(lingstru)) < 0) {
	       Error6("setsockopt(%d, %d, %d, {%d,%d}, "F_Zu,
		      fd, opt->desc->major, opt->desc->minor, lingstru.l_onoff,
		      lingstru.l_linger, sizeof(lingstru));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
#endif /* HAVE_STRUCT_LINGER */
	 } else {
	    switch (opt->desc->type) {
	    case TYPE_BIN:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      opt->value.u_bin.b_data, opt->value.u_bin.b_len)
		   < 0) {
		  Error6("setsockopt(%d, %d, %d, %p, "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 opt->value.u_bin.b_data, opt->value.u_bin.b_len,
			 strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
	    case TYPE_BOOL:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      &opt->value.u_bool, sizeof(opt->value.u_bool))
		   < 0) {
		  Error6("setsockopt(%d, %d, %d, {%d}, "F_Zu"): %s", fd,
			 opt->desc->major, opt->desc->minor,
			 opt->value.u_bool, sizeof(opt->value.u_bool),
			 strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
	    case TYPE_BYTE:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      &opt->value.u_byte, sizeof(uint8_t)) < 0) {
		  Error6("setsockopt(%d, %d, %d, {%u}, "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 opt->value.u_byte, sizeof(uint8_t), strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
	    case TYPE_INT:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      &opt->value.u_int, sizeof(int)) < 0) {
		  Error6("setsockopt(%d, %d, %d, {%d}, "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 opt->value.u_int, sizeof(int), strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
	    case TYPE_LONG:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      &opt->value.u_long, sizeof(long)) < 0) {
		  Error6("setsockopt(%d, %d, %d, {%ld}, "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 opt->value.u_long, sizeof(long), strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
	    case TYPE_STRING:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      opt->value.u_string,
			      strlen(opt->value.u_string)+1) < 0) {
		  Error6("setsockopt(%d, %d, %d, \"%s\", "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 opt->value.u_string, strlen(opt->value.u_string)+1,
			 strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
	    case TYPE_UINT:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      &opt->value.u_uint, sizeof(unsigned int)) < 0) {
		  Error6("setsockopt(%d, %d, %d, {%u}, "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 opt->value.u_uint, sizeof(unsigned int),
			 strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
	    case TYPE_TIMEVAL:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      &opt->value.u_timeval, sizeof(struct timeval)) < 0) {
		  Error7("setsockopt(%d, %d, %d, {%ld,%ld}, "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 opt->value.u_timeval.tv_sec, opt->value.u_timeval.tv_usec,
			 sizeof(struct timeval), strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       break;
#if HAVE_STRUCT_LINGER
	    case TYPE_LINGER:
	       {
		  struct linger lingstru;
		  lingstru.l_onoff = (opt->value.u_linger.l_onoff>=0 ? 1 : 0);
		  lingstru.l_linger = opt->value.u_linger.l_linger;
		  if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
				 &lingstru, sizeof(lingstru)) < 0) {
		     Error6("setsockopt(%d, %d, %d, {%d,%d}): %s",
			    fd, opt->desc->major, opt->desc->minor,
			    lingstru.l_onoff, lingstru.l_linger,
			    strerror(errno));
		     opt->desc = ODESC_ERROR; ++opt; continue;
		  }
	       }
	       break;
#endif /* HAVE_STRUCT_LINGER */
#if defined(HAVE_STRUCT_IP_MREQ) || defined (HAVE_STRUCT_IP_MREQN)
	    case TYPE_IP_MREQN:
	       /* handled in applyopts_single */
	       ++opt; continue;
#endif /* defined(HAVE_STRUCT_IP_MREQ) || defined (HAVE_STRUCT_IP_MREQN) */

	       /*! still many types missing; implement on demand */
#if WITH_IP4
	    case TYPE_IP4NAME:
	       if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			      &opt->value.u_ip4addr, sizeof(opt->value.u_ip4addr)) < 0) {
		  Error6("setsockopt(%d, %d, %d, {0x%x}, "F_Zu"): %s",
			 fd, opt->desc->major, opt->desc->minor,
			 *(uint32_t *)&opt->value.u_ip4addr, sizeof(opt->value.u_ip4addr),
			 strerror(errno));
	       }
	       break;
#endif /* defined(WITH_IP4) */
	    default:
#if !NDEBUG
	       Error1("applyopts(): type %d not implemented",
			    opt->desc->type);
#else
	       Warn1("applyopts(): type %d not implemented",
			    opt->desc->type);
#endif
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	 }

      } else if (opt->desc->func == OFUNC_SOCKOPT_APPEND) {
	 switch (opt->desc->type) {
	    uint8_t data[256];
	    socklen_t oldlen, newlen;
	 case TYPE_BIN:
	    oldlen = sizeof(data);
	    if (Getsockopt(fd, opt->desc->major, opt->desc->minor,
			   data, &oldlen)
		< 0) {
	       Error6("getsockopt(%d, %d, %d, %p, {"F_socklen"}): %s",
		      fd, opt->desc->major, opt->desc->minor, data, oldlen,
		      strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    memcpy(&data[oldlen], opt->value.u_bin.b_data,
		   MIN(opt->value.u_bin.b_len, sizeof(data)-oldlen));
	    newlen = oldlen + MIN(opt->value.u_bin.b_len, sizeof(data)-oldlen);
	    if (Setsockopt(fd, opt->desc->major, opt->desc->minor,
			   data, newlen)
		< 0) {
	       Error6("setsockopt(%d, %d, %d, %p, %d): %s",
		      fd, opt->desc->major, opt->desc->minor, data, newlen,
		      strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 default:
	    Error2("internal: option \"%s\": unimplemented type %d",
		   opt->desc->defname, opt->desc->type);
	    break;
	 }
      } else if (opt->desc->func == OFUNC_SOCKOPT_GENERIC) {
	 switch (opt->desc->type) {
	 case TYPE_INT_INT_INT:
	    if (Setsockopt(fd, opt->value.u_int, opt->value2.u_int,
			   &opt->value3.u_int, sizeof(int)) < 0) {
	       Error6("setsockopt(%d, %d, %d, {%d}, "F_Zu"): %s",
		      fd, opt->value.u_int, opt->value2.u_int,
		      opt->value3.u_int, sizeof(int), strerror(errno));
	    }
	    break;
	 case TYPE_INT_INT_BIN:
	    if (Setsockopt(fd, opt->value.u_int, opt->value2.u_int,
			   opt->value3.u_bin.b_data, opt->value3.u_bin.b_len) < 0) {
	       Error5("setsockopt(%d, %d, %d, {...}, "F_Zu"): %s",
		      fd, opt->value.u_int, opt->value2.u_int,
		      opt->value3.u_bin.b_len, strerror(errno));
	    }
	    break;
	 case TYPE_INT_INT_STRING:
	    if (Setsockopt(fd, opt->value.u_int, opt->value2.u_int,
			   opt->value3.u_string,
			   strlen(opt->value3.u_string)+1) < 0) {
	       Error6("setsockopt(%d, %d, %d, \"%s\", "F_Zu"): %s",
		      fd, opt->value.u_int, opt->value2.u_int,
		      opt->value3.u_string, strlen(opt->value3.u_string)+1,
		      strerror(errno));
	    }
	    break;
	 default:
	    Error1("setsockopt() data type %d not implemented",
		   opt->desc->type);
	 }
#endif /* _WITH_SOCKET */
	 
#if HAVE_FLOCK
      } else if (opt->desc->func == OFUNC_FLOCK) {
	 if (Flock(fd, opt->desc->major) < 0) {
	    Error3("flock(%d, %d): %s",
		   fd, opt->desc->major, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
#endif /* defined(HAVE_FLOCK) */

      } else if (opt->desc->func == OFUNC_SPEC ||
		 opt->desc->func == OFUNC_FLAG) {
	 switch (opt->desc->optcode) {
	 case OPT_USER:
	 case OPT_USER_LATE:
	    if (Fchown(fd, opt->value.u_uidt, -1) < 0) {
	       Error3("fchown(%d, "F_uid", -1): %s",
		      fd, opt->value.u_uidt, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case OPT_GROUP:
	 case OPT_GROUP_LATE:
	    if (Fchown(fd, -1, opt->value.u_gidt) < 0) {
	       Error3("fchown(%d, -1, "F_gid"): %s",
		      fd, opt->value.u_gidt, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case OPT_PERM:
	 case OPT_PERM_LATE:
	    if (Fchmod(fd, opt->value.u_modet) < 0) {
	       Error3("fchmod(%d, %u): %s",
		      fd, opt->value.u_modet, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case OPT_FTRUNCATE32:
	    if (Ftruncate(fd, opt->value.u_off) < 0) {
	       Error3("ftruncate(%d, "F_off"): %s",
		      fd, opt->value.u_off, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
#if HAVE_FTRUNCATE64
	 case OPT_FTRUNCATE64:
	    if (Ftruncate64(fd, opt->value.u_off64) < 0) {
	       Error3("ftruncate64(%d, "F_off64"): %s",
		      fd, opt->value.u_off64, strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
#endif /* HAVE_FTRUNCATE64 */
	    break; 
	 case OPT_F_SETLK_RD:
	 case OPT_F_SETLK_WR:
	 case OPT_F_SETLKW_RD:
	 case OPT_F_SETLKW_WR:
	    {
	       struct flock l;	/* Linux: <asm/fcntl.h> */
	       l.l_type   = opt->desc->minor;
	       l.l_whence = SEEK_SET;
	       l.l_start  = 0;
	       l.l_len    = LONG_MAX;
	       l.l_pid    = 0;	/* hope this uses our current process */
	       if (Fcntl_lock(fd, opt->desc->major, &l) < 0) {
		  Error3("fcntl(%d, %d, {type=F_WRLCK,whence=SEEK_SET,start=0,len=LONG_MAX,pid=0}): %s", fd, opt->desc->major, strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	    }
	    break; 
	 case OPT_SETUID_EARLY:
	 case OPT_SETUID:
	    if (Setuid(opt->value.u_uidt) < 0) {
	       Error2("setuid("F_uid"): %s", opt->value.u_uidt,
		      strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case OPT_SETGID_EARLY:
	 case OPT_SETGID:
	    if (Setgid(opt->value.u_gidt) < 0) {
	       Error2("setgid("F_gid"): %s", opt->value.u_gidt,
		      strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    break;
	 case OPT_SUBSTUSER_EARLY:
	 case OPT_SUBSTUSER:
	    {
	       struct passwd *pwd;
	       if ((pwd = getpwuid(opt->value.u_uidt)) == NULL) {
		  Error1("getpwuid("F_uid"): no such user",
			 opt->value.u_uidt);
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       if (Initgroups(pwd->pw_name, pwd->pw_gid) < 0) {
		  Error3("initgroups(%s, "F_gid"): %s",
			 pwd->pw_name, pwd->pw_gid, strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       if (Setgid(pwd->pw_gid) < 0) {
		  Error2("setgid("F_gid"): %s", pwd->pw_gid,
			 strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       if (Setuid(opt->value.u_uidt) < 0) {
		  Error2("setuid("F_uid"): %s", opt->value.u_uidt,
			 strerror(errno));
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
#if 1
	       if (setenv("USER", pwd->pw_name, 1) < 0)
		  Error1("setenv(\"USER\", \"%s\", 1): insufficient space",
			 pwd->pw_name);
	       if (setenv("LOGNAME", pwd->pw_name, 1) < 0)
		  Error1("setenv(\"LOGNAME\", \"%s\", 1): insufficient space",
			 pwd->pw_name);
	       if (setenv("HOME", pwd->pw_dir, 1) < 0)
		  Error1("setenv(\"HOME\", \"%s\", 1): insufficient space",
			 pwd->pw_dir);
	       if (setenv("SHELL", pwd->pw_shell, 1) < 0)
		  Error1("setenv(\"SHELL\", \"%s\", 1): insufficient space",
			 pwd->pw_shell);
#endif
	    }
	    break;
#if defined(HAVE_SETGRENT) && defined(HAVE_GETGRENT) && defined(HAVE_ENDGRENT)
	 case OPT_SUBSTUSER_DELAYED:
	    {
	       struct passwd *pwd;

	       if ((pwd = getpwuid(opt->value.u_uidt)) == NULL) {
		  Error1("getpwuid("F_uid"): no such user",
			 opt->value.u_uidt);
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       delayeduser_uid = opt->value.u_uidt;
	       delayeduser_gid = pwd->pw_gid;
	       if ((delayeduser_name = strdup(pwd->pw_name)) == NULL) {
		  Error1("strdup("F_Zu"): out of memory",
			 strlen(pwd->pw_name)+1);
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       if ((delayeduser_dir = strdup(pwd->pw_dir)) == NULL) {
		  Error1("strdup("F_Zu"): out of memory",
			 strlen(pwd->pw_dir)+1);
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       if ((delayeduser_shell = strdup(pwd->pw_shell)) == NULL) {
		  Error1("strdup("F_Zu"): out of memory",
			 strlen(pwd->pw_shell)+1);
		  opt->desc = ODESC_ERROR; ++opt; continue;
	       }
	       /* function to get all supplementary groups of user */
	       delayeduser_ngids = sizeof(delayeduser_gids)/sizeof(gid_t);
	       getusergroups(delayeduser_name, delayeduser_gids,
			     &delayeduser_ngids);
	       delayeduser = true;
	    }
	    break;
#endif
	 case OPT_CHROOT_EARLY:
	 case OPT_CHROOT:
	    if (Chroot(opt->value.u_string) < 0) {
	       Error2("chroot(\"%s\"): %s", opt->value.u_string,
		      strerror(errno));
	       opt->desc = ODESC_ERROR; ++opt; continue;
	    }
	    if (Chdir("/") < 0) {
	       Error1("chdir(\"/\"): %s", strerror(errno));
	    }
	    break;
	 case OPT_SETSID:
	    if (Setsid() < 0) {
	       Warn1("setsid(): %s", strerror(errno));
	       if (Setpgid(getpid(), getppid()) < 0) {
		  Warn3("setpgid(%d, %d): %s",
			getpid(), getppid(), strerror(errno));
	       } else {
		  if (Setsid() < 0) {
		     Error1("setsid(): %s", strerror(errno));
		  }		  
	       }
	    }
	    break;
	 case OPT_SETPGID:
	    if (Setpgid(0, opt->value.u_int) < 0) {
	       Warn2("setpgid(0, "F_pid"): %s",
		     opt->value.u_int, strerror(errno));
	    }
	    break;
	 case OPT_TIOCSCTTY:
	    {
	       int mytty;
	       /* this code idea taken from ssh/pty.c: make pty controlling term. */
	       if ((mytty = Open("/dev/tty", O_NOCTTY, 0640)) < 0) {
		  Warn1("open(\"/dev/tty\", O_NOCTTY, 0640): %s", strerror(errno));
	       } else {
		  /*0 Info1("open(\"/dev/tty\", O_NOCTTY, 0640) -> %d", mytty);*/
#ifdef TIOCNOTTY
		  if (Ioctl(mytty, TIOCNOTTY, NULL) < 0) {
		     Warn2("ioctl(%d, TIOCNOTTY, NULL): %s",
			   mytty, strerror(errno));
		  }
#endif
		  if (Close(mytty) < 0) {
		     Info2("close(%d): %s",
			   mytty, strerror(errno));
		  }
	       }
#ifdef TIOCSCTTY
	       if (Ioctl(fd, TIOCSCTTY, NULL) < 0) {
		  Warn2("ioctl(%d, TIOCSCTTY, NULL): %s", fd, strerror(errno));
	       }
#endif
	       if (Tcsetpgrp(0, getpid()) < 0) {
		  Warn2("tcsetpgrp("F_pid"): %s", getpid(), strerror(errno));
	       }
	    }
	    break;

	 default: Error1("applyopts(): option \"%s\" not implemented",
			 opt->desc->defname);
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }

#if WITH_TERMIOS
      } else if (opt->desc->func == OFUNC_TERMIOS_FLAG) {
#if 0
	 union {
	    struct termios termarg;
	    tcflag_t flags[4];
	 } tdata;
	 if (Tcgetattr(fd, &tdata.termarg) < 0) {
	    Error3("tcgetattr(%d, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
	 if (opt->value.u_bool) {
	    tdata.flags[opt->desc->major] |= opt->desc->minor;
	 } else {
	    tdata.flags[opt->desc->major] &= ~opt->desc->minor;
	 }
	 if (Tcsetattr(fd, TCSADRAIN, &tdata.termarg) < 0) {
	    Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
#else
	 if (xiotermiosflag_applyopt(fd, opt) < 0) {
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
#endif

      } else if (opt->desc->func == OFUNC_TERMIOS_VALUE) {
	 union {
	    struct termios termarg;
	    tcflag_t flags[4];
	 } tdata;
	 if (((opt->value.u_uint << opt->desc->arg3) & opt->desc->minor) !=
	     (opt->value.u_uint << opt->desc->arg3)) {
	    Error2("option %s: invalid value %u",
		   opt->desc->defname, opt->value.u_uint);
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
	 if (Tcgetattr(fd, &tdata.termarg) < 0) {
	    Error3("tcgetattr(%d, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }

	 tdata.flags[opt->desc->major] &= ~opt->desc->minor;
	 tdata.flags[opt->desc->major] |=
	    ((opt->value.u_uint << opt->desc->arg3) & opt->desc->minor);
	 if (Tcsetattr(fd, TCSADRAIN, &tdata.termarg) < 0) {
	    Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }

      } else if (opt->desc->func == OFUNC_TERMIOS_PATTERN) {
	 union {
	    struct termios termarg;
	    tcflag_t flags[4];
	 } tdata;
	 if (Tcgetattr(fd, &tdata.termarg) < 0) {
	    Error3("tcgetattr(%d, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
	 tdata.flags[opt->desc->major] &= ~opt->desc->arg3;
	 tdata.flags[opt->desc->major] |= opt->desc->minor;
	 if (Tcsetattr(fd, TCSADRAIN, &tdata.termarg) < 0) {
	    Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR;++opt;  continue;
	 }

      } else if (opt->desc->func == OFUNC_TERMIOS_CHAR) {
	 struct termios termarg;
	 if (Tcgetattr(fd, &termarg) < 0) {
	    Error3("tcgetattr(%d, %p): %s",
		   fd, &termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
	 termarg.c_cc[opt->desc->major] = opt->value.u_byte;
	 if (Tcsetattr(fd, TCSADRAIN, &termarg) < 0) {
	    Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
		   fd, &termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }

#ifdef HAVE_TERMIOS_ISPEED
      } else if (opt->desc->func == OFUNC_TERMIOS_SPEED) {
	 union {
	    struct termios termarg;
	    speed_t speeds[sizeof(struct termios)/sizeof(speed_t)];
	 } tdata;
	 if (Tcgetattr(fd, &tdata.termarg) < 0) {
	    Error3("tcgetattr(%d, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
	 tdata.speeds[opt->desc->major] = opt->value.u_uint;
	 if (Tcsetattr(fd, TCSADRAIN, &tdata.termarg) < 0) {
	    Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
		   fd, &tdata.termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
#endif /* HAVE_TERMIOS_ISPEED */

      } else if (opt->desc->func == OFUNC_TERMIOS_SPEC) {
	 struct termios termarg;
	 if (Tcgetattr(fd, &termarg) < 0) {
	    Error3("tcgetattr(%d, %p): %s",
		   fd, &termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
	 switch (opt->desc->optcode) {
	 case OPT_RAW:
	    termarg.c_iflag &=
	      ~(IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF
#ifdef IUCLC
		|IUCLC
#endif
		|IXANY|IMAXBEL);
	    termarg.c_iflag |= (0);
	    termarg.c_oflag &= ~(OPOST);
	    termarg.c_oflag |= (0);
	    termarg.c_cflag &= ~(0);
	    termarg.c_cflag |= (0);
	    termarg.c_lflag &= ~(ISIG|ICANON
#ifdef XCASE
				 |XCASE
#endif
				 );
	    termarg.c_lflag |= (0);
	    termarg.c_cc[VMIN] = 1;
	    termarg.c_cc[VTIME] = 0;
	    break;
	 case OPT_TERMIOS_RAWER:
	    termarg.c_iflag = 0;
	    termarg.c_oflag = 0;
	    termarg.c_lflag = 0;
	    termarg.c_cflag = (CS8);
	    termarg.c_cc[VMIN] = 1;
	    termarg.c_cc[VTIME] = 0;
	    break;
	 case OPT_SANE:
	    /* cread -ignbrk brkint  -inlcr  -igncr  icrnl
              -ixoff  -iuclc  -ixany  imaxbel opost -olcuc -ocrnl
              onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0
              vt0 ff0 isig icanon iexten echo echoe echok -echonl
              -noflsh -xcase -tostop -echoprt echoctl echoke, and
              also  sets  all special characters to their default
              values.
*/
	    termarg.c_iflag &= ~(IGNBRK|INLCR|IGNCR|IXOFF
#ifdef IUCLC
				 |IUCLC
#endif
				 |IXANY);
	    termarg.c_iflag |= (BRKINT|ICRNL|IMAXBEL);
	    termarg.c_oflag &= ~(0	/* for canonical reasons */
#ifdef OLCUC
				 |OLCUC
#endif
#ifdef OCRNL
				 |OCRNL
#endif
#ifdef ONOCR
				 |ONOCR
#endif
#ifdef ONLRET
				 |ONLRET
#endif
#ifdef OFILL
				 |OFILL
#endif
#ifdef OFDEL
				 |OFDEL
#endif
#ifdef NLDLY
				 |NLDLY
#endif
#ifdef CRDLY
				 |CRDLY
#endif
#ifdef TABDLY
				 |TABDLY
#endif
#ifdef BSDLY
				 |BSDLY
#endif
#ifdef VTDLY
				 |VTDLY
#endif
#ifdef FFDLY
				 |FFDLY
#endif
				 );
	    termarg.c_oflag |= (OPOST|ONLCR
#ifdef NL0
				|NL0
#endif
#ifdef CR0
				|CR0
#endif
#ifdef TAB0
				|TAB0
#endif
#ifdef BS0
				|BS0
#endif
#ifdef VT0
				|VT0
#endif
#ifdef FF0
				|FF0
#endif
				);
	    termarg.c_cflag &= ~(0);
	    termarg.c_cflag |= (CREAD);
	    termarg.c_lflag &= ~(ECHONL|NOFLSH
#ifdef XCASE
				 |XCASE
#endif
				 |TOSTOP
#ifdef ECHOPRT
				 |ECHOPRT
#endif
				 );
	    termarg.c_lflag |= (ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE);
	    /*! "sets characters to their default values... - which? */
	    break;
	 case OPT_TERMIOS_CFMAKERAW:
#if HAVE_CFMAKERAW
	    cfmakeraw(&termarg);
#else
	    /* these setting follow the Linux documenation of cfmakeraw */
	    termarg.c_iflag &=
	      ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	    termarg.c_oflag &= ~(OPOST);
	    termarg.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	    termarg.c_cflag &= ~(CSIZE|PARENB);
	    termarg.c_cflag |= (CS8);
#endif
	    break;
	 default:
	    Error("TERMIOS option not handled - internal error?");
	 }
	 if (Tcsetattr(fd, TCSADRAIN, &termarg) < 0) {
	    Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
		   fd, &termarg, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }

#endif /* WITH_TERMIOS */

#if WITH_STREAMS
#define ENABLE_APPLYOPT
#include "xio-streams.c"
#undef ENABLE_APPLYOPT
#endif /* WITH_STREAMS */

      } else {
	/*Error1("applyopts(): function %d not implemented",
	  opt->desc->func);*/
	 if (opt->desc->func != OFUNC_EXT && opt->desc->func != OFUNC_SIGNAL) {
	    Error1("applyopts(): option \"%s\" does not apply",
		   opt->desc->defname);
	    opt->desc = ODESC_ERROR;
	    ++opt;
	    continue;
	 }
	 ++opt;
	 continue;
      }
      opt->desc = ODESC_DONE;
      ++opt;
   }
   return 0;
}

/* applies to fd all options belonging to phases */
/* note: not all options can be applied this way (e.g. OFUNC_SPEC with PH_OPEN)
   implemented are: OFUNC_FCNTL, OFUNC_SOCKOPT (probably not all types),
   OFUNC_TERMIOS_FLAG, OFUNC_TERMIOS_PATTERN, and some OFUNC_SPEC */
int applyopts2(int fd, struct opt *opts, unsigned int from, unsigned int to) {
   unsigned int i;
   int stat;

   for (i = from; i <= to; ++i) {
      if ((stat = applyopts(fd, opts, i)) < 0)
	 return stat;
   }
   return 0;
}

/* apply and consume all options of type FLAG and group.
   Return 0 if everything went right, or -1 if an error occurred. */
int applyopts_flags(struct opt *opts, int group, flags_t *result) {
   struct opt *opt = opts;

   if (!opts)  return 0;

   while (opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && 
	  (opt->desc->group & group)) {
	 if (opt->desc->func == OFUNC_FLAG) {
	    if (opt->value.u_bool) {
	       *result |= opt->desc->major;
	    } else {
	       *result &= ~opt->desc->major;
	    }
	    opt->desc = ODESC_DONE;
	 } else if (opt->desc->func == OFUNC_FLAG_PATTERN) {
	    *result &= ~opt->desc->minor;
	    *result |= opt->desc->major;
	    opt->desc = ODESC_DONE;
	 }
      }
      ++opt;
   }
   return 0;
}



/* set the FD_CLOEXEC fcntl if the options do not set it to 0 */
int applyopts_cloexec(int fd, struct opt *opts) {
   bool docloexec = 1;

   if (!opts)  return 0;

   retropt_bool(opts, OPT_CLOEXEC, &docloexec);
   if (docloexec) {
      if (Fcntl_l(fd, F_SETFD, FD_CLOEXEC) < 0) {
	 Warn2("fcntl(%d, F_SETFD, FD_CLOEXEC): %s", fd, strerror(errno));
      }
   }
   return 0;
}

int applyopts_fchown(int fd, struct opt *opts) {
   uid_t user = -1;
   gid_t group = -1;

   retropt_uidt(opts, OPT_USER, &user);
   retropt_gidt(opts, OPT_GROUP, &group);

   if (user != (uid_t)-1 || group != (gid_t)-1) {
      if (Fchown(fd, user, group) < 0) {
	 Error4("fchown(%d, "F_uid", "F_gid"): %s", fd, user, group,
		strerror(errno));
	 return STAT_RETRYLATER;
      }
   }
   return 0;
}

/* caller must make sure that option is not yet consumed */
static int applyopt_offset(struct single *xfd, struct opt *opt) {
   unsigned char *ptr;

   ptr = (unsigned char *)xfd + opt->desc->major;
   switch (opt->desc->type) {
   case TYPE_BOOL:
      *(bool *)ptr = opt->value.u_bool;  break;
   case TYPE_INT:
      *(int *)ptr = opt->value.u_int;  break;
   case TYPE_DOUBLE:
      *(double *)ptr = opt->value.u_double;  break;
   case TYPE_TIMEVAL:
      *(struct timeval *)ptr = opt->value.u_timeval;  break;
   case TYPE_STRING_NULL:
      if (opt->value.u_string == NULL) {
	 *(char **)ptr = NULL;
	 break;
      }
      /* PASSTHROUGH */
   case TYPE_STRING:
      if ((*(char **)ptr = strdup(opt->value.u_string)) == NULL) {
	 Error1("strdup("F_Zu"): out of memory",
		strlen(opt->value.u_string)+1);
      }
      break;
   case TYPE_CONST:
      *(int *)ptr = opt->desc->minor;
      break;
   default:
      Error1("applyopt_offset(): type %d not implemented",
	     opt->desc->type);
      return -1;
   }
   opt->desc = ODESC_DONE;
   return 0;
}

int applyopts_offset(struct single *xfd, struct opt *opts) {
   struct opt *opt;

   opt = opts; while (opt->desc != ODESC_END) {
      if ((opt->desc == ODESC_DONE) ||
	  opt->desc->func != OFUNC_OFFSET)  {
	 ++opt; continue; }

      applyopt_offset(xfd, opt);
      opt->desc = ODESC_DONE;
      ++opt;
   }
   return 0;
}

/* applies to xfd all OFUNC_EXT options belonging to phase
   returns -1 if an error occurred */
int applyopts_single(struct single *xfd, struct opt *opts, enum e_phase phase) {
   struct opt *opt;
   int lockrc;

   if (!opts)  return 0;

   opt = opts; while (opt->desc != ODESC_END) {
      if ((opt->desc == ODESC_DONE) ||
	  (opt->desc->phase != phase && phase != PH_ALL)) {
	 /* option not handled in this function */
	 ++opt; continue;
      } else {
     switch (opt->desc->func) {

     case OFUNC_OFFSET:
	applyopt_offset(xfd, opt);
	break;

     case OFUNC_EXT:
      switch (opt->desc->optcode) {
#if 0
      case OPT_IGNOREEOF:
	 xfd->ignoreeof = true;
	 break;
      case OPT_CR:
	 xfd->lineterm = LINETERM_CR;
	 break;
      case OPT_CRNL:
	 xfd->lineterm = LINETERM_CRNL;
	 break;
#endif /* 0 */
      case OPT_READBYTES:
	 xfd->readbytes = opt->value.u_sizet;
	 xfd->actbytes  = xfd->readbytes;
	 break;
      case OPT_LOCKFILE:
	 if (xfd->lock.lockfile) {
	    Error("only one use of options lockfile and waitlock allowed");
	 }
	 xfd->lock.lockfile = strdup(opt->value.u_string);
	 xfd->lock.intervall.tv_sec  = 1;
	 xfd->lock.intervall.tv_nsec = 0;

	 if ((lockrc = xiolock(&xfd->lock)) < 0) {
	    /* error message already printed */
	    return -1;
	 }
	 if (lockrc) {
	    Error1("could not obtain lock \"%s\"", xfd->lock.lockfile);
	 } else {
	    xfd->havelock = true;
	 }
	 break;
      case OPT_WAITLOCK:
	 if (xfd->lock.lockfile) {
	    Error("only one use of options lockfile and waitlock allowed");
	 }
	 xfd->lock.lockfile = strdup(opt->value.u_string);
	 xfd->lock.waitlock = true;
	 xfd->lock.intervall.tv_sec  = 1;
	 xfd->lock.intervall.tv_nsec = 0;

	 /*! this should be integrated into central select()/poll() loop */
	 if (xiolock(&xfd->lock) < 0) {
	    return -1;
	 }
	 xfd->havelock = true;
	 break;
	 
      default:
	 /* just store the value in the correct component of struct single */
	 if (opt->desc->type == TYPE_CONST) {
	    /* only for integral types compatible to int */
	    *(int *)(&((char *)xfd)[opt->desc->major]) = opt->desc->arg3;
	 } else {
	    memcpy(&((char *)xfd)[opt->desc->major], &opt->value, opt->desc->minor);
	 }
      }
      break;

     case OFUNC_OFFSET_MASKS:
	{
	   void *masks = (char *)xfd + opt->desc->major;
	   size_t masksize = opt->desc->minor;
	   unsigned long bit = opt->desc->arg3;
	   switch (masksize>>1) {
	   case sizeof(uint16_t):
	      if (opt->value.u_bool) {
		 ((uint16_t *)masks)[0] |= bit;
	      } else {
		 ((uint16_t *)masks)[1] |= bit;
	      }
	      break;
	   case sizeof(uint32_t):
	      if (opt->value.u_bool) {
		 ((uint32_t *)masks)[0] |= bit;
	      } else {
		 ((uint32_t *)masks)[1] |= bit;
	      }
	      break;
	   default:
	      Info1("sizeof(uint32_t)="F_Zu, sizeof(uint32_t));
	      Error1("applyopts_single: masksize "F_Zu" not implemented",
		     masksize);
	   }
	}
	break;

#if _WITH_SOCKET
     case OFUNC_SOCKOPT:
	 switch (opt->desc->optcode) {
#if WITH_IP4 && (defined(HAVE_STRUCT_IP_MREQ) || defined (HAVE_STRUCT_IP_MREQN))
	 case OPT_IP_ADD_MEMBERSHIP:
	    {
	       union {
#if HAVE_STRUCT_IP_MREQN
		  struct ip_mreqn mreqn;
#endif
		  struct ip_mreq  mreq;
	       } ip4_mreqn = {{{0}}};
	       /* IPv6 not supported - seems to have different handling */
/*
mc:addr:ifname|ifind
mc:ifname|ifind
mc:addr
*/
	       union sockaddr_union sockaddr1;
	       socklen_t socklen1 = sizeof(sockaddr1.ip4);
	       union sockaddr_union sockaddr2;
	       socklen_t socklen2 = sizeof(sockaddr2.ip4);

	       /* first parameter is alway multicast address */
	       /*! result */
	       xiogetaddrinfo(opt->value.u_ip_mreq.multiaddr, NULL,
			      xfd->para.socket.la.soa.sa_family,
			      SOCK_DGRAM, IPPROTO_IP,
			      &sockaddr1, &socklen1, 0, 0);
	       ip4_mreqn.mreq.imr_multiaddr = sockaddr1.ip4.sin_addr;
	       if (0) {
		  ;	/* for canonical reasons */
#if HAVE_STRUCT_IP_MREQN
	       } else if (opt->value.u_ip_mreq.ifindex[0] != '\0') {
		  /* three parameters */
		  /* second parameter is interface address */
		  xiogetaddrinfo(opt->value.u_ip_mreq.param2, NULL,
				 xfd->para.socket.la.soa.sa_family,
				 SOCK_DGRAM, IPPROTO_IP,
				 &sockaddr2, &socklen2, 0, 0);
		  ip4_mreqn.mreq.imr_interface = sockaddr2.ip4.sin_addr;
		  /* third parameter is interface */
		  if (ifindex(opt->value.u_ip_mreq.ifindex,
			      (unsigned int *)&ip4_mreqn.mreqn.imr_ifindex, -1)
		      < 0) {
		     Error1("cannot resolve interface \"%s\"", 
			    opt->value.u_ip_mreq.ifindex);
		  }
#endif /* HAVE_STRUCT_IP_MREQN */
	       } else {
		  /* two parameters */
		  if (0) {
		     ;	/* for canonical reasons */
#if HAVE_STRUCT_IP_MREQN
		  /* there is a form with two parameters that uses mreqn */
		  } else if (ifindex(opt->value.u_ip_mreq.param2,
				     (unsigned int *)&ip4_mreqn.mreqn.imr_ifindex,
				     -1)
			     >= 0) {
		     /* yes, second param converts to interface */
		     ip4_mreqn.mreq.imr_interface.s_addr = htonl(0);
#endif /* HAVE_STRUCT_IP_MREQN */
		  } else {
		     /*! result */
		     xiogetaddrinfo(opt->value.u_ip_mreq.param2, NULL,
				    xfd->para.socket.la.soa.sa_family,
				    SOCK_DGRAM, IPPROTO_IP,
				    &sockaddr2, &socklen2, 0, 0);
		     ip4_mreqn.mreq.imr_interface = sockaddr2.ip4.sin_addr;
		  }
	       }
			
#if LATER
	       if (0) {
		  ; /* for canonical reasons */
	       } else if (xfd->para.socket.la.soa.sa_family == PF_INET) {
	       } else if (xfd->para.socket.la.soa.sa_family == PF_INET6) {
		  ip6_mreqn.mreq.imr_multiaddr = sockaddr1.ip6.sin6_addr;
		  ip6_mreqn.mreq.imr_interface = sockaddr2.ip6.sin6_addr;
	       }
#endif

#if HAVE_STRUCT_IP_MREQN
	       if (Setsockopt(xfd->fd, opt->desc->major, opt->desc->minor,
			      &ip4_mreqn.mreqn, sizeof(ip4_mreqn.mreqn)) < 0) {
		  Error8("setsockopt(%d, %d, %d, {0x%08x,0x%08x,%d}, "F_Zu"): %s",
			 xfd->fd, opt->desc->major, opt->desc->minor,
			 ip4_mreqn.mreqn.imr_multiaddr.s_addr,
			 ip4_mreqn.mreqn.imr_address.s_addr,
			 ip4_mreqn.mreqn.imr_ifindex,
			 sizeof(ip4_mreqn.mreqn),
			 strerror(errno));
		  opt->desc = ODESC_ERROR; continue;
	       }
#else
	       if (Setsockopt(xfd->fd, opt->desc->major, opt->desc->minor,
			      &ip4_mreqn.mreq, sizeof(ip4_mreqn.mreq)) < 0) {
		  Error7("setsockopt(%d, %d, %d, {0x%08x,0x%08x}, "F_Zu"): %s",
			 xfd->fd, opt->desc->major, opt->desc->minor,
			 ip4_mreqn.mreq.imr_multiaddr,
			 ip4_mreqn.mreq.imr_interface,
			 sizeof(ip4_mreqn.mreq),
			 strerror(errno));
		  opt->desc = ODESC_ERROR; continue;
	       }
#endif
	       break;
	    }
	   break;
#endif /* WITH_IP4 && (defined(HAVE_STRUCT_IP_MREQ) || defined (HAVE_STRUCT_IP_MREQN)) */



#if WITH_IP6 && defined(HAVE_STRUCT_IPV6_MREQ)
	 case OPT_IPV6_JOIN_GROUP:
	    {
	       struct ipv6_mreq ip6_mreq = {{{{0}}}};
	       union sockaddr_union sockaddr1;
	       socklen_t socklen1 = sizeof(sockaddr1.ip6);

	       /* always two parameters */
	       /* first parameter is multicast address */
	       /*! result */
	       xiogetaddrinfo(opt->value.u_ip_mreq.multiaddr, NULL,
			      xfd->para.socket.la.soa.sa_family,
			      SOCK_DGRAM, IPPROTO_IP,
			      &sockaddr1, &socklen1, 0, 0);
	       ip6_mreq.ipv6mr_multiaddr = sockaddr1.ip6.sin6_addr;
	       if (ifindex(opt->value.u_ip_mreq.param2,
			   &ip6_mreq.ipv6mr_interface, -1)
		   < 0) {
		  Error1("interface \"%s\" not found",
			 opt->value.u_ip_mreq.param2);
		  ip6_mreq.ipv6mr_interface = htonl(0);
	       }

	       if (Setsockopt(xfd->fd, opt->desc->major, opt->desc->minor,
			      &ip6_mreq, sizeof(ip6_mreq)) < 0) {
		  Error6("setsockopt(%d, %d, %d, {...,0x%08x}, "F_Zu"): %s",
			 xfd->fd, opt->desc->major, opt->desc->minor,
			 ip6_mreq.ipv6mr_interface,
			 sizeof(ip6_mreq),
			 strerror(errno));
		  opt->desc = ODESC_ERROR; continue;
	       }
	    }
	   break;
#endif /* WITH_IP6 && defined(HAVE_STRUCT_IPV6_MREQ) */
	default:
	   /* ignore here */
	   ++opt; continue;
	}
	break;
#endif /* _WITH_SOCKET */

     default:
	++opt;
	continue;
     }
     opt->desc = ODESC_DONE;
     ++opt;
      }
   }
   return 0;
}


/* xfd->para.exec.pid must be set */
int applyopts_signal(struct single *xfd, struct opt *opts) {
   struct opt *opt;

   if (!opts)  return 0;

   opt = opts; while (opt->desc != ODESC_END) {
      if (opt->desc == ODESC_DONE || opt->desc->func != OFUNC_SIGNAL) {
	 ++opt; continue;
      }

      if (xio_opt_signal(xfd->para.exec.pid, opt->desc->major) < 0) {
	 opt->desc = ODESC_ERROR; continue;
      }
      opt->desc = ODESC_DONE;
      ++opt;
   }
   return 0;
}

/* apply remaining options to file descriptor, and tell us if something is
   still unused */
int _xio_openlate(struct single *fd, struct opt *opts) {
   int numleft;
   int result;

   _xioopen_setdelayeduser();

   if ((result = applyopts(fd->fd, opts, PH_LATE)) < 0) {
      return result;
   }
   if ((result = applyopts_single(fd, opts, PH_LATE)) < 0) {
      return result;
   }
   if ((result = applyopts(fd->fd, opts, PH_LATE2)) < 0) {
      return result;
   }

   if ((numleft = leftopts(opts)) > 0) {
      showleft(opts);
      Error1("%d option(s) could not be used", numleft);
      return -1;
   }
   return 0;
}

int dropopts(struct opt *opts, unsigned int phase) {
   struct opt *opt;

   if (phase == PH_ALL) {
      opts[0].desc = ODESC_END;
      return 0;
   }
   opt = opts; while (opt && opt->desc != ODESC_END) {
      if (opt->desc != ODESC_DONE && opt->desc->phase == phase) {
	 Debug1("ignoring option \"%s\"", opt->desc->defname);
	 opt->desc = ODESC_DONE;
      }
      ++opt;
   }
   return 0;
}

int dropopts2(struct opt *opts, unsigned int from, unsigned int to) {
   unsigned int i;

   for (i = from; i <= to; ++i) {
      dropopts(opts, i);
   }
   return 0;
}

