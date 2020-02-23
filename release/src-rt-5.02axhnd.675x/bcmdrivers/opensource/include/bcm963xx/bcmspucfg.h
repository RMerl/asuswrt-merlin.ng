/*
    Copyright 2007-2015 Broadcom Corporation

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

/***************************************************************************
 * File Name  : bcmspucfg.h
 *
 * Description: This file contains the definitions, structures and function
 *              prototypes for the SPU Configuration driver.
 ***************************************************************************/

#if !defined(_BCMSPUCFG_H_)
#define _BCMSPUCFG_H_

/***************************************************************************
 * Constant Definitions
 ***************************************************************************/

#define SPU_TRC_LEVEL_NONE      0x00000000
#define SPU_TRC_LEVEL_DBG       0x00000001  /* flow debug */
#define SPU_TRC_LEVEL_INFO      0x00000004  /* detailed pkt debug */

/* Return status values. */
typedef enum BcmSpuStatus
{
    SPUSTS_SUCCESS = 0,
    SPUSTS_MEMERR,
    SPUSTS_ERROR
} SPU_STATUS;

typedef struct SpuTest
{
   uint32_t         pktId;   /* start packet id  */
   uint32_t         numPkts; /* # of packets */
} SPU_TEST_PARMS, *PSPU_TEST_PARMS;

typedef struct SpuStatistics
{
   uint32_t         encIngress;         /* # of packets passed to SPU (Encryption) */
   uint32_t         encSpuEgress;       /* # of packets successfully processed by SPU (Encryption) */
   uint32_t         encFallback;        /* # of packets requiring fallback (Encryption) */
   uint32_t         encDrops;           /* # of packets dropped */
   uint32_t         encErrors;          /* # of packets with error */
   uint32_t         decIngress;         /* # of packets passed to SPU (Decryption) */
   uint32_t         decSpuEgress;       /* # of packets successfully processed by SPU (Decryption) */
   uint32_t         decFallback;        /* # of packets requiring fallback (Decryption) */
   uint32_t         decDrops;           /* # of packets dropped */
   uint32_t         decErrors;          /* # of packets with error */
} SPU_STAT_PARMS, *PSPU_STAT_PARMS;


/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

#endif /* _BCMSPUCFG_H_ */

