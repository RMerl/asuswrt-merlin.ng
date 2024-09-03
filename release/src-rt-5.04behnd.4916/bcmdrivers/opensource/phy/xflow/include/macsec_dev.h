/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
