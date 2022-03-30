/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
       All Rights Reserved
    
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

#ifndef _BL_LILAC_DRV_RUNNER_UTILS_H
#define _BL_LILAC_DRV_RUNNER_UTILS_H

/* 
 * Macro utilities for reading from trace buffer 
 * You pass in a 16-bit or 32-bit pointer to these macros
 */
#ifdef RUNNER_FWTRACE_32BIT
        /* The layout of the buffer in 32-bit mode is:
                 Word 0: bits 15:0 - Event Num index0, bits 31:16 - Thread Num index0
                 Word 1: Time Count for index0
                 Word 2: bits 15:0  - Event Num index1, bits 31:16 - Thread Num index1
                 Word 3: Time Count for index1
                 etc
             */
        #define RDD_FWTRACE_READ_TIME_CNTR( x ) ( (ntohl(x[i+1]) & 0x0FFFFFFF) )
        #define RDD_FWTRACE_READ_EVENT( x ) ( ntohl(x[i]) & 0xFFFF )
        #define RDD_FWTRACE_READ_THREAD( x ) ((ntohl(x[i]) & 0xFFFF0000) >> 16 )
#else
        /* The layout of the buffer in 16-bit mode is:
                 Word 0: bits 15:0 - Time Count for index0.  bits 23:16 Event Num for index 0, bits 31:24 Thread Num for index 0
                 Word 1: bits 15:0 - Time Count for index1.  bits 23:16 Event Num for index 1, bits 31:24 Thread Num for index 1
                 etc
             */
        #define RDD_FWTRACE_READ_TIME_CNTR( x ) ( ntohl(x[i]) & 0xFFFF )
        #define RDD_FWTRACE_READ_EVENT( x ) (( ntohl(x[i]) & 0xFF0000) >> 16 )
        #define RDD_FWTRACE_READ_THREAD( x ) ((ntohl(x[i]) & 0xFF000000) >> 24 )
#endif



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_fwtrace_enable_set                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Start or stop FW Trace                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function starts or stops the FW Trace on both Runner clusters.       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   enable - 1 to enable, 0 to disable                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE f_rdd_fwtrace_enable_set( uint32_t enable );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_fwtrace_clear                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Clear FW Trace buffer                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function clears the FW Trace buffer in Runner      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE f_rdd_fwtrace_clear ( void );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_fwtrace_get                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Get a single trace from Runner memory     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function copies a single processor's FW Trace and length into the memory */
/*   provided by the caller                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   runner_id - Runner index of trace to get                    */
/*   trace_length - Pointer to memory to store trace length   */
/*   trace_buffer - Pointer to buffer to store trace                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_RUNNER_ID - Invalid runner_id parameter */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE f_rdd_fwtrace_get ( LILAC_RDD_RUNNER_INDEX_DTS runner_id,
                                                      uint32_t *trace_length,
                                                      uint32_t *trace_buffer );


#define MAX_THREAD_NAME_SIZE 55
#define MAX_RNR_THREADS      48
#define MAX_EVENT_NAME_SIZE  25
#define MAX_RNR_EVENTS       16

/******************************************************************************/
/*                                                                                                                                         */
/* The following variables are lookup arrays to convert numerical thread or event IDs to readable strings */
/* Use the Runner thread ID as the first index in RnrATaskNames or RnrBTaskNames to get the thread string */
/* Use the FW Trace event ID as the first index in rdpFwTraceEvents to get the event string                */
/*                                                                                                                                         */
/******************************************************************************/
extern char rnr_a_task_names[MAX_RNR_THREADS][MAX_THREAD_NAME_SIZE];
extern char rnr_b_task_names[MAX_RNR_THREADS][MAX_THREAD_NAME_SIZE];
extern char rdp_fw_trace_events[MAX_RNR_EVENTS][MAX_EVENT_NAME_SIZE];

#endif /* _BL_LILAC_DRV_RUNNER_TM_H */

