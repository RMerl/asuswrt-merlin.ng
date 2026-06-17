/* $Id: getifstats.h,v 1.5 2025/04/03 21:11:35 nanard Exp $ */
/* MiniUPnP project
 * (c) 2006-2025 Thomas Bernard
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef GETIFSTATS_H_INCLUDED
#define GETIFSTATS_H_INCLUDED

/*! \file getifstats.h
 * \brief get network interface statistics
 *
 * Must be implemented for each supported system (BSD/Linux/etc.)
 */

struct ifdata {
	unsigned long opackets;	/*!< \brief output packets */
	unsigned long ipackets;	/*!< \brief input packets */
	unsigned long obytes;	/*!< \brief output bytes */
	unsigned long ibytes;	/*!< \brief input bytes */
	unsigned long baudrate;	/*!< \brief bits per seconds */
};

/*! \brief get interface statistics.
 *
 * Fill the ifdata structure with statistics for network interface ifname.
 * \param[in] ifname network interface name
 * \param[out] data statistics
 * \return 0 on success, -1 on bad arguments or any error */
int
getifstats(const char * ifname, struct ifdata * data);

#endif
