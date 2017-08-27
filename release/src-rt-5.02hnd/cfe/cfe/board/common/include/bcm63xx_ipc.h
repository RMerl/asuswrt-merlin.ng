/*
    Copyright 2000-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
*/                       

/**************************************************************************
 * File Name  : bcm6xx_ipc.h
 *
 * Description: rudimentary platform specific definitions for message passing
 * between cfe rom and rom 
              
 *
 * Updates    : 10/08/2015  Created.
 ***************************************************************************/

#ifndef _BCM63XX_IPC_H

#define _BCM63XX_IPC_H

#include "bcm_map_part.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Adding mailbox mappings per platform */ 

#define RDB_PROFILE_7_POLLING_COMPARE_RESET_VAL   0x1
#define RDB_PROFILE_7_POLLING_COMPARE_MASK        0xffffffff            
#define RDB_PROFILE_7_POLLING_TIMEOUT_RESET_VAL   0xffff
#define RDB_PROFILE_7_POLLING_TIMEOUT_MASK        0xffff

#define CFE_MAILBOX_STATUS       HS_SPI_PROFILES[7].polling_timeout
#define CFE_MAILBOX_STATUS_MASK  (RDB_PROFILE_7_POLLING_TIMEOUT_MASK&0x1)
#define CFE_MAILBOX_STATUS_SHIFT 0x0
#define CFE_MAILBOX_STATUS_VAL   0x0
#define CFE_MAILBOX_MSG          HS_SPI_PROFILES[7].polling_compare          
#define CFE_MAILBOX_MSG_MASK     RDB_PROFILE_7_POLLING_COMPARE_MASK     
#define CFE_MAILBOX_MSG_SHIFT    0x0


/* shared ram between CFE RAM/ROM
*  originally referred as RFS (root fs) offset
*   
*/
#if defined(_BCM963138_) || defined(_BCM963148_)
    #define CFE_SHARED_RAM_OFFSET        16*1024
#elif defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_)
    #define CFE_SHARED_RAM_OFFSET        1024*1024
#else
    #define CFE_SHARED_RAM_OFFSET        0
#endif
 
#define  CFE_SHARED_RAM_OFFSET_SLOT1  (CFE_SHARED_RAM_OFFSET+4)  
#define  CFE_SHARED_RAM_OFFSET_SLOT2  (CFE_SHARED_RAM_OFFSET+8)
#define  CFE_SHARED_RAM_OFFSET_SLOT3  (CFE_SHARED_RAM_OFFSET+12)
#define  CFE_SHARED_RAM_OFFSET_SLOT4  (CFE_SHARED_RAM_OFFSET+16)
#define  CFE_SHARED_RAM_OFFSET_SLOT5  (CFE_SHARED_RAM_OFFSET+20)
#define  CFE_SHARED_RAM_OFFSET_SLOT6  (CFE_SHARED_RAM_OFFSET+24)
#define  CFE_SHARED_RAM_OFFSET_SLOT7  (CFE_SHARED_RAM_OFFSET+28)


extern int cfe_mailbox_status_isset(void);
extern void cfe_mailbox_status_set(void);
extern unsigned int cfe_mailbox_message_get(void);
extern void cfe_mailbox_message_set(unsigned int val);
/*
* Retreives embedded API version (not sources' major/minor)
*
*/
extern unsigned int cfe_get_api_version(void);

#ifdef __cplusplus
}
#endif

#endif /* _BCM63xx_IPC_H */

