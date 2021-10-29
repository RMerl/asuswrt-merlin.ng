/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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

