/*
* <:copyright-BRCM:2006:proprietary:standard
* 
*    Copyright (c) 2006 Broadcom 
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
:>
*/
/***************************************************************************
 * File Name  : obj_hndlr.h
 *
 * Description: common proto types for object handlers
 *              
 ***************************************************************************/
#ifndef __OBJ_HNDLR_H__
#define __OBJ_HNDLR_H__

typedef struct mcpd_oldReporterToDelete {
   char                      valid;
   union {
       struct in_addr        reporter_v4;
       struct in6_addr       reporter_v6;
   };
   char                      rep_ifi;
   struct mcpd_interface_obj *ifp;
}t_MCPD_OLD_REPORTER_TO_DELETE;

/** add new interface
 *
 * @param addr       (IN) IPv4 address
 *
 * @param addr6      (IN) IPv6 Address
 *
 * @param *name      (IN) pointer to interface name
 *
 * @param index      (IN) interface index
 *
 * @param if_type    (IN) interface type
 *
 * @return pointer to t_MCPD_INTERFACE_OBJ
 *
 */
t_MCPD_INTERFACE_OBJ *mcpd_interface_add(struct in_addr  *addr, 
                                         struct in6_addr *addr6,
                                         char            *name, 
                                         int              index,
                                         int              if_type);
/** add new interface
 *
 * @param addr       (IN) IPv4 address
 *
 * @param addr6      (IN) IPv6 Address
 *
 * @param *name      (IN) pointer to interface name
 *
 * @param index      (IN) interface index
 *
 * @param if_type    (IN) interface type
 *
 * @return int
 *
 */
int mcpd_interface_update(t_MCPD_INTERFACE_OBJ  *ifp,
                                struct in_addr  *ifaddr,
                                struct in6_addr *ifaddr6,
                                char            *ifname,
                                UINT16           index,
                                int              if_type);

/** create a reporter object
 *
 * @param proto      (IN) protocol type
 *
 * @param *src_addr  (IN) pointer to IP address
 *
 * @return pointer to t_MCPD_REP_OBJ
 *
 */
t_MCPD_REP_OBJ* mcpd_rep_create(t_MCPD_PROTO_TYPE proto, UINT8 *src_addr);

/** count the number of reporter object
 *
 * @param *gp        (IN) group object
 *
 * @return integer
 *
 */
int mcpd_rep_count(t_MCPD_GROUP_OBJ *gp);

/** compare IPv4 address
 *
 * @param *ip_addr1 (IN) IPv4 address
 *
 * @param *ip_addr2 (IN) IPv4 address
 *
 * @return MCPD_RET_OK if they are same
 *
 */
t_MCPD_RET_CODE mcpd_compare_ipv4_addr(UINT8 const *ip_addr1, UINT8 const *ip_addr2);

/** compare IPv6 address
 *
 * @param *ip_addr1 (IN) IPv6 address
 *
 * @param *ip_addr2 (IN) IPv6 address
 *
 * @return MCPD_RET_OK if they are same
 *
 */
t_MCPD_RET_CODE mcpd_compare_ipv6_addr(UINT8 const *ip_addr1, UINT8 const *ip_addr2);

/** cleanup reporter object
 *
 * @param proto     (IN) protocol type
 *
 * @param *gp       (IN) pointer to group
 *
 * @param *rep      (IN) pointer to reporter
 *
 * @return None
 *
 */
void mcpd_rep_cleanup(t_MCPD_PROTO_TYPE proto, 
                      t_MCPD_GROUP_OBJ *gp, 
                      t_MCPD_REP_OBJ *rep);

/** Look for reporter object in a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *gp       (IN) pointer to group
 *
 * @param *src_addr (IN) pointer to reporter
 *
 * @return pointer to t_MCPD_REP_OBJ
 *
 */
t_MCPD_REP_OBJ* mcpd_group_rep_lookup(t_MCPD_PROTO_TYPE proto, 
                                      t_MCPD_GROUP_OBJ *gp, 
                                      UINT8 *src_addr);


/** Look for reporter object in a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *gp       (IN) pointer to group
 *
 * @param *src_addr (IN) pointer to reporter
 *
 * @param rep_ifi   (IN) ifindex of reporter
 *
 * @return pointer to t_MCPD_REP_OBJ
 *
 */
t_MCPD_REP_OBJ* mcpd_group_rep_port_lookup(t_MCPD_PROTO_TYPE proto,
                                           t_MCPD_GROUP_OBJ *gp,
                                           UINT8 *src_addr,
                                           int rep_ifi);

/** get the num of group_members on the rep port
 *
 * @param *gp       (IN) pointer to group
 *
 * @param rep_ifi   (IN) ifindex of reporter
 *
 * @return the num
 *
 */

int mcpd_group_members_num_on_rep_port(t_MCPD_GROUP_OBJ *gp,UINT16 rep_ifi);


/** Wipe  reporter object in a group
 *
 * @param proto               (IN) protocol type
 *
 * @param *reporterOnOldPort  (IN) reporter info for deletion (includes port number)
 *
 * @return None
 *
 */
void mcpd_wipe_reporter_for_old_port (t_MCPD_PROTO_TYPE proto,
                                      t_MCPD_OLD_REPORTER_TO_DELETE *reporterOnOldPort);

/** get action type by group object
 *
 * @param *group     (IN) pointer to group object
 *
 * @param proto      (IN) the protocol type
 *
 * @param *action    (OUT) pointer to action
 *
 * @return None
 *
 */
void mcpd_group_flood_action_get(t_MCPD_GROUP_OBJ *group,
                                 t_MCPD_PROTO_TYPE proto,
                                 int  *action);

/** add reporter object to group object
 *
 * @param proto     (IN) protocol type
 *
 * @param *gp       (IN) pointer to group
 *
 * @param *src_addr (IN) pointer to reporter
 *
 * @param rep_ifi   (IN) int 
 *
 * @return pointer to t_MCPD_REP_OBJ
 *
 */
t_MCPD_REP_OBJ * mcpd_group_rep_add(t_MCPD_PROTO_TYPE proto,
                                    t_MCPD_GROUP_OBJ *gp, 
                                    UINT8 *srcaddr,
                                    int rep_ifi,
                                    t_MCPD_OLD_REPORTER_TO_DELETE *reporterOnOldPort);

/** look for source in a group-repotor database
 *
 * @param proto     (IN) protocol type
 *
 * @param *rep      (IN) pointer to reporter
 *
 * @param filter    (IN) filter type
 *
 * @return pointer to t_MCPD_SRC_OBJ
 *
 */
t_MCPD_SRC_OBJ * mcpd_group_rep_lookup_src(t_MCPD_PROTO_TYPE proto,
                                           t_MCPD_REP_OBJ *rep, 
                                           int filter);

/** create a source object for a reporter
 *
 * @param proto     (IN) protocol type
 *
 * @param *rep      (IN) pointer to reporter
 *
 * @return pointer to t_MCPD_SRC_OBJ
 *
 */
t_MCPD_SRC_REP_OBJ* mcpd_src_rep_create(t_MCPD_PROTO_TYPE proto, 
                                        t_MCPD_REP_OBJ *rep);

/** count the number of source reporters
 *
 * @param proto     (IN) protocol type
 *
 * @param *src      (IN) source object 
 *
 * @return integer
 *
 */
int mcpd_src_rep_count(t_MCPD_PROTO_TYPE proto, t_MCPD_SRC_OBJ *src);

/** look for srouce reporters in a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *src      (IN) source object 
 *
 * @param *rep      (IN) reporter object 
 *
 * @return pointer to t_MCPD_SRC_REP_OBJ
 *
 */
t_MCPD_SRC_REP_OBJ* mcpd_group_src_rep_lookup(t_MCPD_PROTO_TYPE proto,
                                              t_MCPD_SRC_OBJ* src,
                                              t_MCPD_REP_OBJ* rep);

/** source reporter object cleanup
 *
 * @param proto     (IN) protocol type
 *
 * @param *src      (IN) source object 
 *
 * @param *rep      (IN) reporter object 
 *
 * @return None
 *
 */
void mcpd_src_rep_cleanup(t_MCPD_PROTO_TYPE proto, 
                          t_MCPD_SRC_OBJ* src, 
                          t_MCPD_REP_OBJ* rep);

/** add source reporter object to a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *src      (IN) source object 
 *
 * @param *rep      (IN) reporter object 
 *
 * @return pointer to t_MCPD_SRC_REP_OBJ
 *
 */
t_MCPD_SRC_REP_OBJ* mcpd_group_src_rep_add(t_MCPD_PROTO_TYPE proto, 
                                           t_MCPD_SRC_OBJ *src,
                                           t_MCPD_REP_OBJ *rep);

/** create reporter source object
 *
 * @param proto     (IN) protocol type
 *
 * @param *src      (IN) source object 
 *
 * @return pointer to t_MCPD_REP_SRC_OBJ
 *
 */
t_MCPD_REP_SRC_OBJ* mcpd_rep_src_create(t_MCPD_PROTO_TYPE proto,
                                        t_MCPD_SRC_OBJ *src);

/** count the number of reporter source objects
 *
 * @param proto     (IN) protocol type
 *
 * @param *rep      (IN) reporter object 
 *
 * @return pointer to t_MCPD_REP_SRC_OBJ
 *
 */
int mcpd_rep_src_count(t_MCPD_PROTO_TYPE proto, t_MCPD_REP_OBJ *rep);

/** look for reporter source object in a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *rep      (IN) reporter object 
 *
 * @param *src      (IN) source object 
 *
 * @return pointer to t_MCPD_REP_SRC_OBJ
 *
 */
t_MCPD_REP_SRC_OBJ* mcpd_group_rep_src_lookup(t_MCPD_PROTO_TYPE proto,
                                              t_MCPD_REP_OBJ* rep, 
                                              t_MCPD_SRC_OBJ* src);

/** cleanup the reporter source object
 *
 * @param proto     (IN) protocol type
 *
 * @param *rep      (IN) reporter object 
 *
 * @param *src      (IN) source object 
 *
 * @return None
 *
 */
void mcpd_rep_src_cleanup(t_MCPD_PROTO_TYPE proto,
                          t_MCPD_REP_OBJ* rep,
                          t_MCPD_SRC_OBJ* src);

/** count number of sources in a group
 *
 * @param *gp       (IN) pointer to a group object
 *
 * @return num. sources
 */
int mcpd_grp_src_count(t_MCPD_GROUP_OBJ *gp);

/** add reporter source object to a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *rep      (IN) reporter object 
 *
 * @param *src      (IN) source object 
 *
 * @return pointer to t_MCPD_REP_SRC_OBJ
 *
 */
t_MCPD_REP_SRC_OBJ* mcpd_group_rep_src_add(t_MCPD_PROTO_TYPE proto,
                                           t_MCPD_REP_OBJ *rep, 
                                           t_MCPD_SRC_OBJ *src);

/** create source object
 *
 * @param proto     (IN) protocol type
 *
 * @param *src_addr (IN) source IP address
 *
 * @return pointer to t_MCPD_SRC_OBJ
 *
 */
t_MCPD_SRC_OBJ * mcpd_src_create(t_MCPD_PROTO_TYPE proto, UINT8 *src_addr);

/** cleanup source object
 *
 * @param proto     (IN) protocol type
 *
 * @param *gp       (IN) group
 *
 * @param *src      (IN) source
 *
 * @param filter    (IN) filter type
 *
 * @return None
 *
 */
void mcpd_src_cleanup(t_MCPD_PROTO_TYPE proto, 
                      t_MCPD_GROUP_OBJ *gp,
                      t_MCPD_SRC_OBJ *src, 
                      int filter);

/** lookup source object in a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *gp       (IN) group
 *
 * @param *srcaddr  (IN) source
 *
 * @param filter    (IN) filter type
 *
 * @return t_MCPD_SRC_OBJ
 *
 */
t_MCPD_SRC_OBJ * mcpd_group_src_lookup(t_MCPD_PROTO_TYPE proto,
                                       t_MCPD_GROUP_OBJ *gp, 
                                       UINT8 *srcaddr, 
                                       int filter);

/** add source to a group
 *
 * @param proto     (IN) protocol type
 *
 * @param *gp       (IN) group
 *
 * @param *srcaddr  (IN) source
 *
 * @param filter    (IN) filter type
 *
 * @return t_MCPD_SRC_OBJ
 *
 */
t_MCPD_SRC_OBJ * mcpd_group_src_add(t_MCPD_PROTO_TYPE proto,
                                    t_MCPD_GROUP_OBJ *gp, 
                                    UINT8 *srcaddr, 
                                    int filter);

/** create a group 
 *
 * @param proto     (IN) protocol type
 *
 * @param *groupaddr(IN) group address
 *
 *
 * @return t_MCPD_GROUP_OBJ
 *
 */
t_MCPD_GROUP_OBJ * mcpd_group_create(t_MCPD_PROTO_TYPE proto, UINT8 *groupaddr);

/** cleanup a group 
 *
 * @param proto     (IN) protocol type
 *
 * @param *ifp      (IN) interface object
 *
 * @param *gp       (IN) group object
 *
 *
 * @return None
 *
 */
void mcpd_group_cleanup(t_MCPD_PROTO_TYPE proto,
                        t_MCPD_INTERFACE_OBJ *ifp,
                        t_MCPD_GROUP_OBJ* gp);

/** create a interface object
 *
 * @param proto     (IN) protocol type
 *
 * @param ifaddr    (IN) IPv4 interface address
 *
 * @param *ifaddr6  (IN) IPv6 interface address
 *
 * @param *ifname   (IN) interface name
 *
 * @param index     (IN) interface index
 *
 *
 * @return pointer to t_MCPD_INTERFACE_OBJ
 *
 */
t_MCPD_INTERFACE_OBJ* mcpd_interface_create(struct in_addr   *ifaddr, 
                                            struct in6_addr  *ifaddr6,
                                            char             *ifname, 
                                            UINT16            index);
/** destroy group
 *
 * @param *gp       (IN) group
 *
 *
 * @return None
 *
 */
void mcpd_group_destroy(t_MCPD_PROTO_TYPE proto, t_MCPD_GROUP_OBJ *gp);

/** group count 
 *
 * @param proto      (IN) protocol type igmp/mld
 *
 * @param ifp        (IN) interface object
 *
 * @return count
 *
 */
int mcpd_interface_group_count(t_MCPD_PROTO_TYPE proto,
                               t_MCPD_INTERFACE_OBJ *ifp);

/** clean up the interface object
 *
 * @param *ifp       (IN) interface
 *
 * @param delete     (IN) release interface object
 *
 * @param flushSnoop (IN) flush snooping entries
 *
 * @return None
 *
 */
void mcpd_interface_cleanup(t_MCPD_INTERFACE_OBJ* ifp, int delete, int flushSnoop, t_MCPD_PROTO_TYPE proto);

/** indicate groups for interface need to be refreshed
 *
 * @param *ifp       (IN) interface
 *
 *
 * @return None
 *
 */
void mcpd_interface_refresh_groups(t_MCPD_INTERFACE_OBJ* ifp);

/** add group to interface
 *
 * @param proto       (IN) protocol type
 *
 * @param *ifp        (IN) interface object
 *
 * @param *groupaddr  (IN) group address
 *
 * @param rep_ifi     (IN) ifindex of reporter
 *
 * @return t_MCPD_GROUP_OBJ
 *
 */
t_MCPD_GROUP_OBJ* mcpd_interface_group_add(t_MCPD_PROTO_TYPE proto,
                                           t_MCPD_INTERFACE_OBJ *ifp, 
                                           UINT8 *groupaddr,
                                           int rep_ifi);

/** lookup a group in a interface
 *
 * @param proto       (IN) protocol type
 *
 * @param *ifp        (IN) interface object
 *
 * @param *groupaddr  (IN) group address
 *
 * @return t_MCPD_GROUP_OBJ
 *
 */
t_MCPD_GROUP_OBJ * mcpd_interface_group_lookup(t_MCPD_PROTO_TYPE proto,
                                               t_MCPD_INTERFACE_OBJ *ifp,
                                               UINT8 const *groupaddr);

/** lookup an interface
 *
 * @param if_index    (IN) interface index
 *
 * @return t_MCPD_INTERFACE_OBJ
 *
 */
t_MCPD_INTERFACE_OBJ *mcpd_interface_lookup(int if_index);

/** look up an group address across all interfaces
 *
 * @param protcol         (IN) interface index
 *
 * @param groupAddress    (IN) group address
 *
 * @param UINT8           (OUT) whether the group (if found) is ASM or SSM
 *
 * @return UINT8          (OUT) does the group exist on any interface?
 *
 */
UINT8 mcpd_does_group_exist_anywhere (t_MCPD_PROTO_TYPE protocol, UINT8 const * groupAddress, UINT8 * isSsm);

/** check for valid address
 *
 * @param proto       (IN) protocol type
 *
 * @param *addr       (IN) ip address
 *
 * @return MCPD_RET_OK if valid
 *
 */
t_MCPD_BOOL mcpd_is_valid_addr(t_MCPD_PROTO_TYPE proto, UINT8 *addr);

/** dump mcpd obj tree info
 *
 *
 * @return None
 *
 */
void mcpd_dump_obj_tree(void);

/** remove all group memberships
 *
 *
 * @return None
 *
 */
void mcpd_cleanup_memberships(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_PROTO_TYPE proto);

#endif /* __OBJ_HNDLR_H__ */
