/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
:>
 *
 ************************************************************************/

/*!\file cms_ftrace.h
 * \brief Header file containing constant definitions and helper functions
 *        related to Ftrace.
 *
 */

#ifndef CMS_FTRACE_H_
#define CMS_FTRACE_H_

/** The debugfs filesystem is mounted at /sys/kernel/debug by
 * /etc/rc3.d/S29trace
 *  The directory that contains trace controls is in /sys/kernel/debug/tracing
 *  Following the conventions described in kernel/Documentation/trace/ftrace.txt,
 *  a symlink from /debug -> /sys/kernel/debug/trace is created by
 *  userspace/public/apps/tracectl/Makefile
 */
#define FTRACE_MOUNTPOINT "/sys/kernel/debug"

/** This is the file that controls tracing on or off.
 */
#define FTRACE_ON         "/sys/kernel/debug/tracing/tracing_on"

/** This is the file that allows userspace apps to insert a trace line
 */
#define FTRACE_INSERT_MARKER "/sys/kernel/debug/tracing/trace_marker"

/** This is the file that selects which type of tracing we are doing
 */
#define FTRACE_CURRENT_TRACER  "/sys/kernel/debug/tracing/current_tracer"

/** This is the file that allows you to get and set current options.
 */
#define FTRACE_TRACE_OPTIONS  "/sys/kernel/debug/tracing/trace_options"



/** Enable tracing
 */
void cmsFtr_enable(void);

/** Disable tracing
 */
void cmsFtr_disable(void);

/** Insert a string in the current trace.
 * @param (IN) string to insert
 */
void cmsFtr_insertMarker(const char *s);

/** Set the specified option in trace_option.
 *  This function does not do error checking on the option
 *  string, so make sure the option name is correct.
 */
void cmsFtr_setTraceOption(const char *s);

/** Set the current trace type to function tracing.
 *  Tracing must still be enabled/disabled separately.
 */
void cmsFtr_doFunctionTracing(void);

/** Set the current trace type to function graph tracing.
 *  Tracing must still be enabled/disabled separately.
 */
void cmsFtr_doFunctionGraphTracing(void);

/** Set the current trace type to nothing.
 *  This will also clear the trace buffer.
 */
void cmsFtr_doNopTracing(void);


#endif /* CMS_FTRACE_H_ */
