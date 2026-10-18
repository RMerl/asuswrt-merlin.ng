/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2007-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @file nu_ipaddr.c
 *
 * Implements a class that wraps IPv4/v6 address functionality
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h> // for getaddrinfo()
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <unistd.h>

#include "common_defs.h"
#include "nu_ipaddr.h"
#include "usp_api.h"
#include "data_model.h"
#include "nu_macaddr.h"
#include "device.h"


#ifndef EFAIL
#define EFAIL EINVAL
#endif
#ifndef EMISMATCH
#define EMISMATCH EINVAL
#endif

#ifndef IN6ADDR_LINKLOCAL_ALLNODES_INIT
#define IN6ADDR_LINKLOCAL_ALLNODES_INIT		     \
 {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#endif /* IN6ADDR_LINKLOCAL_ALLNODES_INIT */



//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int tw_ulib_get_dev_ipaddr(const char *dev, char *addr, size_t asiz, bool prefer_ipv6);

/*********************************************************************//**
**
**  nu_ipaddr_get_family
**
**  Returns the address family stored in the specified nu_ipaddr_t
**
** \param   addr - IP address to determine the address family of
** \param   familyp - pointer to variable in which to return the address family
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_get_family(const nu_ipaddr_t *addr, sa_family_t *familyp)
{
#if IPV6_NUIPADDR
	if (IN6_IS_ADDR_V4MAPPED(addr)) {
		*familyp = AF_INET;
	} else {
		*familyp = AF_INET6;
	}
#else /* IPV6_NUIPADDR */
	*familyp = AF_INET;
#endif /* !IPV6_NUIPADDR */
    return USP_ERR_OK;
}


/*********************************************************************//**
**
**  nu_ipaddr_to_inaddr
**
**  Converts the specified nu_ipaddr_t to an IPv4 in_addr structure
**  NOTE: The conversion may fail, if the specified nu_ipaddr_t contains an IPv6 address
**
** \param   addr - IP address to convert to an IPv4 in_addr structure
** \param   p - pointer to structure in which to return IPv4 in_addr
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_to_inaddr(const nu_ipaddr_t *addr, struct in_addr *p)
{
#if IPV6_NUIPADDR
	if (IN6_IS_ADDR_V4MAPPED(addr) || IN6_IS_ADDR_V4COMPAT(addr)) {
		p->s_addr = addr->s6_addr32[3];
		return USP_ERR_OK;
	}

    USP_ERR_SetMessage("%s: Failed trying to convert an IPv6 address to an IPv4 address", __FUNCTION__);
	return USP_ERR_INTERNAL_ERROR;
#else /* IPV6_NUIPADDR */
	p->s_addr = addr->s_addr;
	return USP_ERR_OK;
#endif /* !IPV6_NUIPADDR */
}


/*********************************************************************//**
**
**  nu_ipaddr_to_in6addr
**
**  Converts the specified nu_ipaddr_t to an IPv6 in6_addr structure
**  NOTE: The conversion may fail, if the specified nu_ipaddr_t contains an IPv4 address
**
** \param   addr - IP address to convert to an IPv6 in_addr structure
** \param   p - pointer to structure in which to return IPv6 in6_addr
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_to_in6addr(const nu_ipaddr_t *addr, struct in6_addr *p)
{
#if IPV6_NUIPADDR
    // Exit if the specified nu_ipaddr_t contains an IPv4 address instead of an IPv6 Address
	if (IN6_IS_ADDR_V4MAPPED(addr) || IN6_IS_ADDR_V4COMPAT(addr)) {
        USP_ERR_SetMessage("%s: Failed trying to convert an IPv4 address to an IPv6 address", __FUNCTION__);
    	return USP_ERR_INTERNAL_ERROR;
	}

    memcpy(p, addr, sizeof(*p));
#else /* IPV6_NUIPADDR */
    p->s6_addr32[0] = 0;
    p->s6_addr32[1] = 0;
    p->s6_addr[8] = 0;
    p->s6_addr[9] = 0;
    p->s6_addr[10] = 0xff;
    p->s6_addr[11] = 0xff;
    p->s6_addr32[3] = addr->s_addr;
#endif /* !IPV6_NUIPADDR */
    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  nu_ipaddr_to_sockaddr
**
**  Converts the specified nu_ipaddr_t to a sockaddr (which may contain either an IPv4 or IPv6 address, and a port)
**
** \param   addr - IP address to convert to a sockaddr
** \param   port - Port number to place in sockaddr structure
** \param   sa - pointer to structure in which to return sockaddr
** \param   len_p - pointer to variable in which to return the length of the sockaddr structure.
**                  NOTE: This may be specified as NULL, if the length is not required
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_to_sockaddr(const nu_ipaddr_t *addr, int port, struct sockaddr_storage *sa, socklen_t *len_p)
{
    sa_family_t family;
    int err;

    err = nu_ipaddr_get_family(addr, &family);
    if (err != USP_ERR_OK) {
        return err;
    }

    (void) memset(sa, 0, sizeof(*sa));

    if (family == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
        sin6->sin6_family = family;
        sin6->sin6_port = htons(port);
        err = nu_ipaddr_to_in6addr(addr, &sin6->sin6_addr);
        if (err != USP_ERR_OK) {
            return err;
        }

        if (len_p != NULL) {
            *len_p = sizeof(struct sockaddr_in6);
        }
    } else {
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
        sin->sin_family = family;
        sin->sin_port = htons(port);
        err = nu_ipaddr_to_inaddr(addr, &sin->sin_addr);
        if (err != USP_ERR_OK) {
            return err;
        }

        if (len_p != NULL) {
            *len_p = sizeof(struct sockaddr_in);
        }
    }

    return USP_ERR_OK;
}


/*********************************************************************//**
**
**  nu_ipaddr_to_str
**
**  Converts the specified nu_ipaddr_t into a string format IP address
**
** \param   addr - IP address to convert to a string
** \param   buf - pointer to buffer in which to return the string
** \param   bufsiz - size of buffer in which to return the string. This must be at least NU_IPADDRSTRLEN bytes long.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_to_str(const nu_ipaddr_t *addr, char *buf, int buflen)
{
	const char *cp;
	sa_family_t family;

    buf[0] = '\0';  	// Zero terminate the buffer, so that if the call to inet_ntop() fails, at least the buffer will contain an empty string

#if IPV6_NUIPADDR
    // Handle case of nu_ipaddr_t containing an IPv4 address here, pure IPv6 case is handled at the end of the function
	if (IN6_IS_ADDR_V4MAPPED(addr)) {
		cp = inet_ntop(AF_INET, &addr->s6_addr32[3], buf, buflen);
		if (cp == NULL) {
            USP_ERR_ERRNO("inet_ntop", errno);
			return USP_ERR_INTERNAL_ERROR;
		}
		return USP_ERR_OK;
	}
	family = AF_INET6;
#else /* IPV6_NUIPADDR */
	/* nu_ipaddr_t is only v4 */
	family = AF_INET;
#endif /* !IPV6_NUIPADDR */

	cp = inet_ntop(family, addr, buf, buflen);
	if (cp == NULL) {
        USP_ERR_ERRNO("inet_ntop", errno);
		return USP_ERR_INTERNAL_ERROR;
	}
	return USP_ERR_OK;
}


/*********************************************************************//**
**
**  nu_ipaddr_str
**
**  Convenience function for logging, which always returns a string, given the specified nu_ipaddr_t
**
** \param   addr - IP address to convert to a string
** \param   buf - pointer to buffer in which to return the string
** \param   bufsiz - size of buffer in which to return the string. This must be at least NU_IPADDRSTRLEN bytes long.
**
** \return  buf if successfully converted, 'UNKNOWN' otherwise
**
**************************************************************************/
char *nu_ipaddr_str(const nu_ipaddr_t *addr, char *buf, int buflen)
{
    int err;

    err = nu_ipaddr_to_str(addr, buf, buflen);
    if (err != USP_ERR_OK)
    {
        return "UNKNOWN";
    }

    return buf;
}


/*********************************************************************//**
**
**  nu_ipaddr_from_str
**
**  Converts the specified IP Address string into a nu_ipaddr_t
**
** \param   str - pointer to string containing IP Address to convert
** \param   addr - pointer to structure in which to return the nu_ipaddr_t
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_from_str(const char *str, nu_ipaddr_t *addr)
{
	const char *p;
	const char *begin;
	char buf[64];
	struct in6_addr in6;
	struct in_addr in;
	void *inptr;
	sa_family_t family;
	int err, i;

	/* skip leading space */
	for (p = str; *p != '\0'; p++) {
		if (!isspace(p[0])) {
			break;
		}
	}
	if (*p == '[') {
		p++;	/* strip the IPv6 brackets */
		for (i = 0; (buf[i] = *p); i++, p++) {
			if (i >= sizeof(buf) - 1 || buf[i] == ']') {
				buf[i] = '\0';
				break;
			}
		}
		begin = buf;
	} else {
	begin = p;
	}

	/* determine family */
	family = AF_INET;
	inptr = &in;
	for (p = begin; *p != '\0'; p++) {
		if (p[0] == '.' || isdigit(p[0])) {
			/* valid ipv4 value */
			continue;
		}
		if (p[0] == ':' || isxdigit(p[0])) {
			/* ipv6 only has hex */
			family = AF_INET6;
			inptr = &in6;
			break;
		}

		/* invalid character. Stop */
		break;
	}

	/* attempt to parse using what we've learned */
	err = inet_pton(family, begin, inptr);      // returns 1 on success
	if (err <= 0) {
		return USP_ERR_INTERNAL_ERROR;
	}

	switch (family) {
	case AF_INET:
		err = nu_ipaddr_from_inaddr(&in, addr);
		break;
	case AF_INET6:
		err = nu_ipaddr_from_in6addr(&in6, addr);
		break;
	default:
        TERMINATE_BAD_CASE(family);
		break;
	}

	return err;
}


/*********************************************************************//**
**
**  nu_ipaddr_from_sockaddr_storage
**
**  Converts a sockaddr_storage structure to a nu_ipaddr_t structure and (optionally) a port
**
** \param   p - pointer to sockaddr_storage structure to convert to a nu_ipaddr_t.
** \param   addr - pointer to structure in which to return the nu_ipaddr_t
** \param   port - pointer to variable in which to return the IP port, or NULL, if this is not required
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if address family was not supported.
**          NOTE: This function may be called with unsupported address families. The caller must handle this.
**
**************************************************************************/
int
nu_ipaddr_from_sockaddr_storage(const struct sockaddr_storage *p, nu_ipaddr_t *addr, uint16_t *port)
{
    int err;
    struct sockaddr_in *sin4;
    struct sockaddr_in6 *sin6;

    // Exit if input arguments are incorrectly specified
    if ((p == NULL) || (addr == NULL))
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    if (p->ss_family == AF_INET)
    {
        // IPv4
        sin4 = (struct sockaddr_in *)p;
        err = nu_ipaddr_from_inaddr(&sin4->sin_addr, addr);
        if (err != USP_ERR_OK)
        {
            return err;
        }

	    if (port != NULL)
	    {
	        *port = ntohs(sin4->sin_port);
	    }
    }
    else if (p->ss_family == AF_INET6)
    {
        // IPv6
        sin6 = (struct sockaddr_in6 *)p;
        err = nu_ipaddr_from_in6addr(&sin6->sin6_addr, addr);
        if (err != USP_ERR_OK)
        {
            return err;
        }

	    if (port != NULL)
	    {
	        *port = ntohs(sin6->sin6_port);
	    }
    }
    else
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  nu_ipaddr_from_inaddr
**
**  Converts an (IPv4) in_addr structure to a nu_ipaddr_t structure
**
** \param   p - pointer to in_addr structure to convert to a nu_ipaddr_t. NOTE: in_addr structures are always in network byte order
** \param   addr - pointer to structure in which to return the nu_ipaddr_t
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_from_inaddr(const struct in_addr *p, nu_ipaddr_t *addr)
{
#if IPV6_NUIPADDR
	// Store IPv4 address internally as an IPv4 mapped IPv6 address
	addr->s6_addr32[0] = 0;
	addr->s6_addr32[1] = 0;
	addr->s6_addr[8] = 0;
	addr->s6_addr[9] = 0;
	addr->s6_addr[10] = 0xff;
	addr->s6_addr[11] = 0xff;
	addr->s6_addr32[3] = p->s_addr;
#else /* IPV6_NUIPADDR */
	addr->s_addr = p->s_addr;
#endif /* !IPV6_NUIPADDR */
	return USP_ERR_OK;
}


/*********************************************************************//**
**
**  nu_ipaddr_from_in6addr
**
**  Converts an (IPv6) in6_addr structure to a nu_ipaddr_t structure
**
** \param   p - pointer to in6_addr structure to convert to a nu_ipaddr_t
** \param   addr - pointer to structure in which to return the nu_ipaddr_t
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_from_in6addr(const struct in6_addr *p, nu_ipaddr_t *addr)
{
#if IPV6_NUIPADDR
	memcpy(addr, p, sizeof(*addr));
	return USP_ERR_OK;
#else /* IPV6_NUIPADDR */
    // Check that we have only been passed an IPv4 mapped or IPv4 compatible IPv6 address
	if ((p->s6_addr32[0] == 0) &&
	    (p->s6_addr32[1] == 0) &&
	    (p->s6_addr[8] == 0) &&
	    (p->s6_addr[9] == 0) &&
	    ((p->s6_addr[10] == 0) || (p->s6_addr[10] == 0xff)) &&
	    ((p->s6_addr[11] == 0) || (p->s6_addr[11] == 0xff)))
	{
		addr->s_addr = p->s6_addr32[3];
		return USP_ERR_OK;
	}

    USP_ERR_SetMessage("%s: Unable to convert: Not an IPv4 mapped or IPv4 compatible IPv6 address", __FUNCTION__);
	return USP_ERR_INTERNAL_ERROR;
#endif /* !IPV6_NUIPADDR */
}


/*********************************************************************//**
**
**  nu_ipaddr_equal
**
**  Determines whether two nu_ipaddr_t structures are equal
**
** \param   a1 - pointer to first nu_ipaddr_t structure
** \param   a1 - pointer to second nu_ipaddr_t structure
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_equal(const nu_ipaddr_t *a1, const nu_ipaddr_t *a2,
		bool *equalp)
{
#if IPV6_NUIPADDR
	*equalp = (a1->s6_addr32[0] == a2->s6_addr32[0] &&      // NOTE: This is comparing all 16 bytes of the IPv6 address
		   a1->s6_addr32[1] == a2->s6_addr32[1] &&
		   a1->s6_addr32[2] == a2->s6_addr32[2] &&
		   a1->s6_addr32[3] == a2->s6_addr32[3]);
#else /* IPV6_NUIPADDR */
	*equalp = (a1->s_addr == a2->s_addr);
#endif /* !IPV6_NUIPADDR */
	return USP_ERR_OK;
}

/*********************************************************************//**
**
**  nu_ipaddr_copy
**
**  Copies from a src nu_ipaddr_t structure to a dest
**
** \param   dst - pointer to destination nu_ipaddr_t structure
** \param   src - pointer to source nu_ipaddr_t structure
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_copy(nu_ipaddr_t *dest, const nu_ipaddr_t *src)
{
    memcpy(dest, src, sizeof(nu_ipaddr_t));
	return USP_ERR_OK;
}

/*********************************************************************//**
**
**  nu_ipaddr_set_zero
**
**  Sets the specified nu_ipaddr_t to be the 'zero' IP address
**
** \param   addr - pointer to structure to set to the 'zero' IP address
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
nu_ipaddr_set_zero(nu_ipaddr_t *addr)
{
#if IPV6_NUIPADDR
	addr->s6_addr32[0] = 0;
	addr->s6_addr32[1] = 0;
	addr->s6_addr32[2] = 0;
	addr->s6_addr32[3] = 0;
#else /* IPV6_NUIPADDR */
	addr->s_addr = 0;
#endif /* !IPV6_NUIPADDR */
	return USP_ERR_OK;
}


/*********************************************************************//**
**
**  nu_ipaddr_is_zero
**
**  Returns whether the specified nu_ipaddr_t is the 'zero' IP address
**  The 'zero' IP address is used as a magic number by clients to denote an invalid/uninitialised IP Address
**
** \param   addr - pointer to nu_ipaddr_t
**
** \return  true if the specified nu_ipaddr_t is the 'zero' IP address, false otherwise
**
**************************************************************************/
bool
nu_ipaddr_is_zero(const nu_ipaddr_t *addr)
{
    bool flag;

#if IPV6_NUIPADDR
	if (IN6_IS_ADDR_V4MAPPED(addr)) {
		flag = (addr->s6_addr32[3] == 0);
	} else {
		flag = (addr->s6_addr32[0] == 0 &&
			 addr->s6_addr32[1] == 0 &&
			 addr->s6_addr32[2] == 0 &&
			 addr->s6_addr32[3] == 0);
	}
#else /* IPV6_NUIPADDR */
	flag = (addr->s_addr == 0);
#endif /* !IPV6_NUIPADDR */
	return flag;
}

/*********************************************************************//**
**
**  nu_ipaddr_get_interface_addr_from_dest_addr
**
**  Determines the ip address of the interface on which a packet will be sent,
**  based on the destination address of the packet
**
** \param   dest - destination address of the packet
** \param   if_addr - pointer to structure in which to return the interface address
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int nu_ipaddr_get_interface_addr_from_dest_addr(nu_ipaddr_t *dest, nu_ipaddr_t *if_addr)
{
    int sock_fd;
    struct sockaddr_storage sa;
    socklen_t sa_len;
    int err;
    sa_family_t family;

    // Determine whether destination is IPv4 or IPv6
    err = nu_ipaddr_get_family(dest, &family);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Setup destination address and port
    #define DONT_CARE_PORT 1025
    err = nu_ipaddr_to_sockaddr(dest, DONT_CARE_PORT, &sa, &sa_len);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Open a UDP socket
    sock_fd = socket(family, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        USP_ERR_ERRNO("socket", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Perform a connect
    // This does an implicit bind, using the kernel routing tables to a source interface address
    // Note, because the socket is UDP, nothing is actually sent to the destination - this is why the destination port does not matter
    err = connect(sock_fd, (struct sockaddr*)&sa, sa_len);
    if (err != 0)
    {
        USP_ERR_ERRNO("connect", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Get the source address that the socket has bound to
    sa_len = sizeof(sa);
    err = getsockname(sock_fd, (struct sockaddr*)&sa, &sa_len);
    if (err != 0)
    {
        USP_ERR_ERRNO("getsockname", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Convert the source address to a nu_ipaddr_t
    err = nu_ipaddr_from_sockaddr_storage(&sa, if_addr, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

exit:
    close(sock_fd);
    return err;
}

/*********************************************************************//**
**
**  nu_ipaddr_get_interface_addr_from_sock_fd
**
**  Determines the ip address of the interface on which a packet will be sent,
**  based on a socket that has connected to a destination address
**
** \param   sock_fd - socket
** \param   buf - pointer to buffer in which to return the string
** \param   bufsiz - size of buffer in which to return the string. This must be at least NU_IPADDRSTRLEN bytes long.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int nu_ipaddr_get_interface_addr_from_sock_fd(int sock_fd, char *buf, int bufsiz)
{
    int err;
    struct sockaddr_storage sa = {0};
    socklen_t sa_len;
    nu_ipaddr_t if_addr;

    // Get the source address that the socket has bound to
    sa_len = sizeof(sa);
    err = getsockname(sock_fd, (struct sockaddr*)&sa, &sa_len);
    if (err != 0)
    {
        USP_ERR_ERRNO("getsockname", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Convert the socket address to a nu_ipaddr_t
    err = nu_ipaddr_from_sockaddr_storage(&sa, &if_addr, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Convert the nu_ipaddr_t to a string
    err = nu_ipaddr_to_str(&if_addr, buf, bufsiz);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  nu_ipaddr_get_interface_name_from_src_addr
**
**  Determines the name of an interface given it's source IP address
**
** \param   src_addr - source address of a network interface on the device
** \param   name - pointer to buffer in which to return the name of the interface that has the specified source address
** \param   name_len - size of buffer in which to return the name of the interface that has the specified source address
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int nu_ipaddr_get_interface_name_from_src_addr(char *src_addr, char *name, int name_len)
{
    struct ifaddrs *ifaddr_list;
    struct ifaddrs *iterator;
    int err;
    int family;
    void *in_addr;
    char *str;
    char buf[NU_IPADDRSTRLEN];       // Buffer used to contain the IP address of each interface found in turn

    // Set the default interface name returned
    *name = '\0';

    // Exit if unable to get a linked list containing the IP Addresses of all network interfaces on the current system
    err = getifaddrs(&ifaddr_list);
    if (err != 0)
    {
        USP_ERR_SetMessage("%s: ERROR: getifaddrs() failed: %s", __FUNCTION__, strerror(errno));
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all results in the linked list
    for (iterator=ifaddr_list;   iterator!=NULL;   iterator=iterator->ifa_next)
    {
        // Skip this result, if no IP address available (this might be the case for tunnels)
        if (iterator->ifa_addr == NULL)
        {
            continue;
        }

        // Skip this result, if it is not an IPv4 or IPv6 node
        family = iterator->ifa_addr->sa_family;
        if ((family != AF_INET) && (family != AF_INET6))
        {
            continue;
        }

        // Determine pointer to IPv4 or IPv6 address
        if (family == AF_INET)
        {
            in_addr = &((struct sockaddr_in  *)iterator->ifa_addr)->sin_addr;
        }
        else
        {
            in_addr = &((struct sockaddr_in6 *)iterator->ifa_addr)->sin6_addr;
        }

        // Skip this result, if it is an IPv6 address, but not globally routable
        #define NOT_GLOBAL_UNICAST(addr) \
                    ( (IN6_IS_ADDR_UNSPECIFIED(addr)) || (IN6_IS_ADDR_LOOPBACK(addr))  ||   \
                      (IN6_IS_ADDR_MULTICAST(addr))   || (IN6_IS_ADDR_LINKLOCAL(addr)) ||   \
                      (IN6_IS_ADDR_SITELOCAL(addr)) )
        if ((family == AF_INET6) && (NOT_GLOBAL_UNICAST( (struct in6_addr *)in_addr )))
        {
            continue;
        }

        // Skip this result, if unable to get the string form of the IP address for this interface
        str = (char *) inet_ntop(family, in_addr, buf, sizeof(buf));
        if (str == NULL)
        {
            continue;
        }

        // Exit if found the matching source IP address
        if (strcmp(buf, src_addr)==0)
        {
            USP_STRNCPY(name, iterator->ifa_name, name_len);
            err = USP_ERR_OK;
            goto exit;
        }
    }

    // If the code gets here, then no match was found
    err = USP_ERR_INTERNAL_ERROR;

exit:
    freeifaddrs(ifaddr_list);

    return err;

}

/*********************************************************************//**
**
** nu_ipaddr_has_interface_addr_changed
**
** Determines whether the IP address of the specified interface has changed from the expected address
** This function is used to determine whether to restart a STOMP or CoAP connection
**
** \param   dev - name of interface to get IP Address of
** \param   expected_addr - expected IP address of interface
** \param   has_addr - pointer to variable in which to return whether the network interface has any IP address
**
** \return  true if the IP address of the interface has changed, false otherwise
**
**************************************************************************/
int nu_ipaddr_has_interface_addr_changed(char *dev, char *expected_addr, bool *has_addr)
{
    struct ifaddrs *ifaddr_list;
    struct ifaddrs *iterator;
    int err;
    int family;
    void *in_addr;
    char *str;
    char buf[NU_IPADDRSTRLEN];       // Buffer used to contain the IP address of each interface found in turn
    bool result;

    // Exit if unable to get a linked list containing the IP Addresses of all network interfaces on the current system
    *has_addr = false;
    err = getifaddrs(&ifaddr_list);
    if (err != 0)
    {
        USP_ERR_SetMessage("%s: ERROR: getifaddrs() failed: %s", __FUNCTION__, strerror(errno));
        return true;
    }

    // Iterate over all results in the linked list
    for (iterator=ifaddr_list;   iterator!=NULL;   iterator=iterator->ifa_next)
    {
        // Skip this result, if it does not match the interface name
        if (strcmp(iterator->ifa_name, dev) != 0)
        {
            continue;
        }

        // Skip this result, if no IP address available (this might be the case for tunnels)
        if (iterator->ifa_addr == NULL)
        {
            continue;
        }

        // Skip this result, if it is not an IPv4 or IPv6 node
        family = iterator->ifa_addr->sa_family;
        if ((family != AF_INET) && (family != AF_INET6))
        {
            continue;
        }

        // Determine pointer to IPv4 or IPv6 address
        if (family == AF_INET)
        {
            in_addr = &((struct sockaddr_in  *)iterator->ifa_addr)->sin_addr;
        }
        else
        {
            in_addr = &((struct sockaddr_in6 *)iterator->ifa_addr)->sin6_addr;
        }

        // Skip this result, if it is an IPv6 address, but not globally routable
        #define NOT_GLOBAL_UNICAST(addr) \
                    ( (IN6_IS_ADDR_UNSPECIFIED(addr)) || (IN6_IS_ADDR_LOOPBACK(addr))  ||   \
                      (IN6_IS_ADDR_MULTICAST(addr))   || (IN6_IS_ADDR_LINKLOCAL(addr)) ||   \
                      (IN6_IS_ADDR_SITELOCAL(addr)) )
        if ((family == AF_INET6) && (NOT_GLOBAL_UNICAST( (struct in6_addr *)in_addr )))
        {
            continue;
        }

        // Skip this result, if unable to get the string form of the IP address
        str = (char *) inet_ntop(family, in_addr, buf, sizeof(buf));
        if (str == NULL)
        {
            continue;
        }

        // If the code gets here, then the interface has an IP address
        *has_addr = true;

        // Exit the loop if we've found our expected IP address for this interface - the IP address of the interface has not changed
        if (strcmp(buf, expected_addr)==0)
        {
            result = false;
            goto exit;
        }
    }

    // If the code gets here, then no match was found, so the IP address of the interface has changed
    result = true;

exit:
    freeifaddrs(ifaddr_list);

    return result;
}

/*********************************************************************//**
**
**  nu_ipaddr_get_ip_supported_families
**
**  Determines whether the device has any IPv4 address and any globally routable IPv6 address (on any of its interfaces)
**
** \param   ipv4_supported - pointer to variable in which to store whether IPv4 is supported
** \param   ipv6_supported - pointer to variable in which to store whether IPv6 is supported
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int nu_ipaddr_get_ip_supported_families(bool *ipv4_supported, bool *ipv6_supported)
{
    struct ifaddrs *ifaddr_list;
    struct ifaddrs *iterator;
    int err;
    int family;
    void *in_addr;

    // Assume that the device does not have any IP address
    *ipv4_supported = false;
    *ipv6_supported = false;

    // Exit if unable to get a linked list containing the IP Addresses of all network interfaces on the current system
    err = getifaddrs(&ifaddr_list);
    if (err != 0)
    {
        USP_ERR_ERRNO("getifaddrs", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all results in the linked list
    for (iterator=ifaddr_list;   iterator!=NULL;   iterator=iterator->ifa_next)
    {
        if (iterator->ifa_addr != NULL) {        // ifa_addr may equal NULL for tunnels
            family = iterator->ifa_addr->sa_family;
            switch(family)
            {
                case AF_INET:
                    *ipv4_supported = true;
                    break;

                case AF_INET6:
                    // We are only interested in globally routable IPv6 addresses
                    #define NOT_GLOBAL_UNICAST(addr) \
                                ( (IN6_IS_ADDR_UNSPECIFIED(addr)) || (IN6_IS_ADDR_LOOPBACK(addr))  ||   \
                                  (IN6_IS_ADDR_MULTICAST(addr))   || (IN6_IS_ADDR_LINKLOCAL(addr)) ||   \
                                  (IN6_IS_ADDR_SITELOCAL(addr)) )
                    #define GLOBAL_UNICAST(addr) ( !(NOT_GLOBAL_UNICAST(addr)))
                    in_addr = &((struct sockaddr_in6 *)iterator->ifa_addr)->sin6_addr;
                    if (GLOBAL_UNICAST( (struct in6_addr *)in_addr )) {
                        *ipv6_supported = true;
                    }
                    break;

                default:
                    // Skip all other protocols
                    break;
            }
        }
    }

    freeifaddrs(ifaddr_list);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** nu_ipaddr_is_valid_interface
**
** Determines whether the given interface name is present on the device
**
** \param   dev - name of interface to check
**
** \return  true if the interface exists, false, otherwise
**
**************************************************************************/
bool nu_ipaddr_is_valid_interface(const char *dev)
{
    struct ifaddrs *ifaddr_list;
    struct ifaddrs *iterator;
    int err;
    bool is_found = false;

    // Exit if unable to get a linked list containing the IP Addresses of all network interfaces on the current system
    err = getifaddrs(&ifaddr_list);
    if (err != 0)
    {
        return false;
    }

    // Iterate over all results in the linked list
    for (iterator=ifaddr_list;   iterator!=NULL;   iterator=iterator->ifa_next)
    {
        // Exit the loop, if a match is found
        if (strcmp(iterator->ifa_name, dev) == 0)
        {
            is_found = true;
            break;
        }
    }

    freeifaddrs(ifaddr_list);
    return is_found;
}

/*********************************************************************//**
**
**  tw_ulib_diags_family_to_protocol_version
**
**  Returns the string representing the protocol version to use for DNS lookups of hostname
**  ProtocolVersion selects which DNS record is used when performing the Host lookup for the diagnostic
**
** \param   address_family - address family to convert to a protocol version string
**
** \return  String form of specified IP address family
**
**************************************************************************/
char *tw_ulib_diags_family_to_protocol_version(int address_family)
{
    char *protocol_version;

    switch(address_family)
    {
        case AF_INET:
            protocol_version = "IPv4";
            break;

        case AF_INET6:
            protocol_version = "IPv6";
            break;

        case AF_UNSPEC:
            protocol_version = "Any";
            break;

        default:
            protocol_version = "Unknown";
            break;
    }

    return protocol_version;
}

/*********************************************************************//**
**
**  tw_ulib_diags_lookup_host
**
**  Looks up the specified hostname, converting it into a nu_ipaddr_t IP address structure
**  Note the chosen IP address is determined by the following order :-
**          1) Which globally routable IP addresses the device has
**          2) The address family that the ACS requires (acs_family_pref)
**          3) The local interface IP address that the ACS requires (this may be more specific than the ACS address family
               in the case of address family=ANY, but CPE only has IPv4 or IPv6 address on the ACS specified interface)
**          3) Our dual stack preference
**
** \param   host - pointer to string containing hostname to lookup
** \param   acs_family_pref - The address family that the ACS requires for the Hostname resolution (AF_UNSPEC = don't care)
** \param   prefer_ipv6 - Set to true if we prefer an IPv6 address (and CPE is dual stack, so we have a choice)
** \param   acs_ipaddr_to_bind_to - IP address that the ACS has specified that should be used to contact the remote host (don't care = NULL or the zero address)
** \param   dst - pointer to structure in which to return the IP address of the remote host
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int
tw_ulib_diags_lookup_host(const char *host, int acs_family_pref, bool prefer_ipv6, nu_ipaddr_t *acs_ipaddr_to_bind_to, nu_ipaddr_t *dst)
{
    int err;
    struct addrinfo *addr_list;
    struct addrinfo *iterator;
    struct addrinfo hints;
    int preferred_family;
    bool found_a_result = false;
    int family = AF_INET;
    struct sockaddr_in *a;
    struct sockaddr_in6 *a6;
    bool ipv4_supported;
    bool ipv6_supported;

    // Determine whether to prefer IPv4 or IPv6 addresses on dual stack CPEs (if we have a choice)
    if (prefer_ipv6)
    {
        preferred_family = AF_INET6;
    }
    else
    {
        preferred_family = AF_INET;
    }

    // Update ACS preference based on ACS specified local interface IP address (which may be more specific)
    if ((acs_ipaddr_to_bind_to != NULL) && (nu_ipaddr_is_zero(acs_ipaddr_to_bind_to) == false))
    {
        err = nu_ipaddr_get_family(acs_ipaddr_to_bind_to, (sa_family_t *) &acs_family_pref);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    // Exit if unable to determine which address families are supported by the device
    // NOTE: In theory, setting getaddrinfo hints to AI_ADDRCONFIG, should filter by supported address family
    // However, unfortunately that flag does not take into account whether the address is globally routable (for IPv6) as well
    err = nu_ipaddr_get_ip_supported_families(&ipv4_supported, &ipv6_supported);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Initialise the hints to use, when obtaining the IP address of the specified host using getaddrinfo()
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = acs_family_pref; // Only get DNS records of the address family that the ACS prefers
    hints.ai_flags |= AI_ADDRCONFIG;   // Only provide IPv4 and/or IPv6 addresses of the remote host, if we have a corresponding IPv4 or IPv6 address

    err = getaddrinfo(host, NULL, &hints, &addr_list);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s(host=%s, acs_family_pref=%s): getaddrinfo() failed: %s", __FUNCTION__, host, tw_ulib_diags_family_to_protocol_version(acs_family_pref), gai_strerror(err));
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all results in the linked list, exiting the loop if we have found the preference
    for (iterator=addr_list;   iterator!=NULL;   iterator=iterator->ai_next)
    {
        switch (iterator->ai_family)
        {
            case AF_INET:
                if (ipv4_supported)
                {
                    a = (struct sockaddr_in *) iterator->ai_addr;
                    err = nu_ipaddr_from_inaddr(&a->sin_addr, dst);

                    if (err != USP_ERR_OK)
                    {
                        USP_ERR_SetMessage("%s(%s): nu_ipaddr_from_inaddr() failed: %s", __FUNCTION__, host, strerror(err));
                    }
                    else
                    {
                        family = AF_INET;
                        found_a_result = true;
                    }
                }
                break;

            case AF_INET6:
                if (ipv6_supported)
                {
                    a6 = (struct sockaddr_in6 *) iterator->ai_addr;
                    err = nu_ipaddr_from_in6addr(&a6->sin6_addr, dst);

                    if (err != USP_ERR_OK)
                    {
                        USP_ERR_SetMessage("%s(%s): nu_ipaddr_from_in6addr() failed: %s", __FUNCTION__, host, strerror(err));
                    }
                    else
                    {
                        family = AF_INET6;
                        found_a_result = true;
                    }
                }
                break;

            default:
                // Unexpected address family - Skip it
                continue;
                break;
        }

        // Exit the loop if we have a result which matches what the ACS prefers
        if (acs_family_pref != AF_UNSPEC)
        {
            break;
        }

        // If the code gets here, then the ACS doesn't care if we perform the diagnostic over IPv4 or IPv6
        // In this case, we will use the dual stack preference to determine which we prefer
        // Exit loop if we have found our preference
        if (family == preferred_family)
        {
            break;
        }
    }

    // Exit if no result was found (note if there is no result, we would normally expect the call to getaddrinfo() to fail)
    if (found_a_result==false)
    {
        USP_ERR_SetMessage("%s(%s): failed to resolve", __FUNCTION__, host);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

exit:
    (void) freeaddrinfo(addr_list);
    return err;
}

#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
/*********************************************************************//**
**
** tw_ulib_dev_get_live_wan_address
**
** Gets the current 'live' WAN address (ie not the one stored in the CM DB)
** NOTE: If no IP address is found, then this function will return an empty string and EAGAIN.
**
** \param   buf - pointer to buffer in which to return. This must be at least NU_IPADDRSTRLEN bytes long.
** \param   bufsiz - size of buffer in which to return ASCII form of the IP address
**
** \return  USP_ERR_OK if successful, USP_ERR_INTERNAL_ERROR if no IP address was found, or an error occurred
**
**************************************************************************/
int tw_ulib_dev_get_live_wan_address(char *buf, size_t bufsiz)
{
    int err;
    bool prefer_ipv6;

    // Get name of WAN interface
    const char *dev = nu_macaddr_wan_ifname();

    // Get preference for IPv4 or IPv6 WAN address (in case of Dual Stack CPE)
    prefer_ipv6 = DEVICE_LOCAL_AGENT_GetDualStackPreference();

    // Exit if unable to get current IP address for WAN device.
    err = tw_ulib_get_dev_ipaddr(dev, buf, bufsiz, prefer_ipv6);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}
#endif

/*********************************************************************//**
**
** tw_ulib_get_dev_ipaddr
**
** Gets the current IP address on the specified interface.
** NOTE: If no IP address is found, then this function will return an empty string and USP_ERR_INTERNAL_ERROR.
**
** \param   dev - name of interface to get IP Address of
** \param   addr - buffer in which to return the IP address (which for IPv4 will be a string of the form X.X.X.X)
**                 NOTE: This must be at least NU_IPADDRSTRLEN bytes long.
** \param   asiz - size of buffer in which to return the IP address
** \param   prefer_ipv6 - Set to true if prefer an IPv6 address (and CPE is dual stack, so we have a choice)
**
** \return  USP_ERR_OK if successful, USP_ERR_INTERNAL_ERROR if no IP address was found, or an error occurred
**
**************************************************************************/
int tw_ulib_get_dev_ipaddr(const char *dev, char *addr, size_t asiz, bool prefer_ipv6)
{
    struct ifaddrs *ifaddr_list;
    struct ifaddrs *iterator;
    int err;
    int family;
    void *in_addr;
    int preferred_family;
    char *str;
    bool found_a_result = false;

    // Exit if interface is 'any', this denotes listen on all network interfaces
    if (strcmp(dev, "any")==0)
    {
        if (prefer_ipv6)
        {
            USP_STRNCPY(addr, "[::]", asiz);
        }
        else
        {
            USP_STRNCPY(addr, "0.0.0.0", asiz);
        }
        return USP_ERR_OK;
    }

    // Set the default IP Address returned
    *addr = '\0';

    // Determine whether to prefer IPv4 or IPv6 addresses on dual stack CPEs (if we have a choice)
    if (prefer_ipv6)
    {
        preferred_family = AF_INET6;
    }
    else
    {
        preferred_family = AF_INET;
    }

    // Exit if unable to get a linked list containing the IP Addresses of all network interfaces on the current system
    err = getifaddrs(&ifaddr_list);
    if (err != 0)
    {
        USP_ERR_SetMessage("%s: ERROR: getifaddrs() failed: %s", __FUNCTION__, strerror(errno));
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all results in the linked list
    for (iterator=ifaddr_list;   iterator!=NULL;   iterator=iterator->ifa_next)
    {
        // Skip this result, if it does not match the interface name
        if (strcmp(iterator->ifa_name, dev) != 0)
        {
            continue;
        }

        // Skip this result, if no IP address available (this might be the case for tunnels)
        if (iterator->ifa_addr == NULL)
        {
            continue;
        }

        // Skip this result, if it is not an IPv4 or IPv6 node
        family = iterator->ifa_addr->sa_family;
        if ((family != AF_INET) && (family != AF_INET6))
        {
            continue;
        }

        // Determine pointer to IPv4 or IPv6 address
        if (family == AF_INET)
        {
            in_addr = &((struct sockaddr_in  *)iterator->ifa_addr)->sin_addr;
        }
        else
        {
            in_addr = &((struct sockaddr_in6 *)iterator->ifa_addr)->sin6_addr;
        }

        // Skip this result, if it is an IPv6 address, but not globally routable
        #define NOT_GLOBAL_UNICAST(addr) \
                    ( (IN6_IS_ADDR_UNSPECIFIED(addr)) || (IN6_IS_ADDR_LOOPBACK(addr))  ||   \
                      (IN6_IS_ADDR_MULTICAST(addr))   || (IN6_IS_ADDR_LINKLOCAL(addr)) ||   \
                      (IN6_IS_ADDR_SITELOCAL(addr)) )
        if ((family == AF_INET6) && (NOT_GLOBAL_UNICAST( (struct in6_addr *)in_addr )))
        {
            continue;
        }

        // Skip this result, if unable to get the string form of the IP address
        str = (char *) inet_ntop(family, in_addr, addr, asiz);
        if (str == NULL)
        {
            continue;
        }
        found_a_result = true;

        // Exit the loop if we've found our preferred IPv4 or IPv6 address for this interface
        if (family == preferred_family)
        {
            break;
        }
    }

    if (found_a_result == false)
    {
        // Default to empty string, if no address found on the specified interface
        addr[0] = '\0';
        USP_ERR_SetMessage("%s: No IP address found for interface %s", __FUNCTION__, dev);
        err = USP_ERR_INTERNAL_ERROR;
    }
    else
    {
        err = USP_ERR_OK;
    }

    freeifaddrs(ifaddr_list);

    return err;
}

