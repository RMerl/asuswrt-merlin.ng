/* source: xio-tcp.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for TCP related functions and options */

#include "xiosysincludes.h"

#if WITH_TCP

#include "xioopen.h"
#include "xio-listen.h"
#include "xio-ip4.h"
#include "xio-ipapp.h"
#include "xio-tcp.h"

/****** TCP addresses ******/

#if WITH_IP4 || WITH_IP6
const struct addrdesc addr_tcp_connect = { "tcp-connect", 1+XIO_RDWR, xioopen_ipapp_connect, GROUP_FD|GROUP_SOCKET|GROUP_SOCK_IP4|GROUP_SOCK_IP6|GROUP_IP_TCP|GROUP_CHILD|GROUP_RETRY, SOCK_STREAM, IPPROTO_TCP, PF_UNSPEC HELP(":<host>:<port>") };
#if WITH_LISTEN
const struct addrdesc addr_tcp_listen  = { "tcp-listen",  1+XIO_RDWR, xioopen_ipapp_listen,  GROUP_FD|GROUP_SOCKET|GROUP_SOCK_IP4|GROUP_SOCK_IP6|GROUP_IP_TCP|GROUP_LISTEN|GROUP_CHILD|GROUP_RANGE|GROUP_RETRY, SOCK_STREAM, IPPROTO_TCP, PF_UNSPEC HELP(":<port>") };
#endif
#endif

#if WITH_IP4
const struct addrdesc addr_tcp4_connect = { "tcp4-connect", 1+XIO_RDWR, xioopen_ipapp_connect, GROUP_FD|GROUP_SOCKET|GROUP_SOCK_IP4|GROUP_IP_TCP|GROUP_CHILD|GROUP_RETRY, SOCK_STREAM, IPPROTO_TCP, PF_INET HELP(":<host>:<port>") };
#if WITH_LISTEN
const struct addrdesc addr_tcp4_listen  = { "tcp4-listen", 1+XIO_RDWR, xioopen_ipapp_listen, GROUP_FD|GROUP_SOCKET|GROUP_SOCK_IP4|GROUP_IP_TCP|GROUP_LISTEN|GROUP_CHILD|GROUP_RANGE|GROUP_RETRY, SOCK_STREAM, IPPROTO_TCP, PF_INET HELP(":<port>") };
#endif
#endif /* WITH_IP4 */

#if WITH_IP6
const struct addrdesc addr_tcp6_connect = { "tcp6-connect", 1+XIO_RDWR, xioopen_ipapp_connect, GROUP_FD|GROUP_SOCKET|GROUP_SOCK_IP6|GROUP_IP_TCP|GROUP_CHILD|GROUP_RETRY, SOCK_STREAM, IPPROTO_TCP, PF_INET6 HELP(":<host>:<port>") };
#if WITH_LISTEN
const struct addrdesc addr_tcp6_listen  = { "tcp6-listen", 1+XIO_RDWR, xioopen_ipapp_listen, GROUP_FD|GROUP_SOCKET|GROUP_SOCK_IP6|GROUP_IP_TCP|GROUP_LISTEN|GROUP_CHILD|GROUP_RANGE|GROUP_RETRY, SOCK_STREAM, IPPROTO_TCP, PF_INET6 HELP(":<port>") };
#endif
#endif /* WITH_IP6 */

/****** TCP address options ******/

#ifdef TCP_NODELAY
const struct optdesc opt_tcp_nodelay = { "tcp-nodelay",   "nodelay", OPT_TCP_NODELAY, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT,	OFUNC_SOCKOPT, SOL_TCP, TCP_NODELAY };
#endif
#ifdef TCP_MAXSEG
const struct optdesc opt_tcp_maxseg  = { "tcp-maxseg",    "mss",  OPT_TCP_MAXSEG,  GROUP_IP_TCP, PH_PASTSOCKET,TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_MAXSEG };
const struct optdesc opt_tcp_maxseg_late={"tcp-maxseg-late","mss-late",OPT_TCP_MAXSEG_LATE,GROUP_IP_TCP,PH_CONNECTED,TYPE_INT,OFUNC_SOCKOPT, SOL_TCP, TCP_MAXSEG};
#endif
#ifdef TCP_CORK
const struct optdesc opt_tcp_cork   = { "tcp-cork",     "cork", OPT_TCP_CORK,    GROUP_IP_TCP, PH_PASTSOCKET,    TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_CORK };
#endif
#ifdef TCP_STDURG
const struct optdesc opt_tcp_stdurg = { "tcp-stdurg",   "stdurg", OPT_TCP_STDURG,  GROUP_IP_TCP, PH_PASTSOCKET,    TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_STDURG };
#endif
#ifdef TCP_RFC1323
const struct optdesc opt_tcp_rfc1323= { "tcp-rfc1323",  "rfc1323", OPT_TCP_RFC1323, GROUP_IP_TCP, PH_PASTSOCKET,    TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_RFC1323};
#endif
#ifdef TCP_KEEPIDLE
const struct optdesc opt_tcp_keepidle={ "tcp-keepidle", "keepidle",OPT_TCP_KEEPIDLE,GROUP_IP_TCP, PH_PASTSOCKET,    TYPE_INT, OFUNC_SOCKOPT, SOL_TCP,TCP_KEEPIDLE};
#endif
#ifdef TCP_KEEPINTVL
const struct optdesc opt_tcp_keepintvl={"tcp-keepintvl","keepintvl",OPT_TCP_KEEPINTVL,GROUP_IP_TCP,PH_PASTSOCKET,    TYPE_INT, OFUNC_SOCKOPT, SOL_TCP,TCP_KEEPINTVL};
#endif
#ifdef TCP_KEEPCNT
const struct optdesc opt_tcp_keepcnt= { "tcp-keepcnt",  "keepcnt",  OPT_TCP_KEEPCNT, GROUP_IP_TCP, PH_PASTSOCKET,    TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_KEEPCNT };
#endif
#ifdef TCP_SYNCNT
const struct optdesc opt_tcp_syncnt = { "tcp-syncnt",   "syncnt",   OPT_TCP_SYNCNT,  GROUP_IP_TCP, PH_PASTSOCKET,    TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_SYNCNT };
#endif
#ifdef TCP_LINGER2
const struct optdesc opt_tcp_linger2= { "tcp-linger2",  "linger2",  OPT_TCP_LINGER2, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_LINGER2 };
#endif
#ifdef TCP_DEFER_ACCEPT
const struct optdesc opt_tcp_defer_accept={"tcp-defer-accept","defer-accept",OPT_TCP_DEFER_ACCEPT,GROUP_IP_TCP,PH_PASTSOCKET,TYPE_INT,OFUNC_SOCKOPT,SOL_TCP,TCP_DEFER_ACCEPT };
#endif
#ifdef TCP_WINDOW_CLAMP
const struct optdesc opt_tcp_window_clamp={"tcp-window-clamp","window-clamp",OPT_TCP_WINDOW_CLAMP,GROUP_IP_TCP,PH_PASTSOCKET,TYPE_INT,OFUNC_SOCKOPT,SOL_TCP,TCP_WINDOW_CLAMP };
#endif
#ifdef TCP_INFO
const struct optdesc opt_tcp_info   = { "tcp-info",     "info", OPT_TCP_INFO,    GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_INFO };
#endif
#ifdef TCP_QUICKACK
const struct optdesc opt_tcp_quickack = { "tcp-quickack", "quickack", OPT_TCP_QUICKACK, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_QUICKACK };
#endif
#ifdef TCP_NOOPT
const struct optdesc opt_tcp_noopt  = { "tcp-noopt",   "noopt",  OPT_TCP_NOOPT,  GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT,	OFUNC_SOCKOPT, SOL_TCP, TCP_NOOPT };
#endif
#ifdef TCP_NOPUSH
const struct optdesc opt_tcp_nopush = { "tcp-nopush",  "nopush", OPT_TCP_NOPUSH, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT,	OFUNC_SOCKOPT, SOL_TCP, TCP_NOPUSH };
#endif
#ifdef TCP_MD5SIG
const struct optdesc opt_tcp_md5sig = { "tcp-md5sig",   "md5sig", OPT_TCP_MD5SIG, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_MD5SIG };
#endif
#ifdef TCP_SACK_DISABLE
const struct optdesc opt_tcp_sack_disable = { "tcp-sack-disable", "sack-disable", OPT_TCP_SACK_DISABLE, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_SACK_DISABLE };
#endif
#ifdef TCP_SIGNATURE_ENABLE
const struct optdesc opt_tcp_signature_enable = { "tcp-signature-enable", "signature-enable", OPT_TCP_SIGNATURE_ENABLE, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_SIGNATURE_ENABLE };
#endif
#ifdef TCP_ABORT_THRESHOLD	/* HP-UX */
const struct optdesc opt_tcp_abort_threshold = { "tcp-abort-threshold", "abort-threshold", OPT_TCP_ABORT_THRESHOLD, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_ABORT_THRESHOLD };
#endif
#ifdef TCP_CONN_ABORT_THRESHOLD	/* HP-UX */
const struct optdesc opt_tcp_conn_abort_threshold = { "tcp-conn-abort-threshold", "conn-abort-threshold", OPT_TCP_CONN_ABORT_THRESHOLD, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_CONN_ABORT_THRESHOLD };
#endif
#ifdef TCP_KEEPINIT	/* OSF1 aka Tru64 */
const struct optdesc opt_tcp_keepinit = { "tcp-keepinit", "keepinit", OPT_TCP_KEEPINIT, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_KEEPINIT };
#endif
#ifdef TCP_PAWS	/* OSF1 aka Tru64 */
const struct optdesc opt_tcp_paws = { "tcp-paws", "paws", OPT_TCP_PAWS, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_PAWS };
#endif
#ifdef TCP_SACKENA	/* OSF1 aka Tru64 */
const struct optdesc opt_tcp_sackena = { "tcp-sackena", "sackena", OPT_TCP_SACKENA, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_SACKENA };
#endif
#ifdef TCP_TSOPTENA	/* OSF1 aka Tru64 */
const struct optdesc opt_tcp_tsoptena = { "tcp-tsoptena", "tsoptena", OPT_TCP_TSOPTENA, GROUP_IP_TCP, PH_PASTSOCKET, TYPE_INT, OFUNC_SOCKOPT, SOL_TCP, TCP_TSOPTENA };
#endif

#endif /* WITH_TCP */
