/* source: xio-socket.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_socket_h_included
#define __xio_socket_h_included 1

/* SO_PROTOTYPE is OS defined on Solaris, HP-UX; we lend this for a more
   general purpose */ 
#ifndef SO_PROTOTYPE
#define SO_PROTOTYPE 0x9999
#endif

extern const struct addrdesc xioaddr_socket_connect;
extern const struct addrdesc xioaddr_socket_listen;
extern const struct addrdesc xioaddr_socket_sendto;
extern const struct addrdesc xioaddr_socket_datagram;
extern const struct addrdesc xioaddr_socket_recvfrom;
extern const struct addrdesc xioaddr_socket_recv;

extern const struct optdesc opt_connect_timeout;
extern const struct optdesc opt_so_debug;
extern const struct optdesc opt_so_acceptconn;
extern const struct optdesc opt_so_broadcast;
extern const struct optdesc opt_so_reuseaddr;
extern const struct optdesc opt_so_keepalive;
extern const struct optdesc opt_so_linger;
extern const struct optdesc opt_so_linger;
extern const struct optdesc opt_so_oobinline;
extern const struct optdesc opt_so_sndbuf;
extern const struct optdesc opt_so_sndbuf_late;
extern const struct optdesc opt_so_rcvbuf;
extern const struct optdesc opt_so_rcvbuf_late;
extern const struct optdesc opt_so_error;
extern const struct optdesc opt_so_type;
extern const struct optdesc opt_so_dontroute;
extern const struct optdesc opt_so_rcvlowat;
extern const struct optdesc opt_so_sndlowat;
extern const struct optdesc opt_so_audit;
extern const struct optdesc opt_so_attach_filter;
extern const struct optdesc opt_so_detach_filter;
extern const struct optdesc opt_so_bindtodevice;
extern const struct optdesc opt_so_bsdcompat;
extern const struct optdesc opt_so_cksumrecv;
extern const struct optdesc opt_so_timestamp;
extern const struct optdesc opt_so_kernaccept;
extern const struct optdesc opt_so_no_check;
extern const struct optdesc opt_so_noreuseaddr;
extern const struct optdesc opt_so_passcred;
extern const struct optdesc opt_so_peercred;
extern const struct optdesc opt_so_priority;
extern const struct optdesc opt_so_reuseport;
extern const struct optdesc opt_so_security_authentication;
extern const struct optdesc opt_so_security_encryption_network;
extern const struct optdesc opt_so_security_encryption_transport;
extern const struct optdesc opt_so_use_ifbufs;
extern const struct optdesc opt_so_useloopback;
extern const struct optdesc opt_so_dgram_errind;
extern const struct optdesc opt_so_dontlinger;
extern const struct optdesc opt_so_prototype;
extern const struct optdesc opt_fiosetown;
extern const struct optdesc opt_siocspgrp;
extern const struct optdesc opt_bind;
extern const struct optdesc opt_protocol_family;
extern const struct optdesc opt_setsockopt_int;
extern const struct optdesc opt_setsockopt_bin;
extern const struct optdesc opt_setsockopt_string;
extern const struct optdesc opt_null_eof;


extern
char *xiogetifname(int ind, char *val, int ins);

extern int retropt_socket_pf(struct opt *opts, int *pf);

extern int xioopen_connect(struct single *fd,
			    struct sockaddr *us, size_t uslen,
			    struct sockaddr *them, size_t themlen,
			    struct opt *opts,
			   int pf, int socktype, int protocol,
			    bool alt);
extern int _xioopen_connect(struct single *fd,
			    struct sockaddr *us, size_t uslen,
			    struct sockaddr *them, size_t themlen,
			    struct opt *opts,
			    int pf, int socktype, int protocol,
			    bool alt, int level);

/* common to xioopen_udp_sendto, ..unix_sendto, ..rawip */
extern 
int _xioopen_dgram_sendto(/* them is already in xfd->peersa */
			union sockaddr_union *us, socklen_t uslen,
			struct opt *opts,
			int xioflags, xiosingle_t *xfd, unsigned groups,
			int pf, int socktype, int ipproto);
extern
int _xioopen_dgram_recvfrom(struct single *xfd, int xioflags,
			    struct sockaddr *us, socklen_t uslen,
			    struct opt *opts,
			    int pf, int socktype, int proto, int level);
extern
int _xioopen_dgram_recv(struct single *xfd, int xioflags,
			struct sockaddr *us, socklen_t uslen,
			struct opt *opts, int pf, int socktype, int proto,
			int level);
extern
int xiodopacketinfo(struct msghdr *msgh, bool withlog, bool withenv);
extern 
int xiogetpacketsrc(int fd, struct msghdr *msgh);
extern
int xiocheckpeer(xiosingle_t *xfd,
		 union sockaddr_union *pa, union sockaddr_union *la);
extern
int xiosetsockaddrenv(const char *lr, union sockaddr_union *sau, socklen_t salen, int proto);

extern
int xioparsenetwork(const char *rangename, int pf,
		    struct xiorange *range);
extern 
int xioparserange(const char *rangename, int pf, struct xiorange *range);

extern int
xiosocket(struct opt *opts, int pf, int socktype, int proto, int level);
extern int 
xiosocketpair(struct opt *opts, int pf, int socktype, int proto, int sv[2]);

#endif /* !defined(__xio_socket_h_included) */
