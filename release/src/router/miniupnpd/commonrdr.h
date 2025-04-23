/* $Id: commonrdr.h,v 1.17 2025/04/03 21:11:34 nanard Exp $ */
/* MiniUPnP project
 * (c) 2006-2025 Thomas Bernard
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef COMMONRDR_H_INCLUDED
#define COMMONRDR_H_INCLUDED

/*! \file commonrdr.h
 * \brief common API for all backends
 */

#include "config.h"

/* init and shutdown functions */

/*! \brief init the backend
 * \return 0=OK, -1=error
 */
int
init_redirect(void);

/*! \brief shutdown the backend */
void
shutdown_redirect(void);

/*!
 * \param[in] ifname external interface name
 * \return -1 for error or the number of redirection rules
 */
int
get_redirect_rule_count(const char * ifname);



/*! \brief get port mapping by external port and protocol
 * \param[in] ifname WAN interface name
 * \param[in] eport external port
 * \param[in] proto IPPROTO_TCP/IPPROTO_UDP/etc.
 * \return 0 on success, -1 on error or rule not found */
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * timestamp,
                  u_int64_t * packets, u_int64_t * bytes);

/*! \brief get port mapping by index
 * \param[in] index
 * \return 0 on success, -1 on error or rule not found */
int
get_redirect_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * timestamp,
                           u_int64_t * packets, u_int64_t * bytes);

/*! \brief get "external" ports for which there is a mapping
 * \param[in] startport start of port range
 * \param[in] endport end of port range
 * \param[in] proto IPPROTO_TCP/IPPROTO_UDP/etc.
 * \param[out] size of the returned array
 * \return an (malloc'ed) array */
unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          int proto, unsigned int * number);

/*! \brief update the port mapping internal port, description and timestamp
 * \param[in] ifname WAN interface name
 * \param[in] eport external port
 * \param[in] proto IPPROTO_TCP/IPPROTO_UDP/etc.
 * \param[in] iport internal port to set
 * \param[in] desc new description to set
 * \param[in] timestamp new end of port mapping timestamp to set
 * \return 0 on success, -1 on failure */
int
update_portmapping(const char * ifname, unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp);

/*! \brief update the port mapping description and timestamp
 * \param[in] ifname WAN interface name
 * \param[in] eport external port
 * \param[in] proto IPPROTO_TCP/IPPROTO_UDP/etc.
 * \param[in] desc new description to set
 * \param[in] timestamp new end of port mapping timestamp to set
 * \return 0 on success, -1 on failure */
int
update_portmapping_desc_timestamp(const char * ifname,
                   unsigned short eport, int proto,
                   const char * desc, unsigned int timestamp);

#if defined(USE_NETFILTER)
/*
 * only provided by nftables implementation at the moment.
 * Should be implemented for iptables too, for consistency
 */

typedef enum {
	RDR_TABLE_NAME,
	RDR_NAT_TABLE_NAME,
	RDR_NAT_PREROUTING_CHAIN_NAME,
	RDR_NAT_POSTROUTING_CHAIN_NAME,
	RDR_FORWARD_CHAIN_NAME,
	RDR_FAMILY_SPLIT,
} rdr_name_type;

/*
 * used by the config file parsing in the core
 * to set
 */

int set_rdr_name( rdr_name_type param, const char * string );

#endif

#endif
