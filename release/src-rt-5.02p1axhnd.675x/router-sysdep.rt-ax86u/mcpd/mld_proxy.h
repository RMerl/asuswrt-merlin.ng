/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
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
/***************************************************************************
 * File Name  : mld_proxy.h
 *
 * Description: API prototype for MLD proxy
 *              
 ***************************************************************************/
#ifndef __MLD_PROXY_H__
#define __MLD_PROXY_H__

/** init mld proxy
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_proxy_init(void);

/** arrange upstream ssm source list
 *
 * @param nsources     (IN) # of sources
 *
 * @param *sources     (IN) pointer to source array
 *
 * @return None
 *
 */
void mcpd_mld_order_upstream_ssm_source_list(int nsources, 
                                             struct in6_addr *sources);
/** process upstream ssm source list
 *
 * @param *gp          (IN) pointer to group 
 *
 * @param *ptr_ssm_info (IN) pointer to ptr_ssm_info
 *
 * @param *rep         (IN) pointer to reportor
 *
 * @return None
 *
 */
t_MCPD_RET_CODE mcpd_mld_update_upstream_ssm(t_MCPD_GROUP_OBJ *gp, 
                                   UINT8 *ptr_ssm_info);

/** update kernel IPv6 Multicast MLD filters
 *
 * @param *ifp         (IN) pointer to interface object
 *
 * @param *gp          (IN) pointer to group object
 *
 * @param *ptr_ssm_info (IN) pointer ssm info
 *
 * @param *pkt_info    (IN) pointer to packet info
 *
 * @return t_MCPD_RET_CODE 
 *
 */
t_MCPD_RET_CODE mcpd_mld_krnl_update_ssm_filters(t_MCPD_INTERFACE_OBJ *ifp,
                                                 t_MCPD_GROUP_OBJ *gp,
                                                 UINT8 *ptr_ssm_info,
                                                 t_MCPD_PKT_INFO *pkt_info);

/** drop membership from kernel, If ifp is NULL then membership of each 
 *  upstream interface is changed if no more DS members exist. If ifp
 *  is not NULL, then only memberhsip for ifp is dropped
 *
 * @param *ifp      (IN) interface object
 *
 * @param *gp       (IN) group object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_krnl_drop_membership(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_GROUP_OBJ *gp);

#endif /* __MLD_PROXY_H__ */
