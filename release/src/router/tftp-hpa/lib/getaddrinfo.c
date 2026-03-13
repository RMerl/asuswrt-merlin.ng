/*
 * getaddrinfo.c
 *
 * Simple version of getaddrinfo()
 *
 */

#include "config.h"

extern int errno;
extern int h_errno;

void freeaddrinfo(struct addrinfo *res)
{
    if (!res)
        return;
    if (res->ai_next)
        freeaddrinfo(res->ai_next);
    if (res->ai_addr)
        free(res->ai_addr);
    if (res->ai_canonname)
        free(res->ai_canonname);
    free(res);
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints,
                struct addrinfo **res)
{
    struct hostent  *host;
    struct sockaddr *sa;
    int err, size = 0;

    if ((!node) || (!res)) {
        errno = EINVAL;
        return EAI_SYSTEM;
    }
    *res = NULL;
    /* we do not support service in this version */
    if (service) {
        errno = EINVAL;
        return EAI_SYSTEM;
    }
    host = gethostbyname(node);
    if (!host)
        return EAI_NONAME;
    if (hints) {
        if (hints->ai_family != AF_UNSPEC) {
            if (hints->ai_family != host->h_addrtype)
                return EAI_ADDRFAMILY;
        }
    }
    *res =  malloc(sizeof(struct addrinfo));
    if (!*res) {
        return EAI_MEMORY;
    }
    memset(*res, 0, sizeof(struct addrinfo));
    (*res)->ai_family = host->h_addrtype;
    if (host->h_length) {
        if (host->h_addrtype == AF_INET)
            size = sizeof(struct sockaddr_in);
#ifdef HAVE_IPV6
        else if (host->h_addrtype == AF_INET6)
            size = sizeof(struct sockaddr_in6);
#endif
        else {
            free(*res);
            *res = NULL;
            return EAI_ADDRFAMILY;
        }
        sa = malloc(size);
        if (!sa) {
            free(*res);
            *res = NULL;
            return EAI_MEMORY;
        }
        memset(sa, 0, size);
        (*res)->ai_addr = sa;
        (*res)->ai_addrlen = size;
        sa->sa_family = host->h_addrtype;
        if (host->h_addrtype == AF_INET)
            memcpy(&((struct sockaddr_in *)sa)->sin_addr, host->h_addr, host->h_length);
#ifdef HAVE_IPV6
        else
            memcpy(&((struct sockaddr_in6 *)sa)->sin6_addr, host->h_addr, host->h_length);
#endif
    }
    if (host->h_name)
       (*res)->ai_canonname = strdup(host->h_name);

    /* we only handle the first address entry and do not build a list now */
    return 0;
}



const char *gai_strerror(int errcode)
{
    const char *s = NULL;

    switch(errcode) {
    case 0:
        s = "no error";
        break;
    case EAI_MEMORY:
        s = "no memory";
        break;
    case EAI_SYSTEM:
        s = strerror(errno);
        break;
    case EAI_NONAME:
        s = hstrerror(h_errno);
        break;
    case EAI_ADDRFAMILY:
        s = "address does not match address family";
        break;
    default:
        s = "unknown error code";
        break;
    }
    return s;
}
