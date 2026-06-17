/* $Id: getroute.h,v 1.4 2025/04/03 21:11:35 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2025 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef  GETROUTE_H_INCLUDED
#define  GETROUTE_H_INCLUDED

/*! \file getroute.h
 * \brief access system network routes
 *
 * Must be implemented for each supported system (BSD/Linux/etc.)
 */

/*! \brief get interface for route to
 *
 * \param[in] dst destination of the route
 * \param[out] src pointer to a struct in_addr or in6_addr
 * \param[in] src_len src buffer length
 * \param[out] index interface index
 * \return 0 on success, -1 on error
 */
int
get_src_for_route_to(const struct sockaddr * dst,
                     void * src, size_t * src_len,
                     int * index);

#endif
