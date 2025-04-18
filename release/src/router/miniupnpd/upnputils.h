/* $Id: upnputils.h,v 1.11 2025/04/03 21:11:35 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2011-2025 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPUTILS_H_INCLUDED
#define UPNPUTILS_H_INCLUDED

/*! \file upnputils.h
 * \brief utility functions
 */

/*! \brief convert a struct sockaddr to a human readable string.
 * `[ipv6]:port` or `ipv4:port`
 * \param[in] addr address
 * \param[out] str buffer
 * \param[in] size buffer size
 * \return the number of characters used (as snprintf)
 */
int
sockaddr_to_string(const struct sockaddr * addr, char * str, size_t size);

/*! \brief set the file descriptor as non blocking
 * \param[in] fd file descriptor
 * \return 0 in case of failure, 1 in case of success
 */
int
set_non_blocking(int fd);

/*! \brief get the LAN which the peer belongs to
 * \param[in] peer address
 * \return NULL in case of error
 */
struct lan_addr_s *
get_lan_for_peer(const struct sockaddr * peer);

/**
 * get the time for upnp (release expiration, etc.)
 * Similar to a monotonic time(NULL)
 */
time_t upnp_time(void);

/*! \brief get either the machine or the daemon uptime
 * depending on system_uptime / -U option
 */
time_t upnp_get_uptime(void);

/**
 * get the time for upnp
 * Similar to a monotonic gettimeofday(tv, NULL)
 */
int upnp_gettimeofday(struct timeval * tv);

/**
 * define portability macros
 */
#if defined(__sun)
static __inline size_t _sa_len(const struct sockaddr *addr)
{
        if (addr->sa_family == AF_INET)
                return (sizeof(struct sockaddr_in));
        else if (addr->sa_family == AF_INET6)
                return (sizeof(struct sockaddr_in6));
        else
                return (sizeof(struct sockaddr));
}
# define SA_LEN(sa) (_sa_len(sa))
#else
#if !defined(SA_LEN)
# define SA_LEN(sa) ((sa)->sa_len)
#endif
#endif

#ifndef MAX
# define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#endif

