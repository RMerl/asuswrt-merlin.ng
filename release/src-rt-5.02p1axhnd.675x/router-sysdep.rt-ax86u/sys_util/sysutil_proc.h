/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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

#ifndef __SYSUTIL_PROC_H__
#define __SYSUTIL_PROC_H__

/*!\file sysutil_proc.h
 * \brief Header file for various utility functions dealing with /proc.
 *
 */

#define PROC_THREAD_NAME_LEN  64

/** Structure to hold info about a thread or process, filled in and returned
 *  by sysUtl_getInfoFromProc().  More fields can be added as needed.
 */
typedef struct {
   char name[PROC_THREAD_NAME_LEN];
   char status;  /**< a single letter: R, S, Z, etc */
} ProcThreadInfo;


/** Evaluates to true if ProcThreadInfo struct indicates Zombie. */
#define IS_PROC_THREAD_INFO_ZOMBIE(i) ((i)->status == 'Z')


/** Get info about the thread id or process id.
 *
 * @param tid  (IN)  the thread or process id.
 * @param info (OUT) will be filled in by this function.
 *
 * @return 0 on success, -1 for not found, -2 for all other errors.
 */
int sysUtl_getThreadInfoFromProc(int tid, ProcThreadInfo *info);



#endif /* __SYSUTIL_PROC_H__ */
