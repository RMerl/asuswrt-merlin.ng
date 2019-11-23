/* source: xio-ip6.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_ip6_h_included
#define __xio_ip6_h_included 1

#if WITH_IP6

extern const struct optdesc opt_ipv6_v6only;
extern const struct optdesc opt_ipv6_join_group;
extern const struct optdesc opt_ipv6_pktinfo;
extern const struct optdesc opt_ipv6_recvpktinfo;
extern const struct optdesc opt_ipv6_rthdr;
extern const struct optdesc opt_ipv6_recvrthdr;
extern const struct optdesc opt_ipv6_authhdr;
extern const struct optdesc opt_ipv6_dstopts;
extern const struct optdesc opt_ipv6_recvdstopts;
extern const struct optdesc opt_ipv6_hopopts;
extern const struct optdesc opt_ipv6_unicast_hops;
extern const struct optdesc opt_ipv6_recvhopopts;
extern const struct optdesc opt_ipv6_flowinfo;
extern const struct optdesc opt_ipv6_hoplimit;
extern const struct optdesc opt_ipv6_recvhoplimit;
extern const struct optdesc opt_ipv6_recverr;
extern const struct optdesc opt_ipv6_tclass;
extern const struct optdesc opt_ipv6_recvtclass;
extern const struct optdesc opt_ipv6_recvpathmtu;

extern
int xioparsenetwork_ip6(const char *rangename, struct xiorange *range);
extern int xiorange_ip6andmask(struct xiorange *range);

extern
int xiocheckrange_ip6(struct sockaddr_in6 *pa, struct xiorange *range);
extern
int xiolog_ancillary_ip6(struct cmsghdr *cmsg, int *num,
			 char *typbuff, int typlen,
			 char *nambuff, int namlen,
			 char *envbuff, int envlen,
			 char *valbuff, int vallen);
extern int
xiosetsockaddrenv_ip6(int idx, char *namebuff, size_t namelen,
		      char *valuebuff, size_t valuelen,
		      struct sockaddr_in6 *sa, int ipproto);

#endif /* WITH_IP6 */

#endif /* !defined(__xio_ip6_h_included) */
