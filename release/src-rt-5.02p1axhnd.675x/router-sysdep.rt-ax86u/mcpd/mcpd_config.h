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
 * File Name  : mcpd_config.h
 *
 * Description: API for managing mcpd config
 *              
 ***************************************************************************/
#ifndef __MCPD_CONFIG_H__
#define __MCPD_CONFIG_H__

#define MCPD_DFLT_MAX_GROUPS      25
#define MCPD_DFLT_MAX_SOURCES     10
#define MCPD_DFLT_MAX_REPORTERS   25

/** write mcpd configurations
 *
 * @param *igmp_config (IN) pointer to igmp config
 *
 * @param *mld_config  (IN) pointer to mld config
 *
 * @return 0 on success
 *
 */
int mcpd_config_write(t_MCPD_IGMP_CONFIG *igmp_config,
                      t_MCPD_MLD_CONFIG  *mld_config);

/** read mcpd configurations
 *
 * @param *igmp_config (IN) pointer to igmp config
 *
 * @param *mld_config  (IN) pointer to mld config
 *
 * @return 0 on success
 *
 */
int mcpd_config_read(t_MCPD_IGMP_CONFIG *igmp_config,
                     t_MCPD_MLD_CONFIG  *mld_config);

/** display the configuration
 *
 * @param *igmp_config (IN) pointer to igmp config
 *
 * @param *mld_config  (IN) pointer to mld config
 *
 * @return None
 *
 */
void mcpd_config_display(t_MCPD_IGMP_CONFIG *igmp_config,
                         t_MCPD_MLD_CONFIG *mld_config);



#endif /* __MCPD_CONFIG_H__ */
