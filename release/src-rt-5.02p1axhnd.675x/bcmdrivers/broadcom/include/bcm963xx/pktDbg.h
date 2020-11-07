#ifndef __PKT_DBG_H_INCLUDED__
#define __PKT_DBG_H_INCLUDED__
/*
*
*  Copyright 2013, Broadcom Corporation
*
* <:label-BRCM:2007:proprietary:standard
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

#if defined( __KERNEL__ )
#define print               bcm_printk
#else
#define print               printf
#endif



#include <linux/bcm_colors.h>


/*
 *------------------------------------------------------------------------------
 * Macros for generic debug and assert support per subsystem/layer.
 * These Macros should not be used for data path (per packet processing) from
 * Release 4.10.1 onwards.
 * - declare an int variable per subsystem scope: pktDbgLvl.
 * - define a constant string prefix: DBGsys
 *
 * - define: PKT_DBG_SUPPORTED
 * - define: PKT_ASSERT_SUPPORTED
 *------------------------------------------------------------------------------
 */
#if defined(PKT_DBG_SUPPORTED)
#define DBGCODE(code)       code
#else
#define DBGCODE(code)       NULL_STMT
#endif

/* Suggested debug levels and verbosity */
#define DBG_BASIC           0           /* Independent of pktDbgLvl */
#define DBG_STATE           1           /* Subsystem state change */
#define DBG_EXTIF           2           /* External interface */
#define DBG_INTIF           3           /* Internal interface */
#define DBG_CTLFL           4           /* Algorithm and control flow */
#define DBG_PDATA           5           /* Context, state and data dumps */
#define DBG_BKGRD           6           /* Background timers */

#define DBG(stmts)                                                          \
        DBGCODE( if ( pktDbgLvl ) do { stmts } while(0) )

#define DBGL(lvl, stmts)                                                    \
        DBGCODE( if ( pktDbgLvl >= lvl ) do { stmts } while(0) )

#define dbgl_func(lvl)                                                       \
        DBGCODE( if ( pktDbgLvl >= lvl )                                    \
                     print( CLRsys DBGsys " %-10s: " CLRnl, __FUNCTION__ ) )

#define dbg_print(fmt, arg...)                                              \
        DBGCODE( if ( pktDbgLvl )                                           \
                     print( CLRsys DBGsys fmt CLRnl, ##arg ) )

#define dbgl_print(lvl, fmt, arg...)                                        \
        DBGCODE( if ( pktDbgLvl >= lvl )                                    \
                     print( CLRsys DBGsys " %-10s: " fmt CLRnl,             \
                                          __FUNCTION__, ##arg ) )

#define dbg_error(fmt, arg...)                                              \
        DBGCODE( if ( pktDbgLvl )                                           \
                 print( CLRerr DBGsys " %-10s ERROR: " fmt CLRnl,           \
                                      __FUNCTION__,  ##arg ) )

#define dbg_config(lvl)                                                     \
        DBGCODE( pktDbgLvl = lvl;                                           \
                 print( CLRhigh DBGsys " pktDbgLvl[0x%p]=%d" CLRnl,       \
                                       &pktDbgLvl, pktDbgLvl ) )

#if defined(PKT_ASSERT_SUPPORTED)
#define ASSERTCODE(code)    code
#else
#define ASSERTCODE(code)    NULL_STMT
#endif

#define dbg_assertv(cond)                                                   \
        ASSERTCODE( if ( !cond )                                            \
                    {                                                       \
                        print( CLRerr DBGsys " %-10s ASSERT: "              \
                               #cond CLRnl, __FUNCTION__ );                 \
                        return;                                             \
                    } )

#define dbg_assertr(cond, rtn)                                              \
        ASSERTCODE( if ( !cond )                                            \
                    {                                                       \
                        print( CLRerr DBGsys " %-10s ASSERT: "              \
                               #cond CLRnl, __FUNCTION__ );                 \
                        return rtn;                                         \
                    } )

/*
 *------------------------------------------------------------------------------
 * Macros for generic debug and assert support for data path.
 * - define: PKTDBG
 *------------------------------------------------------------------------------
 */
#if defined(PKTDBG)
#define pkt_dbgl_func   dbgl_func
#define pkt_dbg_print   dbg_print
#define pkt_dbgl_print  dbgl_print
#define pkt_dbg_error   dbg_error
#define pkt_dbg_assertv dbg_assertv
#define pkt_dbg_assertr dbg_assertr
#else
#define pkt_dbgl_func(lvl)               NULL_STMT
#define pkt_dbg_print(fmt, arg...)       NULL_STMT
#define pkt_dbgl_print(lvl, fmt, arg...) NULL_STMT
#define pkt_dbg_error(fmt, arg...)       NULL_STMT
#define pkt_dbg_assertv(cond)            NULL_STMT 
#define pkt_dbg_assertr(cond, rtn)       NULL_STMT
#endif

#endif  /* defined(__PKT_DBG_H_INCLUDED__) */

