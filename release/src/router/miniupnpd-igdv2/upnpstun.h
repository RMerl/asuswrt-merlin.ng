/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2018 Pali Rohár
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPSTUN_H_INCLUDED
#define UPNPSTUN_H_INCLUDED

/*! \file upnpstun.h
 * \brief STUN client implementation */

/*! \brief Perform main STUN operation
 *
 * return external IP address and check
 * if host is behind restrictive, symmetric NAT or behind firewall.
 * Restrictive NAT means any NAT which do some filtering and
 * which is not static full-cone NAT 1:1, basically NAT which is not usable
 * for port forwarding
 * \param[in] if_name WAN network interface name
 * \param[in] if_addr ip v4 address for WAN interface
 * \param[in] stun_host STUN server hostname
 * \param[in] stun_port STUN server port
 * \param[out] ext_addr detected address
 * \param[out] restrictive_nat 0=unrestricted, 1=some restriction
 * \return 0 on success, -1 on error */
int perform_stun(const char *if_name, const char *if_addr, const char *stun_host, unsigned short stun_port, struct in_addr *ext_addr, int *restrictive_nat);

#endif
