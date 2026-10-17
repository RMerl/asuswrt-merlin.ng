/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions for ILK initialization, configuration,
 * and monitoring.
 */

#ifndef __CVMX_HELPER_ILK_H__
#define __CVMX_HELPER_ILK_H__

int __cvmx_helper_ilk_enumerate(int interface);

/**
 * @INTERNAL
 * Clear all calendar entries to the xoff state. This
 * means no data is sent or received.
 *
 * @param interface Interface whose calendar are to be initialized.
 */
void __cvmx_ilk_clear_cal(int interface);

/**
 * @INTERNAL
 * Setup the channel's tx calendar entry.
 *
 * @param interface Interface channel belongs to
 * @param channel Channel whose calendar entry is to be updated
 * @param bpid Bpid assigned to the channel
 */
void __cvmx_ilk_write_tx_cal_entry(int interface, int channel, unsigned char bpid);

/**
 * @INTERNAL
 * Setup the channel's rx calendar entry.
 *
 * @param interface Interface channel belongs to
 * @param channel Channel whose calendar entry is to be updated
 * @param pipe PKO assigned to the channel
 */
void __cvmx_ilk_write_rx_cal_entry(int interface, int channel, unsigned char pipe);

/**
 * @INTERNAL
 * Probe a ILK interface and determine the number of ports
 * connected to it. The ILK interface should still be down after
 * this call.
 *
 * @param xiface Interface to probe
 *
 * Return: Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_ilk_probe(int xiface);

/**
 * @INTERNAL
 * Bringup and enable a ILK interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_ilk_enable(int xiface);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by ILK link status.
 *
 * @param ipd_port IPD/PKO port to query
 *
 * Return: Link state
 */
cvmx_helper_link_info_t __cvmx_helper_ilk_link_get(int ipd_port);

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
int __cvmx_helper_ilk_link_set(int ipd_port, cvmx_helper_link_info_t link_info);

void __cvmx_helper_ilk_show_stats(void);
#endif
