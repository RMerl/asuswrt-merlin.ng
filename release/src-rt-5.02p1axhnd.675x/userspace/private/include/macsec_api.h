/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef _MACSEC_API_H_
#define _MACSEC_API_H_

#include <phy_macsec_common.h>

/*
    Function: set log level
    Input: *ifname - interdace name (i.e. eth0)
           *settings - global MACsec settings
*/
int bcm_macsec_set_log_level(char *ifname, int level);

/*
    Function: Init macsec
    Input: *ifname - interdace name (i.e. eth0)
           *settings - global MACsec settings
*/
int bcm_macsec_init(char *ifname, macsec_api_settings_t *settings);

/*
    Function: Enable/Disable macsec
    Input: *ifname - interdace name (i.e. eth0)
           enable    - enable=1/disable=0
*/
int bcm_macsec_enable_disable(char *ifname, int enable);

/*
    Function: add vPort/SC
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index  - 
*/
int bcm_macsec_vport_add(char *ifname, int direction, int sc_index);

/*
    Function: remove vPort/SC
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index  - 
*/
int bcm_macsec_vport_remove(char *ifname, int direction, int sc_index);

/*
    Function: add SA
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index   
           sa_index
           *sa_params - SA parameters 
*/
int bcm_macsec_sa_add(char *ifname, int direction, int sc_index, int sa_index, macsec_api_sa_t *sa_params);

/*
    Function: chain SA (egress only) 
    Input: *ifname    - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index   
           sa_index
           *sa_params - SA parameters 
*/
int bcm_macsec_sa_chain(char *ifname, int direction, int sc_index, int sa_index, macsec_api_sa_t *sa_params);

/*
    Function: switch SA (egress only) 
    Input: *ifname    - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index   
           sa_index
           *sa_params - SA parameters 
*/
int bcm_macsec_sa_switch(char *ifname, int direction, int sc_index, int sa_index);

/*
    Function: Remove SA
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index   
           sa_index
*/
int bcm_macsec_sa_remove(char *ifname, int direction, int sc_index, int sa_index);

/*
    Function: Add Rule
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index   
           rule_index
           *rule_params - rule parameters
*/
int bcm_macsec_rule_add(char *ifname, int direction, int sc_index, int rule_index, macsec_api_rule_t *rule_params);

/*
    Function: Remove Rule
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           sc_index   
           rule_index
*/
int bcm_macsec_rule_remove(char *ifname, int direction, int sc_index, int rule_index);

/*
    Function: Enable/Disable Rule
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           rule_index
           enable    - enable=1/disable=0
*/
int bcm_macsec_rule_enable(char *ifname, int direction, int rule_index, int enable);

/*
    Function: Get vPort/SC egress statistics
    Input: *ifname   - interface name (i.e. eth0)
           sc_index  
*/
int bcm_macsec_vport_egress_stat_get(char *ifname, int sc_index, macsec_api_secy_e_stats *stats);

/*
    Function: Get vPort/SC ingress statistics
    Input: *ifname   - interface name (i.e. eth0)
           sc_index  
*/
int bcm_macsec_vport_ingress_stat_get(char *ifname, int sc_index, macsec_api_secy_i_stats *stats);

/*
    Function: Get TCAM statistics
    Input: *ifname   - interface name (i.e. eth0)
           direction - egress=0, ingress=1
           rule_index  
*/
int bcm_macsec_tcam_stat_get(char *ifname, int direction, int rule_index, macsec_api_secy_tcam_stats *stats);

/*
    Function: Get RXCAM statistics (for ingress only)
    Input: *ifname   - interface name (i.e. eth0)
           sc_index  
*/
int bcm_macsec_rxcam_stat_get(char *ifname, int sc_index, macsec_api_secy_rxcam_stats *stats);

/*
    Function: Get SA egress statistics
    Input: *ifname   - interface name (i.e. eth0)
           sc_index  
*/
int bcm_macsec_sa_egress_stat_get(char *ifname, int sa_index, macsec_api_secy_sa_e_stats *stats);

/*
    Function: Get SA ingress statistics
    Input: *ifname   - interface name (i.e. eth0)
           sc_index  
*/
int bcm_macsec_sa_ingress_stat_get(char *ifname, int sa_index, macsec_api_secy_sa_i_stats *stats);

#endif