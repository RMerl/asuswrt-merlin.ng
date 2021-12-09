/*
    Copyright 2000-2019 Broadcom Corporation

    <:label-BRCM:2019:DUAL/GPL:standard
    
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
 * File Name  : bcm_mbox_map.h
 *
 * Description: rudimentary platform specific definitions for message passing
 * between across the software stack; 
 *        if SOFT RESET WD SAFE is selected - accross SW RESET 
              
 *
 * Updates    : 04/15/2019  Created.
 ***************************************************************************/

#ifndef _BCM_MBOX_MAP_H
#define _BCM_MBOX_MAP_H

#include "bcm_map_part.h"

typedef enum cfe_boot_st_e {
     CFE_BOOT_NONE = 0,
     CFE_BOOT_UNSECURE = 1,
     CFE_BOOT_SECURE = 2,
     CFE_BOOT_SAFEMODE = 3,
     CFE_BOOT_HALT = 4,
} cfe_boot_st_t;

typedef enum cfe_boot_st_err_inf {
     CFE_BOOT_ERR_OK = 0,
     CFE_BOOT_ERR_ABORTED = 1,
     CFE_BOOT_ERR_CRIT = 2,
} cfe_boot_st_err_info_t;

typedef enum _cfe_boot_st_inf {
     CFE_BOOT_INFO_NONE = 0,
     CFE_BOOT_INFO_ROM = 1,
     CFE_BOOT_INFO_PRIMARY = 2,
     CFE_BOOT_INFO_SECONDARY = 4,
     CFE_BOOT_INFO_LINUX = 8,
} cfe_boot_st_info_t;

/* mailbox  map */


/*
MBOX message
[7-0]   - VERSION
[8]     - SAFE MODE
[30-9]  - MBOX_MSG1
[31]    - BOOT_INACTIVE_IMAGE_ONCE

MBOX1 aka status and message
[0]     - mailbox status
[15-1]  - message MBOX1_MSG
[31-16] - reserved

--------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

#define _BITS_GET(_RVAL, _MSK, _SH) ((_RVAL >> _SH) &  _MSK)
#define _BITS_CLR( _LVAL, _MSK, _SH) do { _LVAL &= ~(_MSK << _SH); } while(0)
#define _BITS_OR(_LVAL, _MSK, _SH, _RVAL) do { _LVAL |= ((_RVAL & _MSK) << _SH); } while(0)
/* copies values TMP_RVAL and back to _LVAL on purpose to avoid potention side effect
if used for io regiser */
#define _BITS_CLR_SET(_LVAL, _MSK, _SH, _RVAL) do {                                   \
                               unsigned int ___rval = _LVAL;                      \
                              _BITS_CLR(___rval, _MSK, _SH);                      \
                              _BITS_OR(___rval, _MSK, _SH, _RVAL);                \
                              _LVAL = ___rval; } while(0);

/* Adding mailbox mappings per platform */ 

#if defined(_BCM96858_) || defined(CONFIG_BCM96858) \
   || defined(_BCM96846_) || defined(CONFIG_BCM96846) \
   || defined(_BCM96878_) || defined(CONFIG_BCM96878) \
   || defined(_BCM96856_) || defined(CONFIG_BCM96856) \
   || defined(_BCM963158_) || defined(CONFIG_BCM963158) \
   || defined(_BCM963178_) || defined(CONFIG_BCM963178) \
   || defined(_BCM947622_) || defined(CONFIG_BCM947622)

#define MISC_SW_DEBUG MISC->miscSWdebugNW
/* bit 31 of this register is used for BOOT_INACTIVE_IMAGE_ONCE_REG */
#define BCM_MBOX       MISC_SW_DEBUG[1]
#define BCM_MBOX1      MISC_SW_DEBUG[0]
#define BCM_MBOX_STATUS    BCM_MBOX1
#define BCM_MBOX_SOFT_RESET_SAFE_EN

#define BCM_MBOX1_RESET_VAL                      0x0
#define BCM_MBOX1_RESET_MASK                     0xffffffff
#define BCM_MBOX1_RESET_SHIFT                    0x0

#define BCM_MBOX1_STATUS_VAL                     0x1
#define BCM_MBOX1_STATUS_MASK                    0x1
#define BCM_MBOX1_STATUS_SHIFT                   0x0

#define BCM_MBOX1_MSG_SHIFT                      0x1 
#define BCM_MBOX1_MSG_MASK                       0x7fff 
#define BCM_MBOX_MSG1_MASK                       0x3fffff  /* 22 bits reserved for boot status*/
#define BCM_MBOX_MSG1_SHIFT                      9


#define BCM_MBOX_MSG_MASK                        0xffffffff
#define BCM_MBOX_MSG_SHIFT                       0x0

#define BCM_MBOX_INIT                                                                                                         \
                         do {                                                                                                 \
                             if (BCM_MBOX1_RESET()) {                                                                         \
                                 BCM_MBOX = 0;                                                                                \
                             }                                                                                                \
                         } while(0);
#else

#define RDB_PROFILE_7_POLLING_COMPARE_RESET_VAL  0x1
#define RDB_PROFILE_7_POLLING_COMPARE_MASK       0xffffffff            
#define RDB_PROFILE_7_POLLING_TIMEOUT_RESET_VAL  0xffff
#define RDB_PROFILE_7_POLLING_TIMEOUT_MASK       0xffff
/*
Ensure that bit capacity of COMPARE and TIMEOUT registers from SPI Profile are
enough to hold status and message for MBOX1 not exceeding  16 bits; 
MBOX not exceeding 32 bits
*/

#define BCM_MBOX1_RESET_VAL                      RDB_PROFILE_7_POLLING_TIMEOUT_RESET_VAL
#define BCM_MBOX1_RESET_MASK                     RDB_PROFILE_7_POLLING_TIMEOUT_MASK
#define BCM_MBOX1_RESET_SHIFT                    0x0

#define BCM_MBOX1_STATUS_VAL                     0x0
#define BCM_MBOX1_STATUS_MASK                    0x1
#define BCM_MBOX1_STATUS_SHIFT                   0x0

#define BCM_MBOX_MSG_MASK                        0xffffffff
#define BCM_MBOX_MSG_SHIFT                       0x0

#define BCM_MBOX1_MSG_SHIFT                      0x1
#define BCM_MBOX1_MSG_MASK                       0x7fff
#define BCM_MBOX_MSG1_MASK                       0x3fffff  /* 22 bits reserved for boot status*/
#define BCM_MBOX_MSG1_SHIFT                      9

#define BCM_MBOX1                                HS_SPI_PROFILES[7].polling_timeout
#define BCM_MBOX                                 HS_SPI_PROFILES[7].polling_compare

#define HS_SPI_GLB_CTRL                          HS_SPI->hs_spiGlobalCtrl
#define HS_SPI_GLB_CTRL_WD_RESET_SAFE_SHIFT      22
#define BCM_MBOX_SOFT_RES_VOL_CTRL_MASK          0x1
#define BCM_MBOX_SOFT_RESET_SAFE_EN            do { HS_SPI_GLB_CTRL |= (1 << HS_SPI_GLB_CTRL_WD_RESET_SAFE_SHIFT); }while(0);

#define BCM_MBOX_INIT                                                                                                         \
                         do {                                                                                                 \
                             if (BCM_MBOX1_RESET()) {                                                                         \
                                 BCM_MBOX = 0; BCM_MBOX1 &= ~(BCM_MBOX1_STATUS_MASK);                                            \
                             }                                                                                                \
                             BCM_MBOX_SOFT_RESET_SAFE_EN;                                                                     \
                         } while(0);


#endif



/* Define bits for various mailbox messages */
#define BCM_MBOX_VER_SHIFT                       0
#define BCM_MBOX_VER_MASK                        0xff 

#define BCM_MBOX_SAFEMODE_SHIFT                  8
#define BCM_MBOX_SAFEMODE_MASK                   0x1

#define BCM_MBOX_INACTIVE_IMAGE_ONCE_SHIFT       30
#define BCM_MBOX_INACTIVE_IMAGE_ONCE_MASK        0x1


/* BASIC manipulation APIs */

#define BCM_MBOX1_RESET()   (_BITS_GET(BCM_MBOX1, BCM_MBOX1_RESET_MASK, BCM_MBOX1_RESET_SHIFT) == BCM_MBOX1_RESET_VAL)
#define BCM_MBOX1_STATUS()   (BCM_MBOX1_STATUS_GET() == BCM_MBOX1_STATUS_VAL)
#define BCM_MBOX_MSG_SET(_VAL) _BITS_CLR_SET(BCM_MBOX, BCM_MBOX_MSG_MASK, BCM_MBOX_MSG_SHIFT, _VAL); BCM_MBOX1_STATUS_SET()
#define BCM_MBOX_MSG_GET() _BITS_GET(BCM_MBOX, BCM_MBOX_MSG_MASK, BCM_MBOX_MSG_SHIFT)

#define BCM_MBOX_MSG1_SET(_VAL) _BITS_CLR_SET(BCM_MBOX, BCM_MBOX_MSG1_MASK, BCM_MBOX_MSG1_SHIFT, _VAL); BCM_MBOX1_STATUS_SET()
#define BCM_MBOX_MSG1_GET() _BITS_GET(BCM_MBOX, BCM_MBOX_MSG1_MASK, BCM_MBOX_MSG1_SHIFT)

#define BCM_MBOX1_MSG_GET() _BITS_GET(BCM_MBOX1, BCM_MBOX1_MSG_MASK, BCM_MBOX1_MSG_SHIFT)
#define BCM_MBOX1_MSG_SET(_VAL) _BITS_CLR_SET(BCM_MBOX1, BCM_MBOX1_MSG_MASK, BCM_MBOX1_MSG_SHIFT, _VAL); BCM_MBOX1_STATUS_SET()

#define BCM_MBOX1_STATUS_SET() _BITS_CLR_SET(BCM_MBOX1, BCM_MBOX1_STATUS_MASK, BCM_MBOX1_STATUS_SHIFT, BCM_MBOX1_STATUS_VAL)
#define BCM_MBOX1_STATUS_GET() _BITS_GET(BCM_MBOX1, BCM_MBOX1_STATUS_MASK, BCM_MBOX1_STATUS_SHIFT)

#define BCM_MBOX_INACTIVE_IMAGE_SET(_VAL) _BITS_CLR_SET(BCM_MBOX, BCM_MBOX_INACTIVE_IMAGE_ONCE_MASK, BCM_MBOX_INACTIVE_IMAGE_ONCE_SHIFT, _VAL); BCM_MBOX1_STATUS_SET()
#define BCM_MBOX_INACTIVE_IMAGE_CLR() _BITS_CLR(BCM_MBOX, BCM_MBOX_INACTIVE_IMAGE_ONCE_MASK, BCM_MBOX_INACTIVE_IMAGE_ONCE_SHIFT)
#define BCM_MBOX_INACTIVE_IMAGE_GET() _BITS_GET(BCM_MBOX, BCM_MBOX_INACTIVE_IMAGE_ONCE_MASK, BCM_MBOX_INACTIVE_IMAGE_ONCE_SHIFT)

#define BCM_MBOX_VER_SET(_VAL) _BITS_CLR_SET(BCM_MBOX, BCM_MBOX_VER_MASK,BCM_MBOX_VER_SHIFT, _VAL); BCM_MBOX1_STATUS_SET()
#define BCM_MBOX_VER_GET() _BITS_GET(BCM_MBOX,BCM_MBOX_VER_MASK, BCM_MBOX_VER_SHIFT)

#define BCM_MBOX_SAFEMODE_SET(_RVAL) _BITS_CLR_SET(BCM_MBOX, BCM_MBOX_SAFEMODE_MASK, BCM_MBOX_SAFEMODE_SHIFT, _RVAL); BCM_MBOX1_STATUS_SET()
#define BCM_MBOX_SAFEMODE_GET() _BITS_GET(BCM_MBOX, BCM_MBOX_SAFEMODE_MASK, BCM_MBOX_SAFEMODE_SHIFT)

/* Various boot states used by bootloader */ 

#define BCM_BLR_BOOT_ST_MASK       0x7
#define BCM_BLR_BOOT_ST_SHIFT      0x0

#define BCM_BLR_BOOT_ERR_MASK      0x3
#define BCM_BLR_BOOT_ERR_SHIFT     0x3

#define BCM_BLR_BOOT_ST_INFO_MASK  0xf
#define BCM_BLR_BOOT_ST_INFO_SHIFT 0x6

#define BCM_BLR_BOOT_STATE_SET_ERR(_LVAL, _RVAL) do{\
                                        _BITS_CLR_SET(_LVAL, BCM_BLR_BOOT_ERR_MASK, BCM_BLR_BOOT_ERR_SHIFT, _RVAL); \
                                      }while(0);

#define BCM_BLR_BOOT_STATE_SET_ST(_LVAL,_RVAL) do{\
                                        _BITS_CLR_SET(_LVAL, BCM_BLR_BOOT_ST_MASK, BCM_BLR_BOOT_ST_SHIFT,_RVAL); \
                                      }while(0);

#define BCM_BLR_BOOT_STATE_SET_INFO(_LVAL,_RVAL) do{\
                                        _BITS_CLR_SET(_LVAL, BCM_BLR_BOOT_ST_INFO_MASK, BCM_BLR_BOOT_ST_INFO_SHIFT, _RVAL); \
                                      }while(0);

#define BCM_BLR_BOOT_STATE_GET_ERR(_RVAL)  _BITS_GET(_RVAL, BCM_BLR_BOOT_ERR_MASK, BCM_BLR_BOOT_ERR_SHIFT)
#define BCM_BLR_BOOT_STATE_GET_ST(_RVAL)   _BITS_GET(_RVAL, BCM_BLR_BOOT_ST_MASK, BCM_BLR_BOOT_ST_SHIFT)
#define BCM_BLR_BOOT_STATE_GET_INFO(_RVAL) _BITS_GET(_RVAL, BCM_BLR_BOOT_ST_INFO_MASK, BCM_BLR_BOOT_ST_INFO_SHIFT)

#ifdef __cplusplus
}
#endif

#endif /* _BCM_MBOX_MAP_H */

