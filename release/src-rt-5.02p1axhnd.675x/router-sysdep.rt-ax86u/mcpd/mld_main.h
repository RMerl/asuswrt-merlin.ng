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
 * File Name  : mld_main.h
 *
 * Description: API prototype for processing mld messages
 *              
 ***************************************************************************/
#ifndef __MLD_MAIN_H__
#define __MLD_MAIN_H__

/** init mld module
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_init(void);

/** init the mld enabled interface
 *
 * @param *ifp         (IN) pointer to interface object 
 *
 * @return 0 on success
 *
 */
t_MCPD_RET_CODE mcpd_mld_interface_init(t_MCPD_INTERFACE_OBJ *ifp);

/** process mld messages
 *
 * @param *pkt_info    (IN) pointer to packet info
 * 
 * @return 0 on success
 *
 */
t_MCPD_RET_CODE mcpd_mld_process_input(t_MCPD_PKT_INFO *pkt_info);

/** process mld reports for version 1
 *
 * @param *ifp         (IN) pointer to interface object
 *
 * @param *src         (IN) pointer to source ip
 *
 * @param *report      (IN) pointer to report 
 *
 * @param len          (IN) len
 *
 * @param *pkt_info    (IN) pointer to packet info
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_interface_membership_report_v1(t_MCPD_INTERFACE_OBJ * ifp,
                                             struct in6_addr *src, 
                                             t_MLDv1_REPORT *report,
                                             int len, 
                                             t_MCPD_PKT_INFO *pkt_info);

/** process mld leave reports for version 1
 *
 * @param *ifp         (IN) pointer to interface object
 *
 * @param *src         (IN) pointer to source ip
 *
 * @param *report      (IN) pointer to report 
 *
 * @param len          (IN) len
 *
 * @param *pkt_info    (IN) pointer to packet info
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_interface_membership_leave_v1(t_MCPD_INTERFACE_OBJ * ifp,
                                            struct in6_addr *src,
                                            t_MLDv1_REPORT *report,
                                            int len,
                                            t_MCPD_PKT_INFO *pkt_info);

/** process mld leave reports for version 2
 *
 * @param *ifp         (IN) pointer to interface object
 *
 * @param *src         (IN) pointer to source ip
 *
 * @param *report      (IN) pointer to report 
 *
 * @param len          (IN) len
 *
 * @param *pkt_info    (IN) pointer to packet info
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_interface_membership_report_v2(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in6_addr *src,
                                              t_MLDv2_REPORT *report,
                                              int len, 
                                              t_MCPD_PKT_INFO *pkt_info);

/** process other querier timer
 *
 * @param *ifp         (IN) pointer to interface object
 *
 * @return None
 *
 */
void mcpd_mld_timer_querier(t_MCPD_INTERFACE_OBJ *ifp);

/** process group timers
 *
 * @param *handle      (IN) pointer to group object
 *
 * @return None
 *
 */
void mcpd_mld_timer_group(void *handle);

/** process reporter timers
 *
 * @param *handle      (IN) pointer to group object
 *
 * @return None
 *
 */
void mcpd_mld_timer_reporter(void *handle);

/** process group source timers
 *
 * @param *handle      (IN) pointer to source obj
 *
 * @return None
 *
 */
void mcpd_mld_timer_source(void *handle);

/** send mld membership query
 *
 * @param *ifp         (IN) pointer to interface object
 *
 * @param *group       (IN) pointer to group
 *
 * @param *sources     (IN) pointer to source
 *
 * @param numsrc       (IN) # of sources
 *
 * @param SRSP         (IN) router side processing
 *
 * @param leave_qry    (IN) is leave query
 *
 * @return None
 *
 */
void mcpd_mld_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                struct in6_addr *group, 
                                struct in6_addr *sources, 
                                int numsrc, 
                                int SRSP,
                                int leave_qry);

/** process mld membership query
 *
 * @param *ifp         (IN) pointer to interface object
 *
 * @param gp           (IN) IPv6 address
 * @param *sources     (IN) pointer to sources
 * @param src_query    (IN) source address
 * @param numsrc       (IN) # of sources
 * @param srsp         (IN) router side processing
 *
 * @return None
 *
 */
void mcpd_mld_receive_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                   struct in6_addr gp,
                                   struct in6_addr *sources,
                                   UINT32 src_query,
                                   int numsrc,
                                   int srsp);


/** update the group timer for last member queries
 *
 * @param *gp          (IN) pointer to reporter object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_set_last_member_rep_timer(t_MCPD_REP_OBJ *rep);

/** update the group timer 
 *
 * @param *gp          (IN) pointer to reporter object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_reset_rep_timer(t_MCPD_REP_OBJ *rep);

/** update reporter timer
 *
 * @param *rep         (IN) pointer to source object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_update_source_timer(t_MCPD_SRC_OBJ *src);

/** process MLDv1 backward compatibility timer
 *
 * @param *handle      (IN) pointer to group
 *
 * @return None
 *
 */
void mcpd_mld_v1_bckcomp_tmr(void *handle);


/** process mld leave membership query
 *
 * @param *handle      (IN) pointer to group object
 *
 * @return None
 *
 */
void mpcd_mld_last_member_query_tmr(void *handle);

/** process mld leave membership query
 *
 * @param *ifp         (IN) pointer to t_MCPD_INTERFACE_OBJ
 * @param *gp          (IN) pointer to t_MCPD_GROUP_OBJ
 * @param *srcs        (IN) pointer to UINT8
 * @param numsrc       (IN) int value of no. of sources
 *
 * @return None
 *
 */
void mcpd_mld_leave_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                      t_MCPD_GROUP_OBJ *gp,
                                      UINT8 *srcs,
                                      int numsrc);

t_MCPD_RET_CODE mcpd_mld_process_group_record(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in6_addr *src,
                                              t_MLDv2_REPORT *report,
                                              int len,
                                              t_MCPD_PKT_INFO *pkt_info,
                                              t_MLD_GRP_RECORD *grp_rec);

/** determine if the join request is allowed
 *
 * @param msg_type        (IN) message type
 *
 * @param rxdev_ifi       (IN) receive interface
 *
 * @param *gp             (IN) group address
 *
 * @param *src            (IN) multicast source address
 *
 * @param *rep            (IN) reporter address
 *
 * @param tci             (IN) vlan
 *
 * @param rep_proto_ver   (IN) IGMP message type
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_admission_control(int msg_type,
                                           int rxdev_ifi,
                                           UINT8 *gp,
                                           UINT8 *src,
                                           UINT8 *rep,
                                           unsigned short tci,
                                           UINT8 rep_proto_ver);

#endif /* __MLD_MAIN_H__*/
