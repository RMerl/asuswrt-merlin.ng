/*
* <:copyright-BRCM:2013-2015:proprietary:standard
*
*    Copyright (c) 2013-2015 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/
/*
 * rdpa_dscp_to_pbit_ex.h
 *
 * rdpa_dscp_to_pbit_ex interface toward RDP / XRDP-specific RDD implementations.
 */

#ifndef _RDPA_DSCP_TO_PBIT_EX_H_
#define _RDPA_DSCP_TO_PBIT_EX_H_

#define RDPA_RDD_DSCP_MAPPING_TABLE_UNDEFINED           0xff

/* Init platform-specific module
 */
bdmf_error_t rdpa_rdd_dscp_to_pbit_init(void);

/* Set DSCP to PBIT mapping.
 */
bdmf_error_t rdpa_rdd_vlan_dscp_pbit_mapping_set(struct bdmf_object *mo, uint8_t table, uint8_t dscp, uint8_t pbit);

/* Set DSCP to PBIT mapping for QoS mapping.
 */
bdmf_error_t rdpa_rdd_qos_dscp_pbit_mapping_set(struct bdmf_object *mo, uint8_t dscp, uint8_t pbit);

/* Set DSCP to PBIT+DEI mapping.
 */
bdmf_error_t rdpa_rdd_qos_dscp_pbit_dei_mapping_set(struct bdmf_object *mo, uint8_t dscp, uint8_t pbit, uint8_t dei);

/* Set interface to DSCP-To-PBIT table mapping.
 * Use table_id=RDPA_RDD_DSCP_MAPPING_TABLE_UNDEFINED to remove the existing mapping
 */
bdmf_error_t rdpa_rdd_port_to_dscp_to_pbit_table_set(struct bdmf_object *mo, rdpa_if port, uint8_t table);

#endif /* _RDPA_DSCP_TO_PBIT_EX_H_ */
