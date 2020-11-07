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
 * File Name  : mcpd_main.h
 *
 * Description: API for MCPD main process init and run
 *              
 ***************************************************************************/
#ifndef __MCPD_MAIN_H__
#define __MCPD_MAIN_H__

/** get the current setting for the Multicast Group Mode (ASM vs SSM)
 *
 * @return  current setting for the Group Mode(see mcpd.h)
 */
enum e_multicast_group_mode mcpd_get_mcast_group_mode(void);

/** start the input packet processing
 *
 *
 * @return Nothing
 *
 */
void mcpd_input_run(void);

/** start the mcpd process run
 *
 *
 * @return Nothing
 *
 */
void mcpd_run(void);

/** clean up all the mcpd objects
 *
 *
 * @return Nothing
 *
 */
void mcpd_cleanup(void);

/** initiate the mcpd multicast routing
 *
 * @param argc   (IN) number of arguments to mcpd process
 *
 * @param **argv (IN) argument array
 *
 * @return MCPD_RET_OK on success or corresponding error on failure
 *
 */
int mcpd_router_init(int argc, char **argv);

/** shutdown the mcpd process
 *
 *
 * @return Nothing
 *
 */
void mcpd_shutdown(void);

/** handle signals for mcpd process
 *
 *
 * @return Nothing
 *
 */
void mcpd_signal(int sig);

/** handle messages from sockets mroute/raw
 *
 * @param sock    (IN) socket descriptor
 *
 * @return Nothing
 *
 */
void mcpd_drain_message(int sock);

/** update interface inforamtion
 *
 * @param protoType    (IN) interface protocol type ipv4/ipv6/all
 *
 * @return Nothing
 *
 */
void mcpd_update_interface_info(int protoType);

/** process query timer
 *
 * @return Nothing
 *
 */
void mcpd_process_query_timer(void *handle);

/** process port down notification
 *
 * @return Nothing
 *
 */
void mcpd_process_port_down(int ifindex);

/** update multicast information
 *
 * @return Nothing
 *
 */
void mcpd_update_multicast_info(void);

/** clean up all objects and flush snooping entries
 *
 * @return t_MCPD_RET_CODE enum
 *
 */
t_MCPD_RET_CODE mcpd_reset_handler(void);

/** update multicast flooding status
 *
 * @param  *pre_igmp_flood (IN) pointer to previous igmp flood config
 *
 * @param  *pre_mld_flood  (IN) pointer to previous mld flood config
 *
 * @return Nothing
 *
 */
void mcpd_update_flooding_status(int *pre_igmp_flood, int *pre_mld_flood);

/** set mode for group (FIRSTIN or IANA)
 *
 * @param newMode (IN) group mode
 *
 * @return Nothing
 *
 */
void mcpd_setMcastGroupMode(enum e_multicast_group_mode newMode);

/** display mode for group
 *
 * @return Nothing
 *
 */
void mcpd_display_group_mode(void);

#endif /* __MCPD_MAIN_H__*/
