/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef _FSL_MC_PRIVATE_H_
#define _FSL_MC_PRIVATE_H_

#include <errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/stringify.h>
#include <phy.h>

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dprc.h>
#include <fsl-mc/fsl_dpbp.h>
#include <fsl-mc/fsl_dpni.h>

extern struct fsl_mc_io *dflt_mc_io;

/**
 * struct dpbp_node - DPBP strucuture
 * @uint16_t handle: DPBP object handle
 * @struct dpbp_attr: DPBP attribute
 */
struct fsl_dpbp_obj {
	uint32_t dpbp_id;
	uint16_t dpbp_handle;
	struct dpbp_attr dpbp_attr;
};

extern struct fsl_dpbp_obj *dflt_dpbp;

/**
 * struct fsl_dpio_obj - DPIO strucuture
 * @int dpio_id: DPIO id
 * @struct qbman_swp *sw_portal: SW portal object
 */
struct fsl_dpio_obj {
	uint32_t dpio_id;
	uint16_t dpio_handle;
	struct qbman_swp *sw_portal; /** SW portal object */
};

extern struct fsl_dpio_obj *dflt_dpio;

/**
 * struct dpni_node - DPNI strucuture
 * @int dpni_id: DPNI id
 * @uint16_t handle: DPNI object handle
 * @struct dpni_attr: DPNI attributes
 * @struct dpni_buffer_layout: DPNI buffer layout
 */
struct fsl_dpni_obj {
	uint32_t dpni_id;
	uint16_t dpni_handle;
	struct dpni_attr dpni_attrs;
	struct dpni_buffer_layout buf_layout;
};

extern struct fsl_dpni_obj *dflt_dpni;

int mc_init(u64 mc_fw_addr, u64 mc_dpc_addr);
int ldpaa_eth_init(int dpmac_id, phy_interface_t enet_if);
int mc_apply_dpl(u64 mc_dpl_addr);
#endif /* _FSL_MC_PRIVATE_H_ */
