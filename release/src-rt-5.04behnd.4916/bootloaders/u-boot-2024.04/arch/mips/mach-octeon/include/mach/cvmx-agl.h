/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for AGL (RGMII) commong initialization, configuration.
 */

#ifndef __CVMX_AGL_H__
#define __CVMX_AGL_H__

/*
 * @param port to enable
 *
 * @return Zero on success, negative on failure
 */
int cvmx_agl_enable(int port);

cvmx_helper_link_info_t cvmx_agl_link_get(int port);

/*
 * Set MII/RGMII link based on mode.
 *
 * @param port   interface port to set the link.
 * @param link_info  Link status
 *
 * @return       0 on success and 1 on failure
 */
int cvmx_agl_link_set(int port, cvmx_helper_link_info_t link_info);

/**
 * Disables the sending of flow control (pause) frames on the specified
 * AGL (RGMII) port(s).
 *
 * @param interface Which interface (0 or 1)
 * @param port_mask Mask (4bits) of which ports on the interface to disable
 *                  backpressure on.
 *                  1 => disable backpressure
 *                  0 => enable backpressure
 *
 * @return 0 on success
 *         -1 on error
 */
int cvmx_agl_set_backpressure_override(u32 interface, uint32_t port_mask);

#endif /* __CVMX_AGL_H__ */
