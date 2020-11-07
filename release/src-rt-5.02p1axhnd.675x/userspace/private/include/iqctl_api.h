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

#ifndef _IQCTL_API_H_
#define _IQCTL_API_H_

int bcm_iqctl_set_status( iqctl_status_t status );
int bcm_iqctl_get_status( iqctl_status_t *status_p );
int bcm_iqctl_dump_status_kernel_mode( void );

int bcm_iqctl_set_hw_accel_cong_ctrl( iqctl_status_t status );
int bcm_iqctl_get_hw_accel_cong_ctrl( iqctl_status_t *status_p );

int bcm_iqctl_add_keymask( iqctl_data_t *iq );
int bcm_iqctl_rem_keymask( iqctl_data_t *iq );
int bcm_iqctl_dump_keymasktbl( void );
int bcm_iqctl_add_key( iqctl_data_t *iq );
int bcm_iqctl_rem_key( iqctl_data_t *iq );
int bcm_iqctl_get_key( iqctl_data_t *iq );
int bcm_iqctl_dump_keytbl( void );
int bcm_iqctl_dump_all( void );
void bcm_iqctl_print_key( iqctl_data_t *iq );

int bcm_iqctl_dump_porttbl_kernel_mode( void );
int bcm_iqctl_flush( void );

/* the old APIs. remain here for backward compatibility */
int bcm_iqctl_add_port( iqctl_proto_t proto, int dport, iqctl_ent_t ent, 
        iqctl_prio_t prio );
int bcm_iqctl_rem_port( iqctl_proto_t proto, int dport, iqctl_ent_t ent );
int bcm_iqctl_get_port( iqctl_proto_t proto, int dport, iqctl_ent_t *ent_p,
        iqctl_prio_t *prio_p );

int bcm_iqctl_dump_porttbl( iqctl_proto_t proto );
int bcm_iqctl_flush_porttbl( iqctl_proto_t proto, iqctl_ent_t ent );

int bcm_iqctl_set_defaultprio( iqctl_prototype_t prototype, int protoval, iqctl_prio_t prio );
int bcm_iqctl_rem_defaultprio( iqctl_prototype_t prototype, int protoval );

#endif /* _IQCTL_API_H_ */
