/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions for RGMII/GMII/MII initialization, configuration,
 * and monitoring.
 */

#ifndef __CVMX_HELPER_RGMII_H__
#define __CVMX_HELPER_RGMII_H__

/**
 * @INTERNAL
 * Probe RGMII ports and determine the number present
 *
 * @param xiface Interface to probe
 *
 * Return: Number of RGMII/GMII/MII ports (0-4).
 */
int __cvmx_helper_rgmii_probe(int xiface);

/**
 * Put an RGMII interface in loopback mode. Internal packets sent
 * out will be received back again on the same port. Externally
 * received packets will echo back out.
 *
 * @param port   IPD port number to loop.
 */
void cvmx_helper_rgmii_internal_loopback(int port);

/**
 * @INTERNAL
 * Configure all of the ASX, GMX, and PKO regsiters required
 * to get RGMII to function on the supplied interface.
 *
 * @param xiface PKO Interface to configure (0 or 1)
 *
 * Return: Zero on success
 */
int __cvmx_helper_rgmii_enable(int xiface);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set().
 *
 * @param ipd_port IPD/PKO port to query
 *
 * Return: Link state
 */
cvmx_helper_link_info_t __cvmx_helper_gmii_link_get(int ipd_port);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set().
 *
 * @param ipd_port IPD/PKO port to query
 *
 * Return: Link state
 */
cvmx_helper_link_info_t __cvmx_helper_rgmii_link_get(int ipd_port);

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead.
 *
 * @param ipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_rgmii_link_set(int ipd_port, cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again.
 *
 * @param ipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * Return: Zero on success, negative on failure.
 */
int __cvmx_helper_rgmii_configure_loopback(int ipd_port, int enable_internal, int enable_external);

#endif
