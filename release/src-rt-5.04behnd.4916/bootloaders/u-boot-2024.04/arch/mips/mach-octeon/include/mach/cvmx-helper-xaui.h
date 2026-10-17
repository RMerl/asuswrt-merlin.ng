/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions for XAUI initialization, configuration,
 * and monitoring.
 */

#ifndef __CVMX_HELPER_XAUI_H__
#define __CVMX_HELPER_XAUI_H__

/**
 * @INTERNAL
 * Probe a XAUI interface and determine the number of ports
 * connected to it. The XAUI interface should still be down
 * after this call.
 *
 * @param xiface Interface to probe
 *
 * Return: Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_xaui_probe(int xiface);
int __cvmx_helper_xaui_enumerate(int xiface);

/**
 * @INTERNAL
 * Bringup and enable a XAUI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_xaui_enable(int xiface);

/**
 * Retrain XAUI interface.
 *
 * GMX is disabled as part of retraining.
 * While GMX is disabled, new received packets are dropped.
 * If GMX was in the middle of recieving a packet when disabled,
 * that packet will be received before GMX idles.
 * Transmitted packets are buffered normally, but not sent.
 * If GMX was in the middle of transmitting a packet when disabled,
 * that packet will be transmitted before GMX idles.
 *
 * @param interface Interface to retrain
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_helper_xaui_link_retrain(int interface);

/**
 * Reinitialize XAUI interface.  Does a probe without changing the hardware
 * state.
 *
 * @param interface	Interface to reinitialize
 *
 * Return:	0 on success, negative on failure
 */
int cvmx_helper_xaui_link_reinit(int interface);

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
cvmx_helper_link_info_t __cvmx_helper_xaui_link_get(int ipd_port);

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
int __cvmx_helper_xaui_link_set(int ipd_port, cvmx_helper_link_info_t link_info);

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
int __cvmx_helper_xaui_configure_loopback(int ipd_port, int enable_internal, int enable_external);

#endif
