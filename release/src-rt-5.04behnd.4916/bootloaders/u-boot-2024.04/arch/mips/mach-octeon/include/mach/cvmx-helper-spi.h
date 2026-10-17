/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions for SPI initialization, configuration,
 * and monitoring.
 */

#ifndef __CVMX_HELPER_SPI_H__
#define __CVMX_HELPER_SPI_H__

#include "cvmx-helper.h"

/**
 * @INTERNAL
 * Probe a SPI interface and determine the number of ports
 * connected to it. The SPI interface should still be down after
 * this call.
 *
 * @param interface Interface to probe
 *
 * Return: Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_spi_probe(int interface);
int __cvmx_helper_spi_enumerate(int interface);

/**
 * @INTERNAL
 * Bringup and enable a SPI interface. After this call packet I/O
 * should be fully functional. This is called with IPD enabled but
 * PKO disabled.
 *
 * @param interface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_spi_enable(int interface);

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
cvmx_helper_link_info_t __cvmx_helper_spi_link_get(int ipd_port);

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
int __cvmx_helper_spi_link_set(int ipd_port, cvmx_helper_link_info_t link_info);

/**
 * Sets the spi timeout in config data
 * @param timeout value
 */
void cvmx_spi_config_set_timeout(int timeout);

#endif
