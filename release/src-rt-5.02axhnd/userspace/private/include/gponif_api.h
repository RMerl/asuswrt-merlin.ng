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

#ifndef _GPONIF_API_H_
#define _GPONIF_API_H_

/* TBD: Use commom defines */
#define MAX_GEM_VALUE  (CONFIG_BCM_MAX_GEM_PORTS - 1)
#define MAX_GPON_IFS   40

/****************************************************************************/
/* open the socket to enet driver                                           */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_init(void);

/****************************************************************************/
/* close the socket                                                         */
/****************************************************************************/
void gponif_close(void);

/****************************************************************************/
/* create a gpon virtual interface                                          */
/* Inputs: ifname = the name for the gpon i/f. If not specified, a name of  */
/*          gponXX will be assigned where XX is the next available number   */ 
/* Returns: 0 on success; a negative value on failure                       */ 
/* Notes: 1. The max number of gpon virtual interfaces is limited to 32.    */ 
/****************************************************************************/
int gponif_create(char *ifname);

#if MAX_GEM_VALUE < 32
/****************************************************************************/
/* remove one or more gem ids to the interface                              */
/* Returns: 0 on success; a negative value on failure                       */ 
/* This API is retained only to provide backward compatibility with 6816.   */
/* Customers who are integrating 6818 are recommended to use                */
/* gponif_remgemidx() function                                              */
/****************************************************************************/
int gponif_remgem(char *ifname, unsigned int gem_map);

/****************************************************************************/
/* add one or more gem ids to the interface                                 */
/* Returns: 0 on success; a negative value on failure                       */ 
/* This API is retained only to provide backward compatibility with 6816.   */
/* Customers who are integrating 6818 are recommended to use                */
/* gponif_addgemidx() function                                              */
/****************************************************************************/
int gponif_addgem(char *ifname, unsigned int gem_map);
#endif

/****************************************************************************/
/* remove gem idx from the interface                                        */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_remgemidx(char *ifname, unsigned int gemIdx);

/****************************************************************************/
/* add gem idx to the interface                                 */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_addgemidx(char *ifname, unsigned int gemIdx);

/****************************************************************************/
/* set multicast gem id to the interface                                 */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_setmcastgem(unsigned int gem_id);

/****************************************************************************/
/* get name of interface that contains the gem id                             */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_get_ifname(const unsigned int gem, char *ifname);

/****************************************************************************/
/* get number of gems that are belonged to the given interface        */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_get_number_of_gems(const char *ifname, unsigned int *gem_num);

/****************************************************************************/
/* show the gpon interface details                                          */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_show(char *ifname);

/****************************************************************************/
/* show the details of all gpon interfaces                                  */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_show_all(void);

/****************************************************************************/
/* delete the given gpon interface                                          */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_delete(char *ifname);

/****************************************************************************/
/* delete all gpon interfaces                                               */
/* Returns: 0 on success; a negative value on failure                       */ 
/****************************************************************************/
int gponif_delete_all(void);

#endif /* _GPONIF_API_H_ */

