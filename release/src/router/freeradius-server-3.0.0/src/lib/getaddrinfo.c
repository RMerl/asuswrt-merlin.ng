/*
 * These functions are defined and used only if the configure
 * cannot detect the standard getaddrinfo(), freeaddrinfo(),
 * gai_strerror() and getnameinfo(). This avoids sprinkling of ifdefs.
 *
 * FIXME: getaddrinfo() & getnameinfo() should
 *	return all IPv4 addresses provided by DNS lookup.
 */

RCSID("$Id$")

#include	<freeradius-devel/libradius.h>

#include	<ctype.h>
#include 	<sys/param.h>

#ifndef HAVE_GETNAMEINFO
#undef LOCAL_GETHOSTBYNAMERSTYLE
#ifndef GETHOSTBYNAMERSTYLE
#define LOCAL_GETHOSTBYNAMERSTYLE 1
#elif (GETHOSTBYNAMERSTYLE != SYSVSTYLE) && (GETHOSTBYNAMERSTYLE != GNUSTYLE)
#define LOCAL_GETHOSTBYNAMERSTYLE 1
#endif /* GETHOSTBYNAMERSTYLE */
#endif

#ifndef HAVE_GETADDRINFO
#undef LOCAL_GETHOSTBYADDRR
#ifndef GETHOSTBYADDRRSTYLE
#define LOCAL_GETHOSTBYADDRR 1
#elif (GETHOSTBYADDRRSTYLE != SYSVSTYLE) && (GETHOSTBYADDRRSTYLE != GNUSTYLE)
#define LOCAL_GETHOSTBYADDRR 1
#endif /* GETHOSTBYADDRRSTYLE */
#endif

#ifdef HAVE_PTHREAD_H
#include	<pthread.h>

/* Thread safe DNS lookups */
/*
 *	FIXME: There are some systems that use the same hostent
 *	structure to return for gethostbyname() & gethostbyaddr(), if
 *	that is the case then use only one mutex instead of separate
 *	mutexes
 */
#ifdef LOCAL_GETHOSTBYNAMERSTYLE
static int fr_hostbyname = 0;
static pthread_mutex_t fr_hostbyname_mutex;
#endif

#ifdef LOCAL_GETHOSTBYNAMERSTYLE
static int fr_hostbyaddr = 0;
static pthread_mutex_t fr_hostbyaddr_mutex;
#endif

#endif

/*
 * gethostbyaddr() & gethostbyname() return hostent structure
 * To make these functions thread safe, we need to
 * copy the data and not pointers
 *
 * struct hostent {
 *    char    *h_name;	* official name of host *
 *    char    **h_aliases;    * alias list *
 *    int     h_addrtype;     * host address type *
 *    int     h_length;       * length of address *
 *    char    **h_addr_list;  * list of addresses *
 * }
 * This struct contains 3 pointers as members.
 * The data from these pointers is copied into a buffer.
 * The buffer is formatted as below to store the data
 *  ---------------------------------------------------------------
 * | h_name\0alias_array\0h_aliases\0..\0addr_array\0h_addr_list\0 |
 *  ---------------------------------------------------------------
 */
#if defined(LOCAL_GETHOSTBYNAMER) || defined(LOCAL_GETHOSTBYADDRR)
#define BUFFER_OVERFLOW 255
static int copy_hostent(struct hostent *from, struct hostent *to,
			char *buffer, int buflen, int *error)
{
    int i, len;
    char *ptr = buffer;

    *error = 0;
    to->h_addrtype = from->h_addrtype;
    to->h_length = from->h_length;
    to->h_name = (char *)ptr;

    /* copy hostname to buffer */
    len=strlen(from->h_name)+1;
    strcpy(ptr, from->h_name);
    ptr += len;

    /* copy aliases to buffer */
    to->h_aliases = (char**)ptr;
    for(i = 0; from->h_aliases[i]; i++);
    ptr += (i+1) * sizeof(char *);

    for(i = 0; from->h_aliases[i]; i++) {
       len = strlen(from->h_aliases[i])+1;
       if ((ptr-buffer)+len < buflen) {
	   to->h_aliases[i] = ptr;
	       strcpy(ptr, from->h_aliases[i]);
	   ptr += len;
       } else {
	   *error = BUFFER_OVERFLOW;
	   return *error;
       }
    }
    to->h_aliases[i] = NULL;

    /* copy addr_list to buffer */
    to->h_addr_list = (char**)ptr;
    for(i = 0; (int *)from->h_addr_list[i] != 0; i++);
    ptr += (i+1) * sizeof(int *);

    for(i = 0; (int *)from->h_addr_list[i] != 0; i++) {
       len = sizeof(int);
       if ((ptr-buffer)+len < buflen) {
	   to->h_addr_list[i] = ptr;
	   memcpy(ptr, from->h_addr_list[i], len);
	   ptr += len;
       } else {
	   *error = BUFFER_OVERFLOW;
	    return *error;
       }
    }
    to->h_addr_list[i] = 0;
    return *error;
}
#endif /* (LOCAL_GETHOSTBYNAMER == 1) || (LOCAL_GETHOSTBYADDRR == 1) */

#ifdef LOCAL_GETHOSTBYNAMERSTYLE
static struct hostent *
gethostbyname_r(char const *hostname, struct hostent *result,
	   char *buffer, int buflen, int *error)
{
    struct hostent *hp;

#ifdef HAVE_PTHREAD_H
    if (fr_hostbyname == 0) {
    	pthread_mutex_init(&fr_hostbyname_mutex, NULL);
	fr_hostbyname = 1;
    }
    pthread_mutex_lock(&fr_hostbyname_mutex);
#endif

    hp = gethostbyname(hostname);
    if ((!hp) || (hp->h_addrtype != AF_INET) || (hp->h_length != 4)) {
	 *error = h_errno;
	 hp = NULL;
    } else {
	 copy_hostent(hp, result, buffer, buflen, error);
	 hp = result;
    }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&fr_hostbyname_mutex);
#endif

    return hp;
}
#endif /* GETHOSTBYNAMERSTYLE */


#ifdef LOCAL_GETHOSTBYADDRR
static struct hostent *
gethostbyaddr_r(char const *addr, int len, int type, struct hostent *result,
		char *buffer, int buflen, int *error)
{
    struct hostent *hp;

#ifdef HAVE_PTHREAD_H
    if (fr_hostbyaddr == 0) {
    	pthread_mutex_init(&fr_hostbyaddr_mutex, NULL);
	fr_hostbyaddr = 1;
    }
    pthread_mutex_lock(&fr_hostbyaddr_mutex);
#endif

    hp = gethostbyaddr(addr, len, type);
    if ((!hp) || (hp->h_addrtype != AF_INET) || (hp->h_length != 4)) {
	 *error = h_errno;
	 hp = NULL;
    } else {
	 copy_hostent(hp, result, buffer, buflen, error);
	 hp = result;
    }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&fr_hostbyaddr_mutex);
#endif

    return hp;
}
#endif /* GETHOSTBYADDRRSTYLE */

/*
 * Mar  8, 2000 by Hajimu UMEMOTO <ume@mahoroba.org>
 *
 * Below code is based on ssh-1.2.27-IPv6-1.5 written by
 * KIKUCHI Takahiro <kick@kyoto.wide.ad.jp>
 */

#ifndef HAVE_GETADDRINFO
static struct addrinfo *
malloc_ai(int port, u_long addr, int socktype, int proto)
{
    struct addrinfo *ai;

    ai = (struct addrinfo *)malloc(sizeof(struct addrinfo) +
				   sizeof(struct sockaddr_in));
    if (ai) {
	memset(ai, 0, sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
	ai->ai_addr = (struct sockaddr *)(ai + 1);
	ai->ai_addrlen = sizeof(struct sockaddr_in);
#ifdef HAVE_SOCKADDR_SA_LEN
	ai->ai_addr->sa_len = sizeof(struct sockaddr_in);
#endif
	ai->ai_addr->sa_family = ai->ai_family = AF_INET;
	((struct sockaddr_in *)(ai)->ai_addr)->sin_port = port;
	((struct sockaddr_in *)(ai)->ai_addr)->sin_addr.s_addr = addr;
	ai->ai_socktype = socktype;
	ai->ai_protocol = proto;
	return ai;
    } else {
	return NULL;
    }
}

char const *
gai_strerror(int ecode)
{
    switch (ecode) {
    case EAI_MEMORY:
	return "memory allocation failure.";
    case EAI_FAMILY:
	return "ai_family not supported.";
    case EAI_NONAME:
	return "hostname nor servname provided, or not known.";
    case EAI_SERVICE:
	return "servname not supported for ai_socktype.";
    default:
	return "unknown error.";
    }
}

void
freeaddrinfo(struct addrinfo *ai)
{
    struct addrinfo *next;

    if (ai->ai_canonname)
	free(ai->ai_canonname);
    do {
	next = ai->ai_next;
	free(ai);
    } while ((ai = next) != NULL);
}

int
getaddrinfo(char const *hostname, char const *servname,
	    struct addrinfo const *hints, struct addrinfo **res)
{
    struct addrinfo *cur, *prev = NULL;
    struct hostent *hp;
    struct hostent result;
    struct in_addr in;
    int i, port = 0, socktype, proto;
    int error;
    char buffer[2048];

    if (hints && hints->ai_family != PF_INET && hints->ai_family != PF_UNSPEC)
	return EAI_FAMILY;

    socktype = (hints && hints->ai_socktype) ? hints->ai_socktype
					     : SOCK_STREAM;
    if (hints && hints->ai_protocol)
	proto = hints->ai_protocol;
    else {
	switch (socktype) {
	case SOCK_DGRAM:
	    proto = IPPROTO_UDP;
	    break;
	case SOCK_STREAM:
	    proto = IPPROTO_TCP;
	    break;
	default:
	    proto = 0;
	    break;
	}
    }
    if (servname) {
	if (isdigit((int)*servname))
	    port = htons(atoi(servname));
	else {
	    struct servent *se;
	    char const *pe_proto;

	    switch (socktype) {
	    case SOCK_DGRAM:
		pe_proto = "udp";
		break;
	    case SOCK_STREAM:
		pe_proto = "tcp";
		break;
	    default:
		pe_proto = NULL;
		break;
	    }
	    if ((se = getservbyname(servname, pe_proto)) == NULL)
		return EAI_SERVICE;
	    port = se->s_port;
	}
    }
    if (!hostname) {
	if (hints && hints->ai_flags & AI_PASSIVE)
	    *res = malloc_ai(port, htonl(0x00000000), socktype, proto);
	else
	    *res = malloc_ai(port, htonl(0x7f000001), socktype, proto);
	if (*res)
	    return 0;
	else
	    return EAI_MEMORY;
    }
    /* Numeric IP Address */
    if (inet_aton(hostname, &in)) {
	*res = malloc_ai(port, in.s_addr, socktype, proto);
	if (*res)
	    return 0;
	else
	    return EAI_MEMORY;
    }
    if (hints && hints->ai_flags & AI_NUMERICHOST)
	return EAI_NONAME;

    /* DNS Lookup */
#ifdef GETHOSTBYNAMERSTYLE
#if GETHOSTBYNAMERSTYLE == SYSVSTYLE
    hp = gethostbyname_r(hostname, &result, buffer, sizeof(buffer), &error);
#elif GETHOSTBYNAMERSTYLE == GNUSTYLE
    if (gethostbyname_r(hostname, &result, buffer,
	 sizeof(buffer), &hp, &error) != 0) {
		hp = NULL;
	}
#else
    hp = gethostbyname_r(hostname, &result, buffer, sizeof(buffer), &error);
#endif
#else
    hp = gethostbyname_r(hostname, &result, buffer, sizeof(buffer), &error);
#endif
    if (hp && hp->h_name && hp->h_name[0] && hp->h_addr_list[0]) {
	for (i = 0; hp->h_addr_list[i]; i++) {
	    if ((cur = malloc_ai(port,
				((struct in_addr *)hp->h_addr_list[i])->s_addr,
				socktype, proto)) == NULL) {
		if (*res)
		    freeaddrinfo(*res);
		return EAI_MEMORY;
	    }
	    if (prev)
		prev->ai_next = cur;
	    else
		*res = cur;
	    prev = cur;
	}
	if (hints && hints->ai_flags & AI_CANONNAME && *res) {
	    if (((*res)->ai_canonname = strdup(hp->h_name)) == NULL) {
		freeaddrinfo(*res);
		return EAI_MEMORY;
	    }
	}
	return 0;
    }
    return EAI_NONAME;
}
#endif /* HAVE_GETADDRINFO */


#ifndef HAVE_GETNAMEINFO
int
getnameinfo(struct sockaddr const  *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen,
	    unsigned int flags)
{
    const struct sockaddr_in *sin = (struct sockaddr_in const *)sa;
    struct hostent *hp;
    struct hostent result;
    char tmpserv[16];
    char buffer[2048];
    int error;

    if (serv) {
	snprintf(tmpserv, sizeof(tmpserv), "%d", ntohs(sin->sin_port));
	if (strlen(tmpserv) > servlen)
	    return EAI_MEMORY;
	else
	    strcpy(serv, tmpserv);
    }
    if (host) {
	if (flags & NI_NUMERICHOST) {
	    /*  No Reverse DNS lookup */
	    if (flags & NI_NAMEREQD)
		return EAI_NONAME;
	    if (strlen(inet_ntoa(sin->sin_addr)) >= hostlen)
		return EAI_MEMORY;
	    else {
		strcpy(host, inet_ntoa(sin->sin_addr));
		return 0;
	    }
	} else {
	/*  Reverse DNS lookup required */
#ifdef GETHOSTBYADDRRSTYLE
#if GETHOSTBYADDRRSTYLE == SYSVSTYLE
	    hp = gethostbyaddr_r((char const *)&sin->sin_addr,
			       salen, AF_INET,
			       &result, buffer, sizeof(buffer), &error);
#elif GETHOSTBYADDRRSTYLE == GNUSTYLE
	    if (gethostbyaddr_r((char const *)&sin->sin_addr,
			       salen, AF_INET,
				    &result, buffer, sizeof(buffer),
				    &hp, &error) != 0) {
			hp = NULL;
	     }
#else
	    hp = gethostbyaddr_r((char const *)&sin->sin_addr,
			       salen, AF_INET,
			       &result, buffer, sizeof(buffer), &error);
#endif
#else
	    hp = gethostbyaddr_r((char const *)&sin->sin_addr,
			       salen, AF_INET,
			       &result, buffer, sizeof(buffer), &error);
#endif
	    if (hp)
		if (strlen(hp->h_name) >= hostlen)
		    return EAI_MEMORY;
		else {
		    strcpy(host, hp->h_name);
		    return 0;
		}
	    else if (flags & NI_NAMEREQD)
		return EAI_NONAME;
	    else if (strlen(inet_ntoa(sin->sin_addr)) >= hostlen)
		return EAI_MEMORY;
	    else {
		strcpy(host, inet_ntoa(sin->sin_addr));
		return 0;
	    }
	}
    }
    return 0;
}
#endif /* HAVE_GETNAMEINFO */
