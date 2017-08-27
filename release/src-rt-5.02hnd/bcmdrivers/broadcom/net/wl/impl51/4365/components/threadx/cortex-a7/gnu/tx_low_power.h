/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2008 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This software is copyrighted by and is the sole property of Express   */ 
/*  Logic, Inc.  All rights, title, ownership, or other interests         */ 
/*  in the software remain the property of Express Logic, Inc.  This      */ 
/*  software may only be used in accordance with the corresponding        */ 
/*  license agreement.  Any unauthorized use, duplication, transmission,  */ 
/*  distribution, or disclosure of this software is expressly forbidden.  */ 
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */ 
/*  written consent of Express Logic, Inc.                                */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** ThreadX Component                                                     */
/**                                                                       */
/**   Timer Management                                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    tx_low_power.h                                      PORTABLE C      */ 
/*                                                           5.1          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines prototypes for the low-power timer additions      */ 
/*    required for sleep mode.                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  07-02-2008     William E. Lamie         Modified comment(s), and      */ 
/*                                            added several new function  */ 
/*                                            prototypes, resulting       */ 
/*                                            in version 5.1              */ 
/*                                                                        */ 
/**************************************************************************/ 

#ifndef  TX_LOW_POWER_H
#define  TX_LOW_POWER_H


/* Define low-power function prototypes.  */

VOID        tx_low_power_enter(VOID);
VOID        tx_low_power_exit(VOID);
VOID        tx_time_increment(ULONG time_increment);
ULONG       tx_timer_get_next(ULONG *next_timer_tick_ptr);

#endif
