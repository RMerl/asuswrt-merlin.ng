/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  sotp_base_defs.h                                         */
/*   PURPOSE: Base sotp related definition.                            */
/*                                                                     */
/***********************************************************************/
#ifndef _SOTP_BASE_DEFS_H
#define _SOTP_BASE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* SOTP base defines */
#define    SOTP_MAX_CNTR     		1000
#define    SOTP_NUM_ROWS		112     /* Maximum rows accessable in SOTP */
#define    SOTP_BITS_PER_ROW		41	/* Bits in each SOTP row, only some rows allow direct programming of all 41 bits */
#define    SOTP_BYTES_PER_ROW  		4       /* User Data bytes in one keyslot row */
#define    SOTP_ROWS_IN_REGION          4

/* LOCK sections */
#define    SOTP_WRAPPER_LOCK_SECTION    1       /* Section # containing write lock bits */
#define    SOTP_PROG_LOCK_SECTION       2       /* Section # containing read lock bits */
#define    SOTP_REGIONS_IN_LOCK		1       /* Number of regions in combined lock sections */
#define    SOTP_FIRST_FUSELOCK_ROW 	8	/* First row containing fuse-lock ctrl bits */	
#define    SOTP_NUM_REG_IN_FUSELOCK_ROW	21      /* Num regions represented in each fuse-lock row */

/* Device CFG section */
#define    SOTP_DEVICE_CFG_SECTION      3       /* Section # containing SOTP device cfg */
#define    SOTP_REGIONS_IN_DEVCFG	1       /* Number of regions in device cfg */

/* General CFG sections */
#define    SOTP_FIRST_CFG_SECTION       4       /* First general CFG section # */
#define    SOTP_FIRST_ECC_CONFIG_ROW    16      /* First row containing ECC protected 32-bit config data */
#define    SOTP_REGIONS_IN_CFG          1       /* Number of regions in the General CFG sections */

/* Keyslot sections */
#define    SOTP_MIN_KEYSLOT		7	/* First keyslot section # */
#define    SOTP_MAX_KEYSLOT		13	/* Last  keyslot section # */
#define    SOTP_REGIONS_IN_KEYSLOT      3       /* Number of regions in each keyslot section */
#define    SOTP_ROWS_IN_KEYSLOT 	12      /* Number of rows in each keyslot section */
#define    SOTP_FIRST_KEYSLOT_ROW 	28      /* First row containing 32-bit credential data */
#define    SOTP_FIRST_KEYSLOT_REGION    7       /* First region containing credential data */
#define    SOTP_FIRST_KEYSLOT_SECTION   7       /* First keyslot section # */
#define    SOTP_REGIONS_MASK_IN_KEYSLOT 7
#define    SOTP_MAX_KEYLEN 		8       /* Maximum key length in 32-bit words */ 

typedef enum
{
   SOTP_S_MODE_MFG,        // The SOTP block has not been personalized yet and is in manufacuring mode
   SOTP_S_MODE_FIELD,      // The SOTP block has been personalized and is ready for deployment
   SOTP_E_MODE_TIMEOUT,	   // The function experienced unexpected FSM timeout from the SOTP block 
   SOTP_E_MODE_ERROR       // The function experienced unexpected register return value from the SOTP block 
} SotpMode;

typedef enum
{
   SOTP_S_ROW_SUCCESS,	   // The function has completed successfully
   SOTP_S_ROW_ECC_COR, 	   // The function corrected 1 bad bit within the row (row is still usable)
   SOTP_E_ROW_ECC_DET,	   // The function detected 2 or more uncorrectable bits within the row (row is bad)
   SOTP_E_ROW_FAIL_SET,	   // The function detected that 1 or both of the fail bits are set (row is bad)
   SOTP_E_ROW_READ_LOCK,   // The function detected that the row is locked from reading
   SOTP_E_ROW_FUSE_LOCK,   // The function detected that the row is locked from further fusing
   SOTP_E_ROW_TIMEOUT,	   // The function experienced unexpected FSM timeout from the SOTP block 
   SOTP_E_ROW_ERROR	   // The function experienced unexpected register return value from the SOTP block 
} SotpRowStatus;


typedef enum
{
   SOTP_S_KEY_SUCCESS,	   // The function has completed successfully, and a credential exists
   SOTP_S_KEY_EMPTY,	   // The function has completed successfully, but the section is empty
   SOTP_E_KEY_BADPARAM,	   // The function received a bad input parameter 
   SOTP_E_KEY_OVERRUN,	   // The function ran out of rows within the section before the entire key and crc was fused
   SOTP_E_KEY_UNDERRUN,	   // The function ran out of valid section rows before reading the entire key and crc
   SOTP_E_KEY_CRC_MIS,	   // The function detected a CRC mismatch during the key read (key is bad or empty)
   SOTP_E_KEY_ERROR	   // The function experienced an unexpected return value  
} SotpKeyStatus;


#ifdef __cplusplus
}
#endif

#endif /* _BCM_HWDEFS_H */

