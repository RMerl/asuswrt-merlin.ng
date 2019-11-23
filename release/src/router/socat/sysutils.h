/* source: sysutils.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __sysutils_h_included
#define __sysutils_h_included 1

#if WITH_IP6
/* not all OSes provide in6_addr that allows splitting to 16 or 32 bit junks of
   the host address part of sockaddr_in6; here we help ourselves */
union xioin6_u {
   uint8_t  u6_addr8[16];
   uint16_t u6_addr16[8];
   uint32_t u6_addr32[4];
} ;
#endif /* WITH_IP6 */

#if _WITH_SOCKET
union sockaddr_union {
   struct sockaddr soa;
#if WITH_UNIX
   struct sockaddr_un un;
#endif /* WITH_UNIX */
#if _WITH_IP4
   struct sockaddr_in ip4;
#endif /* _WITH_IP4 */
#if WITH_IP6
   struct sockaddr_in6 ip6;
#endif /* WITH_IP6 */
#if WITH_INTERFACE
   struct sockaddr_ll ll;
#endif
} ;
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
struct xiorange {
   union sockaddr_union netaddr;
   union sockaddr_union netmask;
} ;
#endif /* _WITH_SOCKET */

extern ssize_t writefull(int fd, const void *buff, size_t bytes);

#if _WITH_SOCKET
extern socklen_t socket_init(int af, union sockaddr_union *sa);
#endif
#if WITH_UNIX
extern void socket_un_init(struct sockaddr_un *sa);
#endif /* WITH_UNIX */
#if _WITH_IP4
extern void socket_in_init(struct sockaddr_in *sa);
#endif /* _WITH_IP4 */
#if _WITH_IP6
extern void socket_in6_init(struct sockaddr_in6 *sa);
#endif /* _WITH_IP4 */

#if _WITH_SOCKET
extern char *sockaddr_info(const struct sockaddr *sa, socklen_t salen, char *buff, size_t blen);
#endif
#if WITH_UNIX
extern char *sockaddr_unix_info(const struct sockaddr_un *sa, socklen_t salen, char *buff, size_t blen);
#endif /* WITH_UNIX */
#if WITH_IP4
extern char *inet4addr_info(uint32_t addr, char *buff, size_t blen);
extern char *sockaddr_inet4_info(const struct sockaddr_in *sa, char *buff, size_t blen);
#endif /* WITH_IP4 */
#if WITH_IP6
extern char *sockaddr_inet6_info(const struct sockaddr_in6 *sa, char *buff, size_t blen);
#endif /* WITH_IP6 */
#if !HAVE_INET_NTOP
extern const char *inet_ntop(int pf, const void *binaddr,
			     char *addrtext, socklen_t textlen);
#endif

#if defined(HAVE_SETGRENT) && defined(HAVE_GETGRENT) && defined(HAVE_ENDGRENT)
extern int getusergroups(const char *user, gid_t *list, int *ngroups);
#endif

#if !HAVE_HSTRERROR
extern const char *hstrerror(int err);
#endif

extern int xiopoll(struct pollfd fds[], unsigned long nfds, struct timeval *timeout);

extern int parseport(const char *portname, int proto);

extern int ifindexbyname(const char *ifname, int anysock);
extern int ifindex(const char *ifname, unsigned int *ifindex, int anysock);

extern int xiosetenv(const char *varname, const char *value, int overwrite, const char *sep);
extern int
xiosetenv2(const char *varname, const char *varname2, const char *value,
	   int overwrite, const char *sep);
extern int
xiosetenv3(const char *varname, const char *varname2, const char *varname3,
	   const char *value, int overwrite, const char *sep);
extern int xiosetenvulong(const char *varname, unsigned long value,
			  int overwrite);
extern int xiosetenvushort(const char *varname, unsigned short value,
			   int overwrite);

#endif /* !defined(__sysutils_h_included) */
