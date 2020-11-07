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
 * File Name  : igmp_main.h
 *
 * Description: API for igmp protocol processing
 *              
 ***************************************************************************/
#ifndef __IGMP_MAIN_H__
#define __IGMP_MAIN_H__

/** initialize IGMP socket and objects
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_init(void);

/** initialize the interfaces for IGMP protocol
 *
 * @param *ifp       (IN) interface object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_interface_init(t_MCPD_INTERFACE_OBJ *ifp);

/** process the IGMP messages from network
 *
 * @param *pkt_info       (IN) packet info related to bridge and if
 *  
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_process_input(t_MCPD_PKT_INFO *pkt_info);

/** process the IGMP report messages for version 1/2
 *
 * @param *ifp            (IN) interface object
 *
 * @param src             (IN) IP address
 *
 * @param *report         (IN) IGMPv1/2 report message
 *
 * @param *pkt_info       (IN) packet info
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_interface_membership_report_v12(t_MCPD_INTERFACE_OBJ *ifp,
                                               struct in_addr *src,
                                               t_IGMPv12_REPORT *report,
                                               t_MCPD_PKT_INFO *pkt_info);

/** process the IGMP leave messages for version 1/2
 *
 * @param *ifp            (IN) interface object
 *
 * @param src             (IN) IP address
 *
 * @param *report         (IN) IGMPv1/2 report message
 *
 * @param *pkt_info       (IN) packet info
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_interface_membership_leave_v2(t_MCPD_INTERFACE_OBJ *ifp,
                                             struct in_addr *src,
                                             t_IGMPv12_REPORT *report,
                                             t_MCPD_PKT_INFO *pkt_info);

/** process the IGMP report messages for version 3 
 *
 * @param *ifp            (IN) interface object
 *
 * @param src             (IN) IP address
 *
 * @param *report         (IN) IGMPv3 report message
 *
 * @param *pkt_info       (IN) packet info
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_interface_membership_report_v3(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in_addr *src,
                                              t_IGMPv3_REPORT *report,
                                              t_MCPD_PKT_INFO *pkt_info);

/** process the querier timers 
 *
 * @param *ifp            (IN) interface object
 *
 * @return None
 *
 */
void mcpd_igmp_timer_querier(t_MCPD_INTERFACE_OBJ *ifp);

/** process the group timers
 *
 * @param *handle         (IN) handle
 *
 * @return None
 *
 */
void mcpd_igmp_timer_group(void *handle);

/** process the reporter timers
 *
 * @param *handle         (IN) handle
 *
 * @return None
 *
 */
void mcpd_igmp_timer_reporter(void *handle);

/** process the source timers
 *
 * @param *handle        (IN) handle object
 *
 * @return None
 *
 */
void mcpd_igmp_timer_source(void *handle);

/** send IGMP membership query
 *
 * @param *ifp            (IN) interface object
 *
 * @param *group          (IN) group 
 *
 * @param *sources        (IN) sources 
 *
 * @param numsrc          (IN) number of sources
 *
 * @param SRSP            (IN) router side processing
 *
 * @param leave_qry       (IN) is leave query
 *
 * @return None
 *
 */
void mcpd_igmp_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                struct in_addr *group, 
                                struct in_addr *sources, 
                                int numsrc, 
                                int SRSP,
                                int leave_qry);

/** process membership query
 *
 * @param *ifp            (IN) interface object
 *
 * @param gp              (IN) group object
 *
 * @param *sources        (IN) sources object
 *
 * @param src_query       (IN) sources query addr
 *
 * @param numsrc          (IN) number of sources
 *
 * @param srsp            (IN) router side processing
 *
 * @return None
 *
 */
void mcpd_igmp_receive_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                   struct in_addr *gp,
                                   struct in_addr *sources,
                                   struct in_addr *src_query,
                                   int numsrc,
                                   int srsp );

/** set IGMP timer to GSQ Timeout Interval
 *
 * @param *rep              (IN) reporter object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_set_last_member_rep_timer(t_MCPD_REP_OBJ *rep);

/** set IGMP timer to GMQ Timeout Interval
 *
 * @param *rep              (IN) reporter object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_reset_rep_timer(t_MCPD_REP_OBJ *rep);

/** update igmp source timer
 *
 * @param *rep            (IN) pointer to reporter object
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_update_source_timer(t_MCPD_SRC_OBJ *src);

/** update igmp v2 backward compatibility timer
 *
 * @param *handle         (IN) group object
 *
 * @return None
 *
 */
void mcpd_igmp_v2_bckcomp_tmr(void *handle);

/** update igmp v1 backward compatilibility timer
 *
 * @param *handle         (IN) group object
 *
 * @return None
 *
 */
void mcpd_igmp_v1_bckcomp_tmr(void *handle);


/** process igmp leave membership query (set timers and begin GSQs)
 *
 * @param *handle      (IN) pointer to t_MCPD_GROUP_OBJ (cast as void)
 *
 * @return None
 *
 */
void mpcd_igmpv2_last_member_query_tmr(void *handle);

/** process igmp leave membership query
 *
 * @param *ifp         (IN) pointer to t_MCPD_INTERFACE_OBJ
 * @param *gp          (IN) pointer to t_MCPD_GROUP_OBJ
 * @param *srcs        (IN) pointer to UINT8
 * @param numsrc       (IN) int value of no. of sources
 *
 * @return None
 *
 */
void mcpd_igmp_leave_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                      t_MCPD_GROUP_OBJ *gp,
                                      UINT8 *srcs,
                                      int numsrc);

void mcpd_igmp_send_membership_report(struct in_addr *group,
                                struct in_addr *sources,
                                int numsrc, int filter_mode);

/** process the IGMP report record for version 3 
 *
 * @param *ifp            (IN) interface object
 *
 * @param src             (IN) IP address
 *
 * @param *report         (IN) IGMPv3 report message
 *
 * @param *pkt_info       (IN) packet info
 *
 * @param *grp_recf       (IN) group record
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_igmp_process_group_record(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in_addr *src,
                                              t_IGMPv3_REPORT *report,
                                              t_MCPD_PKT_INFO *pkt_info,
                                              t_IGMP_GRP_RECORD *grp_rec);

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
t_MCPD_RET_CODE mcpd_igmp_admission_control(int msg_type,
                                            int rxdev_ifi,
                                            UINT8 *gp,
                                            UINT8 *src,
                                            UINT8 *rep,
                                            unsigned short tci,
                                            UINT8 rep_proto_ver);

#endif /* __IGMP_MAIN_H__ */
