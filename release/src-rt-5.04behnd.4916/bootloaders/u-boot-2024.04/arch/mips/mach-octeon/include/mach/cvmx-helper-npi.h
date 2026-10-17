/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions for NPI initialization, configuration,
 * and monitoring.
 */

#ifndef __CVMX_HELPER_NPI_H__
#define __CVMX_HELPER_NPI_H__

/**
 * @INTERNAL
 * Probe a NPI interface and determine the number of ports
 * connected to it. The NPI interface should still be down after
 * this call.
 *
 * @param interface Interface to probe
 *
 * Return: Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_npi_probe(int interface);

/**
 * @INTERNAL
 * Bringup and enable a NPI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_npi_enable(int xiface);

/**
 * Sets the number of pipe used by SLI packet output in the variable,
 * which then later used for setting it up in HW
 */
void cvmx_npi_config_set_num_pipes(int num_pipes);

#endif
