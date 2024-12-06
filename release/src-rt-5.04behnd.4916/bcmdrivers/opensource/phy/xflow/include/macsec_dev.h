/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#ifndef _MACSEC_DEV_H
#define _MACSEC_DEV_H

#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"

/*
 * Context struct to keep track of macsec specific
 * parameters for each phy instance
 */
typedef struct
{
    uint8                                   enabled;
    int                                     macsec_unit;
    int                                     macsec_port;
    int                                     mac_port;
    int                                     assoc_num;
    int                                     include_sci;
    bcm_xflow_macsec_secure_chan_id_t       chan_id_encrypt;
    bcm_xflow_macsec_secure_chan_id_t       chan_id_decrypt;
    bcm_xflow_macsec_secure_assoc_id_t      assoc_id_encrypt[2];
    bcm_xflow_macsec_secure_assoc_id_t      assoc_id_decrypt[4];
    bcm_xflow_macsec_policy_id_t            policy_id_encrypt;
    bcm_xflow_macsec_policy_id_t            policy_id_decrypt;
    bcm_xflow_macsec_flow_id_t              flow_id_decrypt;
    uint64                                  egress_sci;
    uint64                                  ingress_sci;
    uint64                                  egress_sci_mask;
    uint64                                  ingress_sci_mask;
    unsigned int                            svtag_sig;
    uint32                                  ssci_encrypt;
    uint32                                  ssci_decrypt;
    uint8                                   salt_encrypt[12];
    uint8                                   salt_decrypt[12];
} macsec_dev_t;

extern macsec_dev_t msec_devs[SOC_MAX_NUM_DEVICES][CMBB_FL_MACSEC_MAX_PORT_NUM];
#endif
