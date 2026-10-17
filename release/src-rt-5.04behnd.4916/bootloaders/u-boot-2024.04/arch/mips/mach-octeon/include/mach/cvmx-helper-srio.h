/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions for SRIO initialization, configuration,
 * and monitoring.
 */

#ifndef __CVMX_HELPER_SRIO_H__
#define __CVMX_HELPER_SRIO_H__

/**
 * @INTERNAL
 * Convert interface number to sRIO link number
 * per SoC model.
 *
 * @param xiface Interface to convert
 *
 * Return: Srio link number
 */
int __cvmx_helper_srio_port(int xiface);

/**
 * @INTERNAL
 * Probe a SRIO interface and determine the number of ports
 * connected to it. The SRIO interface should still be down after
 * this call.
 *
 * @param xiface Interface to probe
 *
 * Return: Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_srio_probe(int xiface);

/**
 * @INTERNAL
 * Bringup and enable a SRIO interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_srio_enable(int xiface);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by SRIO link status.
 *
 * @param ipd_port IPD/PKO port to query
 *
 * Return: Link state
 */
cvmx_helper_link_info_t __cvmx_helper_srio_link_get(int ipd_port);

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
int __cvmx_helper_srio_link_set(int ipd_port, cvmx_helper_link_info_t link_info);

#endif
