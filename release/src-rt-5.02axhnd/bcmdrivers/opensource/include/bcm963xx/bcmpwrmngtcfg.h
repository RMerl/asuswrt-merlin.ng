/***********************************************************************
//
//  Copyright (c) 2008-2014  Broadcom Corporation
//  All Rights Reserved
//
// <:label-BRCM:2008-2014:DUAL/GPL:standard
// 
// Unless you and Broadcom execute a separate written software license 
// agreement governing use of this software, this software is licensed 
// to you under the terms of the GNU General Public License version 2 
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, 
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give 
//    you permission to link this software with independent modules, and 
//    to copy and distribute the resulting executable under terms of your 
//    choice, provided that you also meet, for each linked independent 
//    module, the terms and conditions of the license of that module. 
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications 
//    of the software.  
// 
// Not withstanding the above, under no circumstances may you combine 
// this software in any way with any other Broadcom software provided 
// under a license other than the GPL, without Broadcom's express prior 
// written consent. 
// 
// :>
//
************************************************************************/
#ifndef _BCMPWRMNGTCFG_H
#define _BCMPWRMNGTCFG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Constant Definitions
 ***************************************************************************/
#define PWRMNGT_ENABLE_DEEP                    2
#define PWRMNGT_ENABLE                         1
#define PWRMNGT_DISABLE                        0
#define PWRMNGT_STOP                          -1

/* MIPS CPU Valid Speeds */
#define PWRMNGT_MIPS_CPU_SPEED_MIN_VAL         0
#define PWRMNGT_MIPS_CPU_SPEED_MAX_VAL         256

/* Return status values. */
typedef enum PwrMngtStatus
{
    PWRMNGTSTS_SUCCESS = 0,
    PWRMNGTSTS_INIT_FAILED,
    PWRMNGTSTS_ERROR,
    PWRMNGTSTS_STATE_ERROR,
    PWRMNGTSTS_PARAMETER_ERROR,
    PWRMNGTSTS_ALLOC_ERROR,
    PWRMNGTSTS_NOT_SUPPORTED,
    PWRMNGTSTS_TIMEOUT,
} PWRMNGT_STATUS;

typedef unsigned int ui32;

/* Masks defined here are used in selecting the required parameter from Mgmt
 * application.
 */
#define PWRMNGT_CFG_PARAM_ALL_MASK               (0x000000FF & ~PWRMNGT_CFG_PARAM_MEM_AVS_LOG_MASK)
#define PWRMNGT_CFG_DEF_PARAM_MASK                0x000000FF

typedef struct _PwrMngtConfigParams {
#define PWRMNGT_CFG_PARAM_CPUSPEED_MASK            0x00000001
   ui32                  cpuspeed;
#define PWRMNGT_CFG_PARAM_CPU_R4K_WAIT_MASK        0x00000002
   ui32                  cpur4kwait;
#define PWRMNGT_CFG_PARAM_MEM_SELF_REFRESH_MASK    0x00000004
   ui32                  dramSelfRefresh;
#define PWRMNGT_CFG_PARAM_MEM_AVS_MASK             0x00000008
   ui32                  avs;
#define PWRMNGT_CFG_PARAM_MEM_AVS_LOG_MASK         0x00000010
   ui32                  avsLog;
} PWRMNGT_CONFIG_PARAMS, *PPWRMNGT_CONFIG_PARAMS ;

/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _BCMPWRMNGTCGG_H */
