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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    tx_initialize.h                                     PORTABLE C      */ 
/*                                                           5.6          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the ThreadX initialization component, including   */ 
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
/*  12-12-2008     William E. Lamie         Modified comment(s), and      */ 
/*                                            added new macro for         */ 
/*                                            defining port-specific data */ 
/*                                            and port-specific           */ 
/*                                            initialization processing,  */ 
/*                                            resulting in version 5.2    */ 
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

#ifndef TX_INITIALIZE_H
#define TX_INITIALIZE_H


/* Define constants that indicate initialization is in progress.  */

#define TX_INITIALIZE_IN_PROGRESS               ((ULONG) 0xF0F0F0F0)
#define TX_INITIALIZE_ALMOST_DONE               ((ULONG) 0xF0F0F0F1)
#define TX_INITIALIZE_IS_FINISHED               ((ULONG) 0x00000000)


/* Define initialization function prototypes.  */

VOID        _tx_initialize_high_level(VOID);
VOID        _tx_initialize_kernel_enter(VOID);
VOID        _tx_initialize_kernel_setup(VOID);
VOID        _tx_initialize_low_level(VOID);


/* Define the macro for adding additional port-specific global data. This macro is defined
   as white space, unless defined by tx_port.h.  */
   
#ifndef TX_PORT_SPECIFIC_DATA
#define TX_PORT_SPECIFIC_DATA
#endif


/* Define the macro for adding additional port-specific pre and post initialization processing. 
   These macros is defined as white space, unless defined by tx_port.h.  */
   
#ifndef TX_PORT_SPECIFIC_PRE_INITIALIZATION
#define TX_PORT_SPECIFIC_PRE_INITIALIZATION
#endif

#ifndef TX_PORT_SPECIFIC_POST_INITIALIZATION
#define TX_PORT_SPECIFIC_POST_INITIALIZATION
#endif

#ifndef TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION
#define TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION
#endif


/* Initialization component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_INITIALIZE_INIT
#define INITIALIZE_DECLARE 
#else
#define INITIALIZE_DECLARE extern
#endif


/* Define the unused memory pointer.  The value of the first available 
   memory address is placed in this variable in the low-level
   initialization function.  The content of this variable is passed 
   to the application's system definition function.  */

INITIALIZE_DECLARE VOID     *_tx_initialize_unused_memory;


#endif
