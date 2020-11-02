/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2012 by Express Logic Inc.               */ 
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
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */ 
/*                                                                        */ 
/*    tx_user.h                                           PORTABLE C      */ 
/*                                                           5.6          */ 
/*                                                                        */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file contains user defines for configuring ThreadX in specific */ 
/*    ways. This file will have an effect only if the application and     */ 
/*    ThreadX library are built with TX_INCLUDE_USER_DEFINE_FILE defined. */ 
/*    Note that all the defines in this file may also be made on the      */ 
/*    command line when building ThreadX library and application objects. */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s), and      */ 
/*                                            added two new conditional   */ 
/*                                            build options, namely:      */ 
/*                                                                        */ 
/*                                                 TX_NO_TIMER  and       */ 
/*                                                 TX_USE_PRESET_DATA     */ 
/*                                                                        */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), added    */ 
/*                                            new defines, and removed    */ 
/*                                            TX_USE_PRESET_DATA          */ 
/*                                            since it is no longer       */ 
/*                                            required, resulting in      */ 
/*                                            version 5.2                 */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.4    */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*                                                                        */ 
/**************************************************************************/ 

#ifndef TX_USER_H
#define TX_USER_H


/* Define various build options for the ThreadX port.  The application should either make changes
   here by commenting or un-commenting the conditional compilation defined OR supply the defines 
   though the compiler's equivalent of the -D option.  
   
   For maximum speed, the following should be defined:

        TX_MAX_PRIORITIES                       32  
        TX_DISABLE_PREEMPTION_THRESHOLD
        TX_DISABLE_REDUNDANT_CLEARING
        TX_DISABLE_NOTIFY_CALLBACKS
        TX_NOT_INTERRUPTABLE
        TX_TIMER_PROCESS_IN_ISR
        TX_REACTIVATE_INLINE
        TX_DISABLE_STACK_FILLING
        TX_INLINE_THREAD_RESUME_SUSPEND
   
   For minimum size, the following should be defined:
   
        TX_MAX_PRIORITIES                       32  
        TX_DISABLE_PREEMPTION_THRESHOLD
        TX_DISABLE_REDUNDANT_CLEARING
        TX_DISABLE_NOTIFY_CALLBACKS
        TX_NOT_INTERRUPTABLE
        TX_TIMER_PROCESS_IN_ISR
   
   Of course, many of these defines reduce functionality and/or change the behavior of the
   system in ways that may not be worth the trade-off. For example, the TX_TIMER_PROCESS_IN_ISR
   results in faster and smaller code, however, it increases the amount of processing in the ISR.
   In addition, some services that are available in timers are not available from ISRs and will
   therefore return an error if this option is used. This may or may not be desirable for a 
   given application.  */


/* Override various options with default values already assigned in tx_port.h. Please also refer
   to tx_port.h for descriptions on each of these options.  */

/*
#define TX_MAX_PRIORITIES                       32  
#define TX_MINIMUM_STACK                        ????         
#define TX_THREAD_USER_EXTENSION                ????
#define TX_TIMER_THREAD_STACK_SIZE              ????
#define TX_TIMER_THREAD_PRIORITY                ????
*/

/* Determine if timer expirations (application timers, timeouts, and tx_thread_sleep calls 
   should be processed within the a system timer thread or directly in the timer ISR. 
   By default, the timer thread is used. When the following is defined, the timer expiration 
   processing is done directly from the timer ISR, thereby eliminating the timer thread control
   block, stack, and context switching to activate it.  */

/*
#define TX_TIMER_PROCESS_IN_ISR
*/

/* Determine if in-line timer reactivation should be used within the timer expiration processing.
   By default, this is disabled and a function call is used. When the following is defined,
   reactivating is performed in-line resulting in faster timer processing but slightly larger
   code size.  */ 

/*
#define TX_REACTIVATE_INLINE 
*/

/* Determine is stack filling is enabled. By default, ThreadX stack filling is enabled,
   which places an 0xEF pattern in each byte of each thread's stack.  This is used by
   debuggers with ThreadX-awareness and by the ThreadX run-time stack checking feature.  */

/*
#define TX_DISABLE_STACK_FILLING 
*/

/* Determine whether or not stack checking is enabled. By default, ThreadX stack checking is 
   disabled. When the following is defined, ThreadX thread stack checking is enabled.  If stack
   checking is enabled (TX_ENABLE_STACK_CHECKING is defined), the TX_DISABLE_STACK_FILLING
   define is negated, thereby forcing the stack fill which is necessary for the stack checking
   logic.  */

/*
#define TX_ENABLE_STACK_CHECKING
*/

/* Determine if preemption-threshold should be disabled. By default, preemption-threshold is 
   enabled. If the application does not use preemption-threshold, it may be disabled to reduce
   code size and improve performance.  */

/*
#define TX_DISABLE_PREEMPTION_THRESHOLD
*/

/* Determine if global ThreadX variables should be cleared. If the compiler startup code clears 
   the .bss section prior to ThreadX running, the define can be used to eliminate unnecessary
   clearing of ThreadX global variables.  */

/*
#define TX_DISABLE_REDUNDANT_CLEARING
*/

/* Determine if no timer processing is required. This option will help eliminate the timer 
   processing when not needed. The user will also have to comment out the call to 
   tx_timer_interrupt, which is typically made from assembly language in 
   tx_initialize_low_level.  */

/* 
#define TX_NO_TIMER
*/

/* Determine if the notify callback option should be disabled. By default, notify callbacks are
   enabled. If the application does not use notify callbacks, they may be disabled to reduce
   code size and improve performance.  */

/*
#define TX_DISABLE_NOTIFY_CALLBACKS
*/


/* Determine if the tx_thread_resume and tx_thread_suspend services should have their internal 
   code in-line. This results in a larger image, but improves the performance of the thread 
   resume and suspend services.  */

/*
#define TX_INLINE_THREAD_RESUME_SUSPEND
*/


/* Determine if the internal ThreadX code is non-interruptable. This results in smaller code 
   size and less processing overhead, but increases the interrupt lockout time.  */

/*
#define TX_NOT_INTERRUPTABLE
*/


/* Determine if the trace event logging code should be enabled. This causes slight increases in 
   code size and overhead, but provides the ability to generate system trace information which 
   is available for viewing in TraceX.  */

/*
#define TX_ENABLE_EVENT_TRACE
*/


/* Determine if block pool performance gathering is required by the application. When the following is
   defined, ThreadX gathers various block pool performance information. */

/*
#define TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO
*/

/* Determine if byte pool performance gathering is required by the application. When the following is
   defined, ThreadX gathers various byte pool performance information. */

/*
#define TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
*/

/* Determine if event flags performance gathering is required by the application. When the following is
   defined, ThreadX gathers various event flags performance information. */

/*
#define TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
*/

/* Determine if mutex performance gathering is required by the application. When the following is
   defined, ThreadX gathers various mutex performance information. */

/*
#define TX_MUTEX_ENABLE_PERFORMANCE_INFO
*/

/* Determine if queue performance gathering is required by the application. When the following is
   defined, ThreadX gathers various queue performance information. */

/*
#define TX_QUEUE_ENABLE_PERFORMANCE_INFO
*/

/* Determine if semaphore performance gathering is required by the application. When the following is
   defined, ThreadX gathers various semaphore performance information. */

/*
#define TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO
*/

/* Determine if thread performance gathering is required by the application. When the following is
   defined, ThreadX gathers various thread performance information. */

/*
#define TX_THREAD_ENABLE_PERFORMANCE_INFO
*/

/* Determine if timer performance gathering is required by the application. When the following is
   defined, ThreadX gathers various timer performance information. */

/*
#define TX_TIMER_ENABLE_PERFORMANCE_INFO
*/

#ifdef RTE_TX_USER_EXT
/* Include user defined extension to TX_THREAD. */
#include "rte_tx_user_ext.h"
#endif

#endif

