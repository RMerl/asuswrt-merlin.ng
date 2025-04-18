/* $Id: getconnstatus.h,v 1.5 2025/04/03 21:11:35 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2011-2025 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef GETCONNSTATUS_H_INCLUDED
#define GETCONNSTATUS_H_INCLUDED

/*! \file getconnstatus.h
 */

/*! \brief Unconfigured */
#define STATUS_UNCONFIGURED (0)
/*! \brief Connecting */
#define STATUS_CONNECTING (1)
/*! \brief Connected */
#define STATUS_CONNECTED (2)
/*! \brief PendingDisconnect */
#define STATUS_PENDINGDISCONNECT (3)
/*! \brief Disconnecting */
#define STATUS_DISCONNECTING (4)
/*! \brief Disconnected */
#define STATUS_DISCONNECTED (5)

/** \brief get the connection status of a network interface
 *
 * \param[in] ifname network interface name
 * \return #STATUS_UNCONFIGURED, #STATUS_CONNECTING, #STATUS_CONNECTED,
 *         #STATUS_PENDINGDISCONNECT, #STATUS_DISCONNECTING, #STATUS_DISCONNECTED
 */
int
get_wan_connection_status(const char * ifname);

/** \brief get the connection status of a network interface
 *
 * return the same value as get_wan_connection_status()
 * as a C string
 * \param[in] ifname network interface name */
const char *
get_wan_connection_status_str(const char * ifname);

#endif
