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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    tx_timer.h                                          PORTABLE C      */ 
/*                                                           5.6          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the ThreadX timer management component, including */ 
/*    data types and external references.  It is assumed that tx_api.h    */
/*    and tx_port.h have already been included.                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s), and      */ 
/*                                            replaced UL constant        */ 
/*                                            modifier with ULONG cast,   */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), and      */ 
/*                                            removed the unnecessary     */ 
/*                                            status return on system     */ 
/*                                            timer activate/deactivate,  */ 
/*                                            resulting in version 5.4    */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s), and      */ 
/*                                            removed unused constants,   */ 
/*                                            resulting in version 5.6    */ 
/*                                                                        */ 
/**************************************************************************/ 

#ifndef TX_TIMER_H
#define TX_TIMER_H


/* Define timer management specific data definitions.  */

#define TX_TIMER_ID                             ((ULONG) 0x4154494D)
#define TX_TIMER_ENTRIES                        32


/* Define timer management function prototypes.  */

UINT        _tx_timer_activate(TX_TIMER *timer_ptr);
UINT        _tx_timer_change(TX_TIMER *timer_ptr, ULONG initial_ticks, ULONG reschedule_ticks);
UINT        _tx_timer_create(TX_TIMER *timer_ptr, CHAR *name_ptr, 
                VOID (*expiration_function)(ULONG), ULONG expiration_input,
                ULONG initial_ticks, ULONG reschedule_ticks, UINT auto_activate);
UINT        _tx_timer_deactivate(TX_TIMER *timer_ptr);
UINT        _tx_timer_delete(TX_TIMER *timer_ptr);
VOID        _tx_timer_expiration_process(VOID);
UINT        _tx_timer_info_get(TX_TIMER *timer_ptr, CHAR **name, UINT *active, ULONG *remaining_ticks, 
                ULONG *reschedule_ticks, TX_TIMER **next_timer);
VOID        _tx_timer_initialize(VOID);
UINT        _tx_timer_performance_info_get(TX_TIMER *timer_ptr, ULONG *activates, ULONG *reactivates,
                ULONG *deactivates, ULONG *expirations, ULONG *expiration_adjusts);
UINT        _tx_timer_performance_system_info_get(ULONG *activates, ULONG *reactivates,
                ULONG *deactivates, ULONG *expirations, ULONG *expiration_adjusts);
VOID        _tx_timer_system_activate(TX_TIMER_INTERNAL *timer_ptr);
VOID        _tx_timer_system_deactivate(TX_TIMER_INTERNAL *timer_ptr);
VOID        _tx_timer_thread_entry(ULONG timer_thread_input);

ULONG       _tx_time_get(VOID);
VOID        _tx_time_set(ULONG new_time);


/* Define error checking shells for API services.  These are only referenced by the 
   application.  */

UINT        _txe_timer_activate(TX_TIMER *timer_ptr);
UINT        _txe_timer_change(TX_TIMER *timer_ptr, ULONG initial_ticks, ULONG reschedule_ticks);
UINT        _txe_timer_create(TX_TIMER *timer_ptr, CHAR *name_ptr, 
                VOID (*expiration_function)(ULONG), ULONG expiration_input,
                ULONG initial_ticks, ULONG reschedule_ticks, UINT auto_activate, UINT timer_control_block_size);
UINT        _txe_timer_deactivate(TX_TIMER *timer_ptr);
UINT        _txe_timer_delete(TX_TIMER *timer_ptr);
UINT        _txe_timer_info_get(TX_TIMER *timer_ptr, CHAR **name, UINT *active, ULONG *remaining_ticks, 
                ULONG *reschedule_ticks, TX_TIMER **next_timer);


/* Timer management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_TIMER_INIT
#define TIMER_DECLARE 
#else
#define TIMER_DECLARE extern
#endif


/* Define the system clock value that is continually incremented by the 
   periodic timer interrupt processing.  */

TIMER_DECLARE ULONG     _tx_timer_system_clock;


/* Define the current time slice value.  If non-zero, a time-slice is active.
   Otherwise, the time_slice is not active.  */

TIMER_DECLARE ULONG     _tx_timer_time_slice;


/* Define the time-slice expiration flag.  This is used to indicate that a time-slice
   has happened.  */

TIMER_DECLARE UINT      _tx_timer_expired_time_slice;


/* Define the thread and application timer entry list.  This list provides a direct access
   method for insertion of times less than TX_TIMER_ENTRIES.  */

TIMER_DECLARE TX_TIMER_INTERNAL *_tx_timer_list[TX_TIMER_ENTRIES];


/* Define the boundary pointers to the list.  These are setup to easily manage
   wrapping the list.  */

TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_list_start;
TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_list_end;


/* Define the current timer pointer in the list.  This pointer is moved sequentially
   through the timer list by the timer interrupt handler.  */

TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_current_ptr;


/* Define the timer expiration flag.  This is used to indicate that a timer 
   has expired.  */

TIMER_DECLARE UINT      _tx_timer_expired;


/* Define the created timer list head pointer.  */

TIMER_DECLARE TX_TIMER *_tx_timer_created_ptr;


/* Define the created timer count.  */

TIMER_DECLARE ULONG     _tx_timer_created_count;


#ifndef TX_TIMER_PROCESS_IN_ISR

/* Define the timer thread's control block.  */

TIMER_DECLARE TX_THREAD _tx_timer_thread;


/* Define the variable that holds the timer thread's starting stack address.  */

TIMER_DECLARE VOID      *_tx_timer_stack_start;


/* Define the variable that holds the timer thread's stack size.  */

TIMER_DECLARE ULONG     _tx_timer_stack_size;


/* Define the variable that holds the timer thread's priority.  */

TIMER_DECLARE UINT      _tx_timer_priority;

/* Define the system timer thread's stack.   The default size is defined
   in tx_port.h.  */

TIMER_DECLARE ULONG     _tx_timer_thread_stack_area[(TX_TIMER_THREAD_STACK_SIZE+(sizeof(ULONG)-1))/sizeof(ULONG)];

#else


/* Define the busy flag that will prevent nested timer ISR processing.  */

TIMER_DECLARE UINT      _tx_timer_processing_active;

#endif

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

/* Define the total number of timer activations.  */

TIMER_DECLARE  ULONG    _tx_timer_performance_activate_count;


/* Define the total number of timer reactivations.  */

TIMER_DECLARE  ULONG    _tx_timer_performance_reactivate_count;


/* Define the total number of timer deactivations.  */

TIMER_DECLARE  ULONG    _tx_timer_performance_deactivate_count;


/* Define the total number of timer expirations.  */

TIMER_DECLARE  ULONG    _tx_timer_performance_expiration_count;


/* Define the total number of timer expiration adjustments. These are required
   if the expiration time is greater than the size of the timer list. In such 
   cases, the timer is placed at the end of the list and then reactivated 
   as many times as necessary to finally achieve the resulting timeout. */

TIMER_DECLARE  ULONG    _tx_timer_performance_expiration_adjust_count;


#endif


#endif
