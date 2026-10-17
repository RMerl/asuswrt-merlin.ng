/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the GMX hardware.
 */

#ifndef __CVMX_GMX_H__
#define __CVMX_GMX_H__

/* CSR typedefs have been moved to cvmx-gmx-defs.h */

int cvmx_gmx_set_backpressure_override(u32 interface, u32 port_mask);
int cvmx_agl_set_backpressure_override(u32 interface, u32 port_mask);

#endif
