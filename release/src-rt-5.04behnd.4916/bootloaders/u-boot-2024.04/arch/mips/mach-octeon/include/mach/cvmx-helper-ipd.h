/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper functions for IPD
 */

#ifndef __CVMX_HELPER_IPD_H__
#define __CVMX_HELPER_IPD_H__

void cvmx_helper_ipd_set_wqe_no_ptr_mode(bool mode);
void cvmx_helper_ipd_pkt_wqe_le_mode(bool mode);
int __cvmx_helper_ipd_global_setup(void);
int __cvmx_helper_ipd_setup_interface(int interface);

#endif /* __CVMX_HELPER_PKI_H__ */
