/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions for LOOP initialization, configuration,
 * and monitoring.
 */

#ifndef __CVMX_HELPER_LOOP_H__
#define __CVMX_HELPER_LOOP_H__

/**
 * @INTERNAL
 * Probe a LOOP interface and determine the number of ports
 * connected to it. The LOOP interface should still be down after
 * this call.
 *
 * @param xiface Interface to probe
 *
 * Return: Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_loop_probe(int xiface);
int __cvmx_helper_loop_enumerate(int xiface);

/**
 * @INTERNAL
 * Bringup and enable a LOOP interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_loop_enable(int xiface);

#endif
